import os
import json

from queue import Queue
from fuzzle.duzzle.core import utils
from pygdbmi.gdbcontroller import GdbController
from fuzzle.duzzle.core.listener import DuzzleListener


class DuzzleContext(object):
    """
    docstring for DuzzleContext
    """

    def __init__(self, arch=None, verbose=False):
        """
        Constructor for DuzzleContext.

        Args:
            verbose: Verbosity flag.

        Returns:
            DuzzleContext object.
        """

        # Super constructor
        super(DuzzleContext, self).__init__()

        # Stop reasons
        self.CLEANEXIT = 'exited-normally'
        self.BREAKPOINT = 'breakpoint-hit'

        # Console
        self.console = []

        # Initialise
        self._arch = arch
        self._token = 0
        self._stopped = []
        self._segments = {}
        self._registers = {}
        self._in_q = Queue()
        self._out_q = Queue()
        self._verbose = verbose

        # Kernel segments
        self.kernel_segments = ['[vvar]',
                                '[vdso]',
                                '[vsyscall]']

        # PID
        self.pid = None

    def set_arch(self, arch):
        """
        Set target architecture.

        Args:
            arch: Imported architecture module.
        """

        self._arch = arch

    def set_verbose(self):
        """
        Turn on verbosity.
        """

        self._verbose = True

    def init(self):
        """
        Initialise context.

        Returns:
            Response from gdb initialisation.
        """

        # Check arch has been set
        if self._arch is None:
            raise Exception('Architecture has not been set')

        # Start listener thread
        self._listener = DuzzleListener(self._out_q, self._in_q, verbose=self._verbose)
        self._listener.daemon = True
        self._listener.start()

        # Initial commands
        resp = self.write('-gdb-set mi-async on')
        if resp['message'] != 'done':
            raise Exception('Cannot enable async mode')

        return resp

    def connect(self, address='127.0.0.1', port=3333):
        """
        Connect to remote gdbserver instance.

        Args:
            address: IP address of gdbserver.
            port: TCP port of gdbserver.

        Returns:
            gdbmi response otherwise raises an exception.
        """ 

        # Connect to gdbserver
        resp = self.write('-target-select remote {}:{}'.format(address, port))
        if resp['message'] != 'connected':
            raise Exception('Cannot connect to target "{}:{}"'.format(address, port))

        return resp

    def disconnect(self):
        """
        Issues the remote target detach command.

        Returns:
            gdbmi response otherwise raises an exception.
        """

        resp = self.write('-target-detach')
        if resp['message'] != 'done':
            raise Exception('Cannot detach from target')

        return resp

    def write(self, gdbmi_cmd):
        """
        Place gdbmi_cmd string in the output queue.

        Args:
            gdbmi_cmd: gdbmi command string.

        Returns:
            Associated gdbmi response message.
        """

        # Build command
        gdbmi_cmd = '{}{}'.format(self._token, gdbmi_cmd)
        self._out_q.put(gdbmi_cmd)

        # Wait for response
        while True:

            if not self._in_q.empty():
                resp = self._in_q.get()

                # Get PID
                if self.pid is None and \
                   resp['payload'] is not None and \
                   'pid' in resp['payload']:

                    # Set PID
                    self.pid = resp['payload']['pid']
                   
                    # Debug
                    utils.dprint('[+] Process PID "{}"'.format(self.pid), self._verbose)

                # Console message
                if resp['type'] == 'console':
                    self.console.append(resp)
                    self._in_q.task_done()
                    continue

                # Stopped message
                elif resp['message'] == 'stopped':
                    self._stopped.append(resp)

                # Response message
                elif resp['message'] is not None and \
                     resp['token'] is not None and \
                     resp['token'] == self._token:
                    self._token += 1
                    self._in_q.task_done()

                    return resp

                # Release lock
                self._in_q.task_done() 

    def wait(self, reason):
        """
        Block until a stop event with specified reason is detected.

        Args:
            reason: Reason the debugger stopped.

        Returns:
            Message from the gdb stop event.
        """

        # Stop event already found
        if len(self._stopped) != 0:
            utils.dprint('[+] Event already stopped', self._verbose)

            for resp in self._stopped:
                if resp['message'] == 'stopped' and \
                   'reason' in resp['payload'] and \
                   resp['payload']['reason'] == reason:
                    
                    return resp

        # Read in_q
        while True:

            if not self._in_q.empty():

                resp = self._in_q.get()

                if resp['message'] == 'stopped' and \
                   'reason' in resp['payload'] and \
                   resp['payload']['reason'] == reason:

                    self._in_q.task_done()
                    return resp

                self._in_q.task_done()

    def run(self):
        """
        Dispatches gdb continue command.

        Returns:
            gdbmi response otherwise raises exception.
        """

        # Dispatch continue
        resp = self.write('-exec-continue')
        if resp['message'] != 'running':
            raise Exception('Cannot continue execution')

        return resp

    def breakpoint(self, address):
        """
        Downloads file from target machine via gdbserver and gdbmi.

        Args:
            src_file: File path of file on the target machine.
            dst_file: File path of file on the local machine.

        Returns:
            Destination file path otherwise raises an exception.
        """

        # Insert breakpoint
        resp = self.write('-break-insert {}'.format(address))
        if resp['message'] != 'done':
            raise Exception('Cannot break on "{}"'.format(address))

        return resp

    def get_file(self, src_file, dst_file):
        """
        Downloads file from target machine via gdbserver and gdbmi.

        Args:
            src_file: Full path of file to download from target machine.
            dst_file: Full path of download file location on local machine.

        Returns:
            The destination file path otherwise raises an exeception.
        """

        # Get file path
        dst_file = utils.file_path(self.pid, dst_file)

        # Download file
        resp = self.write('-target-file-get {} {}'.format(src_file, dst_file))
        if resp['message'] != 'done':
            raise Exception('Cannot download file "{}"'.format(src_file))
        
        # Wait for file
        while True:
            if os.path.isfile(dst_file):
                break

        return dst_file

    def vmmap(self):
        """
        Downloads and parses the inferior memory maps file.

        Returns:
            A parsed Linux maps file in list/dictionary format.
        """

        # Get maps file
        maps_file = self.get_file('/proc/{}/maps'.format(self.pid), 'maps')

        # Debug
        utils.dprint('[+] Downloaded maps file "{}"'.format(maps_file), self._verbose)
        self._segments = utils.parse_map(maps_file)

        return self._segments

    def dump_segment(self, segment):
        """
        Dumps a memory segment from the inferior process to local file.
        
        Args:
            segment: Segment to dump in duzzle format.

        Returns:
           File path where of the dump file otherwise raises an exception.
        """

        # Dump file handle
        dump_file_name = '{}.{}'.format(segment['start'], segment['permissions'])
        dump_file_name = utils.file_path(self.pid, dump_file_name)

        # Dump command
        gdbmi_cmd = '-interpreter-exec console "dump memory {} {} {}"'.format(dump_file_name,
                                                                              segment['start'],
                                                                              segment['end'])

        # Dispatch
        resp = self.write(gdbmi_cmd)
        if resp['message'] != 'done':
            raise Exception('Cannot dump memory region {}-{}'.format(segment['start'],
                                                                     segment['end']))
        return dump_file_name

    def dump_registers(self):
        """
        Combines the register names/values to create a JSON dump and writes it to a file.

        Returns:
            File path of JSON dump file otherwise raises an exception.
        """

        name_list = self._dump_registers_name()
        value_dict = self._dump_registers_value()

        # Create register dictionary
        for num, value in value_dict.items():
            self._registers[name_list[num]] = value

        # Extract architecture specific registers
        self._registers.update(self._arch.dump_registers(self))

        # Dump json
        json_out = json.dumps(self._registers, sort_keys=True, indent=4, separators=(',', ': '))

        # Write to file
        out_file_name = utils.file_path(self.pid, 'regs.json')
        with open(out_file_name, 'w') as file:
            file.write(json_out)

        return out_file_name

    def _dump_registers_name(self):
        """
        Extract register names.

        Returns:
            List of register names otherwise raises an exception.
        """

        resp = self.write('-data-list-register-names')
        if resp['message'] != 'done':
            raise Exception('Cannot dump register names')

        # Parse register name list
        return resp['payload']['register-names']

    def _dump_registers_value(self):
        """
        Extract register values.

        Returns:
            Dictionary indexed by register number containing its value otherwise raises an 
            exception.
        """

        resp = self.write('-data-list-register-values x')
        if resp['message'] != 'done':
            raise Exception('Cannot dump register values')

        # Create register value dictionary
        reg_dict = {}
        for reg in resp['payload']['register-values']:
            reg_dict[int(reg['number'])] = reg['value']

        return reg_dict

    def write_register(self, name, value):
        """
        Write a value to the target register.

        Args:
            name: Name of the target register.
            value: Value to set the register to.

        Returns:
            gdbmi response otherwise raises an exception.
        """

        # Write value to register
        resp = self.write('-gdb-set ${}={}'.format(name, value))
        if resp['message'] != 'done':
            raise Exception('Cannot set register {} to {}'.format(name, value))

        return resp

    def read_bytes(self, address, count):
        """
        Read memory bytes from a target address.
        
        Args:
            address: Abolute address to being reading bytes.
            count: Byte count to read.

        Returns:
            The contents of the address memory location otherwise raises an exception.
        """

        # Read memory as bytes
        resp = self.write('-data-read-memory-bytes {} {}'.format(address, count))
        if resp['message'] != 'done':
            raise Exception('Cannot read {} bytes from {}'.format(count, address))

        return resp['payload']['memory'][0]['contents']

    def write_bytes(self, address, value, count):
        """
        Write a byte value to a target address.

        Args:
            address: Abolute address to begin writing bytes.
            value: Contents to write.
            count: Byte count to write.

        Returns:
            gdbmi response otherwise raises an exception.
        """

        resp = self.write('-data-write-memory-bytes {} {}'.format(address, value, count))
        if resp['message'] != 'done':
            raise Exception('Cannot write {} bytes to {}'.format(count, address))

        return resp

    def shutdown(self):
        """
        Kill all threads and exits gdb.
        """

        # Kill threads
        self._out_q.put('die')
        self._listener.join()
