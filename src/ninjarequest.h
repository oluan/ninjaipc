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

#ifndef NINJAREQUEST_H
#define NINJAREQUEST_H

#include "ninjahandle.h"

nj_bool nj_send_request(ninjahandle *handle, void *buffer, unsigned int buffer_size);

nj_bool nj_ack(ninjahandle *handle);

nj_bool nj_send_response(ninjahandle *handle, void *buffer, unsigned int buffer_size);

#endif
