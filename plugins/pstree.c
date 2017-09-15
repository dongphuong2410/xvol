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

static void add_item(process_t *process, void *data);

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

        g_slist_foreach(processes, (GFunc)add_item, rd);

        rd_print(rd);
        rd_close(rd);

        g_slist_free_full(processes, free);
    }
done:
    vmi_destroy(vmi);
}

static void add_item(process_t *process, void *data) {
    char buf[1024];
    rd_t *rd = (rd_t *)data;
    sprintf(buf, "%d", process->pid);
    rd_add_item(rd, buf);
    rd_add_item(rd, process->name);
    sprintf(buf, "%d", process->ppid);
    rd_add_item(rd, buf);
    sprintf(buf, "%d", process->thds);
    rd_add_item(rd, buf);
    sprintf(buf, "%d", process->hds);
    rd_add_item(rd, buf);
    sprintf(buf, "%lx", process->wow64);
    rd_add_item(rd, buf);
    rd_add_item(rd, process->create_time);
    rd_add_item(rd, process->exit_time);
}
