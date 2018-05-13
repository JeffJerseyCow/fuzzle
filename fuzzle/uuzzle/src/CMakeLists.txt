# Add executables
add_executable(example000 example000.c)

# Libraries
set($LIBS pthread
          ${FUZZLE_LIBRARIES}
          ${CAPSTONE_LIBRARIES}
          ${UNICORN_LIBRARIES})

# Reference libraries
target_link_libraries(example000
                      ${LIBS})
