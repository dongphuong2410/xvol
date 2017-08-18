#ifndef __XVOL_CONSTANTS_H__
#define __XVOL_CONSTANTS_H__

#include "rekall.h"

addr_t EPROCESS_ActiveProcessLinks;
addr_t EPROCESS_ImageFileName;
addr_t EPROCESS_UniqueProcessId;
addr_t EPROCESS_ActiveThreads;
addr_t EPROCESS_InheritedFromUniqueProcessId;
addr_t EPROCESS_ObjectTable;
addr_t EPROCESS_Wow64Process;
addr_t EPROCESS_CreateTime;
addr_t EPROCESS_ExitTime;

addr_t HANDLE_TABLE_HandleCount;

static int constants_init(const char *rekall_profile)
{
    rekall_lookup(rekall_profile, "_EPROCESS", "ActiveProcessLinks", &EPROCESS_ActiveProcessLinks, NULL);
    rekall_lookup(rekall_profile, "_EPROCESS", "ImageFileName", &EPROCESS_ImageFileName, NULL);
    rekall_lookup(rekall_profile, "_EPROCESS", "UniqueProcessId", &EPROCESS_UniqueProcessId, NULL);
    rekall_lookup(rekall_profile, "_EPROCESS", "ActiveThreads", &EPROCESS_ActiveThreads, NULL);
    rekall_lookup(rekall_profile, "_EPROCESS", "InheritedFromUniqueProcessId", &EPROCESS_InheritedFromUniqueProcessId, NULL);
    rekall_lookup(rekall_profile, "_EPROCESS", "ObjectTable", &EPROCESS_ObjectTable, NULL);
    rekall_lookup(rekall_profile, "_EPROCESS", "Wow64Process", &EPROCESS_Wow64Process, NULL);
    rekall_lookup(rekall_profile, "_EPROCESS", "CreateTime", &EPROCESS_CreateTime, NULL);
    rekall_lookup(rekall_profile, "_EPROCESS", "ExitTime", &EPROCESS_ExitTime, NULL);

    rekall_lookup(rekall_profile, "_HANDLE_TABLE", "HandleCount", &HANDLE_TABLE_HandleCount, NULL);
}

#endif
