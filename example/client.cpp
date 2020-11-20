#include "../src/ninja_ipc.hpp"
#include "userdef.h"
#include <iostream>

using ninjaipc::ninjahandle;

int main()
{
    ninjahandle ipc = ninjaipc::connect("Example01");

    if (!ipc.good)
    {
        std::cout << "Failed to connect to IPC server with code: " << static_cast<int>(ipc.last_err) << std::endl;
        return 0;
    }

    ipc_packet packet = { "ninjaipc" , 1 };

    ninjaipc::send_request<ipc_packet>(ipc, packet);
}