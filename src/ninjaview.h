/**
 * This file is part of the "ninjaipc" project.
 *
 * Copyright (c) 2022
 *
 * Luan Devecchi <luan@engineer.com>
 * Julimar Melo <melobrdev@gmail.com>
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

#include "ninjaerr.h"

typedef struct ninjaview {
  void *view_buffer;
  unsigned int view_size;
} ninjaview;

nj_bool nj_write_to_view(ninjaview *view_obj, void *blob,
                         unsigned int blob_size);
