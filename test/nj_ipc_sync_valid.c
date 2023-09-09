#include "../src/ninjaipc.h"
#include <assert.h>
#include <stdio.h>

#define EVENT_NAME "event1"

int main() {
    nj_ipc_sync sync1 = nj_ipc_sync_create(EVENT_NAME);
    assert(sync1.status == SUCCESS && "Failed to create sync1");

    nj_ipc_sync sync2 = nj_ipc_sync_open(EVENT_NAME);
    assert(sync2.status == SUCCESS && "Failed to open sync2");

    nj_ipc_error notify_status = nj_ipc_sync_notify(&sync1);
    assert(notify_status == SUCCESS && "Failed to notify on sync1");

    nj_ipc_error wait_status = nj_ipc_sync_wait(&sync2);
    assert(wait_status == SUCCESS && "Failed to wait on sync2");

    assert(strcmp(sync1.name, EVENT_NAME) == 0 && "Failed to store event name");
    assert(strcmp(sync2.name, EVENT_NAME) == 0 && "Failed to store event name");

    nj_ipc_sync_free(&sync1);
    nj_ipc_sync_free(&sync2);
    return 0;
}
