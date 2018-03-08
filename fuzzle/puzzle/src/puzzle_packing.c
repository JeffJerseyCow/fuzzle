#include <miniz.c>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <puzzle.h>


/***************************************************************/
/*                           PACKING                           */
/***************************************************************/
bool pzl_pack(pzl_ctx_t *context, uint8_t **data, uint64_t *size)
{
    CHECK_PTR(context, "pzl_pack - context");
    CHECK_PTR(size, "pzl_pack - size");

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
    pzl_pack_cmp_dat(cmp_data, (*data), &offset, cmp_size);
    context->pkd_dat = (*data);

    /* Clean up */
    free(cmp_data);
    cmp_data = NULL;    

    return true;
}

/* Pack magic bytes */
bool pzl_pack_mgc(pzl_ctx_t *context, uint8_t *data, uint64_t *offset)
{
    CHECK_PTR(context, "pzl_pack_mgc - context");
    
    memcpy(data + *offset, context->mgc, sizeof(context->mgc));
    *offset += sizeof(context->mgc);

    return true;
}

/* Pack header record */
bool pzl_pack_hdr_rec(pzl_ctx_t *context, uint8_t *data, uint64_t *offset)
{
    CHECK_PTR(context, "pzl_pack_hdr_rec - context");

    /* Type */
    memcpy(data + *offset, &(context->hdr_rec.type), sizeof(context->hdr_rec.type));
    *offset += sizeof(context->hdr_rec.type);

    /* Length */
    memcpy(data + *offset, &(context->hdr_rec.length), sizeof(context->hdr_rec.length));
    *offset += sizeof(context->hdr_rec.length);

    /* Version */
    memcpy(data + *offset, &(context->hdr_rec.version), sizeof(context->hdr_rec.version));
    *offset += sizeof(context->hdr_rec.version);

    /* Architecture */
    memcpy(data + *offset, &(context->hdr_rec.arch), sizeof(context->hdr_rec.arch));
    *offset += sizeof(context->hdr_rec.arch);

    /* Date size  */
    context->hdr_rec.data_size = pzl_get_mem_size(context) + pzl_get_reg_size(context);
    memcpy(data + *offset, &(context->hdr_rec.data_size), sizeof(context->hdr_rec.data_size));
    *offset += sizeof(context->hdr_rec.data_size);

    return true;
}

/* Pack memory records */
bool pzl_pack_mem_rec(pzl_ctx_t *context, uint8_t *data, uint64_t *offset)
{
    CHECK_PTR(context, "pzl_pack_mem_rec - context");
    CHECK_PTR(context->mem_rec, "pzl_pack_mem_rec - context->mem_rec");

    /* Walk list */
    mem_rec_t *cur_mem_rec = context->mem_rec;
    while(cur_mem_rec != NULL)
    {
        /* Type */
        memcpy(data + *offset, &(cur_mem_rec->type), sizeof(cur_mem_rec->type));
        *offset += sizeof(cur_mem_rec->type);

        /* Length */
        memcpy(data + *offset, &(cur_mem_rec->length), sizeof(cur_mem_rec->length));
        *offset += sizeof(cur_mem_rec->length);

        /* Start */
        memcpy(data + *offset, &(cur_mem_rec->start), sizeof(cur_mem_rec->start));
        *offset += sizeof(cur_mem_rec->start);

        /* End */
        memcpy(data + *offset, &(cur_mem_rec->end), sizeof(cur_mem_rec->end));
        *offset += sizeof(cur_mem_rec->end);

        /* Size */
        memcpy(data + *offset, &(cur_mem_rec->size), sizeof(cur_mem_rec->size));
        *offset += sizeof(cur_mem_rec->size);

        /* Permissions */
        memcpy(data + *offset, &(cur_mem_rec->perms), sizeof(cur_mem_rec->perms));
        *offset += sizeof(cur_mem_rec->perms);

        /* String flag */
        memcpy(data + *offset, &(cur_mem_rec->str_flag), sizeof(cur_mem_rec->str_flag));
        *offset += sizeof(cur_mem_rec->str_flag);

        /* String size */
        memcpy(data + *offset, &(cur_mem_rec->str_size), sizeof(cur_mem_rec->str_size));
        *offset += sizeof(cur_mem_rec->str_size);

        /* Data */
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
    CHECK_PTR(context, "pzl_pack_reg_rec - context");
    CHECK_PTR(context->reg_rec, "pzl_pack_reg_rec - context->reg_rec");
    CHECK_PTR(context->reg_rec->user_regs, "pzl_pack_reg_rec - context->reg_rec->user_regs");

    /* Type */
    memcpy(data + *offset, &(context->reg_rec->type), sizeof(context->reg_rec->type));
    *offset += sizeof(context->reg_rec->type);

    /* Length */
    memcpy(data + *offset, &(context->reg_rec->length), sizeof(context->reg_rec->length));
    *offset += sizeof(context->reg_rec->length);

    /* User registers size */
    memcpy(data + *offset, 
           &(context->reg_rec->user_regs_size), 
           sizeof(context->reg_rec->user_regs_size));
    *offset += sizeof(context->reg_rec->user_regs_size);

    /* User registers */
    memcpy(data + *offset, context->reg_rec->user_regs, context->reg_rec->user_regs_size);
    *offset += context->reg_rec->user_regs_size;

    return true;
}

bool pzl_pack_cmp_dat(uint8_t *cmp_data, uint8_t *data, uint64_t *offset, uint64_t size)
{
    CHECK_PTR(cmp_data, "pzl_pack_cmp_dat - cmp_data");
    CHECK_PTR(data, "pzl_pack_cmp_dat - data");

    memcpy(data + *offset, cmp_data, size);
    *offset += size;
    return true;
}

/***************************************************************/
/*                         UNPACKING                           */
/***************************************************************/
bool pzl_unpack(pzl_ctx_t *context, uint8_t *data, uint64_t size)
{
    CHECK_PTR(context, "pzl_unpack - context");
    CHECK_PTR(data, "pzl_unpack - data");

    /* Loccals */
    bool ret;
    uint64_t offset;
    uint8_t *cmp_data;
    uint8_t *uncmp_data;
    uint32_t cmp_status;

    /* Unpack magic */
    ret = pzl_unpack_mgc(context, data, &offset, size);
    if(ret == false)
    {
        printf("pzl_unpack: cannot unpack magic bytes\n");
        return false;
    }

    /* Unpack header */
    ret = pzl_unpack_hdr_rec(context, data, &offset, size);
    if(ret == false)
    {
        printf("pzl_unpack: cannot unpack header record\n");
        return false;
    }

    /* Unpack compressed data */
    ret = pzl_unpack_cmp_dat(&cmp_data, data, &offset, size);
    if(ret == false)
    {
        printf("pzl_unpack: cannot unpack compressed data\n");
        return false;
    }

    /* Allocate uncompressed data */
    uncmp_data = (uint8_t *) malloc(context->hdr_rec.data_size);
    if(uncmp_data == NULL)
    {
        printf("pzl_unpack: cannot allocated uncompressed data\n");
        free(cmp_data);
        cmp_data = NULL;
        return false;
    }
 
    /* Uncompress */
    cmp_status = uncompress(uncmp_data, &(context->hdr_rec.data_size), cmp_data, size - offset);
    if(cmp_status != Z_OK)
    {
        printf("pzl_unpack: cannot decomress data\n");
        free(cmp_data);
        cmp_data = NULL;
        free(uncmp_data);
        uncmp_data = NULL;
        return false;
    }
    free(cmp_data);
    cmp_data = NULL;

    /* Unpack memory records */

    /* Unpack register record */

    /* Clean up */
    free(uncmp_data);
    uncmp_data = NULL;

}

/* Unpack magic bytes */
bool pzl_unpack_mgc(pzl_ctx_t *context, uint8_t *data, uint64_t *offset, uint64_t size)
{
    CHECK_PTR(context, "pzl_unpack_mgc - context");
    CHECK_PTR(data, "pzl_unpack_mgc - data");
    CHECK_PTR(offset, "pzl_unpack_mgc - offset");
    CHECK_SIZE(size, *offset, 3, "pzl_unpack_mgc - data");

    /* Check magic */
    if(strncmp(data + *offset, "\x55\x5a\x4c", 3) != 0)
    {
        printf("pzl_unpack_mgc: unrecognised file type\n");
        return false;
    }
    /* Copy magic to context */
    memcpy(context->mgc, data + *offset, 3);
    *offset += 3;

    return true;
}

/* Unpack header record */
bool pzl_unpack_hdr_rec(pzl_ctx_t *context, uint8_t *data, uint64_t *offset, uint64_t size)
{
    CHECK_PTR(context, "pzl_unpack_hdr_rec - context");
    CHECK_PTR(data, "pzl_unpack_hdr_rec - data");
    CHECK_PTR(offset, "pzl_unpack_hdr_rec - offset");
    CHECK_SIZE(size, *offset, (2 + 8 + 2 + 4 + 8), "pzl_unpack_hdr_rec - data");
    
    /* Check type */
    if(strncmp(data + *offset, "\x00\x00", 2) != 0)
    {
        printf("pzl_unpack_hdr_rec: cannot find puzzle header\n");
        return false;
    }

    /* Type */
    memcpy(&(context->hdr_rec.type), data + *offset, sizeof(context->hdr_rec.type));
    *offset += sizeof(context->hdr_rec.type);

    /* Length */
    memcpy(&(context->hdr_rec.length), data + *offset, sizeof(context->hdr_rec.length));
    *offset += sizeof(context->hdr_rec.length);

    /* Version */
    memcpy(&(context->hdr_rec.version), data + *offset, sizeof(context->hdr_rec.version));
    *offset += sizeof(context->hdr_rec.version);

    /* Architecture */
    memcpy(&(context->hdr_rec.arch), data + *offset, sizeof(context->hdr_rec.arch));
    *offset += sizeof(context->hdr_rec.arch);

    /* Data size */
    memcpy(&(context->hdr_rec.data_size), data + *offset, sizeof(context->hdr_rec.data_size));
    *offset += sizeof(context->hdr_rec.data_size);

    return true;
}

/* Unpack */
bool pzl_unpack_mem_rec(pzl_ctx_t *context, uint8_t *data, uint64_t *offset, uint64_t size)
{

}

bool pzl_unpack_cmp_dat(uint8_t **cmp_data, uint8_t *data, uint64_t *offset, uint64_t size)
{
    CHECK_PTR(data, "pzl_unpack_cmp_dat - data");
    CHECK_PTR(offset, "pzl_unpack_cmp_dat - offset");
    CHECK_SIZE(size, *offset, 1, "pzl_unpack_cmp_dat");

    /* Allocate compressed data */
    (*cmp_data) = (uint8_t *) malloc(size - *offset);

    /* Compressed data */
    memcpy((*cmp_data), data + *offset, size - *offset);

    return true;
}
