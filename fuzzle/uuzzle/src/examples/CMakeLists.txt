# Add executables
add_executable(example000_emulator example000_emulator.c)
add_executable(example001_emulator example001_emulator.c)

# Reference libraries
target_link_libraries(example000_emulator ${LIBS})
target_link_libraries(example001_emulator ${LIBS})
