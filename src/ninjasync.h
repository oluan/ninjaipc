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

#ifndef NINJASYNC_H
#define NINJASYNC_H
#include "ninjaerr.h"

typedef struct ninjasync {
  void *obj_handle;
  nj_bool status;
} ninjasync;

ninjasync nj_create_sync_obj(const char *object_name);

ninjasync nj_open_sync_obj(const char *object_name);

nj_bool nj_notify_sync_obj(ninjasync *sync_obj);

nj_bool nj_wait_notify_sync_obj(ninjasync *sync_obj);

nj_bool nj_wait_notify_sync_obj_timed(ninjasync *sync_obj,
                                      unsigned int timeout);
#endif
