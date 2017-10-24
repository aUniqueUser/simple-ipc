/*
 * ipcb.hpp
 *
 *  Created on: Feb 5, 2017
 *      Author: nullifiedcat
 */

#ifndef IPCB_HPP_
#define IPCB_HPP_

#include <stddef.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <memory.h>

#include <unordered_map>
#include <iostream>
#include <type_traits>
#include <functional>

#include "util.h"
#include "cmp.hpp"

/* This implementation allows up to 32 clients (unsigned int) */
/* very strange mix of P2P and client-server */

namespace cat_ipc {

template<typename S, typename U>
class Peer {
public:
	/*
	 * Actually connects to server
	 */
	void Connect() {
		connected = true;
		int old_mask = umask(0);
		int flags = O_RDWR;
		if (is_manager) flags |= O_CREAT;
		int fd = shm_open(name.c_str(), flags, S_IRWXU | S_IRWXG | S_IRWXO);
		if (fd == -1) {
			throw std::runtime_error("server isn't running");
		}
		ftruncate(fd, sizeof(memory_t));
		umask(old_mask);
		memory = (memory_t*)mmap(0, sizeof(memory_t), PROT_WRITE | PROT_READ | PROT_EXEC, MAP_SHARED, fd, 0);
		close(fd);
		pool = new CatMemoryPool(&memory->pool, pool_size);
		if (is_manager) {
			InitManager();
		}
		if (not is_ghost) {
			client_id = FirstAvailableSlot();
			StorePeerData();
		} else {
			client_id = unsigned(-1);
		}
		if (!process_old_commands) {
			last_command = memory->command_count;
		}
	}

	/*
	 * Checks every slot in memory->peer_data, throws runtime_error if there are no free slots
	 */
	unsigned FirstAvailableSlot() {
		MutexLock lock(this);
		for (unsigned i = 0; i < max_peers; i++) {
			if (memory->peer_data[i].free) {
				return i;
			}
		}
		throw std::runtime_error("no available slots");
	}

	/*
	 * Should be called only once in a lifetime of ipc instance.
	 * this function initializes memory
	 */
	void InitManager() {
		memset(memory, 0, sizeof(memory_t));
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_setpshared(&attr, 1);
		pthread_mutex_init(&memory->mutex, &attr);
		pthread_mutexattr_destroy(&attr);
		for (unsigned i = 0; i < max_peers; i++) memory->peer_data[i].free = true;
		pool->init();
	}

	/*
	 * Marks every dead peer as free
	 */
	void SweepDead() {
		MutexLock lock(this);
		memory->peer_count = 0;
		for (unsigned i = 0; i < max_peers; i++) {
			if (IsPeerDead(i)) {
				memory->peer_data[i].free = true;
			}
			if (!memory->peer_data[i].free) {
				memory->peer_count++;
			}
		}
	}
	/*
	 * Posts a command to memory, increases command_count
	 */
	void SendMessage(const char* data_small, unsigned peer_mask, unsigned command_type, const void* payload, size_t payload_size) {
		MutexLock lock(this);
		command_s& cmd = memory->commands[++memory->command_count % command_buffer];
		if (cmd.payload_size) {
			pool->free(pool->real_pointer<void>((void*)cmd.payload_offset));
			cmd.payload_offset = 0;
			cmd.payload_size = 0;
		}
		if (data_small)
			memcpy(cmd.cmd_data, data_small, sizeof(cmd.cmd_data));
		if (payload_size) {
			void* block = pool->alloc(payload_size);
			memcpy(block, payload, payload_size);
			cmd.payload_offset = (unsigned long)pool->pool_pointer<void>(block);
			cmd.payload_size = payload_size;
		}
		cmd.cmd_type = command_type;
		cmd.sender = client_id;
		cmd.peer_mask = peer_mask;
		cmd.command_number = memory->command_count;
	}
};

}



#endif /* IPCB_HPP_ */
