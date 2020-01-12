#ifndef MEMHASH_HPP
#define MEMHASH_HPP

#include <unordered_map>
#include "hash.hpp"
#include "vertex.hpp"

using memoized_hashes = std::unordered_map<vertex const *, std::tuple<bool, hash::digest>>;
hash::digest vertex_hash (vertex const * const v, memoized_hashes * const table);

#endif // MEMHASH_HPP
