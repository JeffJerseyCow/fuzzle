#include <puzzle.h>

#ifndef __EXAMPLES_H__
#define __EXAMPLES_H__

/* Prototypes */
#if defined __WITH_TRACE__
void example000_emulator_hook_code(uc_engine *uc, uint64_t address,
                                   uint32_t size, void *usr_data);
bool init_capstone(pzl_ctx_t *pzl_ctx);
#endif

#endif
