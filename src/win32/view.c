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

#include "../ninjaerr.h"
#include "../ninjaview.h"
#include <memory.h>
#include <stdlib.h>

nj_bool nj_write_to_view(ninjaview *view_obj, void *blob,
                         unsigned int blob_size) {
  // Set the entire buffer to zero for ensure the message passing
  // clarity, for not getting any old buffer stuff
  // if that occur proceed
  if (memset(view_obj->view_buffer, '\0', view_obj->view_size)) {
    // If blob not invalid
    if (blob) {
      // Copies the blob to the view buffer and returns nj_true
      // if succesfully copied
      if (memcpy(view_obj->view_buffer, blob, blob_size)) {
        return nj_true;
      }
    }
    // Blob invalid
    return nj_false;
  }
  // Memset failed
  return nj_false;
}
