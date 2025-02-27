# Designed for C++ to JavaScript transpiling
# -DCMAKE_TOOLCHAIN_FILE="$CHEERP_PATH/share/cmake/Modules/CheerpToolchain.cmake"
cmake_minimum_required(VERSION 3.14)

project(libdatatree
  LANGUAGES CXX
  VERSION 2.0.0)

include_directories(./libdatatree)

file(GLOB_RECURSE datatree_src libdatatree/*.cpp)
file(GLOB_RECURSE datatree_headers libdatatree/*.hpp)

add_library(datatree SHARED ${datatree_src})

install(TARGETS datatree DESTINATION lib/genericjs)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/libdatatree/crails"
        DESTINATION "include"
        FILES_MATCHING #
        PATTERN "*.hpp")
