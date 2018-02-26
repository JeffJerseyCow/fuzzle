#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <puzzle.h>


/* Initialise pzl library */
pzl_ctx_t* pzl_init()
{
    /* pzl_ctx_t pointer */
    pzl_ctx_t *context;
    context = (pzl_ctx_t *) calloc(1, sizeof(pzl_ctx_t));

    /* Check allocation */
    if(context == NULL)
    {
        printf("pzl_init: context cannot be allocated\n");
        return false;
    }

    /* Set version */
    memcpy(context->magic, "\x55\x5a\x4c", 3);
    context->hdr.version = 0x0;

    return context;
}

/* Deinitialise memory regions */
void pzl_free_mem(pzl_ctx_t *context)
{

}

/* Deinitialise pzl library */
bool pzl_free(pzl_ctx_t *context)
{
    /* Check context pointer */
    if(context == NULL)
    {
        printf("pzl_free: context has not been set\n");
        return false;
    }
    
    /* Free memory regions */
    pzl_free_mem(context);

    /* Free user registers */
    free(context->user_regs);
    context->user_regs = NULL;

    /* Free context pointer */
    free(context);
    context = NULL;

    return true;
}

/* Set processor architecture */
bool pzl_set_arch(pzl_ctx_t *context, arch_t arch)
{
    /* Check context pointer */
    if(context == NULL)
    {
        printf("pzl_set_arch: context has not been set\n");
        return false;
    }

    /* Update context structure */
    context->hdr.arch = arch;

    return true;
}

/* Add memory region */
bool pzl_add_mem(pzl_ctx_t *context, 
             mem_record_t *mem_record, 
             uint8_t *dat, 
             uint8_t *str)
{

    /* Check context pointer */
    if(context == NULL)
    {
        printf("pzl_add_mem: context has not been set\n");
        return false;
    }

    /* Check mem_record pointer */
    if(mem_record == NULL)
    {
        printf("pzl_add_mem: mem_record has not been set\n");
        return false;
    }

    /* Check dat_record pointer */
    if(dat == NULL)
    {
        printf("pzl_add_mem: dat_record has not been set\n");
        return false;
    }

    /* Allocate memory record header */
    mem_record_hdr_t *mem_record_hdr;
    mem_record_hdr = (mem_record_hdr_t *) calloc(1, sizeof(mem_record_hdr_t));

    if(mem_record_hdr == NULL)
    {
        printf("pzl_add_add: mem_record_hdr cannot be allocated\n");
        return false;
    }

    /* Add mem_record */
    memcpy(&(mem_record_hdr->mem_record), mem_record, sizeof(mem_record_hdr->mem_record));

    /* Add dat region */
    mem_record_hdr->dat_size = sizeof(*dat);

    mem_record_hdr->dat = (uint8_t *) calloc(1, mem_record_hdr->dat_size);

    if(mem_record_hdr->dat == NULL)
    {
        printf("pzl_add_mem: mem_record_hdr->dat cannot be allocated\n");
        return false;
    }

    memcpy(mem_record_hdr->dat, dat, mem_record_hdr->dat_size);

    /* Add str if one exists */
    if(str)
    {
        mem_record_hdr->str_size = sizeof(*str);

        mem_record_hdr->str = (uint8_t *) calloc(1, mem_record_hdr->str_size);

        if(mem_record_hdr->str == NULL)
        {
            printf("pzl_add_mem: mem_record_hdr->str cannot be allocated\n");
            return false;
        }

        memcpy(mem_record_hdr->str, str, mem_record_hdr->str_size);
    }

    /* Add to context */
    if(context->mem_regions == NULL)
    {
        context->mem_regions = mem_record_hdr;
    }
    else
    {
        /* Add to end of list */
        mem_record_hdr_t *last;
    }
}

/* Walk memory region headers and return last */
mem_record_hdr_t* pzl_last_mem_region(pzl_ctx_t *context)
{
    /* Check context pointer */
    if(context == NULL)
    {
        printf("pzl_last_mem_region: context has not been set\n");
        return false;
    }

    /* Check mem_regions pointer */
    if(context->mem_regions == NULL)
    {
        printf("pzl_last_mem_region: context->mem_regions has not been set\n");
        return false;
    }

    /* Iterate mem_regions */
    mem_record_hdr_t *first = context->mem_regions;
    while(true)
    {  
        if(first->next == NULL)
            break;
        else
            first = first->next;
    }

    return first;
}

/*pack
memory_region
registers



unpack
*/
