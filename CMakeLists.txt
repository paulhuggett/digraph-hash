cmake_minimum_required (VERSION 3.10)
project (digraph-hash CXX)

option (TRACE_ENABLED "Enable trace output")
option (FNV1_HASH_ENABLED "Enable fnv1-based hashing (rather than simple strings)")

add_subdirectory (googletest)
add_subdirectory (src)
add_subdirectory (unittest)
