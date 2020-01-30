#include "vertex.hpp"

#include <ostream>

vertex::vertex (std::string const & name, std::initializer_list<vertex const *> adjacent)
        : name_{name}
        , adjacent_{adjacent} {}

vertex & vertex::add_edge (vertex const * const d) {
    adjacent_.insert (std::end (adjacent_), d);
    return *this;
}
vertex & vertex::add_edge (std::initializer_list<vertex const *> d) {
    adjacent_.insert (std::end (adjacent_), d);
    return *this;
}

std::ostream & operator<< (std::ostream & os, vertex const & v) {
    return os << "vertex \"" << v.name () << '"';
}
