# Add library files
add_library(puzzle SHARED puzzle.c 
                          puzzle_mem.c 
                          puzzle_reg.c
                          puzzle_pack.c
                          puzzle_unpack.c
                          puzzle_utils.c)

# Add executable
add_executable(main main.c)

# Reference libraries
target_link_libraries(main puzzle)
