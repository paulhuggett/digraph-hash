#ifndef MEMHASH_HPP
#define MEMHASH_HPP

#include <cstdlib>
#include <unordered_map>

#include "hash.hpp"

class vertex;
using memoized_hashes = std::unordered_map<vertex const *, hash::digest>;

/// Computes the hash digest of an invidual graph vertex incorporating the hashes of all
/// transitively reachable vertices.
///
/// \param v  The vertex whose hash digest is to be computed.
/// \param table  Used to record memoized hashes. Pass the same object to multiple calls to this
///    function to improve performance.
/// \returns The hash digest for vertex \p v.
hash::digest vertex_hash (vertex const * const v, memoized_hashes * const table);

#endif // MEMHASH_HPP
