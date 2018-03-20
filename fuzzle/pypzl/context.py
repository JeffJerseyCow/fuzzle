import os
import ctypes


class PuzzleContext(object):
    """
    """

    def __init__(self, arch):
        """
        """

        # Get file location
        pypzl_dir = os.path.abspath(os.path.dirname(__file__))
        libpuzzle_path = os.path.join(pypzl_dir, 'build', 'lib', 'libpuzzle.so')

        # Load Puzzle library
        try:
            self._libpzl = ctypes.cdll.LoadLibrary(libpuzzle_path)
        except OSError:
            print('Cannot load Puzzle library')
            raise OSError

        # Get context
        self._ctx = ctypes.c_void_p()

        # Set 'bool pzl_init(pzl_ctx_t **context, arch_t arch) function
        self._pzl_init = self._libpzl.pzl_init
        self._pzl_init.argtypes = [ctypes.POINTER(ctypes.c_void_p),
                                   ctypes.c_ulonglong]
        self._pzl_init.restype = ctypes.c_bool

        # Initialise puzzle library
        if not self._pzl_init(ctypes.byref(self._ctx), arch):
            raise Exception('Cannot initialise puzzle library')

    def add_mem_rec(self, start, end, size, perms, dat, s_size=0, s_dat=None):
        """
        """

        # Set 'bool pzl_create_mem_rec(pzl_ctx_t *context,
        #                              uint64_t start,
        #                              uint64_t end,
        #                              uint64_t size,
        #                              uint8_t perms,
        #                              uint8_t *dat,
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
                                        dat,
                                        s_size,
                                        s_dat):
            raise Exception('Cannot create memory record')

    def add_reg_rec(self, reg_reg):
        """
        """

        # Set 'bool pzl_create_reg_rec(pzl_ctx_t *context,
        #                              void *reg_rec)
        self._pzl_create_reg_rec = self._libpzl.pzl_create_reg_rec
        self._pzl_create_reg_rec.argtypes = [ctypes.c_void_p,
                                             ctypes.c_void_p]
        self._pzl_create_reg_rec.restype = ctypes.c_bool

        # Add register record
        if not self._pzl_create_reg_rec(self._ctx, reg_reg):
            raise Exception('Cannot create register record')

    def _get_size(self):
        """
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

    def pack(self):
        """
        """

        # Get size
        max_size = self._get_size()
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
        """

        # Set bool pzl_free(pzl_context_t *context) function
        self._pzl_free = self._libpzl.pzl_free
        self._pzl_free.argtypes = [ctypes.c_void_p]
        self._pzl_free.restype = ctypes.c_bool

        # Free puzzle library
        if not self._pzl_free(self._ctx):
            raise Exception('Cannot free puzzle library')


if __name__ == '__main__':
    ctx = PuzzleContext(0)
    dat3 = 'A' * 0x400
    dat2 = 'B' * 0x400
    dat1 = 'C' * 0x400
    dat0 = 'D' * 0x400

    ctx.add_mem_rec(0x4000, 0x4400, len(dat3), 5, str.encode(dat3))
    ctx.add_mem_rec(0x4000,
                    0x4400,
                    len(dat2),
                    5,
                    str.encode(dat2),
                    len('hello derp'),
                    str.encode('hello derp'))
    ctx.add_mem_rec(0x4000,
                    0x4400,
                    len(dat1),
                    5,
                    str.encode(dat1),
                    len('hello'),
                    str.encode('hello'))
    ctx.add_mem_rec(0x4000, 0x4400, len(dat0), 5, str.encode(dat0))
    ctx.add_reg_rec(str.encode("E" * 250))
    dat = ctx.pack()
    ctx.free()
