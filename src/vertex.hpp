#ifndef VERTEX_HPP
#define VERTEX_HPP

#include <initializer_list>
#include <iosfwd>
#include <string>
#include <utility>
#include <vector>

class vertex {
public:
    /// Constructs a new named vertex with zero or more out-going edges.
    explicit vertex (std::string const & name, std::initializer_list<vertex const *> adjacent = {});

    vertex & add_edge (vertex const * const d);
    vertex & add_edge (std::initializer_list<vertex const *> d);

    std::vector<vertex const *> const & out_edges () const noexcept { return adjacent_; }
    std::string const & name () const noexcept { return name_; }

private:
    std::string name_;
    std::vector<vertex const *> adjacent_;
};

std::ostream & operator<< (std::ostream & os, vertex const & v);

#endif // VERTEX_HPP
