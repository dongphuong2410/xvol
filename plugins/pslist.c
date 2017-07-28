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
#include "constants.h"

extern config_t *config;

#define TICKS_PER_SECOND 10000000
#define EPOCH_DIFFERENCE 11644473600LL
time_t wintime_to_unixtime(long long int input)
{
    long long int temp;
    temp = input / TICKS_PER_SECOND;
    temp = temp - EPOCH_DIFFERENCE;
    return (time_t)temp;
}

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

    if (0 == EPROCESS_ActiveProcessLinks) {
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
    printf("%5s %-20s %5s %5s %5s %10s %20s %20s\n", "PID", "Name", "PPID", "Thds", "Hds", "Wow64", "CreateTime", "ExitTime");
    printf("%5s %-20s %5s %5s %5s %10s %20s %20s\n", "---", "----", "----", "----", "---", "-----", "----------", "--------");
    do {
        char *procname = NULL;
        vmi_pid_t pid = 0;
        vmi_pid_t ppid = 0;
        uint32_t thds = 0;
        uint64_t wow64 = 0;
        uint32_t hds = 0;
        uint64_t create_time;
        uint64_t exit_time;
        addr_t object_tbl_addr = 0;
        addr_t addr;
        current_process = next_list_entry - EPROCESS_ActiveProcessLinks;
        status = vmi_read_addr_va(vmi, next_list_entry, 0, &next_list_entry);
        if (status == VMI_FAILURE) {
            writelog(LV_ERROR, "Failed to read next pointer in loop");
            goto done;
        }
        if (next_list_entry == list_head) {
            break;
        }

        vmi_read_32_va(vmi, current_process + EPROCESS_UniqueProcessId, 0, (uint32_t *)&pid);
        procname = vmi_read_str_va(vmi, current_process + EPROCESS_ImageFileName, 0);
        vmi_read_32_va(vmi, current_process + EPROCESS_InheritedFromUniqueProcessId, 0, (uint32_t *)&ppid);
        vmi_read_32_va(vmi, current_process + EPROCESS_ActiveThreads, 0, (uint32_t *)&thds);
        vmi_read_addr_va(vmi, current_process + EPROCESS_ObjectTable, 0, &object_tbl_addr);
        vmi_read_32_va(vmi, object_tbl_addr + HANDLE_TABLE_HandleCount, 0, &hds);
        vmi_read_64_va(vmi, current_process + EPROCESS_Wow64Process, 0, &wow64);
        vmi_read_64_va(vmi, current_process + EPROCESS_CreateTime, 0, &create_time);
        vmi_read_64_va(vmi, current_process + EPROCESS_ExitTime, 0, &exit_time);
        char create_time_str[20] = "";
        char exit_time_str[20] = "";

        if (create_time) {
            time_t utime = wintime_to_unixtime(create_time);
            strftime(create_time_str, 20, "%Y-%m-%d %H:%M:%S", localtime(&utime));
        }
        if (exit_time) {
            time_t utime = wintime_to_unixtime(exit_time);
            strftime(exit_time_str, 20, "%Y-%m-%d %H:%M:%S", localtime(&utime));
        }

        if (!procname) {
            writelog(LV_ERROR, "Failed to find procname");
            goto done;
        }
        printf("%5d %-20s %5d %5d %5d %10lx %20s %20s\n", pid, procname, ppid, thds, hds, wow64, create_time_str, exit_time_str);

    } while (1);

    vmi_resume_vm(vmi);

done:
    vmi_destroy(vmi);
}
