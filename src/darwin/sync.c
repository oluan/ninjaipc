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

#include <fcntl.h>
#include <time.h>
#define __USE_XOPEN2K
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "sem_timedwait.h"

#include "../ninjaerr.h"
#include "../ninjasync.h"
#include "../ninjavalidators.h"

ninjasync nj_create_sync_obj(const char *object_name) {
  ninjasync sync_obj;

  sync_obj.status = nj_false;

  /* If object_name is invalid or empty */
  if (NJ_INVALID_STRING(object_name)) return sync_obj;

  /* Creates semaphore the returned value is its address */
  sync_obj.obj_handle = sem_open(object_name, O_CREAT, 0644, 0);

  /* If the semaphore was sucessfully created set the status to success */
  if (sync_obj.obj_handle != SEM_FAILED) {
    {
      char *name = (char *)malloc(strlen(object_name) + 1);
      strcpy(name, object_name);
      sync_obj.obj_name = name;
    }
    sync_obj.status = nj_true;
  }

  return sync_obj;
}

ninjasync nj_open_sync_obj(const char *object_name) {
  ninjasync sync_obj;

  sync_obj.status = nj_false;

  /* If object_name is invalid or empty */
  if (NJ_INVALID_STRING(object_name)) return sync_obj;

  sync_obj.obj_handle = sem_open(object_name, 0);

  if (SEM_FAILED == sync_obj.obj_handle) return sync_obj;

  sync_obj.status = nj_true;

  return sync_obj;
}

nj_bool nj_notify_sync_obj(ninjasync *sync_obj) {
  if (NULL == sync_obj) return nj_false;

  /* Returns nj_true if everything ocurred as expected and nj_false if doesn't.
   */
  return sem_post(sync_obj->obj_handle) == 0 ? nj_true : nj_false;
}

nj_bool nj_wait_notify_sync_obj(ninjasync *sync_obj) {
  if (NULL == sync_obj) return nj_false;

  /* Returns nj_true if everything ocurred as expected and nj_false if doesn't.
   */
  return sem_wait(sync_obj->obj_handle) == 0 ? nj_true : nj_false;
}

nj_bool nj_wait_notify_sync_obj_timed(ninjasync *sync_obj,
                                      unsigned int timeout) {
  struct timespec time;

  if (NULL == sync_obj) return nj_false;

  if (timeout < 0) return nj_false;

  /*
   * sem_timedwait uses the timespec structure that supports nanoseconds, so
   * we must convert from miliseconds to timespec format
   */
  time.tv_nsec = timeout / 1000;
  time.tv_sec = (timeout % 1000) * 1000000;

  /* Returns nj_true if everything ocurred as expected and nj_false if doesn't.
   */
  return sem_timedwait(sync_obj->obj_handle, &time) == 0 ? nj_true : nj_false;
}

void nj_free_sync_obj(ninjasync *sync_obj) {
  sem_unlink(sync_obj->obj_name);
  free(sync_obj->obj_name);
}
