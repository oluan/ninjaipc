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

#ifndef NINJALL_H
#define NINJALL_H
#include "ninjaerr.h"

typedef void (*callback)(const char *);

typedef struct llnode {
  callback func;
  struct llnode *next;
} ninjall_node;

ninjall_node *ll_init(void);
nj_bool ll_register_callback(ninjall_node *head, callback ptr);
nj_bool ll_unregister_callback(ninjall_node *head, callback ptr);
nj_bool ll_notify_all_callbacks(ninjall_node *head, const char *content);
#endif
