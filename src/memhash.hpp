#ifndef MEMHASH_HPP
#define MEMHASH_HPP

#include <cstdlib>
#include <unordered_map>

#include "hash.hpp"

class vertex;
using memoized_hashes = std::unordered_map<vertex const *, hash::digest>;
hash::digest vertex_hash (vertex const * const v, memoized_hashes * const table);

#endif // MEMHASH_HPP
