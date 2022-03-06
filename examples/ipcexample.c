#include "../src/ninjaerr.h"
#include "../src/ninjall.h"
#include "../src/ninjasync.h"
#include "../src/ninjaview.h"
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void fn(const char *buf) { printf("[RECEIVED] %s\n", buf); }
void fn2(const char *buf) { printf("[RECEIVED2] %s\n", buf); }
void fn3(const char *buf) { printf("[RECEIVED3] %s\n", buf); }

DWORD thread(LPVOID lparam) {
	ninjasync *obj = (ninjasync*)lparam;
	printf("Received: %p\n", obj->obj_handle);
	Sleep(5000);
	printf("Notifying...\n");
	nj_notify_sync_obj(obj);
	return 0;
}

int main() {
  ninjall_node *head = ll_init();
  ll_register_callback(head, fn);
  ll_register_callback(head, fn2);
  ll_register_callback(head, fn3);
  ll_notify_all_callbacks(head, ":)");

  ninjasync obj = nj_create_sync_obj("nj_ipc_sync");

  if (obj.status) {
	  printf("Created\n");
  }

  nj_bool st = nj_wait_notify_sync_obj_timed(&obj, 2000);

  printf("Timeout ? should be 0: (%d)\n", st);

  CreateThread(0, 0, thread, (LPVOID)&obj, 0, 0);

  nj_wait_notify_sync_obj(&obj);
  
  printf("Notified!\n");

  CloseHandle(obj.obj_handle);
}
