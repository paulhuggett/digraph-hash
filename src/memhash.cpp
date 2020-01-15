#include "memhash.hpp"

#include <algorithm>
#include <iostream>
#include <limits>
#include <unordered_map>
#include <utility>

#include "config.hpp"
#include "vertex.hpp"

namespace {

    using visited = std::unordered_map<vertex const *, size_t>;

    void trace () {
        // A trace with no arguments is a no-op.
    }
    template <typename T, typename... Args>
    void trace (T && t, Args &&... args) {
        (void) t;
#ifdef TRACE_ENABLED
        std::cout << t;
#endif
        trace (std::forward<Args> (args)...);
    }

    auto vertex_hash_impl (vertex const * const v, memoized_hashes * const table,
                           visited * const visited) -> std::tuple<bool, hash::digest> {
        // Have we computed the hash for this function already? If so, did it involve a loop?
        // If we have, and there was no loop, we can return the result immediately.
        auto const table_pos = table->find (v);
        if (table_pos != table->end ()) {
            trace ("Returning pre-computed hash for '", v->name (), "'\n");
            return std::tuple<bool, hash::digest>{false, table_pos->second};
        }

        hash h;

        // Have we previously visited this vertex during this search? If so, add a
        // back-reference to the hash and tell the caller that this result should not be
        // memoized.
        auto const visited_pos = visited->find (v);
        if (visited_pos != visited->end ()) {
            trace ("Returning back-ref to ", visited_pos->second, '\n');
            h.update_backref (visited_pos->second);
            return std::tuple<bool, hash::digest>{true, h.finalize ()};
        }

        // Record that we have visited this vertex and give it a numerical identifier. This will
        // be used to form its back-reference if we loop back here in future.
        (*visited)[v] = visited->size ();

        trace ("Computing hash for '", v->name (), "'\n");
        h.update_vertex (*v);

        // Enumerate the adjacent vertices.
        bool looped = false;
        for (auto adj : v->adjacent ()) {
            auto const adj_digest = vertex_hash_impl (adj, table, visited);
            looped = looped || std::get<bool> (adj_digest);
            h.update_digest (std::get<hash::digest> (adj_digest));
        }

        auto const result = std::make_tuple (looped, h.finalize ());
        if (!looped) {
            trace ("Recording result for '", v->name (), "'\n");
            (*table)[v] = std::get<hash::digest> (result);
        }
        return result;
    }

} // end anonymous namespace

hash::digest vertex_hash (vertex const * const v, memoized_hashes * const table) {
    visited visited;
    return std::get<hash::digest> (vertex_hash_impl (v, table, &visited));
}
