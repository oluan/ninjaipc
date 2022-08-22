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

#include <stdlib.h>

#include "../ninjahandle.h"

ninjahandle nj_create_ipc(const char *ipc_name, unsigned int ipc_size) {
  ninjahandle handle;
  char client_sync_obj_name[256] = "nj_ipc_sync_client_";
  char server_sync_obj_name[256] = "nj_ipc_sync_server_";

  handle.status = nj_false;

  handle.view_obj = nj_create_view(ipc_name, ipc_size);

  if (handle.view_obj.status == nj_false) {
    return handle;
  }

  strcat(client_sync_obj_name, ipc_name);
  strcat(server_sync_obj_name, ipc_name);

  handle.client_sync_obj = nj_create_sync_obj(client_sync_obj_name);

  if (handle.client_sync_obj.status == nj_false) {
    nj_free_view(&handle.view_obj);
    return handle;
  }

  handle.server_sync_obj = nj_create_sync_obj(server_sync_obj_name);

  if (handle.server_sync_obj.status == nj_false) {
    nj_free_sync_obj(&handle.client_sync_obj);
    nj_free_view(&handle.view_obj);
    return handle;
  }

  handle.name = (char *)malloc(strlen(ipc_name));
  strcpy(handle.name, ipc_name);

  handle.status = nj_true;

  return handle;
}

ninjahandle nj_open_ipc(const char *ipc_name, unsigned int ipc_size) {
  ninjahandle handle;
  char client_sync_obj_name[256] = "nj_ipc_sync_client_";
  char server_sync_obj_name[256] = "nj_ipc_sync_server_";

  handle.status = nj_false;

  handle.view_obj = nj_open_view(ipc_name, ipc_size);

  if (handle.view_obj.status == nj_false) {
    return handle;
  }

  strcat(client_sync_obj_name, ipc_name);
  strcat(server_sync_obj_name, ipc_name);

  handle.client_sync_obj = nj_open_sync_obj(client_sync_obj_name);

  if (handle.client_sync_obj.status == nj_false) {
    nj_free_view(&handle.view_obj);
    return handle;
  }

  handle.server_sync_obj = nj_open_sync_obj(server_sync_obj_name);

  if (handle.server_sync_obj.status == nj_false) {
    nj_free_sync_obj(&handle.client_sync_obj);
    nj_free_view(&handle.view_obj);
    return handle;
  }

  handle.name = (char *)malloc(strlen(ipc_name));

  strcpy(handle.name, ipc_name);

  handle.status = nj_true;

  return handle;
}

void nj_free_handle(ninjahandle *phandle) {
  nj_free_view(&phandle->view_obj);
  nj_free_sync_obj(&phandle->client_sync_obj);
  nj_free_sync_obj(&phandle->server_sync_obj);
  free(phandle->name);
}