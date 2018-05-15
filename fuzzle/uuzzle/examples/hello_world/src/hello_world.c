#include <stdio.h>
#include <stdlib.h>


int main(int argc, char **argv, char **envp)
{
  if(argc != 2)
  {
      printf("please provide one argument\n");
      return -1;
  }

  printf("Welcome\n");
  printf("Hello world %d\n", atoi(argv[1]));

  if(atoi(argv[1]) == 3)
    printf("You've printed three\n");
  else
    printf("You have not\n");

  return 0;
}
