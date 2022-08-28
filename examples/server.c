#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/ninjahandle.h"
#include "../src/ninjalisten.h"
#include "../src/ninjarequest.h"

ninjahandle ipc_handle;

void data_callback(const char* data) {
  printf("Request: (%s)\n", data);
  nj_send_response(&ipc_handle, (void*)"Pong!", 7);
}

void cleanup(int sig) {
  static int didclean = 0;
  if (!didclean) {
    didclean = 1;
    nj_free_handle(&ipc_handle);
  }
}

int create_ipc() {
  ipc_handle = nj_create_ipc("uwu_ipc", 1024);

  if (ipc_handle.status == nj_false) return 0;

  ninjall_node* root = ll_init();
  ll_register_callback(root, data_callback);

  ipc_handle.callbacks = root;

  puts("create_ipc: ipc created");

  return 1;
}

int main() {
  if (!create_ipc()) return -1;

  signal(SIGINT, cleanup);

  nj_listen(&ipc_handle);

  cleanup(0);
}