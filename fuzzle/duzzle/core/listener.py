import threading

from fuzzle.duzzle.core import utils
from pygdbmi.gdbcontroller import GdbController


class DuzzleListener(threading.Thread):
    """
    docstring for DuzzleListener
    """

    def __init__(self, out_q, in_q, verbose):
        """
        Constructor for DuzzleListener.

        Args:
            out_q: Queue of GDMI commands to be dispatched.
            in_q: Queue of responses from GDB.
        """    

        # Super constructor
        super(DuzzleListener, self).__init__()
    
        # Initialise
        self._out_q = out_q
        self._in_q = in_q
        self._verbose = verbose
        self._communicator = GdbController()

    def run(self):
        """
        Main thread.
        """

        # Main listener loop
        while True:

            # Read from gdb
            resp = self._communicator.get_gdb_response(timeout_sec=0, raise_error_on_timeout=False)

            # Write to out queue
            if len(resp) > 0:
                for msg in resp:

                    # Debug
                    utils.dprint('[+] Read response', self._verbose)
                    utils.dprint(msg, self._verbose)

                    self._in_q.put(msg)

            # Out message to process
            if not self._out_q.empty():

                # Get out message
                out_msg = self._out_q.get()

                # Check for kill command
                if out_msg != 'die':

                    # Debug
                    utils.dprint('[+] Dispatched command "{}"'.format(out_msg), self._verbose)

                    # Write to gdb
                    self._communicator.write(out_msg, read_response=False)
                    self._out_q.task_done()

                # Kill thread
                else:

                    # Kill gdb
                    self._communicator.write('-gdb-exit', read_response=False)

                    # Debug
                    utils.dprint('[+] Killing thread', self._verbose)
                    return
