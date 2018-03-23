#include <stdint.h>
#include <stdbool.h>


#ifndef __PUZZLE_H__
#define __PUZZLE_H__

/* Memory permissions */
#define PZL_READ 0x04
#define PZL_WRITE 0x02
#define PZL_EXECUTE 0x01

/* Checks */
#define CHECK_PTR(__ptr, __msg) \
    if(__ptr == NULL) \
    { \
        printf("%s: has not been allocated\n", __msg); \
        return false; \
    }

#define CHECK_PTR_DO(__ptr, __msg, __act) \
    if(__ptr == NULL) \
    { \
        printf("%s: has not been allocated\n", __msg); \
        __act \
    }


#define CHECK_SIZE(__size, __offset, __value, __msg) \
    if((__size <= 0)) \
    { \
        printf("%s: cannot be zero or less\n", __msg); \
        return false; \
    } \
    else if(((int64_t) __size - (((int64_t) __offset) & 0x7fffffffffffffff)) <= 0) \
    { \
        printf("%s: offset is too big\n", __msg); \
        return false; \
    } \
    else if(((int64_t) __size - (((int64_t) __offset) & 0x7fffffffffffffff)) < __value) \
    { \
        printf("%s: not enough data remaining\n", __msg); \
        return false; \
    }

/* Conversions */
#define BUF_TO_UINT64(__buf) \
    (uint64_t) __buf[0] + \
    ((uint64_t) __buf[1] << 8) + \
    ((uint64_t) __buf[2] << 16) + \
    ((uint64_t) __buf[3] << 24) + \
    ((uint64_t) __buf[4] << 32) + \
    ((uint64_t) __buf[5] << 40) + \
    ((uint64_t) __buf[6] << 48) + \
    ((uint64_t) __buf[7] << 56)

/*
The UZL format will eventually support 8 processor architectures and have the following format.

             ----------------
             |      UZL     | Magic
             ----------------
             |      HDR     | Header
   --------> ---------------
   |         | MEM_RECORD 0 | Memory Record
   |         ----------------
   |         |     Data     |
   |         ----------------
   |         |     Name     | Optional String Name
   |         ----------------
DEFLATE      | MEM_RECORD N | Memory Record
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

Memory Record's will always be appended with a string TLV for the binary absolute path if one
exists.
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
|   usr_reg_struct   | Registers
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

/*
Puzzle Context
*/
typedef struct pzl_ctx_struct
{
    uint8_t mgc[3];
    hdr_rec_t hdr_rec;
    mem_rec_t *mem_rec;
    reg_rec_t *reg_rec;
} pzl_ctx_t;

/* Function prototypes */
bool pzl_init(pzl_ctx_t **context, arch_t arch);
bool pzl_free(pzl_ctx_t *context);
bool pzl_create_mem_rec(pzl_ctx_t *context,
                        uint64_t start,
                        uint64_t end,
                        uint64_t size,
                        uint8_t perms,
                        uint8_t *dat,
                        uint64_t str_size,
                        uint8_t *str);
bool pzl_append_mem_rec(pzl_ctx_t *context, mem_rec_t *mem_rec);
bool pzl_free_mem_rec(mem_rec_t *mem_rec);
bool pzl_create_reg_rec(pzl_ctx_t *context, void *reg_rec);
uint64_t pzl_get_mgc_size(pzl_ctx_t *context);
uint64_t pzl_get_hdr_size(pzl_ctx_t *context);
uint64_t pzl_get_mem_size(pzl_ctx_t *context);
uint64_t pzl_get_reg_size(pzl_ctx_t *context);
uint64_t pzl_get_usr_reg_size(pzl_ctx_t *context);
bool pzl_pack(pzl_ctx_t *context, uint8_t *data, uint64_t *size);
bool pzl_pack_mgc(pzl_ctx_t *context, uint8_t *data, uint64_t *offset);
bool pzl_pack_hdr_rec(pzl_ctx_t *context, uint8_t *data, uint64_t *offset);
bool pzl_pack_mem_rec(pzl_ctx_t *context, uint8_t *data, uint64_t *offset);
bool pzl_pack_reg_rec(pzl_ctx_t *context, uint8_t *data, uint64_t *offset);
bool pzl_pack_cmp_dat(uint8_t *cmp_data, uint8_t *data, uint64_t *offset, uint64_t size);
uint64_t pzl_pack_size(pzl_ctx_t *context);
bool pzl_unpack(pzl_ctx_t *context, uint8_t *data, uint64_t size);
bool pzl_unpack_mgc(pzl_ctx_t *context, uint8_t *data, uint64_t *offset, uint64_t size);
bool pzl_unpack_hdr_rec(pzl_ctx_t *context, uint8_t *data, uint64_t *offset, uint64_t size);
bool pzl_unpack_mem_rec(pzl_ctx_t *context, uint8_t *data, uint64_t *offset, uint64_t size);
bool pzl_unpack_sgl_mem_rec(pzl_ctx_t *context, uint8_t *data, uint64_t *offset, uint64_t size);
bool pzl_unpack_reg_rec(pzl_ctx_t *context, uint8_t *data, uint64_t *offset, uint64_t size);
bool pzl_unpack_reg_rec(pzl_ctx_t *context, uint8_t *data, uint64_t *offset, uint64_t size);
bool pzl_unpack_cmp_dat(uint8_t **cmp_data, uint8_t *data, uint64_t *offset, uint64_t size);

/* Compression */

#endif
