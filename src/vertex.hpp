#ifndef VERTEX_HPP
#define VERTEX_HPP

#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

class vertex {
public:
    explicit vertex (std::string const & name, std::initializer_list<vertex const *> adjacent = {})
            : name_{name}
            , adjacent_{adjacent} {}

    std::vector<vertex const *> const & out_edges () const noexcept { return adjacent_; }

    vertex & add_edge (vertex const * const d) {
        adjacent_.insert (std::end (adjacent_), d);
        return *this;
    }
    vertex & add_edge (std::initializer_list<vertex const *> d) {
        adjacent_.insert (std::end (adjacent_), d);
        return *this;
    }

    std::string const & name () const noexcept { return name_; }

private:
    std::string name_;
    std::vector<vertex const *> adjacent_;
};

#endif // VERTEX_HPP
