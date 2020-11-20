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
#include <string>

#define DIAGNOSE_LISTEN 1

constexpr auto SERVER_TAG = "NINJAIPC_SERVER";
constexpr auto CLIENT_TAG = "NINJAIPC_CLIENT";
static bool g_assert_request_response = true;

namespace ninjaipc {

std::vector< ninja_callback > server_callbacks = {};

void write_buffer(const ninjahandle& handle, void* buffer, std::size_t buffer_size) 
{
    /*
    * Let's clean the whole buffer first
    * so we don't mix up two blocks of data
    *
    * After that we fill the buffer with received data
    * If buffer is null, we are simply cleaning it
    */
#ifdef IS_WINDOWS
    std::memset( handle.file_view, '\0', handle.buffer_size );
    if ( buffer )
        std::memcpy( handle.file_view, buffer, buffer_size );
#endif
#ifdef IS_LINUX
    memset( handle.file_mapping, '\0', handle.buffer_size );
    if ( buffer )
        memcpy( handle.file_mapping, buffer, buffer_size );
#endif
}

ninjahandle create_server(const std::string& name, const std::size_t buffer_size) 
{
    ninjahandle handle = {
        /* handle.buffer_size = */ buffer_size
    };
#ifdef IS_WINDOWS

    /*
    * We will be using file mapping as our
    * shared memory object
    */
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

    /*
    * Ensures that we are creating the file mapping
    * instead of opening an existing one
    */
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

    /*
    * Let's use a suffix so we don't collide
    * with already existing named events
    */
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

    /*
    * Ensures that we are creating the event
    */
    if ( GetLastError() == ERROR_ALREADY_EXISTS ) 
    {
        CloseHandle     ( handle.server_event );
        CloseHandle     ( handle.client_event );
        CloseHandle     ( handle.file_mapping );
        handle.good = false;
        handle.last_err = ninjaerror::WIN_CLIENT_EVENT_ALREADY_EXISTS;
        return handle;
    }

    /*
    * CreateFileMapping doesn't actually maps anything
    * MapViewOfFile does it
    */
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
#ifdef IS_LINUX

    /*
    * The backing file needs to have "/"
    * at the beginning
    */
    std::string slash_tag = { "/" + name };

    /*
    * Let's use O_CREAT so we ensure that we are creating it
    */
    handle.file_descriptor = shm_open( slash_tag.c_str(), O_CREAT | O_RDWR, 0644 );

    if ( handle.file_descriptor < 0 ) 
    {
        handle.good = false;
        handle.last_err = ninjaerror::LINUX_FILE_DESCRIPTOR_CREATION_FAILED;
        return handle;
    }

    /*
    * Reserve space on shared memory backing file
    */
    ftruncate( handle.file_descriptor, buffer_size );

    /*
    * Maps the shared memory object
    */
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
#endif
    return {};
}

void register_server_callback(ninja_callback callback) 
{
    /*
    * This needs to be called at least once
    */
    server_callbacks.push_back( callback );
}

/* listener methods */
bool internal_listen(const ninjahandle& handle) 
{
    assert( !server_callbacks.empty() && "No callbacks registered" );
    assert( handle.good               && "Handle was not good" );

    /*
    * Let's ensure that for every request there's at least
    * one response from the callback(s)
    *
    * Set to false so we are expecting response
    */
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

        /*
        * Check if the callback(s) actually responded
        * So we don't break the request:response rule
        */
        assert( g_assert_request_response && "No response sent from callbacks" );

        return true;
    }

    assert( false && "Unexpected Wait code received" );
#endif
#ifdef IS_LINUX
    if ( sem_wait( handle.client_semaphore ) == 0 )
    {
        for ( auto& callback : server_callbacks )
            callback( handle.file_mapping );

        /*
        * Check for request:response rule
        */
        assert( g_assert_request_response && "No response sent from callbacks" );
        return true;
    }

    assert( false && "Unexpected Wait code received" );
#endif
    return false;
}

void listen(const ninjahandle& handle, ninja_callback callback) 
{
    /*
    * Listen wrapper for minified usage
    */
    register_server_callback( callback );
    listen( handle );
}

void listen(const ninjahandle& handle)
{
    do 
    {
#if DIAGNOSE_LISTEN == 1
        static long long call_n = { 0 };

        std::string request_log = { "Requests: " + std::to_string(call_n) };
        int backspace_amount = request_log.length();
        std::string backspace_block(backspace_amount, '\b');
        std::cout << request_log << backspace_block << std::flush;

        ++call_n;
#endif
    } while ( internal_listen(handle) );
}

void respond(const ninjahandle& handle, void* buffer, std::size_t buffer_size) 
{
    assert( handle.good && "Handle was not good" );

    /*
    * We responded!
    * the request:response rule is preserved
    */
    g_assert_request_response = true;
#ifdef IS_WINDOWS
    assert( handle.file_view && "File view was null" );
    write_buffer( handle, buffer, buffer_size );
    assert( SetEvent( handle.server_event ) && "Failed to set server event" );
#endif
#ifdef IS_LINUX
    assert( handle.file_mapping && "File mapping was null" );
    write_buffer( handle, buffer, buffer_size );
    assert( sem_post( handle.server_semaphore ) == 0 && "Failed to set server semaphore" );
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
#ifdef IS_WINDOWS
    assert( SetEvent( handle.server_event ) && "Failed to set server event" );
#endif
#ifdef IS_LINUX
    assert( sem_post( handle.server_semaphore ) == 0 && "Failed to set server semaphore" );
#endif
}

ninjahandle connect(const std::string& name, std::size_t buffer_size) 
{
    ninjahandle handle = {
        /* handle.buffer_size = */ buffer_size
    };

    /*
    * We are pretty much mimicking the server
    * creation function for opening handles
    * and not creating objects
    */

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
    return {};
}

bool send_request(const ninjahandle& handle, void* buffer, std::size_t buffer_size, std::int32_t timeout) 
{
    assert( handle.good && "Handle was not good" );
#ifdef IS_WINDOWS
    assert( handle.file_view && "File view was null" );
    write_buffer( handle, buffer, buffer_size );

    /*
    * Notifies client is sending a request
    */
    assert( SetEvent( handle.client_event ) && "Failed to set client event" );

    /*
    * Wait for server response
    */
    const DWORD wait_code = WaitForSingleObject( handle.server_event, timeout );
    
    /*
    * Check if timeout occurred
    */
    if ( wait_code == WAIT_TIMEOUT )
        return false;

    /*
    * Check if it's fatal
    */
    assert( wait_code != WAIT_ABANDONED && "Owner thread did not release event" );
    assert( wait_code != WAIT_FAILED    && "WaitForSingleObject failed"         );

    /*
    * Ensures it's == WAIT_OBJECT_0 aka success
    */
    assert( wait_code == WAIT_OBJECT_0  && "Unexpected Wait code received"      );

    return true;
#endif
#ifdef IS_LINUX
    auto ms_to_timespec = [](std::int32_t timeout) -> timespec
    {
        assert( timeout > 0 && "Invalid value passed to ms_to_timespec" );

        /*
        * sem_timedwait expects the timespec structure
        * which does not support milliseconds as member
        * so we need to convert ms:[s,ns]
        */

        return 
        {
            timeout / 1000,
            ( timeout % 1000 ) * 1000000
        };
    };

    assert( handle.file_mapping && "File mapping was null" );
    write_buffer( handle, buffer, buffer_size );
    sem_post( handle.client_semaphore );

    /*
    * Timeout -1 means we are waiting forever
    * the value is for maintaining compatibility
    * with windows INFINITE define
    */
    if ( timeout == -1 )
    {
        const int wait_code = sem_wait( handle.server_semaphore );

        if ( wait_code == 0 )
            return true;

        if ( wait_code == -1 )
        {
            /*
            * Force assert on fatal
            */
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
            /*
            * Check if fatal error
            */
            assert( errno != EDEADLK   && "Deadlock condition was detected"         );
            assert( errno != EINVAL    && "Invalid server semaphore"                );
            /*
            * Fine if it's a timeout
            * just return false to indicate error
            */
            assert( errno == ETIMEDOUT && "Unknown return value on sem_timedwait()" );       
        }

        return false;
    }
#endif
}

} // namespace ninjaipc