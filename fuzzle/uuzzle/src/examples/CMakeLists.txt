# Add executables
add_executable(example000_emulator example000_emulator.c)

# Reference libraries
target_link_libraries(example000_emulator
                      ${LIBS})
