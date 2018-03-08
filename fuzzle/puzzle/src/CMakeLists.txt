# Add library files
add_library(puzzle SHARED puzzle.c 
                          puzzle_mem.c 
                          puzzle_reg.c
                          puzzle_packing.c
                          puzzle_utils.c)

# Add executable
add_executable(pack_test pack_test.c)
add_executable(unpack_test unpack_test.c)

# Reference libraries
target_link_libraries(pack_test puzzle)
target_link_libraries(unpack_test puzzle)
