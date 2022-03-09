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

#include "../ninjahandle.h"
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

ninjaview nj_create_view(const char *view_name, unsigned int view_size) {
  ninjaview view = {.status = nj_false, .view_size = view_size};

  // Invalid view name
  if (NULL == view_name || strcmp(view_name, "") == 0) {
    return view;
  }

  // Creates the shared memory
  view.view_fd = (void *)(uintptr_t)shm_open(view_name, O_CREAT | O_RDWR, 0644);

  if (view.view_fd < 0) {
    return view;
  }

  // Ensure the size of file descriptor
  ftruncate((uintptr_t)view.view_fd, view.view_size);

  // Maps the shared memory to the proc memory
  view.view_buffer = mmap(NULL, view.view_size, PROT_READ | PROT_WRITE,
                          MAP_SHARED, (uintptr_t)view.view_fd, 0);

  // If has failed to map
  if (MAP_FAILED == view.view_buffer) {
    return view;
  }

  // Everything ocurred as expected
  view.status = nj_true;

  // Returns the view
  return view;
}
