# - Try to find Fuzzle
# Once done this will define
#  FUZZLE_FOUND - System has Fuzzle
#  FUZZLE_INCLUDE_DIRS - The Fuzzle include directories
#  FUZZLE_LIBRARIES - The libraries needed to use Fuzzle

# Find includes
find_path(FUZZLE_INCLUDE_DIR
          NAMES "puzzle.h"
          HINTS "${CMAKE_PREFIX_PATH}/include")

# Find libraries
find_library(FUZZLE_LIBRARIY
             NAMES "libpuzzle.so"
             HINTS "${CMAKE_PREFIX_PATH}/lib}")

# Set
set(FUZZLE_INCLUDE_DIRS ${FUZZLE_INCLUDE_DIR})
set(FUZZLE_LIBRARIES ${FUZZLE_LIBRARIY})
set(FUZZLE_FOUND "TRUE")
