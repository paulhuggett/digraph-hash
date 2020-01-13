#ifndef VERTEX_HPP
#define VERTEX_HPP

#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

class vertex {
public:
    explicit vertex (std::string name, std::initializer_list<vertex *> adjacent = {})
            : name_{std::move (name)}
            , adjacent_{adjacent} {}

    void add_adjacent (std::initializer_list<vertex *> d) {
        adjacent_.insert (std::end (adjacent_), d);
    }

    std::string const & name () const noexcept { return name_; }
    std::vector<vertex *> adjacent () const noexcept { return adjacent_; }

private:
    std::string name_;
    std::vector<vertex *> adjacent_;
};

#endif // VERTEX_HPP
