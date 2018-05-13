/*
The UZL format will eventually support 8 processor architectures and have the
following format.

             ----------------
             |      UZL     | Magic ('UZL' - 0x555a4c)
             ----------------
             |      HDR     | Header
   --------> ---------------
   |         | MEM_RECORD 0 | Memory Record
   |         ----------------
   |         |     Data     |
   |         ----------------
   |         |     Name     | Optional String Name
   |         ----------------
(DE/IN)FLATE | MEM_RECORD N | Memory Record
   |         ----------------
   |         |     Data     |
   |         ----------------
   |         |     Name     | Optional String Name
   |         ----------------
   |         |  REG_RECORD  | Register Record
   --------> ----------------
*/

typedef enum arch_enum
{
    X86_64 = 0,
    X86_32 = 1,
    ARM = 2,
    AARCH64 = 3,
    PPC_64 = 4,
    PPC_32 = 5,
    MIPS_64 = 6,
    MIPS_32 = 7,
    UNKN_ARCH = 8
} arch_t;

/*
Header TLV

----------------------
|       0x0000       | Type
----------------------
| 0x000000000000000e | Length
----------------------
|       0x0000       | Version
----------------------
|       0x0000       | Arch
----------------------
| 0x0000000000000000 | Data Size
----------------------
*/
typedef struct hdr_struct
{
    uint16_t type;
    uint64_t length;
    uint16_t version;
    arch_t arch;
    uint64_t data_size;
} hdr_rec_t;

/*
Memory Record TLV

----------------------
|       0x0001       | Type
----------------------
| 0x0000000000000024 | Length
----------------------
| 0x0000000000000000 | Start
----------------------
| 0x0000000000000000 | End
----------------------
| 0x0000000000000000 | Size
----------------------
|        0x00        | Permissions
----------------------
|        0x00        | String Name Flag
----------------------
|       *0x00        | Data
----------------------
|       *0x00        | String Name
----------------------

Memory Record's will always be appended with a string TLV for the binary
absolute path if one exists.
*/
typedef struct mem_rec_struct
{
    uint16_t type;
    uint64_t length;
    uint64_t start;
    uint64_t end;
    uint64_t size;
    uint8_t perms;
    uint8_t str_flag;
    uint64_t str_size;
    uint8_t *dat;
    uint8_t *str;
    struct mem_rec_struct *next;
} mem_rec_t;

/*
Register Record TLV

----------------------
|       0x0002       | Type
----------------------
| 0x0000000000000000 | Length
----------------------
| 0x0000000000000000 | User Register Length
----------------------
|       *0x00        | User Registers
----------------------
*/

typedef struct reg_rec_struct
{
    uint16_t type;
    uint64_t length;
    uint64_t usr_reg_len;
    void *usr_reg;
} reg_rec_t;

/*
arch = x86_64
*/
typedef struct user_regs_struct_x86_64
{
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t rbp;
    uint64_t rbx;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rax;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t orig_rax;
    uint64_t rip;
    uint64_t cs;
    uint64_t eflags;
    uint64_t rsp;
    uint64_t ss;
    uint64_t fs_base;
    uint64_t gs_base;
    uint64_t ds;
    uint64_t es;
    uint64_t fs;
    uint64_t gs;
} user_regs_x86_64_t;
