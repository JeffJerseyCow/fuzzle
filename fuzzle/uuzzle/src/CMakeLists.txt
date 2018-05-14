# Set libraries
set(LIBS ${LIBS}
         pthread
         ${FUZZLE_LIBRARIES}
         ${CAPSTONE_LIBRARIES}
         ${UNICORN_LIBRARIES})

# Add library directories
add_subdirectory(arch)
set(LIBS ${LIBS}
         x86_64)

# Add libraries
add_library(core SHARED core.c)
target_link_libraries(core ${LIBS})
set(LIBS ${LIBS}
         core)

# Add examples
add_subdirectory(examples)
