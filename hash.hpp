//
//  hash.hpp
//  go_hash
//
//  Created by Paul Bowen-Huggett on 12/01/2020.
//

#ifndef hash_hpp
#define hash_hpp

#include <cstddef>
#include <cstdint>
#include <string>

#include "vertex.hpp"

enum class tags : char {
    function = 'F',
    backref = 'R',
    digest = 'D',
};

#define STRING_HASH 1

#ifdef STRING_HASH

class hash {
public:
    using digest = std::string;
    digest finalize () const noexcept { return state_; }

    void update_vertex (vertex const & x);
    void update_backref (size_t backref);
    void update_digest (digest const & d);

private:
    std::string state_;
};

#else  // STRING_HASH

class hash {
public:
    using digest = uint64_t;

    digest finalize () const noexcept { return state_; }

    void update_vertex (vertex const & x) noexcept;
    void update_backref (size_t backref) noexcept;
    void update_digest (digest const & d) noexcept;

private:
    static constexpr uint64_t fnv1a_64_init = 0xcbf29ce4'84222325U;
    static constexpr uint64_t fnv1a_64_prime = 0x00000100'000001b3U;
    uint64_t state_ = fnv1a_64_init;

    void update (void const * ptr, size_t size) noexcept {
        for (auto p = reinterpret_cast<uint8_t const *> (ptr), end = p + size; p != end; ++p) {
            state_ = (state_ ^ static_cast<uint64_t> (*p)) * fnv1a_64_prime;
        }
    }
};
#endif // STRING_HASH

#endif /* hash_hpp */
