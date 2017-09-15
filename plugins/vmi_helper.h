#ifndef __XVOL_VMI_HELPER_H
#define __XVOL_VMI_HELPER_H

#include <libvmi/libvmi.h>

/**
  * Init vmi
  * @param[in] name VM name
  * @param[in] rekall Rekall profile
  * @return vmi instance, return NULL if error
   */
vmi_instance_t vh_init(char *name, char *rekall);

#endif
