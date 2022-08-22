#include <stdio.h>

#include "../src/ninjahandle.h"
#include "../src/ninjalisten.h"
#include "../src/ninjarequest.h"

int main() {
  ninjahandle ipc_handle = nj_open_ipc("uwu_ipc", 1024);

  if (ipc_handle.status == nj_false) return -1;

  puts("Okie");

  for (;;) {
    nj_send_request(&ipc_handle, "Ping", 5);
    printf("Response: (%s)\n", (char *)ipc_handle.view_obj.view_buffer);
  }
}