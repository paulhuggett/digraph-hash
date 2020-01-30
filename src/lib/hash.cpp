#include "hash.hpp"

#include <algorithm>
#include "vertex.hpp"

namespace {

    enum class tags : char {
        backref = 'R',
        digest = 'D',
        end = 'E',
        vertex = 'V',
    };

} // end anonymous namespace

size_t hash::bytes_ = 0;

#ifdef FNV1_HASH_ENABLED

void hash::update_vertex (vertex const & x) noexcept {
    static constexpr auto tag = tags::vertex;
    update (&tag, sizeof (tag));
    auto const & name = x.name ();
    update (name.c_str (), name.length () + 1U);
}
void hash::update_backref (size_t const backref) noexcept {
    static constexpr auto tag = tags::backref;
    update (&tag, sizeof (tag));
    update (&backref, sizeof (backref));
}
void hash::update_digest (digest const & d) noexcept {
    static constexpr auto tag = tags::digest;
    update (&tag, sizeof (tag));
    update (&d, sizeof (d));
}
void hash::update_end () noexcept {
    static constexpr auto tag = tags::digest;
    update (&tag, sizeof (tag));
}

void hash::update (void const * ptr, size_t const size) noexcept {
    auto * const p = reinterpret_cast<uint8_t const *> (ptr);
    std::for_each (p, p + size, [this] (uint8_t c) {
        state_ = (state_ ^ static_cast<uint64_t> (c)) * fnv1a_64_prime;
    });
    bytes_ += size;
}

#else // FNV1_HASH_ENABLED

using namespace std::string_literals;

std::string hash::prefix () const {
    return (state_.length () > 0) ? "/"s : ""s;
}

void hash::update_vertex (vertex const & x) {
    auto const add = prefix () + static_cast<char> (tags::vertex) + x.name ();
    bytes_ += add.length ();
    state_ += add;
}
void hash::update_backref (size_t const backref) {
    auto const add = prefix () + static_cast<char> (tags::backref) + std::to_string (backref);
    bytes_ += add.length ();
    state_ += add;
}
void hash::update_digest (digest const & d) {
    auto const add = prefix () /*+ static_cast<char> (tags::digest)*/ + d;
    bytes_ += add.length ();
    state_ += add;
}
void hash::update_end () {
    static constexpr auto c = static_cast<char> (tags::end);
    bytes_ += sizeof (c);
    state_ += c;
}

#endif // FNV1_HASH_ENABLED
