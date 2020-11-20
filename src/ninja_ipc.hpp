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
#ifndef _NINJA_IPC_HPP_
#define _NINJA_IPC_HPP_
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
    NONE,

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

using ninja_callback = std::function<void(void* buffer)>;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Creates a server.
///
/// @author luan devecchi
/// @date 11/19/2020
///
/// @param  name	    The IPC name.
/// @param  buffer_size (Optional) Size of the buffer.
///
/// @returns The created IPC handle.
////////////////////////////////////////////////////////////////////////////////////////////////////
ninjahandle create_server               (const std::string& name, const std::size_t buffer_size = 1024 * 1024);

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Registers a server callback.
///
/// @author luan devecchi
/// @date 11/19/2020
///
/// @param  callback The callback to be registered.
////////////////////////////////////////////////////////////////////////////////////////////////////
void        register_server_callback    (ninja_callback callback);

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Listens the given IPC handle.
///
/// @author luan devecchi
/// @date 11/19/2020
///
/// @param  handle The IPC handle.
////////////////////////////////////////////////////////////////////////////////////////////////////
void        listen                      (const ninjahandle& handle);

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Minified version of register_server_callback/listen.
///
/// @author luan devecchi
/// @date 11/19/2020
///
/// @param  handle   The IPC handle.
/// @param  callback The only callback to be registered.
////////////////////////////////////////////////////////////////////////////////////////////////////
void        listen                      (const ninjahandle& handle, ninja_callback callback);

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Responds.
///
/// @author luan devecchi
/// @date 11/19/2020
///
/// @param          handle	    The handle.
/// @param          buffer	    If non-null, the buffer.
/// @param          buffer_size Size of the buffer.
////////////////////////////////////////////////////////////////////////////////////////////////////
void        respond                     (const ninjahandle& handle, void* buffer, std::size_t buffer_size);

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Respond text.
///
/// @author luan devecchi
/// @date 11/19/2020
///
/// @param  handle  The IPC handle.
/// @param  content The string content.
////////////////////////////////////////////////////////////////////////////////////////////////////
void        respond_text                (const ninjahandle& handle, const char* content);

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Acknowledge request without responding
///
/// @author luan devecchi
/// @date 11/19/2020
///
/// @param  handle The IPC handle.
////////////////////////////////////////////////////////////////////////////////////////////////////
void        acknowledge_request         (const ninjahandle& handle);

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Template version of respond
/// 
/// @author luan devecchi
/// @date 11/19/2020
///
/// @typeparam T Generic type parameter.
/// @param  handle The IPC handle.
/// @param  buffer The T type buffer.
////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void respond(const ninjahandle& handle, T buffer) 
{
    respond(handle, reinterpret_cast<void*>(&buffer), sizeof(T));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Connect to an IPC server
///
/// @author luan devecchi
/// @date 11/19/2020
///
/// @param  name	    The IPC name.
/// @param  buffer_size (Optional) Size of the buffer.
///
/// @returns The IPC handle.
////////////////////////////////////////////////////////////////////////////////////////////////////
ninjahandle connect(const std::string& name, std::size_t buffer_size = 1024 * 1024);

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Sends a request.
///
/// @author luan devecchi
/// @date 11/19/2020
///
/// @param          handle	    The IPC handle.
/// @param          buffer	    If non-null, the buffer.
/// @param          buffer_size Size of the buffer.
/// @param          timeout	    The timeout threshold.
///
/// @returns True if it succeeds, false if it timeouts.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool        send_request(const ninjahandle& handle, void* buffer, std::size_t buffer_size, std::int32_t timeout);

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Template version of send_request
/// 
/// @author luan devecchi
/// @date 11/19/2020
///
/// @typeparam T Generic type parameter.
/// @param  handle The IPC handle.
/// @param  buffer The T type buffer.
///
/// @returns True if it succeeds, false if it timeouts.
////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
bool send_request(const ninjahandle& handle, T buffer) 
{
    return send_request(handle, reinterpret_cast<void*>(&buffer), sizeof(T), -1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Gets IPC buffer content.
///
/// @author luan devecchi
/// @date 11/19/2020
///
/// @typeparam T Generic type parameter.
/// @param  handle The  IPC handle.
///
/// @returns The IPC content
////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
T* get_ipc_content(const ninjahandle& handle)
{
#ifdef IS_WINDOWS
    return reinterpret_cast<T*>( handle.file_view );
#endif
#ifdef IS_LINUX
    return reinterpret_cast<T*>( handle.file_mapping );
#endif
}

} // namespace ninjaipc
#endif // _NINJA_IPC_HPP_