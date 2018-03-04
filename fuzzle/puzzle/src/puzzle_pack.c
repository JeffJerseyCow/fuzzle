#include <stdio.h>
#include <miniz.c>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <puzzle.h>


/* Pack into binary */
bool pzl_pack(pzl_ctx_t *context, uint8_t **data, uint64_t *size)
{
    /* Locals */
    uint32_t cmp_status;
    uint64_t mgc_size = pzl_get_mgc_size(context);
    uint64_t hdr_size = pzl_get_hdr_size(context);
    uint64_t mem_size = pzl_get_mem_size(context);
    uint64_t reg_size = pzl_get_reg_size(context);
    uint64_t cmp_size;
    uint64_t offset = 0;

    /* Cumulative size */
    *size = 0; 
    *size += mgc_size; /* Magic bytes */
    *size += hdr_size; /* Header record */

    /* Create temporary data buffer */
    uint8_t *tmp_data = (uint8_t *) malloc(mem_size + reg_size);
    if(tmp_data == NULL)
    {
        printf("pzl_pack: cannot allocate space for data buffer\n");
        return false;
    }

    /* Pack for compression */
    pzl_pack_mem_rec(context, tmp_data, &offset);
    pzl_pack_reg_rec(context, tmp_data, &offset);
    
    /* Create compression buffer */
    cmp_size = compressBound(offset);
    uint8_t *cmp_data = (uint8_t *) malloc(cmp_size);
    if(cmp_data == NULL)
    {
        printf("pzl_pack: cannot allocate space for compression buffer\n");
        free(tmp_data);
        tmp_data = NULL;
        return false;
    }

    /* Compress */
    cmp_status = compress(cmp_data, &cmp_size, (const uint8_t *)tmp_data, offset);
    if (cmp_status != Z_OK)
    {
        printf("pzl_pack: compression failed\n");
        free(tmp_data);
        tmp_data = NULL;
        free(cmp_data);
        cmp_data = NULL;
        return false;
    }

    /* Clean up compression */
    free(tmp_data);
    tmp_data = NULL;

    /* Calculate size */
    *size += cmp_size; /* Compressed data */

    /* Allocate data buffer */
    (*data) = (uint8_t *) malloc(*size);
    if((*data) == NULL)
    {
        printf("pzl_pack: cannot allocate space for data buffer\n");
        free(cmp_data);
        cmp_data = NULL;
        return false;
    }
    context->pkd = true;

    /* Pack data */
    offset = 0;
    pzl_pack_mgc(context, (*data), &offset);
    pzl_pack_hdr_rec(context, (*data), &offset);
    pzl_pack_cmp_dat(cmp_data, (*data), cmp_size, &offset);
    context->pkd_dat = (*data);

    /* Clean up */
    free(cmp_data);
    cmp_data = NULL;    

    return true;
}

/* Pack magic bytes */
bool pzl_pack_mgc(pzl_ctx_t *context, uint8_t *data, uint64_t *offset)
{
    memcpy(data + *offset, &context->mgc, MGC_REC_HDR_LEN);
    *offset += MGC_REC_HDR_LEN;

    return true;
}

/* Pack header record */
bool pzl_pack_hdr_rec(pzl_ctx_t *context, uint8_t *data, uint64_t *offset)
{
    memcpy(data + *offset, &context->hdr_rec, HDR_REC_HDR_LEN);
    *offset += HDR_REC_HDR_LEN;

    return true;
}

/* Pack memory records */
bool pzl_pack_mem_rec(pzl_ctx_t *context, uint8_t *data, uint64_t *offset)
{
    /* Check context pointer */
    if(context == NULL)
    {
        printf("pzl_pack: context has not been set\n");
        return false;
    }

    /* Check at least one memory record */
    if(context->mem_rec == NULL)
    {
        printf("pzl_pack: at least one memory record has to be set\n");
        return false;
    }

    /* Walk list */
    mem_rec_t *cur_mem_rec = context->mem_rec;
    while(cur_mem_rec != NULL)
    {
        /* Pack memory record header */
        memcpy(data + *offset, cur_mem_rec, MEM_REC_HDR_LEN);
        *offset += MEM_REC_HDR_LEN;

        /* Pack data */
        memcpy(data + *offset, cur_mem_rec->dat, cur_mem_rec->size);
        *offset += cur_mem_rec->size;

        /* Pack name string */
        if(cur_mem_rec->str_flag == 0x01)
        {
            memcpy(data + *offset, cur_mem_rec->str, cur_mem_rec->str_size);
            *offset += cur_mem_rec->str_size;
        }

        cur_mem_rec = cur_mem_rec->next;
    }

    return true;    
}

/* Pack register record */
bool pzl_pack_reg_rec(pzl_ctx_t *context, uint8_t *data, uint64_t *offset)
{
    /* Pack register record header */
    memcpy(data + *offset, context->reg_rec, REG_REC_HDR_LEN);
    *offset += REG_REC_HDR_LEN;

    /* Pack user registers */
    memcpy(data + *offset, context->reg_rec->user_regs, context->reg_rec->user_regs_size);
    *offset += context->reg_rec->user_regs_size;

    return true;
}

bool pzl_pack_cmp_dat(uint8_t *cmp_data, uint8_t *data, uint64_t size, uint64_t *offset)
{
    memcpy(data + *offset, cmp_data, size);
    *offset += size;
    return true;
}
