#ifndef MEMHASH_HPP
#define MEMHASH_HPP

#include <cstdlib>
#include <unordered_map>

#include "hash.hpp"

using memoized_hashes = std::unordered_map<vertex const *, std::tuple<size_t, hash::digest>>;

class vertex;
hash::digest vertex_hash (vertex const * const v, memoized_hashes * const table);

#endif // MEMHASH_HPP
