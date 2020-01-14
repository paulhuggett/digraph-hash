cmake_minimum_required (VERSION 3.10)
project (digraph-hash CXX)
add_executable (digraph-hash
    main.cpp
    hash.cpp
    hash.hpp
    memhash.cpp
    memhash.hpp
    vertex.hpp
)

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
