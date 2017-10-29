#include "iipc.hpp"

#include <iostream>
#include <thread>
#include <chrono>

constexpr uint32_t message_type_simple = 0;

struct server_info { char name[256]; };
struct client_info { char name[256]; };

cat_ipc::server<server_info, client_info>& server()
{
    static cat_ipc::server<server_info, client_info> object(std::string("example_cat_ipc_chat"));
    return object;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <server name>\n";
        return 1;
    }
    server();
    strncpy(server().memory()->user_server_data.name, argv[1], 255);
    while (true)
    {
        server().update();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}