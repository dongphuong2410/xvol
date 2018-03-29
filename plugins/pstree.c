#include <stdio.h>
#include <stdlib.h>
#include <libvmi/libvmi.h>
#include <glib.h>
#include <unistd.h>
#include <inttypes.h>

#include "process.h"
#include "libxvol.h"
#include "log.h"
#include "config.h"
#include "rekall.h"
#include "constants.h"
#include "renderer.h"
#include "private.h"
#include "vmi_helper.h"

typedef struct _psnode_t {
    uint32_t id;
    uint32_t parent;
    process_t *ps;
    struct _psnode_t *next;
    struct _psnode_t *child;
} psnode_t;

/* Add item to pstree */
static void _add_node(psnode_t **node);

extern config_t *config;

void pstree_exec(void)
{
    char *name = config_get_str(config, "dom");
    char *rekall = config_get_str(config, "rekall_profile");
    vmi_instance_t vmi = vh_init(name, rekall);
    if (!vmi) return;

    GSList *processes = process_list(vmi);
    if (processes) {
        rd_t *rd = rd_init();
        rd_add_header(rd, "PID", ">5");
        rd_add_header(rd, "Name", "<20");
        rd_add_header(rd, "PPID", ">5");
        rd_add_header(rd, "Thds", ">5");
        rd_add_header(rd, "Hds", ">5");
        rd_add_header(rd, "Wow64", ">10");
        rd_add_header(rd, "CreateTime", ">20");
        rd_add_header(rd, "ExitTime", ">20");

        psnode_t *root = NULL;
        for (GSList *it = processes; it != NULL; it = it->next) {
            process_t *ps = (process_t *)(it->data);
            psnode_t *node = (psnode_t *)calloc(1, sizeof(psnode_t));
            node->id = ps->pid;
            node->parent = ps->ppid;
            node->ps = ps;

            _add_node(&root, node);
        }

        rd_print(rd);
        rd_close(rd);

        g_slist_free_full(processes, free);
    }
done:
    vmi_destroy(vmi);
}

static void _add_node(psnode_t **root, psnode_t *node) {
    if (*root) == NULL {
        *root = node;
    }
    else {
        int inserted = 0;

    }
}
