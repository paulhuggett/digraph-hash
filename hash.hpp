#ifndef HASH_HPP
#define HASH_HPP

#include <cstddef>
#include <cstdint>
#include <string>

#include "vertex.hpp"

enum class tags : char {
    function = 'F',
    backref = 'R',
    digest = 'D',
};

// There is a choice of two hash functions: one that simply accumulates a string representation of
// its inputs (good for observing the code's behavior). A second implementation is based on fnv1a.
// Neither makes an pretence of being a decent message-digest function.

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

    void update (void const * ptr, size_t size) noexcept;
};
#endif // STRING_HASH

#endif // HASH_HPP
