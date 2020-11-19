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
#include <cstring>

constexpr auto SERVER_TAG = "NINJAIPC_SERVER";
constexpr auto CLIENT_TAG = "NINJAIPC_CLIENT";
static bool g_assert_request_response = true;

namespace ninjaipc {

std::vector< ninja_callback > server_callbacks = {};

void write_buffer(const ninjahandle& handle, void* buffer, std::size_t buffer_size) 
{
#ifdef IS_WINDOWS
    std::memset( handle.file_view, '\0', handle.buffer_size );
	if ( buffer )
        std::memcpy( handle.file_view, buffer, buffer_size );
#endif // IS_WINDOWS
#ifdef IS_LINUX
    memset( handle.file_mapping, '\0', handle.buffer_size );
    if ( buffer )
        memcpy( handle.file_mapping, buffer, buffer_size );
#endif
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
#endif // IS_WINDOWS
#ifdef IS_LINUX
    std::string slash_tag = { "/" + name };
    handle.file_descriptor = shm_open( slash_tag.c_str(), O_CREAT | O_RDWR, 0644 );

    if ( handle.file_descriptor < 0 ) 
    {
        handle.good = false;
        handle.last_err = ninjaerror::LINUX_FILE_DESCRIPTOR_CREATION_FAILED;
        return handle;
    }

    ftruncate( handle.file_descriptor, buffer_size );

    handle.file_mapping = reinterpret_cast<caddr_t>
    (
        mmap
        (
            nullptr,
            buffer_size,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            handle.file_descriptor,
            0
        )
    );

    if ( handle.file_mapping == reinterpret_cast<caddr_t>(-1) )
    {
        close( handle.file_descriptor );
        handle.good = false;
        handle.last_err = ninjaerror::LINUX_FILE_MAPPING_MAP_FAILED;
        return handle;
    }

    std::string server_semaphore_name{ name + SERVER_TAG };

    handle.server_semaphore = sem_open(
        server_semaphore_name.c_str(),
        O_CREAT,
        0644,
        0
    );

    if ( handle.server_semaphore == reinterpret_cast<void*>(-1) ) 
    {
        close( handle.file_descriptor );
        munmap( handle.file_mapping, buffer_size );
        handle.good = false;
        handle.last_err = ninjaerror::LINUX_SERVER_SEMAPHORE_CREATION_FAILED;
        return handle;
    }

    std::string client_semaphore_name{ name + CLIENT_TAG };

    handle.client_semaphore = sem_open(
        client_semaphore_name.c_str(),
        O_CREAT,
        0644,
        0
    );

    if ( handle.client_semaphore == reinterpret_cast<void*>(-1) ) 
    {
        close( handle.file_descriptor );
        munmap( handle.file_mapping, buffer_size );
        handle.good = false;
        handle.last_err = ninjaerror::LINUX_CLIENT_SEMAPHORE_CREATION_FAILED;
        return handle;
    }

    return handle;
#endif // IS_LINUX
    return {}; // FIXME
}

void register_server_callback(ninja_callback callback) 
{
    server_callbacks.push_back( callback );
}

/* listener methods */
void listen(const ninjahandle& handle) 
{
    assert( !server_callbacks.empty() && "No callbacks registered" );
    assert( handle.good               && "Handle was not good" );
	g_assert_request_response = false;
#ifdef IS_WINDOWS
    const DWORD wait_code = WaitForSingleObject( handle.client_event, INFINITE );

    // skip WAIT_TIMEOUT since we are waiting forever
    assert( wait_code != WAIT_ABANDONED && "Owner thread did not release event" );
    assert( wait_code != WAIT_FAILED    && "WaitForSingleObject failed"         );
    
    if ( wait_code == WAIT_OBJECT_0 )
    {
        for ( auto& callback : server_callbacks )
            callback( handle.file_view );

        assert( g_assert_request_response && "No response sent from callbacks" );

        listen(handle);
    }

    assert( false && "Unexpected Wait code received" );
#endif // IS_WINDOWS
#ifdef IS_LINUX
    if ( sem_wait( handle.client_semaphore ) == 0 )
    {
        for ( auto& callback : server_callbacks )
            callback( handle.file_mapping );

		assert( g_assert_request_response && "No response sent from callbacks" );
        listen(handle);
    }

    assert( false && "Unexpected Wait code received" );
#endif
}

void listen(const ninjahandle& handle, ninja_callback callback) 
{
    register_server_callback( callback );
    listen( handle );
}

void respond(const ninjahandle& handle, void* buffer, std::size_t buffer_size) 
{
    assert( handle.good && "Handle was not good" );
	g_assert_request_response = true;
#ifdef IS_WINDOWS
    assert( handle.file_view && "File view was null" );
    write_buffer( handle, buffer, buffer_size );
    assert( SetEvent( handle.server_event ) && "Failed to set server event" );
#endif // IS_WINDOWS
#ifdef IS_LINUX
    assert( handle.file_mapping && "File mapping was null" );
    write_buffer( handle, buffer, buffer_size );
    sem_post( handle.server_semaphore );
#endif
}

void respond_text(const ninjahandle& handle, const char* content) 
{
    respond( handle, (void*)content, strlen(content) );
}

void acknowledge_request(const ninjahandle& handle)
{
    g_assert_request_response = true;
    write_buffer(handle, nullptr, 0);
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
#endif // IS_WINDOWS
#ifdef IS_LINUX
    std::string slash_tag = { "/" + name };
    handle.file_descriptor = shm_open( slash_tag.c_str(), O_RDWR, 0644 );

    if ( handle.file_descriptor < 0 ) 
    {
        handle.good = false;
        handle.last_err = ninjaerror::LINUX_FILE_DESCRIPTOR_CONNECTION_FAILED;
        return handle;
    }

    handle.file_mapping = reinterpret_cast<caddr_t>
    (
        mmap
        (
            nullptr,
            buffer_size,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            handle.file_descriptor,
            0
        )
    );

    if ( handle.file_mapping == reinterpret_cast<caddr_t>(-1) )
    {
        close( handle.file_descriptor );
        handle.good = false;
        handle.last_err = ninjaerror::LINUX_FILE_MAPPING_MAP_FAILED;
        return handle;
    }

    std::string server_semaphore_name{ name + SERVER_TAG };

    handle.server_semaphore = sem_open(
        server_semaphore_name.c_str(),
        O_CREAT,
        0644,
        0
    );

    if ( handle.server_semaphore == reinterpret_cast<void*>(-1) ) 
    {
        close( handle.file_descriptor );
        munmap( handle.file_mapping, buffer_size );
        handle.good = false;
        handle.last_err = ninjaerror::LINUX_SERVER_SEMAPHORE_CONNECTION_FAILED;
        return handle;
    }

    std::string client_semaphore_name{ name + CLIENT_TAG };

    handle.client_semaphore = sem_open(
        client_semaphore_name.c_str(),
        O_CREAT,
        0644,
        0
    );

    if ( handle.client_semaphore == reinterpret_cast<void*>(-1) ) 
    {
        close( handle.file_descriptor );
        munmap( handle.file_mapping, buffer_size );
        handle.good = false;
        handle.last_err = ninjaerror::LINUX_CLIENT_SEMAPHORE_CONNECTION_FAILED;
        return handle;
    }

    return handle;

#endif
    return {}; // FIXME
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
#endif // IS_WINDOWS
#ifdef IS_LINUX
    auto ms_to_timespec = [](std::int32_t timeout) -> timespec
    {
        assert( timeout > 0 && "Invalid value passed to ms_to_timespec" );
        return 
        {
            timeout / 1000,
            ( timeout % 1000 ) * 1000000
        };
    };

    assert( handle.file_mapping && "File mapping was null" );
    write_buffer( handle, buffer, buffer_size );
    sem_post( handle.client_semaphore );

    if ( timeout == -1 )
    {
        const int wait_code = sem_wait( handle.server_semaphore );

        if ( wait_code == 0 )
            return true;

        if ( wait_code == -1 )
        {
            assert( errno != EINVAL && "Invalid server semaphore" );
        }

        return false;

    }
    else
    {
        timespec timeout_spec = ms_to_timespec(timeout);

        const int wait_code = sem_timedwait( handle.server_semaphore, &timeout_spec );

        if ( wait_code == 0 )
            return true;

        if ( wait_code == -1 )
        {
            assert( errno != EDEADLK   && "Deadlock condition was detected"         );
            assert( errno != EINVAL    && "Invalid server semaphore"                );
            assert( errno == ETIMEDOUT && "Unknown return value on sem_timedwait()" );       
        }

        return false;
    }
#endif
}


} // ninjaipc