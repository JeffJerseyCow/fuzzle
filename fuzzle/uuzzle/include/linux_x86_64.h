#include <stdint.h>
#include <unicorn.h>


#ifndef __LINUX_X86_64_H__
#define __LINUX_X86_64_H__

/* Structs */
typedef struct linux_x86_64_sys_regs {
  uint64_t rax;
  uint64_t rdi;
  uint64_t rsi;
  uint64_t rdx;
  uint64_t r10;
  uint64_t r8;
  uint64_t r9;
} linux_x86_64_sys_regs_t;

/* Syscall table enum */
enum linux_x86_64_sys_table {
  LINUX_X86_64_SYS_WRITE = 0x01
};

/* Prototypes */
void linux_x86_64_sys_hook_cb(uc_engine *uc, void *user_data);
void linux_x86_64_sys_write(uc_engine *uc,
                            linux_x86_64_sys_regs_t *sys_regs);

#endif
