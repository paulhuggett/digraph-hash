#ifndef VERTEX_HPP
#define VERTEX_HPP

#include <initializer_list>
#include <string>
#include <vector>

struct vertex {
    explicit vertex (char const * name_, std::initializer_list<vertex *> dependents_ = {})
            : name{name_}
            , dependents{dependents_} {}

    void add_dependents (std::initializer_list<vertex *> d) {
        dependents.insert (std::end (dependents), d);
    }

    std::string name;
    std::vector<vertex *> dependents;
};

#endif // VERTEX_HPP
