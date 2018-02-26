#include <stdbool.h>
#include <puzzle.h>
#include <stdio.h>


int main(int argc, char **argv, char **envp)
{

    /* Locals */
    bool ret;

    /* Initialise */
    pzl_ctx_t *context = pzl_init();
    if(context == false)
    {
        printf("Context not set\n");
        return false;
    }

    /* Set arch */
    ret = pzl_set_arch(context, x86_64);
    if(ret == false)
    {
        printf("Couldn't set arch\n");
    }

    /* Clean up */
    pzl_free(context);
    return true;
}