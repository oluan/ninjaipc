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

#include "ninjaerr.h"
#include "ninjall.h"
#include "ninjasync.h"
#include "ninjaview.h"

nj_bool notify(ninjall_node *callbacks, ninjaview *view_obj,
               ninjasync *server_sync_obj, ninjasync *client_sync_obj);
