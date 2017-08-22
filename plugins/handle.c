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

void read_obj(vmi_instance_t vmi, addr_t addr);

extern config_t *config;
page_mode_t pm;
win_ver_t winver;
int pid;

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
    pm = vmi_get_page_mode(vmi, 0);
    winver = vmi_get_winver(vmi);

    /* Search for process from pid */
    pid = atoi(param);
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

    switch (table_levels) {
        case 0:
        {
            int i;
            uint32_t lowest_count = VMI_PS_4KB / HANDLE_TABLE_ENTRY_SIZE;
            for (i = 0; i < lowest_count; i++) {
                addr_t obj = 0;
                vmi_read_addr_va(vmi, table_base + i * HANDLE_TABLE_ENTRY_SIZE + HANDLE_TABLE_ENTRY_Object, pid, &obj);
                if (obj) {
                    read_obj(vmi, obj);
                }
            }
            break;
        }
        case 1:
        {
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
                        addr_t obj = 0;
                        vmi_read_addr_va(vmi, entry + HANDLE_TABLE_ENTRY_Object, pid, &obj);
                        if (obj) {
                            read_obj(vmi, obj);
                        }
                    }
                }
            }
            break;
        }
        case 2:
        {
            addr_t table = 0, table2 = 0;
            size_t psize = (pm == VMI_PM_IA32E ? 8 : 4);
            uint32_t low_count = VMI_PS_4KB / HANDLE_TABLE_ENTRY_SIZE;
            uint32_t mid_count = VMI_PS_4KB / psize;
            uint32_t i, j , k;
            uint32_t table_no = VMI_PS_4KB / psize;
            for (i = 0; i < table_no; i++) {
                vmi_read_addr_va(vmi, table_base + i * psize, pid, &table);
                if (table) {
                    for (j = 0; j < mid_count; j++) {
                        vmi_read_addr_va(vmi, table + j * psize, pid, &table2);
                        if (table2) {
                            for (k = 0; k < low_count; k++) {
                                addr_t entry = table2 + k * HANDLE_TABLE_ENTRY_SIZE;
                                addr_t obj = 0;
                                vmi_read_addr_va(vmi, entry + HANDLE_TABLE_ENTRY_Object, pid, &obj);
                                if (obj) {
                                    read_obj(vmi, obj);
                                }
                            }
                        }
                    }
                }
            }
            break;
        }
    }
done:
    g_slist_free_full(processes, free);
    vmi_destroy(vmi);
}

void read_obj(vmi_instance_t vmi, addr_t addr)
{
    switch (winver) {
        case VMI_OS_WINDOWS_7:
            addr &= (~EX_FAST_REF_MASK);
            break;
        case VMI_OS_WINDOWS_8:
            if (pm == VMI_PM_IA32E)
                addr = ((addr & VMI_BIT_MASK(19,63)) >> 16) | 0xFFFFE00000000000;
            else
                addr = addr & VMI_BIT_MASK(2,31);
        default:
        case VMI_OS_WINDOWS_10:
            if (pm == VMI_PM_IA32E)
                addr = ((addr & VMI_BIT_MASK(19,63)) >> 16) | 0xFFFF000000000000;
            else
                addr = addr & VMI_BIT_MASK(2,31);
    }
    uint64_t hdl_cnt = 0, ptr_cnt = 0;
    if (pm == VMI_PM_IA32E) {
        vmi_read_64_va(vmi, addr + OBJECT_HEADER_HandleCount, pid, &hdl_cnt);
        vmi_read_64_va(vmi, addr + OBJECT_HEADER_PointerCount, pid, &ptr_cnt);
    }
    else {
        vmi_read_32_va(vmi, addr + OBJECT_HEADER_HandleCount, pid, (uint32_t *)&hdl_cnt);
        vmi_read_32_va(vmi, addr + OBJECT_HEADER_PointerCount, pid, (uint32_t *)&ptr_cnt);
    }
    uint8_t object_type;
    vmi_read_8_va(vmi, addr + OBJECT_HEADER_TypeIndex, pid, &object_type);
    printf("HandleCount %lu PointerCount %lu Object Type %u\n", hdl_cnt, ptr_cnt, object_type);
}
