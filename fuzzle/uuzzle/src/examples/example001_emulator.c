#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <puzzle.h>
#include <uuzzle.h>
#include <unicorn.h>
#include <examples.h>
#include <capstone.h>


#if defined __WITH_TRACE__
/* Capston globals */
csh handle;
cs_insn *insn;
#endif

/* Entry point */
int main(int argc, char **argv, char **envp)
{

  /* Parse args */
  uzl_opts_t opts;
  if(!uzl_parse_opts(argc, argv, &opts))
  {
    printf("example001_emulator: cannot parse arguments\n");
    return false;
  }

  /* Locals */
  struct stat statbuf;
  int32_t ret;

  /* Stat */
  ret = stat(opts.uzl_file_name, &statbuf);
  if(ret != 0 || S_ISDIR(statbuf.st_mode))
  {
      printf("example001_emulator: cannot read file '%s'\n",
             opts.uzl_file_name);
      return false;
  }
  const uint8_t *file_path = (uint8_t *) opts.uzl_file_name;
  uint64_t file_size = statbuf.st_size;
  if(file_size < 1)
  {
      printf("example001_emulator: cannot read empty file '%s\n'", file_path);
      return false;
  }

  /* Allocate data buffer */
  uint8_t *in_data = (uint8_t *) malloc(file_size);

  /* Read data */
  FILE *in_file;
  in_file = fopen((const char *) file_path, "r");
  if(in_file == NULL)
  {
      printf("example001_emulator: cannot read file '%s'\n", file_path);
      free(in_data);
      in_data = NULL;
      return false;
  }

  /* Read data bytes */
  uint64_t bytes_read = fread(in_data, 1, file_size, in_file);
  if(bytes_read != file_size || bytes_read <= 0)
  {
      printf("example001_emulator: cannot read file correctly\n");
      free(in_data);
      in_data = NULL;
      fclose(in_file);
      return false;
  }

  /* Clean up */
  fclose(in_file);

  /* Initialise puzzle */
  pzl_ctx_t *pzl_ctx;
  pzl_init(&pzl_ctx, UNKN_ARCH);
  if(pzl_ctx == false)
  {
    printf("example001_emulator: initialise pzl_ctx\n");
    return false;
  }

  /* Unpack fuzzle file */
  if(pzl_unpack(pzl_ctx, in_data, bytes_read) == false)
  {
    printf("example001_emulator: cannot unpack data\n");
    goto error;
  }

  /* Unicorn locals */
  uc_engine *uc;
  uc_err err;

  /* Initialise unicorn */
  uint8_t arch, mode;
  uzl_get_uc_arch(pzl_ctx, &arch);
  uzl_get_uc_mode(pzl_ctx, &mode);
  err = uc_open(arch, mode, &uc);
  if(err != UC_ERR_OK)
  {
    printf("example001_emulator: cannot initialise unicorn engine\n");
    goto error;
  }

  /* Map memory */
  if(!uzl_map_memory(pzl_ctx, uc, &opts))
  {
    printf("example001_emulator: cannot map memory regions\n");
    goto error;
  }

  /* Map registers */
  if(!uzl_set_registers(pzl_ctx, uc, &opts))
  {
    printf("example001_emulator: cannot map registers\n");
    goto error;
  }

  /* Get user registers */
  usr_regs_x86_64_t *usr_regs = NULL;
  if(!uzl_get_usr_regs(pzl_ctx, (void **) &usr_regs, &opts))
  {
      printf("example001_emulator: cannot get user registers\n");
      goto error;
  }

  /* Debug */
  printf("buf: %p\n", (void *) usr_regs->rsi);
  printf("sze: %p\n", (void *) usr_regs->rdx);
  printf("rip: %p\n", (void *) usr_regs->rip);


#if defined __WITH_TRACE__

  /* Initialise capstone */
  if(!init_capstone(pzl_ctx))
    goto error;

  /* Register trace hook */
  uc_hook trace_hook;
  uc_hook_add(uc, &trace_hook, UC_HOOK_CODE, example000_emulator_hook_code,
              NULL, 1, 0);
#endif

  /* Register syscalls */
  uc_hook sys_hook;
  if(!uzl_reg_sys(pzl_ctx, uc, &sys_hook, &opts))
  {
    printf("example001_emulator: cannot register syscalls\n");
    goto error;
  }

  /* Emulate */
  uint64_t pc;
  uzl_get_pc(pzl_ctx, &pc);
  err = uc_emu_start(uc, pc, 0, 0, 0);
  if(err != UC_ERR_OK)
  {
    printf("example001_emulator: failed to start emulator '%s'\n",
           uc_strerror(err));
    goto error;
  }

  /* Cleanup */
  pzl_free(pzl_ctx);
  free(in_data);
  in_data = NULL;
  return true;

  error:
    pzl_free(pzl_ctx);
    free(in_data);
    in_data = NULL;
    return false;
}

/* Tracing hook */
#if defined __WITH_TRACE__

/* Initialise capstone */
bool init_capstone(pzl_ctx_t *pzl_ctx)
{
  uint8_t arch, mode;
  uzl_get_cs_arch(pzl_ctx, &arch);
  uzl_get_cs_mode(pzl_ctx, &mode);
  if(cs_open(arch, mode, &handle) != CS_ERR_OK)
  {
    printf("init_capstone: cannot initialise capstone\n");
    return false;
  }

  return true;
}

/* Capstone globals */
void example000_emulator_hook_code(uc_engine *uc,
                                          uint64_t address,
                                          uint32_t size,
                                          void *usr_data)
{
  uint64_t count;
  uint8_t *data = (void *) malloc(size);

  /* Read instruction memory */
  if(uc_mem_read(uc, address, data, size))
  {
    printf("example000_emulator_hook_code: cannot read instruction\n");
    return;
  }
  /* Disassemble */
  count = cs_disasm(handle, data, size, address, 0, &insn);
  if(count > 0)
  {
    for(int i = 0; i < count; i ++)
    {
      printf("%p: %s %s\n",
             (void *) address,
             insn[i].mnemonic,
             insn[i].op_str);
    }
  }
  free(data);
}
#endif
