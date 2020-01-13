cmake_minimum_required(VERSION 3.10)
project (go_hash CXX)
add_executable (go_hash
    main.cpp
    hash.cpp
    hash.hpp
    memhash.cpp
    memhash.hpp
    vertex.hpp
)
set_target_properties (go_hash PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED Yes
  CXX_EXTENSIONS No
)
if (MSVC)
    target_compile_options (go_hash PRIVATE /W4)
elseif (CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options (go_hash PRIVATE -Wall -pedantic)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options (go_hash PRIVATE
        -Weverything
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
        -Wno-padded
    )
endif ()
