#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include "vmi_helper.h"
#include "log.h"


vmi_instance_t vh_init(char *name, char *rekall)
{
    vmi_instance_t vmi = NULL;
    if (VMI_FAILURE == vmi_init(&vmi, VMI_XEN, name, VMI_INIT_DOMAINNAME | VMI_INIT_EVENTS, NULL, NULL)) {
        writelog(LV_ERROR, "Failed to init LibVMI library");
        goto done;
    }
    GHashTable *vmicfg = g_hash_table_new(g_str_hash, g_str_equal);
    g_hash_table_insert(vmicfg, "rekall_profile", rekall);
    g_hash_table_insert(vmicfg, "os_type", "Windows");
    uint64_t flags = VMI_PM_INITFLAG_TRANSITION_PAGES;
    if (VMI_PM_UNKNOWN == vmi_init_paging(vmi, flags)) {
        g_hash_table_destroy(vmicfg);
        writelog(LV_ERROR, "Failed to init LibVMI paging on domain %s", name);
        goto error;
    }
    os_t os = vmi_init_os(vmi, VMI_CONFIG_GHASHTABLE, vmicfg, NULL);
    if (os != VMI_OS_WINDOWS) {
        g_hash_table_destroy(vmicfg);
        writelog(LV_ERROR, "Failed to init LibVMI library on domain %s", name);
        goto error;
    }
    g_hash_table_destroy(vmicfg);
    goto done;

error:
    vmi_destroy(vmi);
done:
    return vmi;
}
