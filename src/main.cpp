/*
 * main.cpp
 *
 *  Created on: Feb 5, 2017
 *      Author: nullifiedcat
 */

#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <time.h>
#include <csignal>

#include "include/cmp.hpp"
#include "include/ipcb.hpp"

static bool is_server = false;

struct server_data { int one; };
struct user_data { bool test; };

using peer_t = cat_ipc::Peer<server_data, user_data>;

peer_t& peer() {
	static peer_t object("test_ipc_cat", false, is_server);
	return object;
}

void process_message(cat_ipc::command_s& cmd, void* payload) {
	if (payload) {
		printf("%u says: %s\n", cmd.sender, (char*)payload);
	} else {
		printf("%u says: %s\n", cmd.sender, cmd.cmd_data);
	}
}

void* listen_for_messages(void* argument) {
	while (true) {
		peer().ProcessCommands();
		usleep(1000);
	}
	return 0;
}

/*
 * This requires a little bit of explanation.
 * server (manager) 's job is to allocate/free shared memory segment and collect dead clients.
 * everything else is done by peers
 */

int main(int argc, char** argv) {
	if (argc < 2) {
		printf("usage: %s [server|client]\n", argv[0]);
		return 0;
	}
	if (!strcmp(argv[1], "server")) {
		is_server = true;
		while (true) {
			peer().ProcessCommands();
			peer().SweepDead();
			printf("peer count: %u command count: %lu\n", peer().memory->peer_count, peer().memory->command_count);
			for (unsigned i = 0; i < cat_ipc::max_peers; i++) {
				if (!peer().memory->peer_data[i].free)
					printf("%i: [%i] ", i, peer().memory->peer_data[i].pid);
			}
			printf("\n");
			sleep(5);
		}
	} else if (!strcmp(argv[1], "client")) {
		peer().SetCallback(process_message);
		pthread_t thread;
		pthread_create(&thread, 0, listen_for_messages, 0);
		char buffer[256];
		while (true) {
			fgets(buffer, 256, stdin);
			buffer[strlen(buffer) - 1] = '\0';
			if (strlen(buffer) > 63) {
				peer().SendMessage(buffer, 0, buffer, 256);
			} else {
				peer().SendMessage(buffer, 0, 0, 0);
			}
		}
	}

}
