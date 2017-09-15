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

void add_item(process_t *process, void *data);

extern config_t *config;

void pslist_exec(void)
{
    vmi_instance_t vmi;
    status_t status;

    char *name = config_get_str(config, "dom");
    if (VMI_FAILURE == vmi_init(&vmi, VMI_XEN, name, VMI_INIT_DOMAINNAME | VMI_INIT_EVENTS, NULL, NULL)) {
        writelog(LV_ERROR, "Failed to init LibVMI library");
        return;
    }
    char *rekall_profile = config_get_str(config, "rekall_profile");
    GHashTable *vmicfg = g_hash_table_new(g_str_hash, g_str_equal);
    g_hash_table_insert(vmicfg, "rekall_profile", rekall_profile);
    g_hash_table_insert(vmicfg, "os_type", "Windows");
    uint64_t flags = VMI_PM_INITFLAG_TRANSITION_PAGES;
    if (VMI_PM_UNKNOWN == vmi_init_paging(vmi, flags)) {
        g_hash_table_destroy(vmicfg);
        vmi_destroy(vmi);
        writelog(LV_ERROR, "Failed to init LibVMI paging on domain %s", name);
        return;
    }
    os_t os = vmi_init_os(vmi, VMI_CONFIG_GHASHTABLE, vmicfg, NULL);
    if (os != VMI_OS_WINDOWS) {
        g_hash_table_destroy(vmicfg);
        vmi_destroy(vmi);
        writelog(LV_ERROR, "Failed to init LibVMI library on domain %s", name);
        return;
    }
    g_hash_table_destroy(vmicfg);

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

void add_item(process_t *process, void *data) {
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
