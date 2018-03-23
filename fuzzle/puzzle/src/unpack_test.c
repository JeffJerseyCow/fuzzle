#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <puzzle.h>


int main(int argc, char **argv, char **envp)
{

    /* Parse args */
    if(argc != 2)
    {
        printf("[-] Missing file argument\n");
        return false;
    }

    /* Locals */
    struct stat statbuf;
    int32_t ret;

    /* Stat */
    ret = stat(argv[1], &statbuf);
    if(ret != 0 || S_ISDIR(statbuf.st_mode))
    {
        printf("[-] Cannot read file '%s'\n", argv[1]);
        return false;
    }
    const uint8_t *file_path = (uint8_t *) argv[1];
    uint64_t file_size = statbuf.st_size;
    if(file_size < 1)
    {
        printf("[-] Cannot read empty file '%s\n'", file_path);
        return false;
    }

    /* Allocate data buffer */
    uint8_t *in_data = (uint8_t *) malloc(file_size);

    /* Read data */
    FILE *in_file;
    in_file = fopen((const char *) file_path, "r");
    if(in_file == NULL)
    {
        printf("[-] Cannot read file '%s'\n", file_path);
        free(in_data);
        in_data = NULL;
        return false;
    }

    /* Read data bytes */
    uint64_t bytes_read = fread(in_data, 1, file_size, in_file);
    if(bytes_read != file_size || bytes_read <= 0)
    {
        printf("[-] Cannot read file correctly\n");
        free(in_data);
        in_data = NULL;
        fclose(in_file);
        return false;
    }

    /* Clean up */
    fclose(in_file);

    /* Unpack */
    pzl_ctx_t *context;
    pzl_init(&context, UNKN_ARCH);
    if(context == false)
    {
      printf("Cannot initialise context\n");
      return false;
    }

    /* Test */
    if(pzl_unpack(context, in_data, bytes_read) == false)
    {
      printf("Cannot unpack data\n");
      goto cleanup;
    }

    /* Print architecture */
    const char *arch_str[] = {
        "X86_64",
        "X86_32",
        "ARM",
        "AARCH64",
        "PPC_64",
        "PPC_32",
        "MIPS_64",
        "MIPS_32",
        "UNKN_ARCH"
    };

    /* Check architecture range */
    if(context->hdr_rec.arch >= (sizeof(arch_str)/sizeof(arch_str[0])))
    {
        printf("[*] Unknown arch\n");
        goto cleanup;
    }

    printf("Architecture: %s\n", arch_str[context->hdr_rec.arch]);
    printf("Data size: %lu\n", context->hdr_rec.data_size);
    mem_rec_t *tmp_mem_rec = context->mem_rec;
    while(tmp_mem_rec != NULL)
    {
        printf("--- Memory record ---\n");
        printf("Start address: %p\n", (void *) tmp_mem_rec->start);
        printf("End address: %p\n", (void *) tmp_mem_rec->end);
        printf("Size: %p\n", (void *) tmp_mem_rec->size);
        printf("Permissions: 0x%x\n", tmp_mem_rec->perms);

        /* String */
        if(tmp_mem_rec->str_flag == 0x01)
        {
            uint8_t *str = (uint8_t *) malloc(tmp_mem_rec->str_size + 1);
            memset(str, 0, tmp_mem_rec->str_size + 1);
            memcpy(str, tmp_mem_rec->str, tmp_mem_rec->str_size);
            printf("String: %s\n", str);
            free(str);
            str = NULL;
        }

        tmp_mem_rec = tmp_mem_rec->next;
    };

    printf("--- Register record ---\n");
    user_regs_x86_64_t usr_reg;
    CHECK_PTR_DO(context,
                 "unpack_test - context",
                 goto cleanup;);
    CHECK_PTR_DO(context->reg_rec,
                 "unpack_test - context->reg_reg",
                 goto cleanup;);
    CHECK_PTR_DO(context->reg_rec->usr_reg,
                 "unpack_test - context->reg_rec->usr_reg",
                 goto cleanup;);

    memcpy(&usr_reg, context->reg_rec->usr_reg, context->reg_rec->usr_reg_len);
    printf("r15: %p\n", (void *) usr_reg.r15);
    printf("r14: %p\n", (void *) usr_reg.r14);
    printf("r13: %p\n", (void *) usr_reg.r13);
    printf("r12: %p\n", (void *) usr_reg.r12);
    printf("rbp: %p\n", (void *) usr_reg.rbp);
    printf("rbx: %p\n", (void *) usr_reg.rbx);
    printf("r11: %p\n", (void *) usr_reg.r11);
    printf("r10: %p\n", (void *) usr_reg.r10);
    printf("r9: %p\n", (void *) usr_reg.r9);
    printf("r8: %p\n", (void *) usr_reg.r8);
    printf("rax: %p\n", (void *) usr_reg.rax);
    printf("rcx: %p\n", (void *) usr_reg.rcx);
    printf("rdx: %p\n", (void *) usr_reg.rdx);
    printf("rsi: %p\n", (void *) usr_reg.rsi);
    printf("rdi: %p\n", (void *) usr_reg.rdi);
    printf("orig_rax: %p\n", (void *) usr_reg.orig_rax);
    printf("rip: %p\n", (void *) usr_reg.rip);
    printf("cs: %p\n", (void *) usr_reg.cs);
    printf("eflags: %p\n", (void *) usr_reg.eflags);
    printf("rsp: %p\n", (void *) usr_reg.rsp);
    printf("ss: %p\n", (void *) usr_reg.ss);
    printf("fs_base: %p\n", (void *) usr_reg.fs_base);
    printf("gs_base: %p\n", (void *) usr_reg.gs_base);
    printf("ds: %p\n", (void *) usr_reg.ds);
    printf("es: %p\n", (void *) usr_reg.es);
    printf("fs: %p\n", (void *) usr_reg.fs);
    printf("gs: %p\n", (void *) usr_reg.gs);

    /* Free library */
    cleanup:
    pzl_free(context);
    free(in_data);
    in_data = NULL;
}
