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
bool pzl_pack(pzl_ctx_t *context, uint8_t *data, uint64_t *size)
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

    /* Pack data */
    offset = 0;
    pzl_pack_mgc(context, data, &offset);
    pzl_pack_hdr_rec(context, data, &offset);
    pzl_pack_cmp_dat(cmp_data, data, &offset, cmp_size);

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
    CHECK_PTR(context->reg_rec->usr_reg, "pzl_pack_reg_rec - context->reg_rec->usr_reg");

    /* Type */
    memcpy(data + *offset, &(context->reg_rec->type), sizeof(context->reg_rec->type));
    *offset += sizeof(context->reg_rec->type);

    /* Length */
    memcpy(data + *offset, &(context->reg_rec->length), sizeof(context->reg_rec->length));
    *offset += sizeof(context->reg_rec->length);

    /* User registers size */
    memcpy(data + *offset,
           &(context->reg_rec->usr_reg_len),
           sizeof(context->reg_rec->usr_reg_len));
    *offset += sizeof(context->reg_rec->usr_reg_len);

    /* User registers */
    memcpy(data + *offset, context->reg_rec->usr_reg, context->reg_rec->usr_reg_len);
    *offset += context->reg_rec->usr_reg_len;

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

uint64_t pzl_pack_size(pzl_ctx_t *context)
{
  CHECK_PTR(context, "pzl_pack_size - context");

  /* Cummulative size */
  uint64_t cum_size = 0;
  cum_size += pzl_get_mgc_size(context);
  cum_size += pzl_get_hdr_size(context);
  cum_size += compressBound(pzl_get_mem_size(context) + \
                            pzl_get_reg_size(context));

  return cum_size;
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
    offset = 0;
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
    offset = 0;
    ret = pzl_unpack_mem_rec(context, uncmp_data, &offset, context->hdr_rec.data_size);
    if(ret == false)
    {
        printf("pzl_unpack: cannot unpack memory records\n");
        free(uncmp_data);
        uncmp_data = NULL;
        return false;
    }

    /* Unpack register record */
    ret = pzl_unpack_reg_rec(context, uncmp_data, &offset, context->hdr_rec.data_size);
    if(ret == false)
    {
        printf("pzl_unpack: cannot unpack register record\n");
        free(uncmp_data);
        uncmp_data = NULL;
        return false;
    }

    /* Clean up */
    free(uncmp_data);
    uncmp_data = NULL;

    return true;
}

/* Unpack magic bytes */
bool pzl_unpack_mgc(pzl_ctx_t *context, uint8_t *data, uint64_t *offset, uint64_t size)
{
    CHECK_PTR(context, "pzl_unpack_mgc - context");
    CHECK_PTR(data, "pzl_unpack_mgc - data");
    CHECK_PTR(offset, "pzl_unpack_mgc - offset");
    CHECK_SIZE(size, *offset, 3, "pzl_unpack_mgc - data");

    /* Check magic */
    if(strncmp((const char *) (data + *offset), "\x55\x5a\x4c", 3) != 0)
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
    if(strncmp((const char *) (data + *offset), "\x00\x00", 2) != 0)
    {
        printf("pzl_unpack_hdr_rec: cannot find header\n");
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

/* Unpack memory records */
bool pzl_unpack_mem_rec(pzl_ctx_t *context, uint8_t *data, uint64_t *offset, uint64_t size)
{
    CHECK_PTR(context, "pzl_unpack_mem_rec - context");
    CHECK_PTR(data, "pzl_unpack_mem_rec - data");
    CHECK_PTR(offset, "pzl_unpack_mem_rec - offset");

    /* Unpack */
    while(pzl_unpack_sgl_mem_rec(context, data, offset, size) == true);

    /* Require at least one memory record */
    if(context->mem_rec == NULL)
    {
        printf("pzl_unpack_mem_rec: no memory records found\n");
        return false;
    }

    return true;
}

/* Unpack single memory record */
bool pzl_unpack_sgl_mem_rec(pzl_ctx_t *context, uint8_t *data, uint64_t *offset, uint64_t size)
{
    CHECK_PTR(context, "pzl_unpack_sgl_mem_rec - context");
    CHECK_PTR(data, "pzl_unpack_sgl_mem_rec - data");
    CHECK_PTR(offset, "pzl_unpack_sgl_mem_rec - offset");

    /* Check type */
    if(strncmp((const char *) (data + *offset), "\x01\x00", 2) != 0)
    {
        return false;
    }
    *offset += 2;

    /* Length */
    uint8_t mem_len_buf[8];
    memcpy(mem_len_buf, data + *offset, 8);
    *offset += 8;
    uint64_t mem_len = BUF_TO_UINT64(mem_len_buf);
    if(mem_len > (size - *offset - 8 - 2))
    {
        printf("pzl_unpack_sgl_mem_rec: not enough data remaining\n");
        return false;
    }

    /* Start */
    uint8_t mem_start_buf[8];
    memcpy(mem_start_buf, data + *offset, 8);
    *offset += 8;
    uint64_t mem_start = BUF_TO_UINT64(mem_start_buf);

    /* End */
    uint8_t mem_end_buf[8];
    memcpy(mem_end_buf, data + *offset, 8);
    *offset += 8;
    uint64_t mem_end = BUF_TO_UINT64(mem_end_buf);

    /* Size */
    uint8_t mem_size_buf[8];
    memcpy(mem_size_buf, data + *offset, 8);
    *offset += 8;
    uint64_t mem_size = BUF_TO_UINT64(mem_size_buf);

    /* Permissions */
    uint8_t mem_perms;
    memcpy(&mem_perms, data + *offset, 1);
    *offset += 1;

    /* String flag */
    uint8_t mem_str_flag;
    memcpy(&mem_str_flag, data + *offset, 1);
    *offset += 1;

    /* String length */
    uint64_t mem_str_len;
    if(mem_str_flag == 0x01)
    {
        uint8_t mem_str_len_buf[8];
        memcpy(mem_str_len_buf, data + *offset, 8);
        mem_str_len = BUF_TO_UINT64(mem_str_len_buf);
    }
    else
    {
        mem_str_len = 0x00;
    }
    *offset += 8;

    /* Data */
    uint8_t *mem_data = (uint8_t *) malloc(mem_size);
    if(mem_data == NULL)
    {
        printf("pzl_unpack_sgl_mem_rec: cannot allocate data buffer\n");
        return false;
    }
    memcpy(mem_data, data + *offset, mem_size);
    *offset += mem_size;

    /* String */
    uint8_t *mem_str = NULL;
    if(mem_str_flag == 0x01)
    {
        mem_str = (uint8_t *) malloc(mem_str_len);
        if(mem_str == NULL)
        {
            printf("pzl_unpack_sgl_mem_rec: cannot allocated string buffer\n");
            free(mem_data);
            mem_data = NULL;
        }
        memcpy(mem_str, data + *offset, mem_str_len);
    }
    *offset += mem_str_len;

    /* Create memory record */
    if(pzl_create_mem_rec(context,
                          mem_start,
                          mem_end,
                          mem_size,
                          mem_perms,
                          mem_data,
                          mem_str_len,
                          mem_str) == false)
    {
        printf("pzl_unpack_sgl_mem_rec: cannot create memory record\n");
        free(mem_str);
        mem_str = NULL;
        free(mem_data);
        mem_data = NULL;

        return false;
    }

    /* Clean up */
    free(mem_str);
    mem_str = NULL;
    free(mem_data);
    mem_data = NULL;

    return true;
}

/* Unpack register record */
bool pzl_unpack_reg_rec(pzl_ctx_t *context, uint8_t *data, uint64_t *offset, uint64_t size)
{
    CHECK_PTR(context, "pzl_unpack_reg_rec - context");
    CHECK_PTR(data, "pzl_unpack_reg_rec - data");
    CHECK_PTR(offset, "pzl_unpack_reg_rec - offset");

    /* Check type */
    if(strncmp((const char *) (data + *offset), "\x02\x00", 2) != 0)
    {
        printf("pzl_unpack_reg_rec: cannot find register record\n");
        return false;
    }
    *offset += 2;

    /* Length */
    uint8_t reg_len_buf[8];
    memcpy(reg_len_buf, data + *offset, 8);
    *offset += 8;
    uint64_t reg_len = BUF_TO_UINT64(reg_len_buf);

    /* Check remaining data */
    if(reg_len > (size - (*offset - 8 - 2)))
    {
        printf("pzl_unpack_reg_rec: not enough data remaining\n");
        return false;
    }

    /* User data registers size */
    uint8_t usr_reg_len_buf[8];
    memcpy(usr_reg_len_buf, data + *offset, 8);
    *offset += 8;
    uint64_t usr_reg_len = BUF_TO_UINT64(usr_reg_len_buf);

    /* Check size of user registers */
    uint64_t tmp_usr_reg_len = pzl_get_usr_reg_size(context);
    if(usr_reg_len != tmp_usr_reg_len)
    {
        printf("pzl_unpack_reg_rec: incorrect architecture set\n");
        return false;
    }

    /* Create register record */
    uint8_t *usr_reg = (uint8_t *) malloc(usr_reg_len);
    memcpy(usr_reg, data + *offset, usr_reg_len);
    *offset += usr_reg_len;

    bool ret;
    ret = pzl_create_reg_rec(context, usr_reg);
    if(ret == false)
    {
        printf("pzl_unpack_reg_rec: cannot create register record\n");
        return false;
    }

    /* Clean up */
    free(usr_reg);
    usr_reg = NULL;

    return true;
}

/* Unpack compressed data */
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
