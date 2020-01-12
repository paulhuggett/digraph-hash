cmake_minimum_required(VERSION 3.10)
project (go_hash CXX)
add_executable (go_hash main.cpp)
set_target_properties (go_hash PROPERTIES
  CXX_STANDARD 17
)
