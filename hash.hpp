#ifndef HASH_HPP
#define HASH_HPP

#include <cstddef>
#include <cstdint>
#include <string>

enum class tags : char {
    backref = 'R',
    memoized = 'M',
    vertex = 'V',
};

class vertex;

// There is a choice of two hash functions: one that simply accumulates a string representation of
// its inputs (good for observing the code's behavior). A second implementation is based on fnv1a.
// Neither makes any pretence of being a decent message-digest function.

#define STRING_HASH 1
#ifdef STRING_HASH

class hash {
public:
    using digest = std::string;
    digest finalize () const noexcept { return state_; }

    void update_vertex (vertex const & x);
    void update_backref (size_t backref);
    void update_digest (digest const & d);

    static size_t total () noexcept { return bytes_; }

private:
    static size_t bytes_;
    std::string state_;
    std::string prefix () const;
};

#else  // STRING_HASH

class hash {
public:
    using digest = uint64_t;

    digest finalize () const noexcept { return state_; }

    void update_vertex (vertex const & x) noexcept;
    void update_backref (size_t backref) noexcept;
    void update_digest (digest const & d) noexcept;

    static size_t total () noexcept { return bytes_; }

private:
    static constexpr uint64_t fnv1a_64_init = 0xcbf29ce4'84222325U;
    static constexpr uint64_t fnv1a_64_prime = 0x00000100'000001b3U;
    static size_t bytes_;
    uint64_t state_ = fnv1a_64_init;

    void update (void const * ptr, size_t size) noexcept;
};
#endif // STRING_HASH

#endif // HASH_HPP
