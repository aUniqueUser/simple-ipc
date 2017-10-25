#include "iipc.hpp"

#include <string>
#include <iostream>
#include <unistd.h>
 
constexpr uint32_t message_type_simple = 0;

struct server_info { char name[256]; };
struct client_info { char name[256]; };

cat_ipc::client<server_info, client_info>& client()
{
    static cat_ipc::client<server_info, client_info> object(std::string("example_cat_ipc_chat"), cat_ipc::client<server_info, client_info>::client_type::normal);
    return object;
}

void *listener_thread(void *argument)
{
    while (true)
    {
        client().process_new_commands();
        usleep(1000);
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <name>\n";
        return 1;
    }
    
    strncpy(client().client_data()->name, argv[1], 255);
    std::cout << "Welcome to " << client().memory()->user_server_data.name << ", " << argv[1] << "!\n";
    client().setup_specialized_handler([](cat_ipc::internal::command_data& cmd, const uint8_t *payload)
    {
        std::cout << client().memory()->user_client_data[cmd.sender].name << " says: ";
        std::cout << payload;
    }, message_type_simple);
    pthread_t thread;
	pthread_create(&thread, 0, listener_thread, 0);
	char* buffer = new char[1024 * 1024];
	while (true) {
        fgets(buffer, 1024 * 1024, stdin);
		buffer[strlen(buffer)] = '\0';
		client().send_message(client().ghost_id, message_type_simple, (const uint8_t *)buffer, strlen(buffer) + 1);
	}
}