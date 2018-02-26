import re
import sys
import argparse
import importlib

from fuzzle.duzzle.core.utils import dprint
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
    parser = argparse.ArgumentParser(description='Remote process dumping utility for fuzzle')
    parser.add_argument('address', help='GDBServer IPv4 address')
    parser.add_argument('port', type=int, help='GDBServer TCP port')
    parser.add_argument('--arch', 
                        '-a', 
                        required=True, 
                        help='Target architecture [x86_64]')
    parser.add_argument('--breakpoint', '-b', required=True, help='Location to insert break')
    parser.add_argument('--verbose', '-v', action='store_true', help='Enable verbosity')
    parser.add_argument('--version', action='version', version='0.0.2')
    args = parser.parse_args()

    # Clean up args
    verbose = args.verbose
    address = args.address
    port = args.port

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

    # Break on main
    duzzle.breakpoint(breakpoint)

    # Run
    duzzle.run()

    # Wait for break point
    duzzle.wait(duzzle.BREAKPOINT)

    # Dump process space
    for segment in duzzle.vmmap():

        if segment['permissions'][0] == 'r' and \
           segment['name'] not in duzzle.kernel_segments:

            try:
                resp = duzzle.dump_segment(segment)
            except:
                print('[*] Cannot dump {}'.format(segment['start']))

            print('[*] Dumped {} to {} - {}'.format(segment['start'],
                                                    segment['end'],
                                                    segment['name']))

    duzzle.dump_registers()

    # System message
    print('[*] Output directory "duzzle_{}"'.format(duzzle.pid))

    # Clean up
    duzzle.disconnect()
    duzzle.shutdown()
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
