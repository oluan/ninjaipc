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

#ifndef NINJAHANDLE_H
#define NINJAHANDLE_H
#include "ninjall.h"
#include "ninjasync.h"
#include "ninjaview.h"

typedef struct ninjahandle {
  ninjasync client_sync_obj;
  ninjasync server_sync_obj;
  ninjaview view_obj;
  ninjall_node *callbacks;
  nj_bool status;
  char *name;
} ninjahandle;

ninjahandle nj_create_ipc(const char *ipc_name, unsigned int ipc_size);

ninjahandle nj_open_ipc(const char *ipc_name, unsigned int ipc_size);

void nj_free_handle(ninjahandle *phandle);

#endif
