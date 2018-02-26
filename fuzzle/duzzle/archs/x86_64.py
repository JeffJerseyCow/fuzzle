import struct

from fuzzle.duzzle.core import utils


def dump_registers(duzzle):
    """
    Extract the fs_base and gs_base of the process running under gdbserver.

    Args:
        duzzle: duzzle context object.

    Returns:
        A name address dictionary containing the fs_base and gs_base.
    """
    
    # Regiters dictionary
    registers = {}

    # Syscall codes
    arch_get_fs = '0x1003'
    arch_get_gs = '0x1004'

    # Extract fs base
    registers['fs_base'] = _get_base(duzzle, arch_get_fs)

    # Extract gs base
    registers['gs_base'] = _get_base(duzzle, arch_get_gs)

    return registers

def _get_base(duzzle, code):
    """
    Execute the arch_prctl syscall using executable gadget to extract segment register base.

    Args:
        duzzle: duzzle context object.
        code: Mode of operation for the syscall.

    Returns:
        The string of the desired segment register base.
    """

    # Get syscall address
    syscall_addr = _syscall_addr(duzzle)
    pointer_addr = _pointer_addr(duzzle)

    # Set breakpoint after syscall
    duzzle.breakpoint('*{}'.format(hex(syscall_addr + 2)))

    # Save orignal data
    data = duzzle.read_bytes(hex(pointer_addr), 8)

    # Set registers for syscall
    duzzle.write_register('rax', '0x9e') # Syscall number 
    duzzle.write_register('rdi', code) # Code
    duzzle.write_register('rsi', hex(pointer_addr)) # Pointer address
    duzzle.write_register('rip', hex(syscall_addr)) # Set instruction pointer

    # Execute syscall
    duzzle.run()

    # Wait for syscall to complete
    duzzle.wait(duzzle.BREAKPOINT)

    # Get segment address
    register_base = int(duzzle.read_bytes(hex(pointer_addr), 8), 16)
    register_base = struct.pack('<Q', register_base)

    # Restore clobbered data
    duzzle.write_bytes(hex(pointer_addr), data, 8)

    # Restore clobbered registers
    duzzle.write_register('rax', duzzle._registers['rax'])
    duzzle.write_register('rdi', duzzle._registers['rdi'])
    duzzle.write_register('rsi', duzzle._registers['rsi'])
    duzzle.write_register('rip', duzzle._registers['rip'])

    # Unpack and return segment base
    return hex(struct.unpack('>Q', register_base)[0])

def _pointer_addr(duzzle):
    """
    Searches for the first writable memory segment.

    Args:
        duzzle: duzzle context object.

    Returns:
        Asbolute address of first writeable memory segment.
    """

    # Extract writable segments
    segments = list(filter(lambda x: 'w' in x['permissions'], duzzle._segments))

    # Get first address
    return int(segments[0]['start'], 16)

def _syscall_addr(duzzle):
    """
    Searches for the syscall opcode in executable memory segment.

    Args:
        duzzle: duzzle context object.

    Returns:
        Abolute address of syscall instruction within executable memory segment.
    """

    # Extract executable segments
    segments = list(filter(lambda x: 'x' in x['permissions'], duzzle._segments))

    # Locate syscall gadget
    for segment in segments:

        # Check kernel segment
        if segment['name'] in duzzle.kernel_segments:
            continue

        # File path
        file_path = utils.file_path(duzzle.pid, '{}.{}'.format(segment['start'],
                                                               segment['permissions']))
        # Open raw dump
        with open(file_path, 'rb') as file:
            data = file.read()

        # Iterate bytes
        for offset in range(len(data) - 1):

            # Find syscall instruction
            if data[offset] == 0x0f and data[offset + 1] == 0x05:

                # Resolve address
                return int(segment['start'], 16) + offset
