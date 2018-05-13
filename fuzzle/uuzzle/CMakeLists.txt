cmake_minimum_required(VERSION 3.10.0)
project(uuzzle)

# Flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

# Module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Find packages
find_package(Fuzzle REQUIRED)
if(NOT FUZZLE_FOUND)
  message(FATAL_ERROR "Cannot find 'fuzzle' library")
endif(NOT FUZZLE_FOUND)

find_package(Capstone REQUIRED)
if(NOT CAPSTONE_FOUND)
  message(FATAL_ERROR "Cannot find 'capstone' library")
endif(NOT CAPSTONE_FOUND)

find_package(Unicorn REQUIRED)
if(NOT UNICORN_FOUND)
  message(FATAL_ERROR "Cannot find 'unicorn' library")
endif(NOT UNICORN_FOUND)

# Includes
include_directories(include
                    ${FUZZLE_INCLUDE_DIRS}
                    ${CAPSTONE_INCLUDE_DIRS}
                    ${UNICORN_INCLUDE_DIRS})

# Output
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add subdirectories
add_subdirectory(src)
