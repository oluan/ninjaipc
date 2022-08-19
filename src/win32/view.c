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
#include "../ninjaview.h"
#include <Windows.h>
#include <memory.h>
#include <stdlib.h>

ninjaview nj_create_view(const char *view_name, unsigned int view_size) {
  ninjaview view = {.status = nj_false, .view_size = view_size};
  
  /* If object_name is invalid or empty */
  if (NULL == view_name || strcmp(view_name, "") == 0) {
    return view;
  }

  /* If view size is invalid */
  if (view_size <= 0) {
    return view;
  }

  view.view_fd = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                                    NULL, view_size, view_name);

  if (NULL == view.view_fd) {
    return view;
  }

  if (GetLastError() == ERROR_ALREADY_EXISTS) {
    CloseHandle(view.view_fd);
    return view;
  }

  view.view_buffer =
      MapViewOfFile(view.view_fd, FILE_MAP_ALL_ACCESS, NULL, NULL, view_size);

  if (NULL == view.view_buffer) {
    CloseHandle(view.view_fd);
    return view;
  }

  view.status = nj_true;

  return view;
}
