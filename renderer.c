#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#include "renderer.h"

#define MAX_COL 20
#define MAX_ROW 1000
#define STR_BUF 128

struct _rd {
    uint16_t header_no;
    uint16_t sizes[MAX_COL];
    char *headers[MAX_COL];
    char *formats[MAX_COL];
    uint16_t item_no;
    char *items[MAX_COL * MAX_ROW];
};

rd_t *rd_init(void)
{
    rd_t *rd = (rd_t *)calloc(1, sizeof(rd_t));
    rd->header_no = 0;
    rd->item_no = 0;
}

void rd_add_header(rd_t *rd, const char *text, const char *format)
{
    uint16_t col = rd->header_no;
    if (MAX_COL == col) {
        fprintf(stderr, "Max column exceeded\n");
        return;
    }
    rd->headers[col] = strdup(text);
    char tmp[STR_BUF];
    sprintf(tmp, "%%%c%ss\t", format[0] == '<' ? '-' : '+', format + 1);
    rd->formats[col] = strdup(tmp);
    rd->sizes[col] = atoi(format + 1);
    rd->header_no++;
}

void rd_add_item(rd_t *rd, const char *item)
{
    uint16_t item_no = rd->item_no;
    if (MAX_COL * MAX_ROW == item_no) {
        fprintf(stderr, "Max number of items exceeded\n");
        return;
    }
    rd->items[item_no] = strdup(item);
    rd->item_no++;
}

int rd_print(rd_t *rd)
{
    int status = 0;
    if (0 != rd->item_no % rd->header_no) {
        fprintf(stderr, "Number of items does not compatible with number of headers\n");
        goto done;
    }
    int i, j;
    //Print headers
    for (i = 0; i < rd->header_no; i++) {
        printf(rd->formats[i], rd->headers[i]);
    }
    printf("\n");
    for (i = 0; i < rd->header_no; i++) {
        for (j = 0; j < rd->sizes[i]; j++)
            printf("%c", '-');
        printf("\t");
    }
    printf("\n");
    //Print items
    j = 0;
    for (i = 0; i < rd->item_no; i++) {
        printf(rd->formats[j], rd->items[i]);
        j++;
        if (0 == (i+1) % rd->header_no) {
            j = 0;
            printf("\n");
        }
    }
done:
    return status;
}

void rd_close(rd_t *rd)
{
    int i;
    for (i = 0; i < rd->header_no; i++) {
        free(rd->headers[i]);
        free(rd->formats[i]);
    }
    for (i = 0; i < rd->item_no; i++) {
        free(rd->items[i]);
    }
    free(rd);
}
