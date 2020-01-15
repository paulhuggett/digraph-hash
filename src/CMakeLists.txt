configure_file (
    config.hpp.in
    "${CMAKE_CURRENT_BINARY_DIR}/config.hpp"
    ESCAPE_QUOTES
)

add_executable (digraph-hash
    "${CMAKE_CURRENT_BINARY_DIR}/config.hpp"
    hash.cpp
    hash.hpp
    main.cpp
    memhash.cpp
    memhash.hpp
    vertex.hpp
)

target_include_directories (digraph-hash PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")

set_target_properties (digraph-hash PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED Yes
  CXX_EXTENSIONS No
)

# Enable more warnings.
if (MSVC)
    target_compile_options (digraph-hash PRIVATE /W4)
elseif (CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options (digraph-hash PRIVATE -Wall -Wextra -pedantic)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options (digraph-hash PRIVATE
        -Weverything
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
        -Wno-padded
    )
endif ()
