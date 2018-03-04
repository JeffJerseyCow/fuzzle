#include <stdio.h>
#include <puzzle.h>


/* Get magic size */
uint64_t pzl_get_mgc_size(pzl_ctx_t *context)
{
    return MGC_REC_HDR_LEN;
}

/* Get header size */
uint64_t pzl_get_hdr_size(pzl_ctx_t *context)
{
    return HDR_REC_HDR_LEN;
}

/* Get memory records total size */
uint64_t pzl_get_mem_size(pzl_ctx_t *context)
{
    /* Check context pointer */
    if(context == NULL)
    {
        printf("pzl_get_total_mem_size: context has not been set\n");
        return false;
    }

    /* Check at least one memory record */
    if(context->mem_rec == NULL)
    {
        printf("pzl_get_total_mem_size: at least one memory record has to be set\n");
        return false;
    }

    /* Walk list */
    uint64_t cum_size = 0;
    mem_rec_t *mem_rec = context->mem_rec;
    while(mem_rec != NULL)
    { 
        cum_size += mem_rec->length;
        mem_rec = mem_rec->next;
    }

    return cum_size;
}

/* Get register record size */
uint64_t pzl_get_reg_size(pzl_ctx_t *context)
{
    return context->reg_rec->length;
}