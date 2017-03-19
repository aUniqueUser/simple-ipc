/*
 * ipcb.cpp
 *
 *  Created on: Feb 5, 2017
 *      Author: nullifiedcat
 */

#include "ipcb.hpp"

#include "util.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <dirent.h>
#include <sys/stat.h>
#include <memory.h>
#include <unistd.h>

#include <iostream>

unsigned CIPCPeer::FirstAvailableSlot() {
	MutexLock lock(this);
	for (int i = 0; i < MAX_PEERS; i++) {
		if (memory->manager_data.peer_data[i].free) {
			return i;
		}
	}
	throw std::runtime_error("no available slots");
}

void CIPCPeer::InitManager() {
	memset(memory, 0, sizeof(memory_layout));
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_setpshared(&attr, 1);
	pthread_mutex_init(&memory->mutex, &attr);
	pthread_mutexattr_destroy(&attr);
	for (int i = 0; i < MAX_PEERS; i++) memory->manager_data.peer_data[i].free = true;
	pool->init();
}

void CIPCPeer::SweepDead() {
	MutexLock lock(this);
	memory->manager_data.peer_count = 0;
	for (int i = 0; i < MAX_PEERS; i++) {
		if (IsPeerDead(i)) {
			memory->manager_data.peer_data[i].free = true;
		}
		if (!memory->manager_data.peer_data[i].free) {
			memory->manager_data.peer_count++;
		}
	}
}

bool CIPCPeer::IsPeerDead(unsigned id) {
	if (memory->manager_data.peer_data[id].free) return true;
	proc_stat_s stat;
	read_stat(memory->manager_data.peer_data[id].pid, &stat);
	if (stat.starttime != memory->manager_data.peer_data[id].starttime) return true;
	return false;
}

CIPCPeer::CIPCPeer(const char* name, bool is_manager) : name(name), is_manager(is_manager) {
	int old_mask = umask(0);
	int fd = shm_open(name, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
	ftruncate(fd, sizeof(memory_layout));
	umask(old_mask);
	memory = (memory_layout*)mmap(0, sizeof(memory_layout), PROT_WRITE | PROT_READ | PROT_EXEC, MAP_SHARED, fd, 0);
	close(fd);

	pool = new CatMemoryPool(&memory->pool, POOL_SIZE);

	if (is_manager) {
		InitManager();
	}

	client_id = FirstAvailableSlot();
	StoreClientData();
}

void CIPCPeer::StoreClientData() {
	MutexLock lock(this);
	proc_stat_s stat;
	read_stat(getpid(), &stat);
	memory->manager_data.peer_data[client_id].free = false;
	memory->manager_data.peer_data[client_id].pid = getpid();
	memory->manager_data.peer_data[client_id].starttime = stat.starttime;
}

CIPCPeer::~CIPCPeer() {
	MutexLock lock(this);
	memory->manager_data.peer_data[client_id].free = false;
	if (is_manager) {
		pthread_mutex_destroy(&memory->mutex);
		shm_unlink(name);
		munmap(memory, sizeof(memory_layout));
	}
}

void CIPCPeer::ProcessCommands() {
	MutexLock lock(this);
	for (int i = 0; i < COMMAND_COUNT; i++) {
		command_metadata_s& cmd = memory->commands[i];
		if (cmd.command_number > last_command) {
			last_command = cmd.command_number;
			if (cmd.sender != client_id && (!cmd.peer_mask || ((1 << client_id) & cmd.peer_mask))) {
				if (callback) {
					callback(cmd, cmd.payload_size ? pool->real_pointer<void>((void*)cmd.payload_offset) : 0);
				}
			}
		}
	}
}

void CIPCPeer::SetCallback(CIPCPeer::CommandCallbackFn_t new_callback) {
	callback = new_callback;
}

void CIPCPeer::SendMessage(char* data_small, unsigned user_mask, char* payload, size_t payload_size) {
	MutexLock lock(this);
	command_metadata_s& cmd = memory->commands[++memory->command_count % COMMAND_COUNT];
	if (cmd.payload_size) {
		pool->free(pool->real_pointer<void>((void*)cmd.payload_offset));
		cmd.payload_offset = 0;
		cmd.payload_size = 0;
	}
	memcpy(cmd.cmd_data, data_small, sizeof(cmd.cmd_data));
	if (payload_size) {
		void* block = pool->alloc(payload_size);
		memcpy(block, payload, payload_size);
		cmd.payload_offset = (unsigned long)pool->pool_pointer<void>(block);
		cmd.payload_size = payload_size;
	}
	cmd.sender = client_id;
	cmd.peer_mask = user_mask;
	cmd.command_number = memory->command_count;
}
