import os
import re
import tempfile

from fuzzle import pypzl


def dprint(msg, verbose):
    """
    Debug print.

    Args:
        msg: Message string to print.
        verbose: Boolean verbosity flag.
    """
    if verbose:
        print(msg)

def file_path(pid, file_name):
    """
    Create directory if it doesn't exists and return full file path.

    Args:
        pid: pid of the process to use as directory name.
        file_name: File name to use.

    Returns:
        Absolute path to requested file name.
    """

    # Check directory exists
    path = os.path.join(tempfile.gettempdir(), 'duzzle_{}'.format(pid))
    os.makedirs(path, exist_ok=True)

    return os.path.join(path, file_name)

def parse_map(file_name):
    """
    Parses a Linux maps file and returns data dictionary.

    Args:
        file_name: Location of maps file on local disk.

    Returns:
        A parsed list of dictionaries containing segment information.
    """

    # Create regex
    addr_regex = re.compile(r'^([0-9a-fA-F]*)-([0-9a-fA-F]*)\s*([rwxaps-]*)\s')

    # Map dictionary
    maps = []

    with open(file_name) as file:
        for line in file.readlines():
            segment = {}
            line = line.rstrip()

            # Extract segment data
            addr_match = re.search(addr_regex, line)
            if addr_match:

                # Build map
                segment['start'] = '0x{}'.format(addr_match.group(1))
                segment['end'] = '0x{}'.format(addr_match.group(2))
                segment['size'] = int(addr_match.group(2), 16) - int(addr_match.group(1), 16)

                str_perms = addr_match.group(3)
                perms = 0
                # Check read
                if 'r' in str_perms:
                    perms = perms | pypzl.READ

                # Check write
                if 'w' in str_perms:
                    perms = perms | pypzl.WRITE

                # Check execute
                if 'x' in str_perms:
                    perms = perms | pypzl.EXECUTE

                segment['perms'] = perms

                # Extract name if exists
                fields = list(filter(lambda x: x != '', line.split(' ')))
                if len(fields) == 6:
                    segment['name'] = fields[-1:][0]
                else:
                    segment['name'] = None

                # Append to maps
                maps.append(segment)

    return maps

def read_file_bytes(file_path):
    """
    """

    # Efficently read
    data_byte_array = bytearray(read_file(file_path))
    data_bytes = bytes(data_byte_array)
    return data_bytes

def read_file(file_path):
    """
    """

    # Read file as bytes
    with open(file_path, 'rb') as file:
        while True:
            data_bytes = file.read(1024)

            # EOF
            if not data_bytes:
                break

            # Read chunk
            for byte in data_bytes:
                yield byte
