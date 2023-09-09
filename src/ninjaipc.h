/**
 * This file is part of the "ninjaipc" project.
 *
 * Copyright (c) 2023, ninjaipc authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _NINJAIPC_H
#define _NINJAIPC_H

#ifdef __linux__ 
    #define LINUX
#elif _WIN32
    #define WINDOWS
    #include <Windows.h>
#else

#endif

#include <stdbool.h>
#include <stdlib.h>
#include <memory.h>

/* Shared error codes */
typedef enum {
    SUCCESS,
    ERR,
    INVALID_NAME,

    SYNC_CREATE_FAIL,
    SYNC_ALREADY_EXISTS_FAIL,
    SYNC_OPEN_FAIL,
    SYNC_INVALID_OBJECT,
    SYNC_NOTIFY_FAILED,
    SYNC_WAIT_FAILED,

    SHMEM_INVALID_SIZE,
    SHMEM_CREATE_FAIL,
    SHMEM_ALREADY_EXISTS_FAIL,
    SHMEM_MAPPING_FAIL,
    SHMEM_OPEN_FAIL,
} nj_ipc_error;

/* String Utils */
#define nj_ipc_str_make(len) (char*) malloc(len + 1)
#define nj_ipc_str_copy(str) _strdup(str)
#define nj_ipc_str_invalid(str) (str == NULL || strcmp(str, "") == 0)

/* Synchronization API */
typedef struct nj_ipc_sync {
    void *handle;
    char *name;
    nj_ipc_error status;
} nj_ipc_sync;

/**
 * Create a new IPC synchronization object.
 *
 * @param name The name of the synchronization object.
 * @return A new nj_ipc_sync object.
 */
nj_ipc_sync
nj_ipc_sync_create(const char *name) {
    nj_ipc_sync object;
    object.status = ERR;

    if (nj_ipc_str_invalid(name)) {
        object.status = INVALID_NAME;
        return object;
    }

#ifdef WINDOWS
    object.handle = CreateEventA(NULL, FALSE, FALSE, name);

    if (!object.handle) {
        object.status = SYNC_CREATE_FAIL;
        return object;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        object.status = SYNC_ALREADY_EXISTS_FAIL;
        return object;
    }

    object.name = nj_ipc_str_copy(name);
    object.status = SUCCESS;

    return object;
#endif
    return object;
}

/**
 * Opens an existing IPC synchronization object.
 *
 * @param name The name of the synchronization object.
 * @return The open nj_ipc_sync object.
 */
nj_ipc_sync 
nj_ipc_sync_open(const char *name) {
    nj_ipc_sync object;
    object.status = ERR;

    if (nj_ipc_str_invalid(name)) {
        object.status = INVALID_NAME;
        return object;
    }

#ifdef WINDOWS
    object.handle = OpenEventA(EVENT_ALL_ACCESS, FALSE, name);

    if (!object.handle) {
        object.status = SYNC_OPEN_FAIL;
        return object;
    }

    object.name = nj_ipc_str_copy(name);
    object.status = SUCCESS;

    return object;
#endif
    return object;
}

/**
 * Opens an existing IPC synchronization object.
 *
 * @param sync The synchronization object to notify.
 * @return Notification status
 */
nj_ipc_error
nj_ipc_sync_notify(nj_ipc_sync *sync) {
    if (!sync || !sync->handle) {
        return SYNC_INVALID_OBJECT;
    }
#ifdef WINDOWS
    return SetEvent(sync->handle) ? SUCCESS :  SYNC_NOTIFY_FAILED;
#endif
}

/**
 * Waits for an synchronization object to be notified.
 *
 * @param sync The synchronization object to wait for notification.
 * @return Wait status
 */
nj_ipc_error
nj_ipc_sync_wait(nj_ipc_sync *sync) {
    if (!sync || !sync->handle) {
        return SYNC_INVALID_OBJECT;
    }
#ifdef WINDOWS
    DWORD waitcode = WaitForSingleObject(sync->handle, INFINITE);

    switch (waitcode) { /* @todo: Should we have platform specific errors? */
        case WAIT_ABANDONED:
            return SYNC_WAIT_FAILED;
        case WAIT_FAILED:
            return SYNC_WAIT_FAILED;
        case WAIT_OBJECT_0:
            return SUCCESS;
        default:
            return ERR;
    }
#endif
}

/**
 * Frees a synchronization object
 *
 * @param sync The synchronization object to be freed.
 * @return Nothing
 */
void
nj_ipc_sync_free(nj_ipc_sync *sync) {
    if (!sync || !sync->handle) {
        return;
    }
#ifdef WINDOWS
    CloseHandle(sync->handle);
#endif
    free(sync->name);
}

/* Shared Memory API */
typedef struct nj_ipc_shmem {
    void *handle;
    void *view;
    unsigned int view_size;
    nj_ipc_error status;
    char *name;
} nj_ipc_shmem;

/**
 * Create a new Shared memory object.
 *
 * @param name The name of the shared memory object.
 * @return A new nj_ipc_shmem object.
 */
nj_ipc_shmem
nj_ipc_shmem_create(const char *name, unsigned int shmem_size) {
    nj_ipc_shmem object;
    object.status = ERR;

    if (nj_ipc_str_invalid(name)) {
        object.status = INVALID_NAME;
        return object;
    }

    if (!shmem_size) {
        object.status = SHMEM_INVALID_SIZE;
        return object;
    }

#ifdef WINDOWS
    object.handle = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, shmem_size, name);

    if (!object.handle) {
        object.status = SHMEM_CREATE_FAIL;
        return object;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        object.status = SHMEM_ALREADY_EXISTS_FAIL;
        return object;
    }

    object.view = MapViewOfFile(object.handle, FILE_MAP_ALL_ACCESS, 0, 0, shmem_size);

    if (!object.view) {
        CloseHandle(object.handle);
        object.status = SHMEM_MAPPING_FAIL;
        return object;
    }

    object.name = nj_ipc_str_copy(name);
    object.status = SUCCESS;

    return object;
#endif
    return object;
}

/**
 * Opens an existing Shared memory object.
 *
 * @param name The name of the shared memory object to be open.
 * @return A new nj_ipc_shmem object.
 */
nj_ipc_shmem
nj_ipc_shmem_open(const char *name, unsigned int shmem_size) {
    nj_ipc_shmem object;
    object.status = ERR;

    if (nj_ipc_str_invalid(name)) {
        object.status = INVALID_NAME;
        return object;
    }

    if (!shmem_size) {
        object.status = SHMEM_INVALID_SIZE;
        return object;
    }

#ifdef WINDOWS
    object.handle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, name);

    if (!object.handle) {
        object.status = SHMEM_OPEN_FAIL;
        return object;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        object.status = SHMEM_ALREADY_EXISTS_FAIL;
        return object;
    }

    object.view = MapViewOfFile(object.handle, FILE_MAP_ALL_ACCESS, 0, 0, shmem_size);

    if (!object.view) {
        CloseHandle(object.handle);
        object.status = SHMEM_MAPPING_FAIL;
        return object;
    }

    object.name = nj_ipc_str_copy(name);
    object.status = SUCCESS;

    return object;
#endif
    return object;
}

/**
 * Frees a shared memory object
 *
 * @param sync The shared memory object to be freed.
 * @return Nothing
 */
void
nj_ipc_shmem_free(nj_ipc_shmem *shmem) {
    if (!shmem) {
        return;
    }
#ifdef WINDOWS
    if (shmem->handle) CloseHandle(shmem->handle);
    if (shmem->view) UnmapViewOfFile(shmem->view);
#endif
    free(shmem->name);
}

#endif
