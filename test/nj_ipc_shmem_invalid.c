#include "../src/ninjaipc.h"
#include <assert.h>
#include <stdio.h>

void test_shmem_create_with_invalid_name() {
    nj_ipc_shmem shmem = nj_ipc_shmem_create(NULL, 1024);
    assert(shmem.status == INVALID_NAME);
    printf("Test for creating shmem with NULL name passed.\n");

    shmem = nj_ipc_shmem_create("", 1024);
    assert(shmem.status == INVALID_NAME);
    printf("Test for creating shmem with empty string name passed.\n");
}

void test_shmem_create_with_invalid_size() {
    nj_ipc_shmem shmem = nj_ipc_shmem_create("shmem1", 0);
    assert(shmem.status == SHMEM_INVALID_SIZE);
    printf("Test for creating shmem with zero size passed.\n");
}

void test_shmem_open_with_invalid_name() {
    nj_ipc_shmem shmem = nj_ipc_shmem_open(NULL, 1024);
    assert(shmem.status == INVALID_NAME);
    printf("Test for opening shmem with NULL name passed.\n");

    shmem = nj_ipc_shmem_open("", 1024);
    assert(shmem.status == INVALID_NAME);
    printf("Test for opening shmem with empty string name passed.\n");
}

void test_shmem_open_with_invalid_size() {
    nj_ipc_shmem shmem = nj_ipc_shmem_open("shmem1", 0);
    assert(shmem.status == SHMEM_INVALID_SIZE);
    printf("Test for opening shmem with zero size passed.\n");
}

int main() {
    test_shmem_create_with_invalid_name();
    test_shmem_create_with_invalid_size();
    test_shmem_open_with_invalid_name();
    test_shmem_open_with_invalid_size();
    printf("All shmem tests passed!\n");
    return 0;
}
