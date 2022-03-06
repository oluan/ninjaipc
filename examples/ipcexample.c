#include "../src/ninjaerr.h"
#include "../src/ninjall.h"
// This should be removed for some abstraction in the future
// just for testing right now
#include "../src/ninjasync.h"
#include "../src/ninjaview.h"
#include <pthread.h>
#include <unistd.h>
// ====
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void fn(const char *buf) { printf("[RECEIVED] %s\n", buf); }
void fn2(const char *buf) { printf("[RECEIVED2] %s\n", buf); }
void fn3(const char *buf) { printf("[RECEIVED3] %s\n", buf); }

void unlock_psyncobj() {
  printf("Thread to unlock sync object for test created!\n");

  ninjasync psync_obj = nj_create_sync_obj("/ninjaipcsync");

  if (psync_obj.status == nj_true) {
    printf("Thread opened sync linux object\n");
  }

  if (nj_wait_notify_sync_obj(&psync_obj) == nj_true) {
    printf("Thread acquired the sync linux object\n");
  }

  printf("Waiting ten seconds for notify linux object\n");

  sleep(10);

  if (nj_notify_sync_obj(&psync_obj) == nj_true) {
    printf("Unlocked by the thread!\n");
  }
}

int test_linux() {
  ninjasync psync_obj = nj_create_sync_obj("/ninjaipcsync");

  if (psync_obj.status == nj_true) {
    printf("Creating linux sync object passed!\n");
  }

  pthread_t threads[4];

  pthread_create(&threads[0], NULL, unlock_psyncobj, NULL);

  // Ensure that thread was created
  sleep(2);
  if (nj_wait_notify_sync_obj(&psync_obj) == nj_true) {
    printf("Passed test sync linux object!\n");
  }

  return 0;
}

int main() {
  test_linux();

  // ninjall_node *head = ll_init();
  // memset(head, 0, sizeof(*head));

  // ll_register_callback(head, fn);
  // ll_register_callback(head, fn2);
  // ll_register_callback(head, fn3);

  // ll_notify_all_callbacks(head, ":)");
}
