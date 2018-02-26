#include <stdint.h>
#include <stdbool.h>

#ifndef __PUZZLE_H__
#define __PUZZLE_H__

/* 
The UZL format will eventually support 8 processor architectures and have the following format.

             ----------------
             |      UZL     | Magic
             ----------------
             |      HDR     | Header
   --------> ---------------
   |         | MEM_RECORD 0 | Memory Record
   |         ----------------
   |         | DAT_RECORD 0 | Data Record
   |         ----------------
   |         | STR_RECORD 0 | String Record (Optional)
   |         ----------------
DEFLATE     | MEM_RECORD N | Memory Record
   |         ----------------
   |         | DAT_RECORD N | Data Record
   |         ----------------
   |         | STR_RECORD N | String Record (Optional)
   |         ----------------
   |         |  REG_RECORD  | Register Record
   --------> ----------------
*/

typedef enum arch_enum
{
    x86_64 = 0,
    x86_32 = 1,
    arm = 2,
    aarch64 = 3,
    ppc_64 = 4,
    ppc_32 = 5,
    mips_64 = 6,
    mips_32 = 7
} arch_t;

/*
Header TLV

----------------------
|       0x0000       | Type
----------------------
| 0x0000000000000004 | Length
---------------------- 
|       0x0000       | Version
---------------------- 
|       0x0000       | Arch
----------------------
*/
typedef struct hdr_struct
{
    uint16_t version;
    arch_t arch; 
} hdr_t;

/*
Memory Record TLV

----------------------
|       0x0001       | Type
----------------------
| 0x0000000000000019 | Length
----------------------
| 0x0000000000000000 | Start
----------------------
| 0x0000000000000000 | End
----------------------
| 0x0000000000000000 | Size
----------------------
|        0x00        | Permissions
----------------------

Memory Record's will always be appended with a string TLV for the binary absolute path if one 
exists.
*/
typedef struct mem_record_struct
{
    uint64_t start;
    uint64_t end;
    uint64_t size;
    uint8_t read : 1;
    uint8_t write : 1;
    uint8_t execute : 1;
    uint8_t padding : 5;
} mem_record_t;

/*
Data Record TLV

----------------------
|       0x0002       | Type
----------------------
| 0x0000000000000000 | Length
----------------------
|        ...         | Data
----------------------
*/
typedef struct dat_record_struct
{
    uint8_t *dat;
} dat_record_t;

/*
String Record TLV

----------------------
|       0x0003       | Type
----------------------
| 0x0000000000000000 | Length
----------------------
|        ...         | String
----------------------
*/
typedef struct str_record_struct
{
    uint8_t *str;
} str_record_t;

/*
Register Record TLV

----------------------
|       0x0004       | Type 
----------------------
| 0x0000000000000000 | Length
----------------------
|  user_regs_struct  | Registers
----------------------
*/

/* 
arch = x86_64 
*/

/* user_64.h */
struct user_regs_struct_x86_64 
{
    unsigned long   r15;
    unsigned long   r14;
    unsigned long   r13;
    unsigned long   r12;
    unsigned long   bp;
    unsigned long   bx;
    unsigned long   r11;
    unsigned long   r10;
    unsigned long   r9;
    unsigned long   r8;
    unsigned long   ax;
    unsigned long   cx;
    unsigned long   dx;
    unsigned long   si;
    unsigned long   di;
    unsigned long   orig_ax;
    unsigned long   ip;
    unsigned long   cs;
    unsigned long   flags;
    unsigned long   sp;
    unsigned long   ss;
    unsigned long   fs_base;
    unsigned long   gs_base;
    unsigned long   ds;
    unsigned long   es;
    unsigned long   fs;
    unsigned long   gs;
};
typedef struct user_regs_struct_x86_64 user_regs_x86_64_t;

/*
Puzzle Context
*/
typedef struct pzl_ctx_struct
{
    uint8_t magic[3];
    hdr_t hdr;
    struct mem_region_hdr_struct *mem_regions;
    void *user_regs;
} pzl_ctx_t;

/*
Memory Record Header
*/
typedef struct mem_region_hdr_struct
{
    struct mem_region_hdr_struct *next;
    mem_record_t mem_record;
    uint64_t dat_size;
    uint8_t *dat;
    uint64_t str_size;
    uint8_t *str;
} mem_record_hdr_t;

/* Function prototypes */
pzl_ctx_t* pzl_init();
void pzl_free_mem(pzl_ctx_t *context);
bool pzl_free(pzl_ctx_t *context);
bool pzl_set_arch(pzl_ctx_t *context, arch_t arch);
bool pzl_add_mem(pzl_ctx_t *context, 
                 mem_record_t *mem_record, 
                 uint8_t *dat, 
                 uint8_t *str);
mem_record_hdr_t* pzl_last_mem_region(pzl_ctx_t *context);

#endif
