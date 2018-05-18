#include <stdio.h>
#include <puzzle.h>


/* Get magic size */
uint64_t pzl_get_mgc_size(pzl_ctx_t *context)
{
    CHECK_PTR(context, "pzl_get_mgc_size - context");

    return (3);
}

/* Get header size */
uint64_t pzl_get_hdr_size(pzl_ctx_t *context)
{
    CHECK_PTR(context, "pzl_get_hdr_size - context");

    return context->hdr_rec.length;
}

/* Get memory records total size */
uint64_t pzl_get_mem_size(pzl_ctx_t *context)
{
    /* Check context pointer */
    CHECK_PTR(context, "pzl_get_mem_size - context");
    CHECK_PTR(context->mem_rec, "pzl_get_mem_size - context->mem_rec");

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
    CHECK_PTR(context, "pzl_get_reg_size - context");
    CHECK_PTR(context->reg_rec, "pzl_get_reg_size - context->reg_rec");

    return context->reg_rec->length;
}

uint64_t pzl_get_usr_reg_size(pzl_ctx_t *context)
{
    CHECK_PTR(context, "pzl_get_usr_reg_size - context");

    switch(context->hdr_rec.arch)
    {
        case X86_64:
            return sizeof(usr_regs_x86_64_t);
        default:
            printf("pzl_get_usr_reg_size: unknow architecture\n");
            return false;
    }
}
