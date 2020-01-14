#include "memhash.hpp"
#include <iostream>
#include <limits>
#include <optional>
#include <unordered_map>
#include <utility>

#include "vertex.hpp"

namespace {

    using visited = std::unordered_map<vertex const *, size_t>;

    void trace () {}
    template <typename T, typename... Args>
    void trace (T const & t, Args... args) {
        // std::cout << t;
        trace (std::forward<Args> (args)...);
    }

    std::tuple<size_t, hash::digest> vertex_hash_impl (vertex const * const v,
                                                       memoized_hashes * const table,
                                                       visited * const visited) {
        static constexpr auto noloop = std::numeric_limits<size_t>::max ();

        {
            // Have we computed the hash for this function already? If so, did it involve a loop?
            // If we have, and there was no loop, we can return the result immediately.
            auto const pos = table->find (v);
            if (pos != std::end (*table) && std::get<size_t> (pos->second) == noloop) {
                trace ("Returning pre-computed hash for '", v->name (), "'\n");
                return {noloop, std::get<hash::digest> (pos->second)};
            }
        }

        hash h;
        {
            // Have we previously visited this vertex during this search? If so, add a
            // back-reference to the hash and tell the caller that this result should not be
            // memoized.
            auto const back_ref_pos = visited->find (v);
            if (back_ref_pos != std::end (*visited)) {
                trace ("Returning back-ref to ", back_ref_pos->second, '\n');
                h.update_backref (back_ref_pos->second);
                return {back_ref_pos->second, h.finalize ()};
            }
        }

        // Record that we have visited this vertex and give it a numerical identifier. This will
        // be used to form its back-reference if we loop back here in future.
        size_t const index = visited->size () + 1U;
        (*visited)[v] = index;

        trace ("Computing hash for '", v->name (), "'\n");
        h.update_vertex (*v);
        auto loop_index = noloop;
        for (auto adj : v->adjacent ()) {
            auto adj_digest = vertex_hash_impl (adj, table, visited);
            loop_index = std::min (std::get<size_t> (adj_digest), loop_index);
            h.update_digest (std::get<hash::digest> (adj_digest));
        }
        auto const result = std::make_tuple (loop_index, h.finalize ());
        if (loop_index > index) {
            trace ("Recording result for '", v->name (), "'\n");
            (*table)[v] = result;
        }
        return result;
    }

} // end anonymous namespace

hash::digest vertex_hash (vertex const * const v, memoized_hashes * const table) {
    visited visited;
    return std::get<hash::digest> (vertex_hash_impl (v, table, &visited));
}
