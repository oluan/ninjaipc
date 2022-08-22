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

#include <stdio.h>

#include "../ninjalisten.h"
#include "../ninjall.h"
#include "../ninjasync.h"
#include "../ninjaview.h"
#ifndef _WIN32
#include <semaphore.h>
#include <errno.h>
#endif

nj_bool __listen(ninjahandle *handle) {
  nj_bool sync_status;

  sync_status = nj_wait_notify_sync_obj(&handle->server_sync_obj);

  if (sync_status == nj_false) {
#ifndef _WIN32
    /* close semaphore so we don't leak */
    if (errno == EINTR) sem_close(handle->server_sync_obj.obj_handle);
#endif
    return sync_status;
  }

  ll_notify_all_callbacks(handle->callbacks,
                          (char *)handle->view_obj.view_buffer);
  
  return nj_true;
}

nj_bool nj_listen_until(ninjahandle *handle, nj_bool *ptermination_flag) {
  if (NULL == handle) {
    return nj_false;
  }

  for (;;) {
    if (ptermination_flag != (nj_bool *)-1 && *ptermination_flag) {
      break;
    }
    if (!__listen(handle)) {
      break;
    }
  }

  return ptermination_flag == (nj_bool *)-1 ? nj_false : nj_true;
}

nj_bool nj_listen(ninjahandle *handle) {
  return nj_listen_until(handle, (nj_bool *)-1);
}