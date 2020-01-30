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

