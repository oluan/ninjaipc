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

#ifndef NINJAVIEW_H
#define NINJAVIEW_H
#include "ninjaerr.h"
#include <memory.h>
#include <stdio.h>

typedef struct ninjaview {
  void *view_fd;
  void *view_buffer;
  unsigned int view_size;
  nj_bool status;
  char *view_name;
} ninjaview;

ninjaview nj_create_view(const char *view_name, unsigned int view_size);

ninjaview nj_open_view(const char *view_name, unsigned int view_size);

nj_bool nj_free_view(ninjaview *pview);

#endif
