#ifndef memhash_hpp
#define memhash_hpp

#include <unordered_map>
#include "hash.hpp"
#include "vertex.hpp"

using memoized_hashes = std::unordered_map<vertex const *, std::tuple<bool, hash::digest>>;
hash::digest vertex_hash (vertex const * const v, memoized_hashes * const table);

#endif /* memhash_hpp */
