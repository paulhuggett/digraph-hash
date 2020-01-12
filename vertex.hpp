
#ifndef vertex_h
#define vertex_h

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

#endif /* vertex_h */
