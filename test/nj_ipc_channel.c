#include "../src/ninjaipc.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_channel_create_open() {
    nj_ipc_channel ch1 = nj_ipc_channel_create("test_channel", 1024);
    assert(ch1.status == SUCCESS);

    nj_ipc_channel ch2 = nj_ipc_channel_open("test_channel", 1024);
    assert(ch2.status == SUCCESS);

    printf("Test for create and open IPC channels passed.\n");

    nj_ipc_channel_free(&ch1);
    nj_ipc_channel_free(&ch2);
}

void test_channel_write_read() {
    char buffer[1024];
    const char *message = "Hello, IPC!";

    nj_ipc_channel ch1 = nj_ipc_channel_create("test_channel", 1024);
    assert(ch1.status == SUCCESS);

    nj_ipc_error err = nj_ipc_channel_write(&ch1, (void*)message, strlen(message) + 1);
    assert(err == SUCCESS);

    nj_ipc_channel ch2 = nj_ipc_channel_open("test_channel", 1024);
    assert(ch2.status == SUCCESS);

    err = nj_ipc_channel_read(&ch2, buffer, strlen(message) + 1);
    assert(err == SUCCESS);

    assert(strcmp(buffer, message) == 0);

    printf("Test for write and read IPC channels passed.\n");

    nj_ipc_channel_free(&ch1);
    nj_ipc_channel_free(&ch2);
}

void test_channel_wait_notify() {
    nj_ipc_channel ch1 = nj_ipc_channel_create("test_channel", 1024);
    assert(ch1.status == SUCCESS);

    nj_ipc_error err = nj_ipc_channel_notify_server(&ch1);
    assert(err == SUCCESS);

    nj_ipc_channel ch2 = nj_ipc_channel_open("test_channel", 1024);
    assert(ch2.status == SUCCESS);

    err = nj_ipc_channel_wait_server(&ch2);
    assert(err == SUCCESS);

    printf("Test for wait and notify IPC channels passed.\n");

    nj_ipc_channel_free(&ch1);
    nj_ipc_channel_free(&ch2);
}

int main() {
    test_channel_create_open();
    test_channel_write_read();
    test_channel_wait_notify();
    printf("All High-Level IPC API tests passed!\n");
    return 0;
}
