#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <puzzle.h>


/* Create register record */
bool pzl_create_reg_rec(pzl_ctx_t *context, void *user_regs)
{
    /* Check context pointer */
    if(context == NULL)
    {
        printf("pzl_create_reg_record: context has not been set\n");
        return false;
    }

    /* Check user registers pointer */
    if(user_regs == NULL)
    {
        printf("pzl_create_reg_record: user_regs has not been set\n");
        return false;
    }

    /* Create register record */
    reg_rec_t *reg_rec = (reg_rec_t *) malloc(sizeof(reg_rec_t));
    if(reg_rec == NULL)
    {
        printf("pzl_create_reg_record: register recored cannot be allocated\n");
        return false;
    }
    reg_rec->type = 0x0002;

    /* Allocate and copy user registers */
    switch(context->hdr_rec.arch)
    {
        case x86_64:
            reg_rec->user_regs = (void *) malloc(sizeof(user_regs_x86_64_t));
            memcpy(reg_rec->user_regs, user_regs, sizeof(user_regs_x86_64_t));
            reg_rec->length = REG_REC_HDR_LEN + sizeof(user_regs_x86_64_t);
            reg_rec->user_regs_size = sizeof(user_regs_x86_64_t);
            context->reg_rec = reg_rec;
            break;

        default:
            printf("pzl_create_reg_record: processor arch not recognised\n");
            free(reg_rec);
            reg_rec = NULL;
            return false;
    }

    return true;
}
