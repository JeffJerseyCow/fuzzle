#include <getopt.h>
#include <stdint.h>
#include <uuzzle.h>
#include <puzzle.h>
#include <unicorn.h>
#include <capstone.h>


/* Return unicorn architecture */
bool uzl_get_uc_arch(pzl_ctx_t *pzl_ctx, uint8_t *arch)
{
  switch(pzl_ctx->hdr_rec.arch)
  {
    case X86_64:
      *arch = UC_ARCH_X86;
      return true;
    case X86_32:
    case ARM:
    case AARCH64:
    case PPC_64:
    case PPC_32:
    case MIPS_64:
    case MIPS_32:
    case UNKN_ARCH:
    default:
      printf("uzl_uc_arch: unknown arch\n");
      return -1;
  }
}

/* Return unicorn mode */
bool uzl_get_uc_mode(pzl_ctx_t *pzl_ctx, uint8_t *mode)
{
  switch(pzl_ctx->hdr_rec.arch)
  {
    case X86_64:
      *mode = UC_MODE_64;
      return true;
    case X86_32:
    case ARM:
    case AARCH64:
    case PPC_64:
    case PPC_32:
    case MIPS_64:
    case MIPS_32:
    case UNKN_ARCH:
    default:
      printf("uzl_uc_mode: unknown mode\n");
      return -1;
  }
}

/* Return unicorn architecture */
bool uzl_get_cs_arch(pzl_ctx_t *pzl_ctx, uint8_t *arch)
{
  switch(pzl_ctx->hdr_rec.arch)
  {
    case X86_64:
      *arch = CS_ARCH_X86;
      return true;
    case X86_32:
    case ARM:
    case AARCH64:
    case PPC_64:
    case PPC_32:
    case MIPS_64:
    case MIPS_32:
    case UNKN_ARCH:
    default:
      printf("uzl_cs_arch: unknown arch\n");
      return -1;
  }
}

/* Return unicorn mode */
bool uzl_get_cs_mode(pzl_ctx_t *pzl_ctx, uint8_t *mode)
{
  switch(pzl_ctx->hdr_rec.arch)
  {
    case X86_64:
      *mode = CS_MODE_64;
      return true;
    case X86_32:
    case ARM:
    case AARCH64:
    case PPC_64:
    case PPC_32:
    case MIPS_64:
    case MIPS_32:
    case UNKN_ARCH:
    default:
      printf("uzl_cs_mode: unknown mode\n");
      return -1;
  }
}

/* Set registers base on architecture */
bool uzl_get_pc(pzl_ctx_t *pzl_ctx, uint64_t *pc)
{
  switch(pzl_ctx->hdr_rec.arch)
  {
    case X86_64:
      return uzl_get_x86_64_pc(pzl_ctx, pc);
      break;
    case X86_32:
    case ARM:
    case AARCH64:
    case PPC_64:
    case PPC_32:
    case MIPS_64:
    case MIPS_32:
    case UNKN_ARCH:
    default:
      printf("uzl_get_pc: unknown arch\n");
      return false;
  }
}

/* Get user registers */
bool uzl_get_usr_regs(pzl_ctx_t *pzl_ctx, void **usr_regs, uzl_opts_t *opts)
{
  switch(pzl_ctx->hdr_rec.arch)
  {
    case X86_64:
      return uzl_get_usr_regs_x86_64(pzl_ctx, usr_regs, opts);
      break;
    case X86_32:
    case ARM:
    case AARCH64:
    case PPC_64:
    case PPC_32:
    case MIPS_64:
    case MIPS_32:
    case UNKN_ARCH:
    default:
      printf("uzl_reg_sys: unknown arch\n");
      return false;
  }
}

/* Set registers base on architecture */
bool uzl_set_registers(pzl_ctx_t *pzl_ctx, uc_engine *uc, uzl_opts_t *opts)
{
  switch(pzl_ctx->hdr_rec.arch)
  {
    case X86_64:
      return uzl_set_x86_64_registers(pzl_ctx, uc, opts);
      break;
    case X86_32:
    case ARM:
    case AARCH64:
    case PPC_64:
    case PPC_32:
    case MIPS_64:
    case MIPS_32:
    case UNKN_ARCH:
    default:
      printf("uzl_set_registers: unknown arch\n");
      return false;
  }
}

/* Map memory regions from uuzzle file to unicorn */
bool uzl_map_memory(pzl_ctx_t *pzl_ctx, uc_engine *uc, uzl_opts_t *opts)
{
  uc_err err;
  mem_rec_t *tmp_mem_rec = pzl_ctx->mem_rec;
  while(tmp_mem_rec != NULL)
  {
    err = uc_mem_map_ptr(uc, tmp_mem_rec->start, tmp_mem_rec->size,
                         PERMS(tmp_mem_rec->perms), tmp_mem_rec->dat);
    if(err != UC_ERR_OK)
    {
      printf("uzl_map_memory: cannot map memory region %p\n",
             (void *) tmp_mem_rec->start);
      return false;
    }

    tmp_mem_rec = tmp_mem_rec->next;
  }
  return true;
}

/* Register syscall handlers */
bool uzl_reg_sys(pzl_ctx_t *pzl_ctx, uc_engine *uc, uc_hook *sys_hook,
                 uzl_opts_t *opts)
{
  switch(pzl_ctx->hdr_rec.arch)
  {
    case X86_64:
      return uzl_reg_linux_x86_64_sys(pzl_ctx, uc, sys_hook, opts);
      break;
    case X86_32:
    case ARM:
    case AARCH64:
    case PPC_64:
    case PPC_32:
    case MIPS_64:
    case MIPS_32:
    case UNKN_ARCH:
    default:
      printf("uzl_reg_sys: unknown arch\n");
      return false;
  }
}

/* Parse uuzzle arguments */
bool uzl_parse_opts(int argc, char **argv, uzl_opts_t *opts)
{

  /* Initialise defaults */
  opts->verbose = false;
  opts->follow_child = false;
  opts->uzl_file_name = NULL;

  /* Parse arguments */
  int8_t c;
  struct option long_options[] =
  {
    {"verbose", no_argument, 0, 'v'},
    {"follow_child", no_argument, 0, 'f'},
    {0, 0, 0, 0}
  };

  uint64_t option_index = 0;
  while((c = getopt_long(argc, argv, "fv", long_options,
                        (int *) &option_index)) != -1)
  {
    switch(c)
    {
      case 'v':
        opts->verbose = true;
        break;
      case 'f':
        opts->follow_child = true;
        break;
      case '?':
        return false;
    }
  }

  /* Get positional argument */
  if(argc != optind + 1)
  {
    printf("uzl_parse_opts: incorrect argument count\n");
    return false;
  }
  opts->uzl_file_name = argv[argc - 1];

  /* Debug */
  if(opts->verbose)
    printf("verbosity enabled\n");

  /* Cleanup */
  return true;
}
