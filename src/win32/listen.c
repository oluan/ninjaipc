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

  sync_status = nj_wait_notify_sync_obj(&handle->sync_obj);

  if (sync_status == nj_false) {
    return sync_status;
  }

  ll_notify_all_callbacks(handle->callbacks, (char*) handle->view_obj.view_buffer);
}

nj_bool nj_listen(ninjahandle *handle) {
  if (NULL == handle) {
    return nj_false;
  }

  for (;;) {
    if (!__listen(handle)) {
      break;
    }
  }

  /* panic */
  return nj_false;
}
