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

extern config_t *config;

void handle_exec(char *param)
{
    vmi_instance_t vmi;
    status_t status;

    /* Init vmi */
    char *name = config_get_str(config, "dom");
    if (VMI_FAILURE == vmi_init_complete(&vmi, name, VMI_INIT_DOMAINNAME, NULL, VMI_CONFIG_GLOBAL_FILE_ENTRY, NULL, NULL)) {
        writelog(LV_ERROR, "Failed to init LibVMI library");
        return;
    }

    /* Search for process from pid */
    int pid = atoi(param);
    GSList *processes = process_list(vmi);
    GSList *iterator = NULL;
    process_t *found = NULL;
    for (iterator = processes; iterator; iterator = iterator->next) {
        process_t *p = iterator->data;
        if (p->pid == pid) {
            found = p;
        }
    }

    /* Read handle table from the process */
    if (!found) {
        goto done;
    }
    addr_t tablecode;
    if (VMI_FAILURE == vmi_read_addr_va(vmi, found->handle_table + HANDLE_TABLE_TableCode, pid, &tablecode)) {
        writelog(LV_ERROR, "Failed to read tablecode");
        goto done;
    }
    uint32_t handlecount = found->hds;
    addr_t table_base = tablecode & ~EX_FAST_REF_MASK;
    uint32_t table_levels = tablecode & EX_FAST_REF_MASK;
    page_mode_t pm = vmi_get_page_mode(vmi, 0);

    switch (table_levels) {
        case 0:
            printf("Level 0\n");
            break;
        case 1:
            printf("Level 1\n");
            addr_t table = 0;
            size_t psize = (pm == VMI_PM_IA32E ? 8 : 4);
            uint32_t lowest_count = VMI_PS_4KB / HANDLE_TABLE_ENTRY_SIZE;
            uint32_t table_no = VMI_PS_4KB / psize;
            int i, j;
            for (i = 0; i < table_no; i++) {
                vmi_read_addr_va(vmi, table_base + i * psize, pid, &table);
                if (table) {
                    for (j = 0; j < lowest_count; j++) {
                        addr_t entry = table + j * HANDLE_TABLE_ENTRY_SIZE;
                        addr_t entry_info = 0;
                        vmi_read_addr_va(vmi, entry + HANDLE_TABLE_ENTRY_Info, pid, &entry_info);
                        if (entry_info) {
                            printf("Handle table entry info %lx\n", entry_info);
                        }
                    }
                    printf("Table address %lx\n", table);
                }
            }
            break;
        case 2:
            printf("Level 2\n");
            break;
    }
    printf("table base %lx table levels %u\n", table_base, table_levels);
done:
    g_slist_free_full(processes, free);
    vmi_destroy(vmi);
}

