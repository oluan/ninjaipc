/**
 * This file is part of the "ninjaipc" project.
 *
 * Copyright (c) 2022
 *
 * Luan Devecchi <luan@engineer.com>
 * Julimar Melo <melobrdev@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ninjall.h"
#include "ninjaerr.h"
#include <stdlib.h>

ninjall_node *ll_init() { return malloc(sizeof(ninjall_node)); }

nj_bool ll_register_callback(ninjall_node *head, callback ptr) {
  if (NULL == head) {
    return nj_false;
  }

  ninjall_node *new_node = malloc(sizeof(*head));

  if (NULL == new_node) {
    return nj_false;
  }

  new_node->func = ptr;

  ninjall_node *last = head;

  while (last->next) {
    last = last->next;
  }

  last->next = new_node;

  return nj_true;
}

nj_bool ll_notify_all_callbacks(ninjall_node *head, const char *content) {
  if (NULL == head) {
    return nj_false;
  }

  ninjall_node *current = NULL;

  /* first node should be reserved for implementation */

  for (current = head; current; current = current->next) {
    if (current->func) {
      current->func(content);
    }
  }

  return nj_true;
}
