#include "memhash.hpp"

#include <algorithm>
#include <cassert>
#include <limits>
#include <tuple>
#include <unordered_map>
#include <utility>

#include "trace.hpp"
#include "vertex.hpp"

namespace {

    using visited = std::unordered_map<vertex const *, std::size_t>;

    auto vertex_hash_impl (vertex const * const v, memoized_hashes * const table,
                           visited * const visited) -> std::tuple<std::size_t, hash::digest> {
        auto const depth = visited->size ();
        trace ("Computing hash for ", *v, " (#", depth, ')');

        // Have we computed the hash for this function already? If so, we can return the result
        // immediately.
        auto const table_pos = table->find (v);
        if (table_pos != table->end ()) {
            trace ("Returning pre-computed hash for ", *v);
            return std::make_tuple (depth, table_pos->second);
        }

        hash h;

        // Have we previously visited this vertex on this path? If so, add to the hash a
        // back-reference to that vertex and return its position to the caller. If not, record that
        // we have visited this vertex and its depth. This will be used to form a back-reference if
        // we loop back here in future.
        auto const [visited_pos, inserted] = visited->try_emplace (v, depth);
        if (!inserted) {
            // Back-references are encoded as a number relative to the depth of the current vertex.
            // Larger values are further back in the encoding.
            assert (depth > visited_pos->second);
            h.update_backref (depth - visited_pos->second - 1U);
            trace ("Returning back-ref to #", visited_pos->second);
            return std::make_tuple (visited_pos->second, h.finalize ());
        }

        // Add vertex v (and any properties it has) to the hash.
        h.update_vertex (*v);

        // Enumerate the adjacent vertices.
        auto loop_point = std::numeric_limits<std::size_t>::max ();
        for (vertex const * const out : v->out_edges ()) {
            // Add  any properties of the edge from 'v' to 'out' to the hash here.

            // Encode the out-going vertex.
            auto const adj_digest = vertex_hash_impl (out, table, visited);
            // A out-edge that points back to this same vertex doesn't count as a loop.
            if (out != v) {
                loop_point = std::min (loop_point, std::get<std::size_t> (adj_digest));
            }
            h.update_digest (std::get<hash::digest> (adj_digest));
        }
        // We've encoded the final edge. Record that in the hash.
        h.update_end ();

        auto const result = std::make_tuple (loop_point, h.finalize ());
        if (loop_point > depth) {
            trace ("Recording result for ", *v);
            (*table)[v] = std::get<hash::digest> (result);
        }
        visited->erase (v);
        return result;
    }

} // end anonymous namespace

hash::digest vertex_hash (vertex const * const v, memoized_hashes * const table) {
    visited visited;
    return std::get<hash::digest> (vertex_hash_impl (v, table, &visited));
}
