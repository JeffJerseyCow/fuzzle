#include <stdio.h>
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
    const uint8_t *file_path = argv[1];
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
    in_file = fopen(file_path, "r");
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

    /* Test */
    pzl_unpack(context, in_data, bytes_read);

    /* Free library */
    pzl_free(context);
    free(in_data);
    in_data = NULL;
}
