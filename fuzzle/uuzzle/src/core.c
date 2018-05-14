#include <stdint.h>
#include <uuzzle.h>
#include <puzzle.h>
#include <unicorn.h>
#include <capstone.h>


/* Return unicorn architecture */
uint8_t uzl_uc_arch(pzl_ctx_t *context)
{
  switch(context->hdr_rec.arch)
  {
    case X86_64:
      return UC_ARCH_X86;
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
uint8_t uzl_uc_mode(pzl_ctx_t *context)
{
  switch(context->hdr_rec.arch)
  {
    case X86_64:
      return UC_MODE_64;
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
uint8_t uzl_cs_arch(pzl_ctx_t *context)
{
  switch(context->hdr_rec.arch)
  {
    case X86_64:
      return CS_ARCH_X86;
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
uint8_t uzl_cs_mode(pzl_ctx_t *context)
{
  switch(context->hdr_rec.arch)
  {
    case X86_64:
      return CS_MODE_64;
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

/* Map memory regions from uuzzle file to unicorn */
bool uzl_map_memory(pzl_ctx_t *context, uc_engine *uc)
{
  uc_err err;
  mem_rec_t *tmp_mem_rec = context->mem_rec;
  while(tmp_mem_rec != NULL)
  {
    err = uc_mem_map_ptr(uc,
                         tmp_mem_rec->start,
                         tmp_mem_rec->size,
                         PERMS(tmp_mem_rec->perms),
                         tmp_mem_rec->dat);
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

/* Set registers base on architecture */
bool uzl_set_registers(pzl_ctx_t *context, uc_engine *uc)
{
  switch(context->hdr_rec.arch)
  {
    case X86_64:
      return uzl_set_x86_64_registers(context, uc);
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

/* Set registers base on architecture */
uint64_t uzl_get_pc(pzl_ctx_t *context)
{
  switch(context->hdr_rec.arch)
  {
    case X86_64:
      return uzl_get_x86_64_pc(context);
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
