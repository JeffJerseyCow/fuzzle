import os
import sys
import ctypes


# Architecture types
X86_64 = 0
X86_32 = 1
ARM = 2
AARCH64 = 3
PPC_64 = 4
PPC_32 = 5
MIPS_64 = 6
MIPS_32 = 7
UNKN_ARCH = 8

# Main class
class PuzzleContext(object):
    """
    Python binding for the puzzle library.
    """

    def __init__(self, arch):
        """
        Initialise puzzle context.

        Args:
            arch: Processor archiecture.
        """

        # Get file location
        pypzl_dir = os.path.abspath(os.path.dirname(__file__))
        libpuzzle_path = os.path.join(pypzl_dir, 'build', 'lib', 'libpuzzle.so')

        # Load Puzzle library
        try:
            self._libpzl = ctypes.cdll.LoadLibrary(libpuzzle_path)
        except OSError:
            raise OSError('Cannot load Puzzle library')

        # Set context pointer
        self._ctx = ctypes.c_void_p()

        # Set 'bool pzl_init(pzl_ctx_t **context, arch_t arch) function
        self._pzl_init = self._libpzl.pzl_init
        self._pzl_init.argtypes = [ctypes.POINTER(ctypes.c_void_p),
                                   ctypes.c_ulonglong]
        self._pzl_init.restype = ctypes.c_bool

        # Initialise puzzle library
        if not self._pzl_init(ctypes.byref(self._ctx), arch):
            raise Exception('Cannot initialise puzzle library')

    def _get_user_reg_size(self):
        """
        Calculates the correct user register size for the architecture.

        Returns:
            The user register size.
        """

        # Set 'uint64_t pzl_get_usr_reg_size(pzl_ctx_t *context)'
        self._pzl_get_user_reg_size = self._libpzl.pzl_get_usr_reg_size
        self._pzl_get_user_reg_size.argtypes = [ctypes.c_void_p]
        self._pzl_get_user_reg_size.restype = ctypes.c_uint64

        # Get user register size
        user_reg_size = self._pzl_get_user_reg_size(self._ctx)
        if not user_reg_size:
            raise Exception('Cannot get user registers size')

        return user_reg_size

    def _get_pack_size(self):
        """
        Calculates the maximum size of the puzzle context data when packed
        including compression.

        Returns:
            Maximum data size of packed data.
        """

        # Set 'uint64_t pzl_pack_size(pzl_ctx_t *context)'
        self._pzl_pack_size = self._libpzl.pzl_pack_size
        self._pzl_pack_size.argtypes = [ctypes.c_void_p]
        self._pzl_pack_size.restype = ctypes.c_uint64

        # Get packed Size
        pack_size = self._pzl_pack_size(self._ctx)
        if not pack_size:
            raise Exception('Cannot get pack size')

        return pack_size

    def add_mem_rec(self, start, end, perms, data, s_data=None):
        """
        Add memory record to puzzle context.

        Args:
            start: Memory segments start virtual address.
            end: Memory segments end virtual address.
            size: Size of memory segment.
            perms: Permissions of memory segment.
            dat: Raw memory segment data.
            s_size: Size of optional string.
            s_dat: Optional string data.
        """

        # Check data
        if data is None or type(data) != bytes:
            raise Exception('Data must be of type bytes')

        # Check string data
        if s_data is not None and type(s_data) != bytes:
            raise Exception('String data must be of type bytes')

        # Check data size
        data_len = len(data)
        size = end - start
        if size != data_len:
            raise Exception('Memory/data length mismatch')

        # Check string size
        s_size = 0
        if s_data is not None:
            s_size = len(s_data)

        # Set 'bool pzl_create_mem_rec(pzl_ctx_t *context,
        #                              uint64_t start,
        #                              uint64_t end,
        #                              uint64_t size,0x4400
        #                              uint8_t perms,
        #                              uint8_t *data,
        #                              uint64_t str_size,
        #                              uint8_t *str)
        self._pzl_create_mem_rec = self._libpzl.pzl_create_mem_rec
        self._pzl_create_mem_rec.argtypes = [ctypes.c_void_p,
                                             ctypes.c_uint64,
                                             ctypes.c_uint64,
                                             ctypes.c_uint64,
                                             ctypes.c_uint8,
                                             ctypes.c_void_p,
                                             ctypes.c_uint64,
                                             ctypes.c_void_p]
        self._pzl_create_mem_rec.restype = ctypes.c_bool

        # Add memory record
        if not self._pzl_create_mem_rec(self._ctx,
                                        start,
                                        end,
                                        size,
                                        perms,
                                        data,
                                        s_size,
                                        s_data):
            raise Exception('Cannot create memory record')

    def add_reg_rec(self, reg_data):
        """
        Add register record to puzzle context.

        Args:
            reg_rec: Raw register record.
        """

        # Check user register type
        if reg_data is None or type(reg_data) != bytes:
            raise Exception('Register data must be of type bytes')

        # Check user register size
        if len(reg_data) != self._get_user_reg_size():
            raise Exception('User register/length mismatch')

        # Set 'bool pzl_create_reg_rec(pzl_ctx_t *context,
        #                              void *reg_rec)
        self._pzl_create_reg_rec = self._libpzl.pzl_create_reg_rec
        self._pzl_create_reg_rec.argtypes = [ctypes.c_void_p,
                                             ctypes.c_void_p]
        self._pzl_create_reg_rec.restype = ctypes.c_bool

        # Add register record
        if not self._pzl_create_reg_rec(self._ctx, reg_data):
            raise Exception('Cannot create register record')

    def pack(self):
        """
        Packs the puzzle context into UZL format.

        Returns:
            Packed data as a byte array.
        """

        # Get size
        max_size = self._get_pack_size()
        dat = (ctypes.c_uint8 * max_size)()
        size = ctypes.c_uint64()

        # Set 'bool pzl_pack(pzl_ctx_t *context, uint8_t *data, uint64_t *size)'
        self._pzl_pack = self._libpzl.pzl_pack
        self._pzl_pack.argtypes = [ctypes.c_void_p,
                                   ctypes.POINTER(ctypes.c_uint8),
                                   ctypes.POINTER(ctypes.c_uint64)]
        self._pzl_pack.restype = ctypes.c_bool

        # Pack
        if not self._pzl_pack(self._ctx, dat, ctypes.byref(size)):
            raise Exception('Cannot pack data')

        return bytes(dat[0:size.value])

    def free(self):
        """
        Frees the puzzle context.
        """

        # Set bool pzl_free(pzl_context_t *context) function
        self._pzl_free = self._libpzl.pzl_free
        self._pzl_free.argtypes = [ctypes.c_void_p]
        self._pzl_free.restype = ctypes.c_bool

        # Free puzzle library
        if not self._pzl_free(self._ctx):
            raise Exception('Cannot free puzzle library')


if __name__ == '__main__':
    """
    Example python usage that matches bin/pack_test output.
    """

    ctx = PuzzleContext(X86_64)
    dat3 = 'A' * 0x400
    dat2 = 'B' * 0x400
    dat1 = 'C' * 0x400
    dat0 = 'D' * 0x400
    ctx.add_mem_rec(0x4000, 0x4400, 5, str.encode(dat3))
    ctx.add_mem_rec(0x4000,
                    0x4400,
                    5,
                    str.encode(dat2),
                    str.encode('hello derp'))
    ctx.add_mem_rec(0x4000,
                    0x4400,
                    5,
                    str.encode(dat1),
                    str.encode('hello'))
    ctx.add_mem_rec(0x4000, 0x4400, 5, str.encode(dat0))
    ctx.add_reg_rec(str.encode("E" * 216))
    dat = ctx.pack()
    sys.stdout.buffer.write(dat)
    ctx.free()
