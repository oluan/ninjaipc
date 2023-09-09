#include "../src/ninjaipc.h"
#include <assert.h>
#include <stdio.h>

static int callback_called_count = 0;
static void* last_callback_data = NULL;

void test_callback(void* data) {
    callback_called_count++;
    last_callback_data = data;
}

void test_add_and_execute_callbacks() {
    int data = 42;
    
    // Add a callback
    nj_ipc_callback_add(test_callback);
    
    // Execute callbacks
    nj_ipc_callback_execute(&data);
    
    assert(callback_called_count == 1);
    assert(last_callback_data == &data);
    
    printf("Test for add and execute callbacks passed.\n");
}

void test_free_callbacks() {
    nj_ipc_callback_free();
    
    // Reset counters and data
    callback_called_count = 0;
    last_callback_data = NULL;
    
    // Shouldn't affect counters and data
    nj_ipc_callback_execute(NULL);
    
    assert(callback_called_count == 0);
    assert(last_callback_data == NULL);
    
    printf("Test for free callbacks passed.\n");
}

int main() {
    test_add_and_execute_callbacks();
    test_free_callbacks();

    printf("All Callback Storage API tests passed!\n");
    return 0;
}
