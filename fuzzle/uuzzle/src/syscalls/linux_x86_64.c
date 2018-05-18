#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <puzzle.h>
#include <uuzzle.h>
#include <unicorn.h>
#include <stdbool.h>
#include <linux_x86_64.h>


/* Register linux x86_64 syscalls */
bool uzl_reg_linux_x86_64_sys(pzl_ctx_t *pzl_ctx, uc_engine *uc,
                              uc_hook *sys_hook, uzl_opts_t *opts)
{
  uc_hook_add(uc, sys_hook, UC_HOOK_INSN, linux_x86_64_sys_hook_cb,
              (void *) opts, 1, 0, UC_X86_INS_SYSCALL);
  return true;
}

/* Syscall callback */
void linux_x86_64_sys_hook_cb(uc_engine *uc, void *usr_data, uzl_opts_t *opts)
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
      linux_x86_64_sys_write(uc, &sys_regs, (uzl_opts_t *) usr_data);
      break;
    case LINUX_X86_64_SYS_CLONE:
      linux_x86_64_sys_clone(uc, &sys_regs, (uzl_opts_t *) usr_data);
      break;
    case LINUX_X86_64_SYS_FORK:
      linux_x86_64_sys_fork(uc, &sys_regs, (uzl_opts_t *) usr_data);
      break;
    default:
      return;
  }
}

/* Write syscall */
void linux_x86_64_sys_write(uc_engine *uc, linux_x86_64_sys_regs_t *sys_regs,
                            uzl_opts_t *opts)
{
  /* Quiet mode */ 
  if(opts->quiet)
    return;

  /* Emulate syscall */
  uint8_t *buf = calloc(1, sys_regs->rdx);
  uc_mem_read(uc, sys_regs->rsi, buf, sys_regs->rdx);

  /* Write to stdout */
  switch(sys_regs->rdi)
  {
    case STDIN_FILENO:
      printf("stdin>:  %s", buf);
      break;
    case STDOUT_FILENO:
      printf("stdout>: %s", buf);
      break;
    case STDERR_FILENO:
      printf("stderr>: %s", buf);
      break;
    default:
      printf("fd %lu>:\t%s", (uint64_t) sys_regs->rdi, buf);
  }

  /* Return code*/
  uc_reg_write(uc, UC_X86_REG_RAX, &(sys_regs->rdx));

  /* Cleanup */
  free(buf);
}

/* Fork syscall */
void linux_x86_64_sys_fork(uc_engine *uc, linux_x86_64_sys_regs_t *sys_regs,
                          uzl_opts_t *opts)
{
  /* Return code*/
  if(opts->follow_child)
    sys_regs->rax = 0;
  uc_reg_write(uc, UC_X86_REG_RAX, &(sys_regs->rax));
}

/* Fork syscall */
void linux_x86_64_sys_clone(uc_engine *uc, linux_x86_64_sys_regs_t *sys_regs,
                           uzl_opts_t *opts)
{
  /* Return code*/
  if(opts->follow_child)
    sys_regs->rax = 0;
  uc_reg_write(uc, UC_X86_REG_RAX, &(sys_regs->rax));
}
