#ifndef TRACE_HPP
#define TRACE_HPP

#ifdef TRACE_ENABLED
#    include <iostream>
#endif

#include "config.hpp"

#ifdef TRACE_ENABLED
void trace_impl () {
    // A trace with no arguments is a no-op.
}
template <typename T, typename... Args>
void trace_impl (T && t, Args &&... args) {
    std::cout << t;
    trace_impl (std::forward<Args> (args)...);
}
#else
struct sink {
    template <typename... Args>
    explicit sink (Args const &...) {}
};
#endif // TRACE_ENABLED

template <typename... Args>
void trace (Args &&... args) {
#ifdef TRACE_ENABLED
    trace_impl (std::forward<Args> (args)...);
    std::cout << '\n';
#else
    sink{args...}; // eat all unused arguments!
#endif // TRACE_ENABLED
}

#endif // TRACE_HPP
