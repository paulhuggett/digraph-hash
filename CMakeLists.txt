cmake_minimum_required (VERSION 3.10)
project (digraph-hash CXX)

option (TRACE_ENABLED "Enable trace output")
option (FNV1_HASH_ENABLED "Enable fnv1-based hashing (rather than simple strings)")

# Tell gtest to link against the "Multi-threaded Debug DLL runtime
# library" on Windows.
set (gtest_force_shared_crt ON CACHE BOOL "Always use msvcrt.dll")

# We don't want to install either gtest or gmock.
set (INSTALL_GTEST Off CACHE BOOL "disable gtest install")
set (INSTALL_GMOCK Off CACHE BOOL "disable gmock install")

add_subdirectory (googletest)
add_subdirectory (src)
add_subdirectory (unittests)
