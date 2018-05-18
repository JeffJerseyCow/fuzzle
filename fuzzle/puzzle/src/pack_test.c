#include <stdbool.h>
#include <puzzle.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>


int main(int argc, char **argv, char **envp)
{

    /* Locals */
    bool ret;

    /* Initialise */
    pzl_ctx_t *context;
    ret = pzl_init(&context, X86_64);
    if(!ret)
    {
        printf("Context not set\n");
        return false;
    }

    /* Create dummy memory record */
    uint64_t dat_size = 1024;

    /* Create dummy mem data */
    uint8_t *dat0 = (uint8_t *) malloc(dat_size);
    uint8_t *dat1 = (uint8_t *) malloc(dat_size);
    uint8_t *dat2 = (uint8_t *) malloc(dat_size);
    memset(dat0, 0x41, dat_size);
    memset(dat1, 0x42, dat_size);
    memset(dat2, 0x43, dat_size);

    /* Create memory record */
    if(pzl_create_mem_rec(context,
                             0x4000,
                             0x4400,
                             0x400,
                             PZL_READ | PZL_EXECUTE,
                             dat0,
                             0,
                             NULL) == false)
    {
        printf("main: couldn't create memory record\n");
    }
    uint8_t str[] = "hello derp";

    /* Create memory record */
    if(pzl_create_mem_rec(context,
                             0x4000,
                             0x4400,
                             0x400,
                             PZL_READ | PZL_EXECUTE,
                             dat1,
                             strlen("hello derp"),
                             str) == false)
    {
        printf("main: couldn't create memory record\n");
    }

    uint8_t *str_s = (uint8_t *) malloc(20);
    sprintf((char *) str_s, "hello");

    /* Create memory record */
    if(pzl_create_mem_rec(context,
                             0x4000,
                             0x4400,
                             0x400,
                             PZL_READ | PZL_EXECUTE,
                             dat2,
                             strlen("hello"),
                             str_s) == false)
    {
        printf("main: couldn't create memory record\n");
    }

    uint8_t *dat3 = (uint8_t *) malloc(dat_size);
    memset(dat3, 0x44, dat_size);

    /* Create memory record */
    if(pzl_create_mem_rec(context,
                             0x4000,
                             0x4400,
                             0x400,
                             PZL_READ | PZL_EXECUTE,
                             dat3,
                             0,
                             NULL) == false)
    {
        printf("main: couldn't create memory record\n");
    }

    /* Create reg record */
    usr_regs_x86_64_t usr_reg;
    memset(&usr_reg, 0x45, sizeof(usr_regs_x86_64_t));
    pzl_create_reg_rec(context, &usr_reg);

    /* Pack data */
    uint8_t *dat;
    uint64_t size;
    dat = (uint8_t *) malloc(pzl_pack_size(context));
    if(dat == NULL)
    {
      printf("main: cannot allocate dat buffer\n");
    }
    else
    {
      pzl_pack(context, dat, &size);

      /* Debug */
      for(uint64_t offset = 0; offset < size; offset++)
          printf("%c", *(dat + offset));
    }

    /* Clean up */
    free(dat);
    free(dat0);
    free(dat1);
    free(dat2);
    free(dat3);
    free(str_s);
    pzl_free(context);

    return true;
}
