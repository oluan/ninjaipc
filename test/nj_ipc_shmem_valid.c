#include "../src/ninjaipc.h"
#include <assert.h>
#include <stdio.h>

void test_shmem_create_and_free_valid() {
    nj_ipc_shmem shmem = nj_ipc_shmem_create("validShmem", 1024);
    assert(shmem.status == SUCCESS);
    printf("Test for creating shmem with valid parameters passed.\n");

    nj_ipc_shmem_free(&shmem);
    printf("Test for freeing valid shmem passed.\n");
}

void test_shmem_open_and_free_valid() {
    // First create a shared memory object to open
    nj_ipc_shmem shmem_create = nj_ipc_shmem_create("validShmemOpen", 1024);
    assert(shmem_create.status == SUCCESS);

    nj_ipc_shmem shmem = nj_ipc_shmem_open("validShmemOpen", 1024);
    assert(shmem.status == SUCCESS);
    printf("Test for opening shmem with valid parameters passed.\n");

    nj_ipc_shmem_free(&shmem);
    nj_ipc_shmem_free(&shmem_create);
    printf("Test for freeing valid opened shmem passed.\n");
}

int main() {
    test_shmem_create_and_free_valid();
    test_shmem_open_and_free_valid();

    printf("All valid shmem tests passed!\n");
    return 0;
}
