# Add library files
add_library(puzzle SHARED puzzle.c)

# Add executable
add_executable(main main.c)

# Reference libraries
target_link_libraries(main puzzle)
