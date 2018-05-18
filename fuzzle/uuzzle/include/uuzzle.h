/* Includes */
#include <puzzle.h>
#include <stdbool.h>
#include <unicorn.h>


#ifndef __UUZZLE_H__
#define __UUZZLE_H__

/* Definitions */
#define PERMS(__perms) \
        ((__perms & 0x1) << 2) \
        | (__perms & 0x2) \
        | ((__perms &0x4) >> 2)

/* Structures */
typedef struct uzl_options {
  bool verbose;
  bool follow_child;
  char *uzl_file_name;
} uzl_opts_t;

/* Prototypes */
/* Core */
bool uzl_get_uc_arch(pzl_ctx_t *pzl_ctx, uint8_t *arch);
bool uzl_get_uc_mode(pzl_ctx_t *pzl_ctx, uint8_t *mode);
bool uzl_get_cs_arch(pzl_ctx_t *pzl_ctx, uint8_t *arch);
bool uzl_get_cs_mode(pzl_ctx_t *pzl_ctx, uint8_t *mode);
bool uzl_get_pc(pzl_ctx_t *pzl_ctx, uint64_t *pc);
bool uzl_get_usr_regs(pzl_ctx_t *pzl_ctx, void **usr_regs, uzl_opts_t *opts);
bool uzl_set_registers(pzl_ctx_t *pzl_ctx, uc_engine *uc, uzl_opts_t *opts);
bool uzl_map_memory(pzl_ctx_t *pzl_ctx, uc_engine *uc, uzl_opts_t *opts);
bool uzl_reg_sys(pzl_ctx_t *pzl_ctx, uc_engine *uc, uc_hook *sys_hook,
                 uzl_opts_t *opts);
bool uzl_parse_opts(int argc, char **argv, uzl_opts_t *opts);

/* x86_64 */
bool uzl_get_usr_regs_x86_64(pzl_ctx_t *pzl_ctx, void **usr_regs,
                             uzl_opts_t *opts);
bool uzl_get_x86_64_pc(pzl_ctx_t *pzl_ctx, uint64_t *pc);
bool uzl_set_x86_64_registers(pzl_ctx_t *pzl_ctx, uc_engine *uc,
                              uzl_opts_t *opts);
bool uzl_set_x86_64_msr(pzl_ctx_t *pzl_ctx, uc_engine *uc,
                        usr_regs_x86_64_t usr_reg, uzl_opts_t *opts);
bool uzl_reg_linux_x86_64_sys(pzl_ctx_t *pzl_ctx, uc_engine *uc,
                              uc_hook *sys_hook, uzl_opts_t *opts);

#endif
