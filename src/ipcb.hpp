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

#include <functional>

#include "cmp.hpp"

/* This implementation allows up to 32 clients (unsigned int) */
/* very strange mix of P2P and client-server */

#define MAX_PEERS 32
#define COMMAND_COUNT 64
#define POOL_SIZE 1024 * 256

// EXAMPLE STRUCT - THIS CAN BE ANYTHING!
struct peer_user_data_s {
	char name[32];
};

struct peer_data_s {
	pid_t pid;
	unsigned long starttime;
	bool free;
};

struct command_metadata_s {
	unsigned sender;
	unsigned command_number;
	unsigned long payload_offset;
	unsigned payload_size;
	unsigned peer_mask;
	char cmd_data[64];
};

struct manager_data_s {
	unsigned peer_count;
	peer_data_s peer_data[MAX_PEERS];
	peer_user_data_s peer_user_data[MAX_PEERS];
};

struct memory_layout {
	pthread_mutex_t mutex;
	manager_data_s manager_data;
	unsigned long command_count;
	command_metadata_s commands[COMMAND_COUNT];
	char pool[POOL_SIZE];
};

class CIPCPeer {
public:
	CIPCPeer(const char* name, bool is_manager = false);
	~CIPCPeer();

	typedef std::function<void(command_metadata_s&, void*)> CommandCallbackFn_t;

	class MutexLock {
	public:
		MutexLock(CIPCPeer* parent) : parent(parent) { pthread_mutex_lock(&parent->memory->mutex); }
		~MutexLock() { pthread_mutex_unlock(&parent->memory->mutex); }

		CIPCPeer* parent;
	};

	unsigned client_id { 0 };
	unsigned long last_command { 0 };
	unsigned FirstAvailableSlot();
	bool IsPeerDead(unsigned id);
	void InitManager();
	void SweepDead();
	void StoreClientData();
	void SetCallback(CommandCallbackFn_t);
	void ProcessCommands();
	void SendMessage(char* data_small, unsigned peer_mask, char* payload, size_t payload_size);

	CommandCallbackFn_t callback { nullptr };
	CatMemoryPool* pool { nullptr };
	const char* name;
	memory_layout* memory { nullptr };
	bool is_manager { false };
};

#endif /* IPCB_HPP_ */
