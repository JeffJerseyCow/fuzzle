cmake_minimum_required(VERSION 3.10.0)
project(puzzle)

# Flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

# Includes
include_directories(include)

# Output
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add subdirectories
add_subdirectory(src)
