#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <puzzle.h>
#include <uuzzle.h>
#include <examples.h>
#include <capstone.h>


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

  /* Initialise puzzle */
  pzl_ctx_t *context;
  pzl_init(&context, UNKN_ARCH);
  if(context == false)
  {
    printf("Cannot initialise context\n");
    return false;
  }

  /* Unpack fuzzle file */
  if(pzl_unpack(context, in_data, bytes_read) == false)
  {
    printf("Cannot unpack data\n");
    goto error;
  }

  error:
    pzl_free(context);
    free(in_data);
    in_data = NULL;
}
