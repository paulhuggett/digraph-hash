add_executable (unittests
    test_memhash.cpp
)
target_link_libraries (unittests PRIVATE digraph-hash gmock_main)
set_target_properties (unittests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED Yes
    CXX_EXTENSIONS No
)

