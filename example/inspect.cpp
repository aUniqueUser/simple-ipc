#include "iipc.hpp"

#include <string>
#include <iostream>
#include <unistd.h>

struct server_info { char name[256]; };
struct client_info { char name[256]; };

cat_ipc::client<server_info, client_info>& client()
{
    static cat_ipc::client<server_info, client_info> object(std::string("example_cat_ipc_chat"), cat_ipc::client<server_info, client_info>::client_type::ghost);
    return object;
}

int main(int argc, char **argv)
{
    std::cout << "Inspecting server\n";
    std::cout << "\tClient count: " << client().memory()->client_count << "\n";
    std::cout << "\tCommand count: " << client().memory()->command_count << "\n";
}