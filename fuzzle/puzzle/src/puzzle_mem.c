#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <puzzle.h>


/* Create memory record */
bool pzl_create_mem_rec(pzl_ctx_t *context,
                           uint64_t start,
                           uint64_t end,
                           uint64_t size,
                           uint8_t perms,
                           uint8_t *dat,
                           uint8_t *str)
{
    /* Check context pointer */
    if(context == NULL)
    {
        printf("pzl_create_mem_record: context has not been set\n");
        return false;
    }

    /* Check dat pointer */
    if(dat == NULL)
    {
        printf("pzl_create_mem_record: dat has not been set\n");
        return false;
    }

    /* Create mem_rec */
    mem_rec_t *mem_rec = (mem_rec_t *) malloc(sizeof(mem_rec_t));
    if(mem_rec == NULL)
    {
        printf("pzl_create_mem_record: cannot allocate space for memory record\n");
        return false;
    }

    /* Create data buffer */
    uint8_t *dat_buf = (uint8_t *) malloc(size);
    if(dat_buf == NULL)
    {
        printf("pzl_create_mem_record: cannot allocate space for data buffer\n");
        pzl_free_mem_rec(mem_rec);
        return false;
    }

    /* Initalise */
    if(str == NULL)
    {
        mem_rec->str_flag = 0x00;
        mem_rec->str_size = 0x0000000000000000;
        mem_rec->str = NULL;
    }
    else
    {
        /* Create string buffer */
        uint64_t str_size = strlen(str);
        uint8_t *str_buf = (uint8_t *) malloc(str_size);
        if(str_buf == NULL)
        {
            printf("pzl_create_mem_record: cannot allocate space for data buffer\n");
            free(dat_buf);
            dat_buf = NULL;
            pzl_free_mem_rec(mem_rec);
            return false;
        }
        memcpy(str_buf, str, str_size);

        mem_rec->str_flag = 0x01;
        mem_rec->str_size = str_size;
        mem_rec->str = str_buf;
    }

    mem_rec->type = 0x0001;
    mem_rec->length = MEM_REC_HDR_LEN + size + mem_rec->str_size;
    mem_rec->start = start;
    mem_rec->end = end;
    mem_rec->size = size;
    mem_rec->perms = perms;

    /* Copy data buffer */
    memcpy(dat_buf, dat, size);
    mem_rec->dat = dat_buf;
    mem_rec->next = NULL;

    if(pzl_append_mem_rec(context, mem_rec) == false)
    {
        printf("pzl_create_mem_record: cannot append mem_rec to context\n");
        pzl_free_mem_rec(mem_rec);
        return false;
    }

    return true;
}

/* Append new memory record to context */
bool pzl_append_mem_rec(pzl_ctx_t *context, mem_rec_t *mem_rec)
{
    /* Check context pointer */
    if(context == NULL)
    {
        printf("pzl_append_mem_record: context has not been set\n");
        return false;
    }

    /* Set head */
    if(context->mem_rec == NULL)
        context->mem_rec = mem_rec;
    /* Append to list*/
    else
    {
        mem_rec_t *cur_mem_rec = context->mem_rec;
        
        /* Walk context mem_record list */
        while(cur_mem_rec->next != NULL)
        {
            /* Get next node */
            cur_mem_rec = cur_mem_rec->next;
        }

        /* Update if head only */
        cur_mem_rec->next = mem_rec;
    }

    return true;
}

/* Free memory record */
bool pzl_free_mem_rec(mem_rec_t *mem_rec)
{
    /* Check mem_record pointer */
    if(mem_rec == NULL)    
        return true;

    /* Free memory */
    free(mem_rec->str);
    mem_rec->str = NULL;
    free(mem_rec->dat);
    mem_rec->dat = NULL;
    free(mem_rec);
    mem_rec = NULL;

    return true;
}
