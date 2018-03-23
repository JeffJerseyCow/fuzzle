import struct

from fuzzle import pypzl
from fuzzle.duzzle.core import utils


# Set architecture
ARCH = pypzl.X86_64

def pack(user_regs):
    """
    Pack x86_64 registers into bytes object.

    Args:
        user_regs: A dictionary of dumped user registers.

    Returns:
        A bytes object packed in the puzzle format.
    """

    # Pack x86_64 registers
    user_regs_data = struct.pack('<Q', int(user_regs['r15'], 16))
    user_regs_data += struct.pack('<Q', int(user_regs['r14'], 16))
    user_regs_data += struct.pack('<Q', int(user_regs['r13'], 16))
    user_regs_data += struct.pack('<Q', int(user_regs['r12'], 16))
    user_regs_data += struct.pack('<Q', int(user_regs['rbp'], 16))
    user_regs_data += struct.pack('<Q', int(user_regs['rbx'], 16))
    user_regs_data += struct.pack('<Q', int(user_regs['r11'], 16))
    user_regs_data += struct.pack('<Q', int(user_regs['r10'], 16))
    user_regs_data += struct.pack('<Q', int(user_regs['r9'], 16))
    user_regs_data += struct.pack('<Q', int(user_regs['r8'], 16))
    user_regs_data += struct.pack('<Q', int(user_regs['rax'], 16))
    user_regs_data += struct.pack('<Q', int(user_regs['rcx'], 16))
    user_regs_data += struct.pack('<Q', int(user_regs['rdx'], 16))
    user_regs_data += struct.pack('<Q', int(user_regs['rsi'], 16))
    user_regs_data += struct.pack('<Q', int(user_regs['rdi'], 16))
    user_regs_data += struct.pack('<Q', int(user_regs['orig_rax'], 16))
    user_regs_data += struct.pack('<Q', int(user_regs['rip'], 16))
    user_regs_data += struct.pack('<Q', int(user_regs['cs'], 16))
    user_regs_data += struct.pack('<Q', int(user_regs['eflags'], 16))
    user_regs_data += struct.pack('<Q', int(user_regs['rsp'], 16))
    user_regs_data += struct.pack('<Q', int(user_regs['ss'], 16))
    user_regs_data += struct.pack('<Q', int(user_regs['fs_base'], 16))
    user_regs_data += struct.pack('<Q', int(user_regs['gs_base'], 16))
    user_regs_data += struct.pack('<Q', int(user_regs['ds'], 16))
    user_regs_data += struct.pack('<Q', int(user_regs['es'], 16))
    user_regs_data += struct.pack('<Q', int(user_regs['fs'], 16))
    user_regs_data += struct.pack('<Q', int(user_regs['gs'], 16))

    return user_regs_data

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
    segments = list(filter(lambda x: (x['perms'] & pypzl.WRITE != 0),
                                     duzzle._segments))

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
    segments = list(filter(lambda x: (x['perms'] & pypzl.EXECUTE != 0),
                                     duzzle._segments))

    # Locate syscall gadget
    for segment in segments:

        # Check kernel segment
        if segment['name'] in duzzle.kernel_segments:
            continue

        # File path
        file_path = utils.file_path(duzzle.pid, '{}.{}'.format(segment['start'],
                                                               segment['perms']))
        # Open raw dump
        with open(file_path, 'rb') as file:
            data = file.read()

        # Iterate bytes
        for offset in range(len(data) - 1):

            # Find syscall instruction
            if data[offset] == 0x0f and data[offset + 1] == 0x05:

                # Resolve address
                return int(segment['start'], 16) + offset
