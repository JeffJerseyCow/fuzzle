#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <puzzle.h>
#include <uuzzle.h>
#include <unicorn.h>
#include <stdbool.h>
#include <linux_x86_64.h>

/* Register linux x86_64 syscalls */
bool uzl_reg_linux_x86_64_sys(pzl_ctx_t *context,
                              uc_engine *uc,
                              uc_hook *sys_hook)
{
  uc_hook_add(uc,
              sys_hook,
              UC_HOOK_INSN,
              linux_x86_64_sys_hook_cb,
              NULL,
              1,
              0,
              UC_X86_INS_SYSCALL);
  return true;
}

/* Syscall callback */
void linux_x86_64_sys_hook_cb(uc_engine *uc, void *user_data)
{
  linux_x86_64_sys_regs_t sys_regs;

  /* Get sys parameters */
  uc_reg_read(uc, UC_X86_REG_RAX, &(sys_regs.rax));
  uc_reg_read(uc, UC_X86_REG_RDI, &(sys_regs.rdi));
  uc_reg_read(uc, UC_X86_REG_RSI, &(sys_regs.rsi));
  uc_reg_read(uc, UC_X86_REG_RDX, &(sys_regs.rdx));
  uc_reg_read(uc, UC_X86_REG_R10, &(sys_regs.r10));
  uc_reg_read(uc, UC_X86_REG_R8, &(sys_regs.r8));
  uc_reg_read(uc, UC_X86_REG_R9, &(sys_regs.r9));

  /* Parse sys number */
  switch(sys_regs.rax)
  {
    case LINUX_X86_64_SYS_WRITE:
      linux_x86_64_sys_write(uc, &sys_regs);
      break;
    default:
      return;
  }
}

/* Write syscall */
void linux_x86_64_sys_write(uc_engine *uc,
                            linux_x86_64_sys_regs_t *sys_regs)
{
  /* Emulate syscall */
  uint8_t *buf = calloc(1, sys_regs->rdx);
  uc_mem_read(uc, sys_regs->rsi, buf, sys_regs->rdx);
  printf("%s", buf);

  /* Return code*/
  uc_reg_write(uc, UC_X86_REG_RAX, &(sys_regs->rdx));

  /* Cleanup */
  free(buf);
}
