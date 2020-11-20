#include "../src/ninja_ipc.hpp"
#include "userdef.h"
#include <iostream>

using ninjaipc::ninjahandle;

int main()
{
    ninjahandle ipc = ninjaipc::create_server("Example01");

    if (!ipc.good)
    {
        std::cout << "Failed to create IPC server with code: " << static_cast<int>(ipc.last_err) << std::endl;
        return 0;
    }

    ninjaipc::listen(ipc, [ipc](void* buffer) -> void
    {
        std::cout << "Request received!" << std::endl;

        auto* packet = (ipc_packet*)buffer;

        std::cout << packet->lib_name << " Version: " << packet->version << std::endl;

        ninjaipc::acknowledge_request(ipc);
    });
}