# Specific version
cmake_minimum_required(VERSION 3.10.0)

# Set project
project(puzzle)

# Flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -fsanitize=address -Wall")

# Includes
include_directories(include)

# Output
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add subdirectories
add_subdirectory(src)
