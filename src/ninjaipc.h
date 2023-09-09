/*
 * ninjaipc: A cross-platform header-only library for Inter-Process Communication (IPC)
 * 
 * Features:
 * - Synchronization API: Allows for inter-process signaling using synchronization objects.
 * - Shared Memory API: Allows for sharing memory between processes for data interchange.
 * - Callback Storage API: Provides a way to store and execute callback functions.
 * 
 * The library currently supports Windows. POSIX support is planned for future releases.
 * 
 * Please see the individual API functions for more details.
 * 
 * Naming of APIs are namespaced, the format is always: nj_ipc_${API}_${FUNCTION}
 */

#ifndef _NINJAIPC_H
#define _NINJAIPC_H

#ifdef __linux__ 
    #define LINUX
#elif _WIN32
    #define WINDOWS
    #define _CRT_SECURE_NO_WARNINGS
    #include <Windows.h>
    #undef _CRT_SECURE_NO_WARNINGS
#else

#endif

#include <stdlib.h>
#include <stdio.h>
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

    CHANNEL_WRITE_INVALID_SHMEM,
    CHANNEL_WRITE_TOO_BIG,

    CHANNEL_READ_INVALID_SHMEM,
    CHANNEL_READ_TOO_BIG,

    CHANNEL_WAIT_INVALID_EVENT,
    CHANNEL_NOTIFY_INVALID_EVENT,
} nj_ipc_error;

/* String Utils */
#define nj_ipc_str_make(len) (char*) malloc(len + 1)
#define nj_ipc_str_copy(str) _strdup(str)
#define nj_ipc_str_invalid(str) (str == NULL || strcmp(str, "") == 0)

/* Callback Storage API */
typedef void (*nj_ipc_callback_t)(void* data);

typedef struct nj_ipc_callback_node {
    nj_ipc_callback_t func;
    struct nj_ipc_callback_node* next;
} nj_ipc_callback_node;

nj_ipc_callback_node* callback_lst_head = NULL;

/**
 * Add a new callback
 *
 * @param func The name of the synchronization object.
 * @return Nothing.
 */
void
nj_ipc_callback_add(nj_ipc_callback_t func) {
    nj_ipc_callback_node* new_node = (nj_ipc_callback_node*) malloc(sizeof(*new_node));
    new_node->func = func;
    new_node->next = callback_lst_head;
    callback_lst_head = new_node;
}

/**
 * Execute all callbacks with data
 *
 * @param data Binary data to send to the callbacks.
 * @return Nothing.
 */
void
nj_ipc_callback_execute(void *data) {
    nj_ipc_callback_node* current = callback_lst_head;
    while (current != NULL) {
        current->func(data);
        current = current->next;
    }
}

/**
 * Frees the callback list
 *
 * @return Nothing
 */
void
nj_ipc_callback_free() {
    nj_ipc_callback_node* current = callback_lst_head;
    nj_ipc_callback_node* next;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    callback_lst_head = NULL;
}

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
 * @return Notification status.
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
 * @return Wait status.
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
 * @return Nothing.
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
    object.view_size = shmem_size;
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
    object.view_size = shmem_size;
    object.status = SUCCESS;

    return object;
#endif
    return object;
}

/**
 * Frees a shared memory object
 *
 * @param sync The shared memory object to be freed.
 * @return Nothing.
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

/* High-Level IPC API */
typedef struct nj_ipc_channel {
    nj_ipc_sync server_event;
    nj_ipc_sync client_event;
    nj_ipc_shmem shmem;
    nj_ipc_error status;
    char *name;
} nj_ipc_channel;

/**
 * Create a new IPC channel.
 *
 * @param name The name of the IPC channel.
 * @param shmem_size Size of the shared memory in bytes.
 * @return A new nj_ipc_channel object.
 */
nj_ipc_channel
nj_ipc_channel_create(const char *name, unsigned int shmem_size) {
    char server_event_name[256], client_event_name[256];
    nj_ipc_channel ch;
    ch.status = ERR;

    if (nj_ipc_str_invalid(name)) {
        ch.status = INVALID_NAME;
        return ch;
    }

    sprintf(server_event_name, "%s_server_njipc", name);
    sprintf(client_event_name, "%s_client_njipc", name);

    ch.server_event = nj_ipc_sync_create(server_event_name);

    if (ch.server_event.status != SUCCESS) {
        ch.status = ch.server_event.status;
        return ch;
    }

    ch.client_event = nj_ipc_sync_create(client_event_name);

    if (ch.client_event.status != SUCCESS) {
        nj_ipc_sync_free(&(ch.server_event));
        ch.status = ch.client_event.status;
        return ch;
    }

    ch.shmem = nj_ipc_shmem_create(name, shmem_size);

    if (ch.shmem.status != SUCCESS) {
        nj_ipc_sync_free(&(ch.server_event));
        nj_ipc_sync_free(&(ch.client_event));
        ch.status = ch.shmem.status;
        return ch;
    }

    ch.name = nj_ipc_str_copy(name);
    ch.status = SUCCESS;

    return ch;
}

/**
 * Open an existing IPC channel.
 *
 * @param name The name of the IPC channel.
 * @param shmem_size Size of the shared memory in bytes.
 * @return An opened nj_ipc_channel object.
 */
nj_ipc_channel
nj_ipc_channel_open(const char *name, unsigned int shmem_size) {
    char server_event_name[256], client_event_name[256];
    nj_ipc_channel ch;
    ch.status = ERR;

    if (nj_ipc_str_invalid(name)) {
        ch.status = INVALID_NAME;
        return ch;
    }

    sprintf(server_event_name, "%s_server_njipc", name);
    sprintf(client_event_name, "%s_client_njipc", name);

    ch.server_event = nj_ipc_sync_open(server_event_name);

    if (ch.server_event.status != SUCCESS) {
        ch.status = ch.server_event.status;
        return ch;
    }

    ch.client_event = nj_ipc_sync_open(client_event_name);

    if (ch.client_event.status != SUCCESS) {
        nj_ipc_sync_free(&(ch.server_event));
        ch.status = ch.client_event.status;
        return ch;
    }

    ch.shmem = nj_ipc_shmem_open(name, shmem_size);

    if (ch.shmem.status != SUCCESS) {
        nj_ipc_sync_free(&(ch.server_event));
        nj_ipc_sync_free(&(ch.client_event));
        ch.status = ch.shmem.status;
        return ch;
    }

    ch.name = nj_ipc_str_copy(name);
    ch.status = SUCCESS;

    return ch;
}

/**
 * Write data into the shared memory of the IPC channel.
 *
 * @param channel Pointer to the nj_ipc_channel object.
 * @param data The data to be written.
 * @param data_size The size of the data beign written.
 * @return The write status.
 */
nj_ipc_error
nj_ipc_channel_write(nj_ipc_channel *channel, void *data, size_t data_size) {
    if (!channel || !channel->shmem.view) {
        return CHANNEL_WRITE_INVALID_SHMEM;
    }

    if (data_size > channel->shmem.view_size) {
        return CHANNEL_WRITE_TOO_BIG;
    }

    memcpy(channel->shmem.view, data, data_size);
    return SUCCESS;
}

/**
 * Reads data into from the shared memory of the IPC channel.
 *
 * @param channel Pointer to the nj_ipc_channel object.
 * @param buffer The buffer to read into.
 * @param read_size The size of the data beign read.
 * @return The read status.
 */
nj_ipc_error
nj_ipc_channel_read(nj_ipc_channel *channel, void *buffer, size_t read_size) {
    if (!channel || !channel->shmem.view) {
        return CHANNEL_READ_INVALID_SHMEM;
    }

    if (read_size > channel->shmem.view_size) {
        return CHANNEL_READ_TOO_BIG;
    }

    memcpy(buffer, channel->shmem.view, read_size);
    return SUCCESS;
}

/**
 * Wait for a server event on the IPC channel.
 *
 * @param ch Pointer to the nj_ipc_channel object.
 * @return The wait status.
 */
nj_ipc_error
nj_ipc_channel_wait_server(nj_ipc_channel *ch) {
    if (!ch || !ch->server_event.handle) {
        return CHANNEL_WAIT_INVALID_EVENT;
    }

    return nj_ipc_sync_wait(&(ch->server_event));
}

/**
 * Wait for a client event on the IPC channel.
 *
 * @param ch Pointer to the nj_ipc_channel object.
 * @return The wait status.
 */
nj_ipc_error
nj_ipc_channel_wait_client(nj_ipc_channel *ch) {
    if (!ch || !ch->client_event.handle) {
        return CHANNEL_WAIT_INVALID_EVENT;
    }

    return nj_ipc_sync_wait(&(ch->client_event));
}

/**
 * Notify the server event on the IPC channel.
 *
 * @param ch Pointer to the nj_ipc_channel object.
 * @return The notify status.
 */
nj_ipc_error
nj_ipc_channel_notify_server(nj_ipc_channel *ch) {
    if (!ch || !ch->server_event.handle) {
        return CHANNEL_NOTIFY_INVALID_EVENT;
    }

    return nj_ipc_sync_notify(&(ch->server_event));
}

/**
 * Notify the client event on the IPC channel.
 *
 * @param ch Pointer to the nj_ipc_channel object.
 * @return The notify status.
 */
nj_ipc_error
nj_ipc_channel_notify_client(nj_ipc_channel *ch) {
    if (!ch || !ch->client_event.handle) {
        return CHANNEL_NOTIFY_INVALID_EVENT;
    }

    return nj_ipc_sync_notify(&(ch->client_event));
}

/**
 * Frees an IPC channel
 *
 * @param channel Pointer to the nj_ipc_channel object to be freed.
 * @return Nothing.
 */
void
nj_ipc_channel_free(nj_ipc_channel *ch) {
    if (!ch) {
        return;
    }
    if (ch->server_event.handle) nj_ipc_sync_free(&(ch->server_event));
    if (ch->client_event.handle) nj_ipc_sync_free(&(ch->client_event));
    if (ch->shmem.handle) nj_ipc_shmem_free(&(ch->shmem));
    if (ch->name) free(ch->name);
}

#endif
