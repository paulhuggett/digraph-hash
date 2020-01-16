#ifndef HASH_HPP
#define HASH_HPP

#include <cstddef>
#include <cstdint>
#include <string>

#include "config.hpp"

class vertex;

// There is a choice of two hash functions: one that simply accumulates a string representation of
// its inputs (good for observing the code's behavior). A second implementation is based on fnv1a.
// Neither makes any pretence of being a decent message-digest function. Choose which of the two to
// enable using the cmake FNV1_HASH_ENABLED option.

#ifdef FNV1_HASH_ENABLED

class hash {
public:
    using digest = uint64_t;

    digest finalize () const noexcept { return state_; }

    void update_vertex (vertex const & x) noexcept;
    void update_backref (size_t backref) noexcept;
    void update_digest (digest const & d) noexcept;
    void update_end () noexcept;

    static size_t total () noexcept { return bytes_; }

private:
    static constexpr uint64_t fnv1a_64_init = 0xcbf29ce4'84222325U;
    static constexpr uint64_t fnv1a_64_prime = 0x00000100'000001b3U;
    static size_t bytes_;
    uint64_t state_ = fnv1a_64_init;

    void update (void const * ptr, size_t size) noexcept;
};

#else // FNV1_HASH_ENABLED

class hash {
public:
    using digest = std::string;
    digest finalize () const noexcept { return state_; }

    void update_vertex (vertex const & x);
    void update_backref (size_t backref);
    void update_digest (digest const & d);
    void update_end ();

    static size_t total () noexcept { return bytes_; }

private:
    static size_t bytes_;
    std::string state_;
    std::string prefix () const;
};

#endif // FNV1_HASH_ENABLED

#endif // HASH_HPP
