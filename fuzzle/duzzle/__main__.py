import re
import sys
import argparse
import importlib

from fuzzle import pypzl
from fuzzle.duzzle.core.utils import dprint, read_file_bytes
from fuzzle.duzzle.core.context import DuzzleContext


def main(duzzle=DuzzleContext()):
    """
    Main function for duzzle.

    Args:
        duzzle: Duzzle context object.

    Returns:
        True if execution completes expectedly.
    """

    # Build parser
    parser = argparse.ArgumentParser(description= 'Remote process dumping' \
                                                  'utility for fuzzle')
    parser.add_argument('address', help='GDBServer IPv4 address')
    parser.add_argument('port', type=int, help='GDBServer TCP port')
    parser.add_argument('--arch',
                        '-a',
                        required=True,
                        help='Target architecture [x86_64]')
    parser.add_argument('--breakpoint',
                        '-b',
                        required=True,
                        help='Location to insert break')
    parser.add_argument('--out-file',
                        '-o',
                        required=True,
                        help='Output file path to write packed UZL file')
    parser.add_argument('--follow-child',
                        '-f',
                        action='store_true',
                        help='Follow child processes')
    parser.add_argument('--verbose',
                        '-v',
                        action='store_true',
                        help='Enable verbosity')
    parser.add_argument('--version', action='version', version='0.0.9')
    args = parser.parse_args()

    # Clean up args
    address = args.address
    port = args.port
    out_file = args.out_file
    follow_child = args.follow_child
    verbose = args.verbose

    # Import target architecture
    arch = importlib.import_module('fuzzle.duzzle.archs.{}'.format(args.arch))

    # Set context architecture
    duzzle.set_arch(arch)

    # Extract breakpoint
    breakpoint = ''
    if re.search(r'^0x[0-9a-fA-F]*$', args.breakpoint):
        breakpoint = '*{}'.format(args.breakpoint)
    else:
        breakpoint = args.breakpoint

    # Debug
    dprint('[+] Verbosity enabled', verbose)
    if verbose:
        duzzle.set_verbose()

    # Initialise context
    duzzle.init()

    # Connect to remote gdb instance
    duzzle.connect(address=address, port=port)
    print('[*] Process PID "{}"'.format(duzzle.pid))

    # Set breakpoint
    duzzle.breakpoint(breakpoint)

    # Follow child processes
    if follow_child:
        duzzle.follow_child()

    # Continue execution
    duzzle.run()

    # Wait for break point
    duzzle.wait(duzzle.BREAKPOINT)

    # Dump memory segments
    mem_segments = []
    for segment in duzzle.vmmap():

        if (segment['perms'] & pypzl.READ) != 0 and \
           segment['name'] not in duzzle.kernel_segments:

            try:
                file_path = duzzle.dump_segment(segment)
                mem_segments.append({'start': segment['start'],
                                     'end': segment['end'],
                                     'perms': segment['perms'],
                                     'name': segment['name'],
                                     'data': file_path})

                # Debug
                print('[*] Dumped {} to {} - {}'.format(segment['start'],
                                                        segment['end'],
                                                        segment['name']))

            except Exception:
                print('[*] Cannot dump {}'.format(segment['start']))

    # Dump user registers
    user_regs = duzzle.dump_registers()
    print('[*] Dumped user registers')

    # Shutdown
    duzzle.shutdown()

    # Pack
    ctx = pypzl.PuzzleContext(arch.ARCH)

    # Add memory segments
    for segment in mem_segments:

        # Debug
        print('[*] Packing "{}"'.format(segment['data']))

        # Add memory record
        ctx.add_mem_rec(int(segment['start'], 16),
                        int(segment['end'], 16),
                        segment['perms'],
                        read_file_bytes(segment['data']),
                        None if not segment['name'] else \
                        str.encode(segment['name']))

    # Add user registers
    ctx.add_reg_rec(arch.pack(user_regs))

    # Pack data
    pack_data = ctx.pack()

    # Write to file
    with open(out_file, 'wb') as file:
        file.write(pack_data)

    # Clean up

    ctx.free()

    return True


if __name__ == '__main__':
    """
    duzzle entry point.
    """

    # Create context
    duzzle = DuzzleContext()

    # Catch exit
    try:
        sys.exit(main(duzzle))
    except KeyboardInterrupt:

        # Kill threads
        duzzle._out_q.put('die')
        duzzle._listener.join()

        print('[*] Exiting')
