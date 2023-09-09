#include "../src/ninjaipc.h"
#include <assert.h>
#include <stdio.h>

void test_create_with_invalid_name() {
    nj_ipc_sync sync = nj_ipc_sync_create(NULL);
    assert(sync.status == INVALID_NAME);
    printf("Test for creating with NULL name passed.\n");

    sync = nj_ipc_sync_create("");
    assert(sync.status == INVALID_NAME);
    printf("Test for creating with empty string name passed.\n");
}

void test_open_with_invalid_name() {
    nj_ipc_sync sync = nj_ipc_sync_open(NULL);
    assert(sync.status == INVALID_NAME);
    printf("Test for opening with NULL name passed.\n");

    sync = nj_ipc_sync_open("");
    assert(sync.status == INVALID_NAME);
    printf("Test for opening with empty string name passed.\n");
}

void test_notify_with_invalid_object() {
    nj_ipc_error status = nj_ipc_sync_notify(NULL);
    assert(status == SYNC_INVALID_OBJECT);
    printf("Test for notify with NULL object passed.\n");
}

void test_wait_with_invalid_object() {
    nj_ipc_error status = nj_ipc_sync_wait(NULL);
    assert(status == SYNC_INVALID_OBJECT);
    printf("Test for wait with NULL object passed.\n");
}

int main() {
    test_create_with_invalid_name();
    test_open_with_invalid_name();
    test_notify_with_invalid_object();
    test_wait_with_invalid_object();
    return 0;
}
