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

#include "../ninjarequest.h"

nj_bool ___send_message(ninjahandle *handle, void *buffer,
                        unsigned int buffer_size) {
  if (handle->status == nj_false) {
    return nj_false;
  }

  if (nj_write_to_view(&handle->view_obj, buffer, buffer_size) == nj_false) {
    return nj_false;
  }

  nj_notify_sync_obj(&handle->sync_obj);

  /* nj_wait_notify_sync_obj(&handle->sync_obj); */

  return nj_true;
}

nj_bool nj_send_request(ninjahandle *handle, void *buffer, unsigned int buffer_size) {
  return ___send_message(handle, buffer, buffer_size);
}

nj_bool nj_send_response(ninjahandle *handle, void* buffer, unsigned int buffer_size) {
  return ___send_message(handle, buffer, buffer_size);
}

nj_bool nj_ack(ninjahandle *handle) {
  if (handle->status == nj_false) {
    return nj_false;
  }

  memset(handle->view_obj.view_buffer, 0, handle->view_obj.view_size);
  return nj_notify_sync_obj(&handle->sync_obj);
}
