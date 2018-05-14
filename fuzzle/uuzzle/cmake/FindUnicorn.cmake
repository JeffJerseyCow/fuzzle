# - Try to find Unicorn
# Once done this will define
#  UNICORN_FOUND - System has Unicorn
#  UNICORN_INCLUDE_DIRS - The Unicorn include directories
#  UNICORN_LIBRARIES - The libraries needed to use UNICORN
#  UNICORN_DEFINITIONS - Compiler switches required for using Unicorn

find_package(PkgConfig)
pkg_check_modules(PC_UNICORN REQUIRED unicorn)
set(UNICORN_DEFINITIONS ${PC_UNICORN_CFLAGS_OTHER})

find_path(UNICORN_INCLUDE_DIR
          NAMES unicorn/unicorn.h
          HINTS ${PC_UNICORN_INCLUDEDIR} ${PC_UNICORN_INCLUDE_DIRS})

find_library(UNICORN_LIBRARY
             NAMES libunicorn.so
             HINTS ${PC_UNICORN_LIBDIR} ${PC_UNICORN_LIBRARY_DIRS} )

set(UNICORN_LIBRARIES ${UNICORN_LIBRARY})
set(UNICORN_INCLUDE_DIRS ${UNICORN_INCLUDE_DIR}/unicorn)
set(UNICORN_FOUND TRUE)
