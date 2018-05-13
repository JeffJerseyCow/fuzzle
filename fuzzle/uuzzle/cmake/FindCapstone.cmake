# - Try to find Capstone
# Once done this will define
#  CAPSTONE_FOUND - System has Capstone
#  CAPSTONE_INCLUDE_DIRS - The Capstone include directories
#  CAPSTONE_LIBRARIES - The libraries needed to use Capstone
#  CAPSTONE_DEFINITIONS - Compiler switches required for using Capstone

find_package(PkgConfig)
pkg_check_modules(PC_CAPSTONE REQUIRED capstone)
set(CAPSTONE_DEFINITIONS ${PC_CAPSTONE_CFLAGS_OTHER})

find_path(CAPSTONE_INCLUDE_DIR
          NAMES capstone.h
          HINTS ${PC_CAPSTONE_INCLUDEDIR} ${PC_CAPSTONE_INCLUDE_DIRS})

find_library(CAPSTONE_LIBRARY
             NAMES libcapstone.so
             HINTS ${PC_CAPSTONE_LIBDIR} ${PC_CAPSTONE_LIBRARY_DIRS} )

set(CAPSTONE_LIBRARIES ${CAPSTONE_LIBRARY})
set(CAPSTONE_INCLUDE_DIRS ${CAPSTONE_INCLUDE_DIR})
set(CAPSTONE_FOUND TRUE)
