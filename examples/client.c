#include "../src/ninjahandle.h"
#include "../src/ninjalisten.h"
#include "../src/ninjarequest.h"
#include <stdio.h>

int main() {
	ninjahandle ipc_handle = nj_open_ipc("uwu_ipc", 1024);
  if (ipc_handle.status == nj_false)
    return -1;
  puts("Okie");

  nj_send_request(&ipc_handle, "end", 4);
}