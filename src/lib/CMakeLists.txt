configure_file (
    config.hpp.in
    "${CMAKE_CURRENT_BINARY_DIR}/config.hpp"
    ESCAPE_QUOTES
)

add_library (digraph-hash
    STATIC
    "${CMAKE_CURRENT_BINARY_DIR}/config.hpp"
    hash.cpp
    hash.hpp
    memhash.cpp
    memhash.hpp
    trace.hpp
    vertex.hpp
    vertex.cpp
)
target_include_directories (digraph-hash PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
)

configure_target (digraph-hash)
