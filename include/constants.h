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
addr_t HANDLE_TABLE_TableCode;

addr_t HANDLE_TABLE_ENTRY_SIZE;
addr_t HANDLE_TABLE_ENTRY_Object;

addr_t OBJECT_HEADER_HandleCount;
addr_t OBJECT_HEADER_PointerCount;
addr_t OBJECT_HEADER_TypeIndex;
addr_t OBJECT_HEADER_Body;

addr_t OBJECT_HEADER_NAME_INFO_Name;

addr_t OBJECT_DIRECTORY_HashBuckets;
addr_t OBJECT_DIRECTORY_Lock;

addr_t OBJECT_DIRECTORY_ENTRY_Object;
addr_t OBJECT_DIRECTORY_ENTRY_ChainLink;

addr_t OBJECT_SYMBOLIC_LINK_LinkTarget;
addr_t OBJECT_SYMBOLIC_LINK_DosDeviceDriveIndex;

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
    rekall_lookup(rekall_profile, "_HANDLE_TABLE", "TableCode", &HANDLE_TABLE_TableCode, NULL);

    rekall_lookup(rekall_profile, "_HANDLE_TABLE_ENTRY", NULL, NULL, &HANDLE_TABLE_ENTRY_SIZE);
    rekall_lookup(rekall_profile, "_HANDLE_TABLE_ENTRY", "Info", &HANDLE_TABLE_ENTRY_Object, NULL);

    rekall_lookup(rekall_profile, "_OBJECT_HEADER", "HandleCount", &OBJECT_HEADER_HandleCount, NULL);
    rekall_lookup(rekall_profile, "_OBJECT_HEADER", "PointerCount", &OBJECT_HEADER_PointerCount, NULL);
    rekall_lookup(rekall_profile, "_OBJECT_HEADER", "TypeIndex", &OBJECT_HEADER_TypeIndex, NULL);
    rekall_lookup(rekall_profile, "_OBJECT_HEADER", "Body", &OBJECT_HEADER_Body, NULL);

    rekall_lookup(rekall_profile, "_OBJECT_HEADER_NAME_INFO", "Name", &OBJECT_HEADER_NAME_INFO_Name, NULL);

    rekall_lookup(rekall_profile, "_OBJECT_DIRECTORY", "HashBuckets", &OBJECT_DIRECTORY_HashBuckets, NULL);
    rekall_lookup(rekall_profile, "_OBJECT_DIRECTORY", "Lock", &OBJECT_DIRECTORY_Lock, NULL);

    rekall_lookup(rekall_profile, "_OBJECT_DIRECTORY_ENTRY", "Object", &OBJECT_DIRECTORY_ENTRY_Object, NULL);
    rekall_lookup(rekall_profile, "_OBJECT_DIRECTORY_ENTRY", "ChainLink", &OBJECT_DIRECTORY_ENTRY_ChainLink, NULL);

    rekall_lookup(rekall_profile, "_OBJECT_SYMBOLIC_LINK", "LinkTarget", &OBJECT_SYMBOLIC_LINK_LinkTarget, NULL);
    rekall_lookup(rekall_profile, "_OBJECT_SYMBOLIC_LINK", "DosDeviceDriveIndex", &OBJECT_SYMBOLIC_LINK_DosDeviceDriveIndex, NULL);
}

#endif
