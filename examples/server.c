#include "../src/ninjahandle.h"
#include "../src/ninjalisten.h"
#include "../src/ninjarequest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

ninjahandle ipc_handle;
nj_bool should_end;

void data_callback(const char* data) {
	printf("data_callback: (%s)\n", data);
  if (strcmp(data, "end") == 0) {
    printf("should end!\n");
    should_end = nj_true;
  }
	nj_ack(&ipc_handle);
}

void cleanup(int sig) {
  should_end = 1;
  nj_free_handle(&ipc_handle);
  printf("after cleanup\n");
}

int create_ipc() {
	ipc_handle = nj_create_ipc("uwu_ipc", 1024);

	if (ipc_handle.status == nj_false)
		return 0;

	ninjall_node* root = ll_init();
	ll_register_callback(root, data_callback);

  ipc_handle.callbacks = root;

	puts("create_ipc: ipc created");

	return 1;
}

int main() {
	if (!create_ipc())
		return -1;

  signal(SIGINT, cleanup);

	nj_listen_until(&ipc_handle, &should_end);
}