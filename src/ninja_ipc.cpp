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
#include "ninja_ipc.hpp"
#include <cassert>
#include <vector>

constexpr auto SERVER_TAG = "NINJAIPC_SERVER";
constexpr auto CLIENT_TAG = "NINJAIPC_CLIENT";

namespace ninjaipc {

std::vector< t_ninja_callback > server_callbacks = {};

void write_buffer(const ninjahandle& handle, void* buffer, std::size_t buffer_size) 
{
    std::memset( handle.file_view, '\0', handle.buffer_size );
    std::memcpy( handle.file_view, buffer, buffer_size );
}

ninjahandle create_server(const std::string& name, const std::size_t buffer_size) 
{
    ninjahandle handle = {
        buffer_size
    };
#ifdef IS_WINDOWS
    handle.file_mapping = CreateFileMappingA
    (
        INVALID_HANDLE_VALUE, // pages as file
        nullptr,
        PAGE_READWRITE,
        0,
        buffer_size,
        name.c_str()
    );

    if ( !handle.file_mapping ) 
    {
        handle.good = false;
        handle.last_err = ninjaerror::SHARED_MEM_CREATION_FAILED;
        return handle;
    }

    if ( GetLastError() == ERROR_ALREADY_EXISTS ) 
    {
        CloseHandle     ( handle.file_mapping );
        handle.good = false;
        handle.last_err = ninjaerror::SHARED_MEM_ALREADY_EXISTS;
        return handle;
    }

    std::string server_event_name{ name + SERVER_TAG };

    handle.server_event = CreateEventA
    (
        nullptr,
        false,
        false,
        server_event_name.c_str()
    );

    if ( !handle.server_event ) 
    {
        CloseHandle     ( handle.file_mapping );
        handle.good = false;
        handle.last_err = ninjaerror::WIN_SERVER_EVENT_CREATION_FAILED;
        return handle;
    }

    if ( GetLastError() == ERROR_ALREADY_EXISTS ) 
    {
        CloseHandle     ( handle.server_event );
        CloseHandle     ( handle.file_mapping );
        handle.good = false;
        handle.last_err = ninjaerror::SHARED_MEM_ALREADY_EXISTS;
        return handle;
    }

    std::string client_event_name{ name + CLIENT_TAG };

    handle.client_event = CreateEventA
    (
        nullptr,
        false,
        false,
        client_event_name.c_str()
    );

    if ( !handle.client_event ) 
    {
        CloseHandle     ( handle.server_event );
        CloseHandle     ( handle.file_mapping );
        handle.good = false;
        handle.last_err = ninjaerror::WIN_CLIENT_EVENT_CREATION_FAILED;
        return handle;
    }

    if ( GetLastError() == ERROR_ALREADY_EXISTS ) 
    {
        CloseHandle     ( handle.server_event );
        CloseHandle     ( handle.client_event );
        CloseHandle     ( handle.file_mapping );
        handle.good = false;
        handle.last_err = ninjaerror::WIN_CLIENT_EVENT_ALREADY_EXISTS;
        return handle;
    }

    handle.file_view = MapViewOfFile
    (
        handle.file_mapping,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        buffer_size
    );

    if ( !handle.file_view ) 
    {
        CloseHandle     ( handle.server_event );
        CloseHandle     ( handle.client_event );
        CloseHandle     ( handle.file_mapping );
        handle.good = false;
        handle.last_err = ninjaerror::WIN_SHARED_MEM_VIEW_FAILED;
    }

    return handle;
#endif
}

void register_server_callback(t_ninja_callback callback) 
{
    server_callbacks.push_back( callback );
}

/* listener methods */
void listen(const ninjahandle& handle) 
{
    assert( !server_callbacks.empty() && "No callbacks registered" );
    assert( handle.good               && "Handle was not good" );

#ifdef IS_WINDOWS
    const DWORD wait_code = WaitForSingleObject( handle.client_event, INFINITE );

    // skip WAIT_TIMEOUT since we are waiting forever
    assert( wait_code != WAIT_ABANDONED && "Owner thread did not release event" );
    assert( wait_code != WAIT_FAILED    && "WaitForSingleObject failed"         );
    assert( wait_code == WAIT_OBJECT_0  && "Unexpected Wait code received"      );

    for ( auto& callback : server_callbacks )
        callback( handle.file_view );
#endif
}

void listen(const ninjahandle& handle, t_ninja_callback callback) 
{
    register_server_callback( callback );
    listen( handle );
}

void respond(const ninjahandle& handle, void* buffer, std::size_t buffer_size) 
{
    assert( handle.good && "Handle was not good" );
#ifdef IS_WINDOWS
    assert( handle.file_view && "File view was null" );
    write_buffer( handle, buffer, buffer_size );
    assert( SetEvent( handle.server_event ) && "Failed to set server event" );
#endif
}

void respond_text(const ninjahandle& handle, const char* content) 
{
    respond( handle, (void*)content, strlen(content) );
}

ninjahandle connect(const std::string& name, std::size_t buffer_size) 
{
    ninjahandle handle = {
        buffer_size 
    };

#ifdef IS_WINDOWS
    handle.file_mapping = OpenFileMappingA
    (
        FILE_MAP_ALL_ACCESS,
        false,
        name.c_str()
    );

    if ( !handle.file_mapping ) 
    {
        handle.good = false;
        handle.last_err = ninjaerror::SHARED_MEM_CONNECTION_FAILED;
        return handle;
    }

    handle.file_view = MapViewOfFile
    (
        handle.file_mapping,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        buffer_size
    );

    if ( !handle.file_view ) 
    {
        CloseHandle( handle.file_mapping );
        handle.good = false;
        handle.last_err = ninjaerror::WIN_SHARED_MEM_VIEW_FAILED;
        return handle;
    }

    std::string server_event_name{ name + SERVER_TAG };
    handle.server_event = OpenEventA( EVENT_ALL_ACCESS, false, server_event_name.c_str() );

    if ( !handle.server_event ) 
    {
        UnmapViewOfFile ( handle.file_view );
        CloseHandle     ( handle.file_mapping );
        CloseHandle     ( handle.file_view );
        handle.good = false;
        handle.last_err = ninjaerror::WIN_SERVER_EVENT_CONNECTION_FAILED;
        return handle;
    }

    std::string client_event_name{ name + CLIENT_TAG };
    handle.client_event = OpenEventA( EVENT_ALL_ACCESS, false, client_event_name.c_str() );

    if ( !handle.client_event ) 
    {
        UnmapViewOfFile ( handle.file_view );
        CloseHandle     ( handle.file_mapping );
        CloseHandle     ( handle.file_view );
        CloseHandle     ( handle.server_event );
        handle.good = false;
        handle.last_err = ninjaerror::WIN_CLIENT_EVENT_CONNECTION_FAILED;
        return handle;
    }

    return handle;
#endif
}

bool send_request(const ninjahandle& handle, void* buffer, std::size_t buffer_size, std::int32_t timeout) 
{
    assert( handle.good && "Handle was not good" );
#ifdef IS_WINDOWS
    assert( handle.file_view && "File view was null" );
    write_buffer( handle, buffer, buffer_size );
    assert( SetEvent( handle.client_event ) && "Failed to set client event" );

    const DWORD wait_code = WaitForSingleObject( handle.server_event, timeout );
    
    if ( wait_code == WAIT_TIMEOUT )
        return false;

    assert( wait_code != WAIT_ABANDONED && "Owner thread did not release event" );
    assert( wait_code != WAIT_FAILED    && "WaitForSingleObject failed"         );
    assert( wait_code == WAIT_OBJECT_0  && "Unexpected Wait code received"      );

    return true;
#endif
    return true;
}


} // ninjaipc