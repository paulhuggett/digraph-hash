#include "hash.hpp"

#include <algorithm>

#ifdef STRING_HASH

void hash::update_vertex (vertex const & x) {
    state_ += static_cast<char> (tags::function) + x.name + '/';
}
void hash::update_backref (size_t backref) {
    state_ += static_cast<char> (tags::backref) + std::to_string (backref) + '/';
}
void hash::update_digest (digest const & d) {
    state_ += static_cast<char> (tags::digest) + d;
}

#else // STRING_HASH

void hash::update_vertex (vertex const & x) noexcept {
    auto const tag = tags::function;
    update (&tag, sizeof (tag));
    update (x.name.c_str (), x.name.length () + 1U);
}
void hash::update_backref (size_t backref) noexcept {
    auto const tag = tags::backref;
    update (&tag, sizeof (tag));
    update (&backref, sizeof (backref));
}
void hash::update_digest (digest const & d) noexcept {
    auto const tag = tags::digest;
    update (&tag, sizeof (tag));
    update (&d, sizeof (d));
}

void hash::update (void const * ptr, size_t size) noexcept {
    auto const p = reinterpret_cast<uint8_t const *> (ptr);
    std::for_each (p, p + size, [this] (uint8_t c) {
        state_ = (state_ ^ static_cast<uint64_t> (c)) * fnv1a_64_prime;
    });
}

#endif // STRING_HASH
