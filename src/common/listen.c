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

#include "../ninjalisten.h"
#include "../ninjasync.h"
#include "../ninjaview.h"
#include "../ninjall.h"
#include <stdio.h>

nj_bool __listen(ninjahandle *handle) {
  nj_bool sync_status;

  printf("__listen: waiting for %p - %p\n", &handle->sync_obj, handle->sync_obj.obj_handle);

  sync_status = nj_wait_notify_sync_obj(&handle->sync_obj);

  printf("__listen: notified\n");

  if (sync_status == nj_false) {
    return sync_status;
  }

  ll_notify_all_callbacks(handle->callbacks, (char*)handle->view_obj.view_buffer);
}

nj_bool nj_listen_until(ninjahandle *handle, nj_bool *ptermination_flag) {
  if (NULL == handle) {
    return nj_false;
  }

  printf("nj_listen_until: %p %p %p\n", &handle->view_obj, &handle->sync_obj.obj_handle, handle->callbacks);

  for (;;) {
    if (ptermination_flag != (nj_bool*)-1 && *ptermination_flag) {
      break;
    }
    if (!__listen(handle)) {
      break;
    }
  }

  return ptermination_flag == (nj_bool*)-1 ? nj_false : nj_true;
}

nj_bool nj_listen(ninjahandle *handle) {
  return nj_listen_until(handle, (nj_bool*)-1);
}