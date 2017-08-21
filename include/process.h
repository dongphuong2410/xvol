#ifndef __XVOL_PROCESS_H__
#define __XVOL_PROCESS_H__

#include <stdlib.h>
#include <libvmi/libvmi.h>
#include <glib.h>

#include "private.h"

typedef struct _process {
    uint32_t    pid;
    char        name[STR_BUFF];
    uint32_t    ppid;
    uint32_t    thds;
    uint32_t    hds;
    uint64_t    wow64;
    char        create_time[STR_BUFF];
    char        exit_time[STR_BUFF];
} process_t;

/**
  * return list of processes of the VM
  * @param vmi vmi instance
  * @return GList of process_t
  */
GSList *process_list(vmi_instance_t vmi);

#endif
