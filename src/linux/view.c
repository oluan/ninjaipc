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

#include "../ninjahandle.h"
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

ninjaview nj_create_view(const char *view_name, unsigned int view_size) {
  char *obj_name;
  ninjaview view;
  view.status = nj_false;
  view.view_size = view_size;

  /* Invalid view name */
  if (NULL == view_name || strcmp(view_name, "") == 0) {
    return view;
  }

  /* Creates the shared memory */
  view.view_fd = (void*)((unsigned long)shm_open(view_name, O_CREAT | O_RDWR, 0644));

  if ((unsigned long)view.view_fd < 0) {
    return view;
  }

  /* Ensure the size of file descriptor */
  ftruncate(view.view_fd, view.view_size);

  /* Maps the shared memory to the proc memory */
  view.view_buffer = mmap(NULL, view.view_size, PROT_READ | PROT_WRITE,
                          MAP_SHARED, (unsigned long)view.view_fd, 0);

  /* If has failed to map */
  if (MAP_FAILED == view.view_buffer) {
    close((unsigned long)view.view_fd);
    return view;
  }

  obj_name = (char*)malloc(strlen(view_name) + 1);
  strcpy(obj_name, view_name);

  view.view_name = obj_name;

  /* Everything ocurred as expected */
  view.status = nj_true;

  /* Returns the view */
  return view;
}

ninjaview nj_open_view(const char *view_name, unsigned int view_size) {
  ninjaview view;
  view.status = nj_false;
  view.view_size = view_size;

  /* Invalid view name */
  if (NULL == view_name || strcmp(view_name, "") == 0) {
    return view;
  }

  /* Creates the shared memory */
  view.view_fd = (void*)((unsigned long)shm_open(view_name, O_RDWR, 0644));

  if ((unsigned long)view.view_fd < 0) {
    return view;
  }

  /* Maps the shared memory to the proc memory */
  view.view_buffer = mmap(NULL, view.view_size, PROT_READ | PROT_WRITE,
                          MAP_SHARED, (unsigned long)view.view_fd, 0);

  view.status = nj_true;

  return view;
}

nj_bool nj_free_view(ninjaview *pview) {
  shm_unlink(pview->view_name);
  munmap(pview->view_buffer, pview->view_size);
  close((unsigned long)pview->view_fd);
  free(pview->view_name);
}
