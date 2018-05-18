#include <stdint.h>
#include <string.h>
#include <puzzle.h>
#include <uuzzle.h>
#include <unicorn.h>
#include <stdbool.h>


/* Get user registers */
bool uzl_get_usr_regs_x86_64(pzl_ctx_t *pzl_ctx, void **usr_regs,
  uzl_opts_t *opts)
  {
    *usr_regs = pzl_ctx->reg_rec->usr_reg;
    return true;
  }

/* Get program counter */
bool uzl_get_x86_64_pc(pzl_ctx_t *pzl_ctx, uint64_t *pc)
{
  usr_regs_x86_64_t usr_reg;
  memcpy(&usr_reg, pzl_ctx->reg_rec->usr_reg, pzl_ctx->reg_rec->usr_reg_len);
  *pc = usr_reg.rip;
  return true;
}

/* Set x86_64 specific registers */
bool uzl_set_x86_64_registers(pzl_ctx_t *pzl_ctx, uc_engine *uc,
                              uzl_opts_t *opts)
{
  usr_regs_x86_64_t usr_reg;
  memcpy(&usr_reg, pzl_ctx->reg_rec->usr_reg, pzl_ctx->reg_rec->usr_reg_len);

  /* Set up msrs */
  if(!uzl_set_x86_64_msr(pzl_ctx, uc, usr_reg, opts))
    return false;

  /* Set up registers */
  uc_reg_write(uc, UC_X86_REG_R15, &(usr_reg.r15));
  uc_reg_write(uc, UC_X86_REG_R14, &(usr_reg.r14));
  uc_reg_write(uc, UC_X86_REG_R13, &(usr_reg.r13));
  uc_reg_write(uc, UC_X86_REG_R12, &(usr_reg.r12));
  uc_reg_write(uc, UC_X86_REG_RBP, &(usr_reg.rbp));
  uc_reg_write(uc, UC_X86_REG_RBX, &(usr_reg.rbx));
  uc_reg_write(uc, UC_X86_REG_R11, &(usr_reg.r11));
  uc_reg_write(uc, UC_X86_REG_R10, &(usr_reg.r10));
  uc_reg_write(uc, UC_X86_REG_R9, &(usr_reg.r9));
  uc_reg_write(uc, UC_X86_REG_R8, &(usr_reg.r8));
  uc_reg_write(uc, UC_X86_REG_RAX, &(usr_reg.rax));
  uc_reg_write(uc, UC_X86_REG_RCX, &(usr_reg.rcx));
  uc_reg_write(uc, UC_X86_REG_RDX, &(usr_reg.rdx));
  uc_reg_write(uc, UC_X86_REG_RSI, &(usr_reg.rsi));
  uc_reg_write(uc, UC_X86_REG_RDI, &(usr_reg.rdi));
  uc_reg_write(uc, UC_X86_REG_RIP, &(usr_reg.rip));
  uc_reg_write(uc, UC_X86_REG_CS, &(usr_reg.cs));
  uc_reg_write(uc, UC_X86_REG_EFLAGS, &(usr_reg.eflags));
  uc_reg_write(uc, UC_X86_REG_RSP, &(usr_reg.rsp));
  uc_reg_write(uc, UC_X86_REG_SS, &(usr_reg.ss));
  uc_reg_write(uc, UC_X86_REG_DS, &(usr_reg.ds));
  uc_reg_write(uc, UC_X86_REG_ES, &(usr_reg.es));
  uc_reg_write(uc, UC_X86_REG_FS, &(usr_reg.fs));
  uc_reg_write(uc, UC_X86_REG_GS, &(usr_reg.gs));

  return true;
}

/* Set up msrs */
bool uzl_set_x86_64_msr(pzl_ctx_t *pzl_ctx, uc_engine *uc,
                        usr_regs_x86_64_t usr_reg, uzl_opts_t *opts)
{
  uc_err err;
  uint8_t write_msr[] = "\x0f\x30";
  uint64_t w_rax, w_rdx, w_rcx;

  /* Map scratch space */
  uc_mem_map(uc, 0x1000, 2 * 1024 * 1024, UC_PROT_ALL);
  if(uc_mem_write(uc, 0x1000, write_msr, sizeof(write_msr) - 1))
  {
    printf("uzl_set_x86_64_msr: cannot write msr instructions\n");
    return false;
  }

  /* Set fs msr */
  w_rax = usr_reg.fs_base & 0xffffffff;
  uc_reg_write(uc, UC_X86_REG_RAX, &w_rax);
  w_rdx = (usr_reg.fs_base >> 32) & 0xffffffff;
  uc_reg_write(uc, UC_X86_REG_RDX, &w_rdx);
  w_rcx = 0xc0000100;
  uc_reg_write(uc, UC_X86_REG_RCX, &w_rcx);

  err = uc_emu_start(uc, 0x1000, 0x1000 + sizeof(write_msr) - 1, 0, 0);
  if(err != UC_ERR_OK)
  {
    printf("uzl_set_x86_64_msr: cannot initialise fs_base\n");
    return false;
  }

  /* Set gs msr */
  w_rax = usr_reg.gs_base & 0xffffffff;
  uc_reg_write(uc, UC_X86_REG_RAX, &w_rax);
  w_rdx = (usr_reg.gs_base >> 32) & 0xffffffff;
  uc_reg_write(uc, UC_X86_REG_RDX, &w_rdx);
  w_rcx = 0xc0000101;
  uc_reg_write(uc, UC_X86_REG_RCX, &w_rcx);

  err = uc_emu_start(uc, 0x1000, 0x1000 + sizeof(write_msr) - 1, 0, 0);
  if(err != UC_ERR_OK)
  {
    printf("uzl_set_x86_64_msr: cannot initialise gs_base\n");
    return false;
  }

  /* Unmap scratch space */
  uc_mem_unmap(uc, 0x1000,  2 * 1024 * 1024);

  return true;
}
