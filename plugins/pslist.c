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
#include "renderer.h"

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
    if (VMI_FAILURE == vmi_init_complete(&vmi, name, VMI_INIT_DOMAINNAME, NULL, VMI_CONFIG_GLOBAL_FILE_ENTRY, NULL, NULL)) {
        writelog(LV_ERROR, "Failed to init LibVMI library");
        return;
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
    rd_t *rd = rd_init();
    rd_add_header(rd, "PID", ">5");
    rd_add_header(rd, "Name", "<20");
    rd_add_header(rd, "PPID", ">5");
    rd_add_header(rd, "Thds", ">5");
    rd_add_header(rd, "Hds", ">5");
    rd_add_header(rd, "Wow64", ">10");
    rd_add_header(rd, "CreateTime", ">20");
    rd_add_header(rd, "ExitTime", ">20");
    /* Walk the task list */
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
        char buf[1024];
        sprintf(buf, "%d", pid);
        rd_add_item(rd, buf);
        rd_add_item(rd, procname);
        sprintf(buf, "%d", ppid);
        rd_add_item(rd, buf);
        sprintf(buf, "%d", thds);
        rd_add_item(rd, buf);
        sprintf(buf, "%d", hds);
        rd_add_item(rd, buf);
        sprintf(buf, "%lx", wow64);
        rd_add_item(rd, buf);
        rd_add_item(rd, create_time_str);
        rd_add_item(rd, exit_time_str);

    } while (1);

    rd_print(rd);
    rd_close(rd);
    vmi_resume_vm(vmi);

done:
    vmi_destroy(vmi);
}
