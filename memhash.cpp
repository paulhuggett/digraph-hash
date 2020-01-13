#include "memhash.hpp"
#include <optional>
#include <unordered_map>


namespace {

    using visited = std::unordered_map<vertex const *, size_t>;

    std::optional<hash::digest> vertex_hash_impl (vertex const * const v,
                                                  memoized_hashes * const table, hash * const h,
                                                  visited * const visited) {
        {
            // Have we computed the hash for this function already? If so, did it involve a loop?
            // If we have, and there was no loop, we can return the result immediately.
            auto const pos = table->find (v);
            if (pos != std::end (*table) && !std::get<bool> (pos->second)) {
                // std::cout << "Returning pre-computed hash for " << v->name << '\n';
                return {std::get<hash::digest> (pos->second)};
            }
        }
        {
            // Have we previously visited this vertex during this search? If so, add a
            // back-reference to the hash and tell the caller that this result should not be
            // memoized.
            auto const back_ref_pos = visited->find (v);
            if (back_ref_pos != std::end (*visited)) {
                // std::cout << "Hashing back reference " << back_ref_pos->second << '\n';
                h->update_backref (back_ref_pos->second);
                return {std::nullopt};
            }
        }
        (*visited)[v] = visited->size ();

        // std::cout << "Computing hash for " << v->name << '\n';
        hash h2;
        h2.update_vertex (*v);
        h->update_vertex (*v);
        bool looped = false;
        for (auto dependent : v->adjacent ()) {
            if (auto const r = vertex_hash_impl (dependent, table, h, visited)) {
                h2.update_digest (r.value ());
            } else {
                looped = true;
            }
        }
        auto const result = h2.finalize ();
        (*table)[v] = std::make_tuple (looped, result);
        if (looped) {
            return {std::nullopt};
        }
        return {result};
    }

} // end anonymous namespace

hash::digest vertex_hash (vertex const * const v, memoized_hashes * const table) {
    hash h;
    visited visited;
    if (std::optional<hash::digest> const result = vertex_hash_impl (v, table, &h, &visited)) {
        return result.value ();
    }
    return h.finalize ();
}
