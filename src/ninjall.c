/**
 * This file is part of the "ninjaipc" project.
 *
 * Copyright (c) 2022, ninjaipc authors.
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

#include <memory.h>
#include <stdlib.h>

#include "ninjaerr.h"

ninjall_node *ll_init() {
  ninjall_node *head = (ninjall_node*) malloc(sizeof(ninjall_node));
  if (NULL == head) {
    return NULL;
  }
  memset(head, 0, sizeof(*head));
  return head;
}

nj_bool ll_register_callback(ninjall_node *head, callback ptr) {
  ninjall_node *new_node;
  ninjall_node *last;

  if (NULL == head) {
    return nj_false;
  }

  new_node = malloc(sizeof(*head));

  if (NULL == new_node) {
    return nj_false;
  }

  memset(new_node, 0, sizeof(*head));

  new_node->func = ptr;

  last = head;

  while (last->next) {
    last = last->next;
  }

  last->next = new_node;

  return nj_true;
}

nj_bool ll_notify_all_callbacks(ninjall_node *head, const char *content) {
  ninjall_node *current = NULL;

  if (NULL == head) {
    return nj_false;
  }

  /* first node should be reserved for implementation */
  for (current = head; current; current = current->next) {
    if (current->func) {
      current->func(content);
    }
  }

  return nj_true;
}

nj_bool ll_unregister_callback(ninjall_node *head, callback ptr) {
  ninjall_node *last = NULL;
  ninjall_node *current = NULL;

  /* just check the first one*/
  if(head->func == ptr) {
    last = head;
    /* skips the node */
    head = head->next;
    free(last);
    return nj_true;
  }

  for(current = head; current; current = current->next) {
    /* find the callback and compares to the callback to be removed*/
    if(current->next->func == ptr) {
      last = current->next;
      current->next = current->next->next;
      free(last);

      /* found the callback and skipped it */
      return nj_true;
    }
  }

  /* not found the callback */
  return nj_false;
}