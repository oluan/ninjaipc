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

#include "../ninjaerr.h"
#include "../ninjasync.h"
#include <Windows.h>

nj_bool __generic_wfso(void *obj, unsigned int timeout) {
  if (NULL == obj) {
    return nj_false;
  }

  DWORD wait_code = WaitForSingleObject(obj, timeout);

  /* Timed out */
  if (timeout > 0 && wait_code == WAIT_TIMEOUT) {
    return nj_false;
  }

  /* Owner thread did not release event */
  if (wait_code == WAIT_ABANDONED) {
    return nj_false;
  }

  /* WaitForSingleObject Failed */
  if (wait_code == WAIT_FAILED) {
    return nj_false;
  }

  return wait_code == WAIT_OBJECT_0 ? nj_true : nj_false;
}

ninjasync nj_create_sync_obj(const char *object_name) {
  ninjasync sync_obj;

  sync_obj.status = nj_false;

  /* If object_name is invalid or empty */
  if (NULL == object_name || strcmp(object_name, "") == 0)
    return sync_obj;

  /* Creates semaphore the returned value is its address */
  sync_obj.obj_handle = CreateEventA(NULL, nj_false, nj_false, object_name);

  /* CreateEventA Failed */
  if (NULL == sync_obj.obj_handle) {
    return sync_obj;
  }

  /* Event already exists */
  if (GetLastError() == ERROR_ALREADY_EXISTS) {
    CloseHandle(sync_obj.obj_handle);
    return sync_obj;
  }

  /* Else it has been created successfully */
  sync_obj.status = nj_true;

  return sync_obj;
}

ninjasync nj_open_sync_obj(const char *object_name) {
  ninjasync sync_obj;

  sync_obj.status = nj_false;

  /* If object_name is invalid or empty */
  if (NULL == object_name || strcmp(object_name, "") == 0)
    return sync_obj;

  sync_obj.obj_handle = OpenEventA(EVENT_ALL_ACCESS, nj_false, object_name);

  if (NULL == sync_obj.obj_handle) {
    return sync_obj;
  }

  sync_obj.status = nj_true;

  return sync_obj;
}

nj_bool nj_notify_sync_obj(ninjasync *sync_obj) {
  if (NULL == sync_obj) {
    return nj_false;
  }

  return SetEvent(sync_obj->obj_handle);
}

nj_bool nj_wait_notify_sync_obj(ninjasync *sync_obj) {
  if (NULL == sync_obj) {
    return nj_false;
  }

  return __generic_wfso(sync_obj->obj_handle, INFINITE);
}

nj_bool nj_wait_notify_sync_obj_timed(ninjasync *sync_obj,
                                      unsigned int timeout) {
  if (NULL == sync_obj) {
    return nj_false;
  }

  if (timeout < 0) {
    return nj_false;
  }

  return __generic_wfso(sync_obj->obj_handle, timeout);
}
