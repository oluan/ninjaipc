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

#define _CRT_SECURE_NO_WARNINGS /* no thanks */

#include <Windows.h>
#include <memory.h>
#include <stdlib.h>
#include <assert.h>

#include "../ninjaerr.h"
#include "../ninjaview.h"

ninjaview nj_create_view(const char *view_name, unsigned int view_size) {
  ninjaview view;
  view.status = nj_false;
  view.view_size = view_size;

  /* If object_name is invalid or empty */
  if (NULL == view_name || strcmp(view_name, "") == 0) {
    return view;
  }

  /* If view size is invalid */
  if (view_size <= 0) {
    return view;
  }

  view.view_fd = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                                    0, view_size, view_name);

  if (NULL == view.view_fd) {
    return view;
  }

  if (GetLastError() == ERROR_ALREADY_EXISTS) {
    CloseHandle(view.view_fd);
    return view;
  }

  view.view_buffer =
      MapViewOfFile(view.view_fd, FILE_MAP_ALL_ACCESS, 0, 0, view_size);

  if (NULL == view.view_buffer) {
    CloseHandle(view.view_fd);
    return view;
  }

  view.view_name = (char*) malloc(strlen(view_name) + 1);
  strcpy(view.view_name, view_name);

  view.status = nj_true;

  return view;
}

ninjaview nj_open_view(const char *view_name, unsigned int view_size) {
  ninjaview view;
  view.status = nj_false;
  view.view_size = view_size;

  /* If object_name is invalid or empty */
  if (NULL == view_name || strcmp(view_name, "") == 0) {
    return view;
  }

  /* If view size is invalid */
  if (view_size <= 0) {
    return view;
  }

  view.view_fd = OpenFileMappingA(FILE_MAP_ALL_ACCESS, nj_false, view_name);

  if (NULL == view.view_fd) {
    return view;
  }

  view.view_buffer =
      MapViewOfFile(view.view_fd, FILE_MAP_ALL_ACCESS, 0, 0, view_size);

  if (NULL == view.view_buffer) {
    CloseHandle(view.view_fd);
    return view;
  }

  view.view_name = (char*) malloc(strlen(view_name) + 1);
  strcpy(view.view_name, view_name);

  view.status = nj_true;

  return view;
}

void nj_free_view(ninjaview *view) {
  UnmapViewOfFile(view->view_buffer);
  CloseHandle(view->view_fd);
  free(view->view_name);
}
