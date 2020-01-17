configure_file (
    config.hpp.in
    "${CMAKE_CURRENT_BINARY_DIR}/config.hpp"
    ESCAPE_QUOTES
)

function (configure_target target)
    set_target_properties (${target} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED Yes
        CXX_EXTENSIONS No
    )

    # Tweak options for each compiler.
    if (MSVC)
        # Enable more warnings.
        target_compile_options (${target} PRIVATE /W4)
    elseif (CMAKE_COMPILER_IS_GNUCXX)
        # Enable more warnings.
        target_compile_options (${target} PRIVATE
            -Wall
            -Wextra
            -pedantic
        )
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # Enable more warnings and request libc++
        target_compile_options (${target} PRIVATE
            -Weverything
            -Wno-c++98-compat
            -Wno-c++98-compat-pedantic
            -Wno-padded
            -stdlib=libc++
        )

    endif ()

endfunction()

add_library (digraph-hash
    STATIC
    "${CMAKE_CURRENT_BINARY_DIR}/config.hpp"
    hash.cpp
    hash.hpp

    memhash.cpp
    memhash.hpp
    vertex.hpp
    vertex.cpp
)
target_include_directories (digraph-hash PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
)

configure_target (digraph-hash)



add_executable (digraph-harness main.cpp)
configure_target (digraph-harness)
target_link_libraries (digraph-harness PUBLIC digraph-hash)

# Run the unit tests before trying to link the harness executable.
set (OUT_XML "${CMAKE_BINARY_DIR}/unittests.xml")
add_custom_command (
    TARGET digraph-harness
    PRE_LINK
    COMMAND unittests "--gtest_output=xml:${OUT_XML}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Running unittests"
    DEPENDS unittests
    BYPRODUCTS ${OUT_XML}
    VERBATIM
)

