#include "memhash.hpp"

#include <algorithm>
#include <cassert>
#include <limits>
#include <tuple>
#include <unordered_map>
#include <utility>

#ifdef TRACE_ENABLED
#    include <iostream>
#endif

#include "config.hpp"
#include "vertex.hpp"

namespace {

    using visited = std::unordered_map<vertex const *, size_t>;

#ifdef TRACE_ENABLED
    void trace_impl () {
        // A trace with no arguments is a no-op.
    }
    template <typename T, typename... Args>
    void trace_impl (T && t, Args &&... args) {
        std::cout << t;
        trace_impl (std::forward<Args> (args)...);
    }
#else
    struct sink {
        template <typename... Args>
        explicit sink (Args const &...) {}
    };
#endif // TRACE_ENABLED

    template <typename... Args>
    void trace (Args &&... args) {
#ifdef TRACE_ENABLED
        trace_impl (std::forward<Args> (args)...);
        std::cout << '\n';
#else
        sink{args...}; // eat all unused arguments!
#endif // TRACE_ENABLED
    }

    auto vertex_hash_impl (vertex const * const v, memoized_hashes * const table,
                           visited * const visited) -> std::tuple<size_t, hash::digest> {
        auto const num_visited = visited->size ();
        trace ("Computing hash for ", *v, " (#", num_visited, ')');

        // Have we computed the hash for this function already? If so, did it involve a loop?
        // If we have, and there was no loop, we can return the result immediately.
        auto const table_pos = table->find (v);
        if (table_pos != table->end ()) {
            trace ("Returning pre-computed hash for ", *v);
            return std::make_tuple (num_visited, table_pos->second);
        }

        hash h;

        // Have we previously visited this vertex during this search? If so, add a
        // back-reference to the hash and tell the caller that this result should not be
        // memoized.
        auto const visited_pos = visited->find (v);
        if (visited_pos != visited->end ()) {
            // Back-references are encoded as as number relative to the index of the current vertex.
            // Larger values are further back in the encoding.
            assert (num_visited > visited_pos->second);
            h.update_backref (num_visited - visited_pos->second - 1U);
            trace ("Returning back-ref to #", visited_pos->second);
            return std::make_tuple (visited_pos->second, h.finalize ());
        }

        // Record that we have visited this vertex and give it a numerical identifier. This will
        // be used to form its back-reference if we loop back here in future.
        (*visited)[v] = num_visited;

        // Add vertex v (and any properties it has) to the hash.
        h.update_vertex (*v);

        // Enumerate the adjacent vertices.
        auto loop_point = std::numeric_limits<size_t>::max ();
        for (vertex const * const out : v->out_edges ()) {
            // Add an properties of the edge from 'v' to 'out' here.
            //
            // Encode the out-going vertex.
            auto const adj_digest = vertex_hash_impl (out, table, visited);
            // A out-edge that points back to this same vertex doesn't count as a loop.
            if (out != v) {
                loop_point = std::min (loop_point, std::get<0> (adj_digest));
            }
            h.update_digest (std::get<1> (adj_digest));
        }
        // We've encoded the final edge. Record that in the hash.
        h.update_end ();

        auto const result = std::make_tuple (loop_point, h.finalize ());
        if (loop_point > num_visited) {
            trace ("Recording result for ", *v);
            (*table)[v] = std::get<1> (result);
        }
        return result;
    }

} // end anonymous namespace

hash::digest vertex_hash (vertex const * const v, memoized_hashes * const table) {
    visited visited;
    return std::get<1> (vertex_hash_impl (v, table, &visited));
}
