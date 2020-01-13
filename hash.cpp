#include "hash.hpp"

#include <algorithm>
#include "vertex.hpp"

size_t hash::bytes_ = 0;

#ifdef STRING_HASH

using namespace std::string_literals;

std::string hash::prefix () const {
    return (state_.length () > 0) ? "/"s : ""s;
}

void hash::update_vertex (vertex const & x) {
    auto const add = prefix () + static_cast<char> (tags::vertex) + x.name ();
    bytes_ += add.length ();
    state_ += add;
}
void hash::update_backref (size_t backref) {
    auto const add = prefix () + static_cast<char> (tags::backref) + std::to_string (backref);
    bytes_ += add.length ();
    state_ += add;
}
void hash::update_digest (digest const & d) {
    auto const add = prefix () + static_cast<char> (tags::memoized) + d;
    bytes_ += add.length ();
    state_ += add;
}

#else // STRING_HASH

void hash::update_vertex (vertex const & x) noexcept {
    auto const tag = tags::vertex;
    update (&tag, sizeof (tag));
    auto const & name = x.name ();
    update (name.c_str (), name.length () + 1U);
}
void hash::update_backref (size_t backref) noexcept {
    auto const tag = tags::backref;
    update (&tag, sizeof (tag));
    update (&backref, sizeof (backref));
}
void hash::update_digest (digest const & d) noexcept {
    auto const tag = tags::memoized;
    update (&tag, sizeof (tag));
    update (&d, sizeof (d));
}

void hash::update (void const * ptr, size_t size) noexcept {
    auto const p = reinterpret_cast<uint8_t const *> (ptr);
    std::for_each (p, p + size, [this] (uint8_t c) {
        state_ = (state_ ^ static_cast<uint64_t> (c)) * fnv1a_64_prime;
    });
    bytes_ += size;
}

#endif // STRING_HASH
