#include <stdio.h>
#include <stdlib.h>
#include <libvmi/libvmi.h>
#include <glib.h>
#include <unistd.h>
#include <inttypes.h>

#include "libxvol.h"
#include "log.h"
#include "config.h"
#include "rekall.h"

extern config_t *config;

void pslist_exec(void)
{
    vmi_instance_t vmi;
    status_t status;

    char *name = config_get_str(config, "dom");
    if (VMI_FAILURE == vmi_init(&vmi, VMI_XEN, name, VMI_INIT_DOMAINNAME, NULL, NULL)) {
        writelog(LV_ERROR, "Failed to init LibVMI library");
        return;
    }
    char *rekall_profile = config_get_str(config, "rekall_profile");
    GHashTable *vmicfg = g_hash_table_new(g_str_hash, g_str_equal);
    g_hash_table_insert(vmicfg, "rekall_profile", rekall_profile);
    g_hash_table_insert(vmicfg, "os_type", "Windows");
    uint64_t flags = VMI_PM_INITFLAG_TRANSITION_PAGES;
    if (VMI_PM_UNKNOWN == vmi_init_paging(vmi, flags)) {
        writelog(LV_ERROR, "Failed to init LibVMI paging");
        goto done;
    }
    os_t os = vmi_init_os(vmi, VMI_CONFIG_GHASHTABLE, vmicfg, NULL);
    g_hash_table_destroy(vmicfg);
    if (os != VMI_OS_WINDOWS) {
        writelog(LV_ERROR, "Failed to init LibVMI library");
    }

    /* init the offset values */
    unsigned long tasks_offset = 0;
    unsigned long pid_offset = 0;
    unsigned long name_offset = 0;
    unsigned long ppid_offset = 0;
    unsigned long thds_offset = 0;
    unsigned long wow64_offset = 0;
    tasks_offset = vmi_get_offset(vmi, "win_tasks");
    rekall_lookup(rekall_profile, "_EPROCESS", "ImageFileName", &name_offset, NULL);
    rekall_lookup(rekall_profile, "_EPROCESS", "UniqueProcessId", &pid_offset, NULL);
    rekall_lookup(rekall_profile, "_EPROCESS", "InheritedFromUniqueProcessId", &ppid_offset, NULL);
    rekall_lookup(rekall_profile, "_EPROCESS", "ActiveThreads", &thds_offset, NULL);
    rekall_lookup(rekall_profile, "_EPROCESS", "Wow64Process", &wow64_offset, NULL);

    if (0 == tasks_offset) {
        writelog(LV_ERROR, "Failed to find win_tasks");
        goto done;
    }

    vmi_pause_vm(vmi);
    addr_t list_head = 0, next_list_entry = 0;
    addr_t current_process = 0;

    if (VMI_FAILURE == vmi_read_addr_ksym(vmi, "PsActiveProcessHead", &list_head)) {
        writelog(LV_ERROR, "Failed to find PsActiveProcessHead");
        goto done;
    }
    next_list_entry = list_head;
    /* Walk the task list */
    printf("%5s %-20s %5s %5s %5s\n", "PID", "Name", "PPID", "Thds", "Wow64");
    printf("%5s %-20s %5s %5s %5s\n", "---", "----", "----", "----", "-----");
    do {
        char *procname = NULL;
        vmi_pid_t pid = 0;
        vmi_pid_t ppid = 0;
        uint32_t thds = 0;
        uint32_t wow64 = 0;
        addr_t addr;
        current_process = next_list_entry - tasks_offset;
        vmi_read_32_va(vmi, current_process + pid_offset, 0, (uint32_t *)&pid);
        procname = vmi_read_str_va(vmi, current_process + name_offset, 0);
        vmi_read_32_va(vmi, current_process + ppid_offset, 0, (uint32_t *)&ppid);
        vmi_read_32_va(vmi, current_process + thds_offset, 0, (uint32_t *)&thds);
        vmi_read_32_va(vmi, current_process + wow64_offset, 0, (uint32_t *)&wow64);
        if (!procname) {
            writelog(LV_ERROR, "Failed to find procname");
            goto done;
        }
        printf("%5d %-20s %5d %5d %5d\n", pid, procname, ppid, thds, wow64);

        status = vmi_read_addr_va(vmi, next_list_entry, 0, &next_list_entry);
        if (status == VMI_FAILURE) {
            writelog(LV_ERROR, "Failed to read next pointer in loop");
            goto done;
        }
    } while (next_list_entry != list_head);

    vmi_resume_vm(vmi);

done:
    vmi_destroy(vmi);
}
