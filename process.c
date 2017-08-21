#include <stdlib.h>
#include <stdio.h>

#include "process.h"
#include "constants.h"
#include "log.h"

GSList *process_list(vmi_instance_t vmi)
{
    GSList *list = NULL;
    status_t status;
    char *str;

    vmi_pause_vm(vmi);
    addr_t list_head = 0, next_list_entry = 0;
    addr_t current_process = 0;
    if (VMI_FAILURE == vmi_read_addr_ksym(vmi, "PsActiveProcessHead", &list_head)) {
        writelog(LV_ERROR, "Failed to find PsActiveProcessHead");
        goto done;
    }
    next_list_entry = list_head;
    /* Walk the task list */
    do {
        current_process = next_list_entry - EPROCESS_ActiveProcessLinks;
        status = vmi_read_addr_va(vmi, next_list_entry, 0, &next_list_entry);
        if (status == VMI_FAILURE) {
            writelog(LV_ERROR, "Failed to read next pointer in loop");
            goto done;
        }
        if (next_list_entry == list_head) {
            break;
        }
        process_t *p = (process_t *)calloc(1, sizeof(process_t));
        vmi_read_32_va(vmi, current_process + EPROCESS_UniqueProcessId, 0, (uint32_t *)&(p->pid));
        str = vmi_read_str_va(vmi, current_process + EPROCESS_ImageFileName, 0);
        strncpy(p->name, str, STR_BUFF);
        free(str);
        vmi_read_32_va(vmi, current_process + EPROCESS_InheritedFromUniqueProcessId, 0, (uint32_t *)&(p->ppid));
        vmi_read_32_va(vmi, current_process + EPROCESS_ActiveThreads, 0, (uint32_t *)&(p->thds));
        addr_t object_tbl_addr = 0;
        vmi_read_addr_va(vmi, current_process + EPROCESS_ObjectTable, 0, &object_tbl_addr);
        vmi_read_32_va(vmi, object_tbl_addr + HANDLE_TABLE_HandleCount, 0, &(p->hds));
        vmi_read_64_va(vmi, current_process + EPROCESS_Wow64Process, 0, &(p->wow64));
        uint64_t create_time, exit_time;
        vmi_read_64_va(vmi, current_process + EPROCESS_CreateTime, 0, &create_time);
        vmi_read_64_va(vmi, current_process + EPROCESS_ExitTime, 0, &exit_time);
        if (create_time) {
            time_t utime = wintime_to_unixtime(create_time);
            strftime(p->create_time, 20, "%Y-%m-%d %H:%M:%S", localtime(&utime));
        }
        if (exit_time) {
            time_t utime = wintime_to_unixtime(exit_time);
            strftime(p->exit_time, 20, "%Y-%m-%d %H:%M:%S", localtime(&utime));
        }
        list = g_slist_append(list, p);
    } while (1);
done:
    vmi_resume_vm(vmi);
    return list;
}
