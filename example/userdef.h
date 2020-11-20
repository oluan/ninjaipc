#pragma once

/*
* IPC User-Defined Request
*/
struct ipc_packet
{
    char lib_name[255];
    int version;
};