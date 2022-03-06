#include "../src/ninjall.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void fn(const char *buf) { printf("[RECEIVED] %s\n", buf); }
void fn2(const char *buf) { printf("[RECEIVED2] %s\n", buf); }
void fn3(const char *buf) { printf("[RECEIVED3] %s\n", buf); }

int main() {
  ninjall_node *head = ll_init();
  memset(head, 0, sizeof(*head));

  ll_register_callback(head, fn);
  ll_register_callback(head, fn2);
  ll_register_callback(head, fn3);

  ll_notify_all_callbacks(head, ":)");
}
