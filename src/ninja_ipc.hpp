/**
 * This file is part of the "ninjaipc" project.
 *   Copyright (c) 2020 Luan Devecchi <luan@engineer.com>
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
#include <iostream>
#include <functional>
#ifdef _WIN32
#include <Windows.h>
#define IS_WINDOWS
#endif
#ifdef __linux__
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#define IS_LINUX
#endif

namespace ninjaipc {

enum class ninjaerror : std::uint32_t {
    SHARED_MEM_CREATION_FAILED,
    SHARED_MEM_CONNECTION_FAILED,
    SHARED_MEM_ALREADY_EXISTS,
    /*
    Windows Specific
    */
    WIN_SERVER_EVENT_CREATION_FAILED,
    WIN_SERVER_EVENT_ALREADY_EXISTS,
    WIN_SERVER_EVENT_CONNECTION_FAILED,
    WIN_CLIENT_EVENT_CREATION_FAILED,
    WIN_CLIENT_EVENT_ALREADY_EXISTS,
    WIN_CLIENT_EVENT_CONNECTION_FAILED,
    WIN_SHARED_MEM_VIEW_FAILED,
    /*
    GNU/Linux Specific
    */
    LINUX_FILE_DESCRIPTOR_CREATION_FAILED,
    LINUX_FILE_DESCRIPTOR_CONNECTION_FAILED,
    LINUX_FILE_MAPPING_MAP_FAILED,
    LINUX_SERVER_SEMAPHORE_CREATION_FAILED,
    LINUX_CLIENT_SEMAPHORE_CONNECTION_FAILED,
    LINUX_SERVER_SEMAPHORE_CONNECTION_FAILED,
    LINUX_CLIENT_SEMAPHORE_CREATION_FAILED,
};

struct ninjahandle {
    std::size_t buffer_size = {};
    ninjaerror last_err = {};
    bool good = { true };
#ifdef IS_WINDOWS
    HANDLE file_mapping = {},
    client_event = {},
    server_event = {};
    void* file_view = {};
#endif
#ifdef IS_LINUX
    int file_descriptor = {};
    caddr_t file_mapping = {};
    sem_t* client_semaphore = {},
    *server_semaphore = {};
#endif
};

using t_ninja_callback = std::function<void(void* buffer)>;

/* server */
ninjahandle create_server               (const std::string& name, const std::size_t buffer_size = 1024 * 1024);
void        register_server_callback    (t_ninja_callback callback);
void        listen                      (const ninjahandle& handle);
void        listen                      (const ninjahandle& handle, t_ninja_callback callback);
void        respond                     (const ninjahandle& handle, void* buffer, std::size_t buffer_size);
void        respond_text                (const ninjahandle& handle, const char* content);

template<typename T>
void respond(const ninjahandle& handle, T buffer) {
    respond(handle, reinterpret_cast<void*>(&buffer), sizeof(T));
}

/* client */
ninjahandle connect(const std::string& name, std::size_t buffer_size = 1024 * 1024);
bool        send_request(const ninjahandle& handle, void* buffer, std::size_t buffer_size, std::int32_t timeout);

template<typename T>
bool send_request(const ninjahandle& handle, T buffer) {
    return send_request(handle, reinterpret_cast<void*>(&buffer), sizeof(T), -1);
}

} // ninjaipc