#include "iipc.hpp"

namespace cat_ipc
{

client::client(std::string name) : name_(name)
{
    
}

client::~client()
{
}

bool client::connect(bool ghost)
{
    ghost_ = ghost;
    if (!ghost)
    {
        
    }
}

void client::setup_message_handler(unsigned type, callback_t callback)
{
}

bool client::has_new_commands() const
{
}

void client::process_commands()
{
	for (unsigned i = 0; i < command_buffer; i++) {
		command_data& cmd = memory_->commands[i];
		if (cmd.number > last_command_) {
			last_command = cmd.number;
			if (cmd.sender != id_ && (cmd.target == ghost_id || cmd.target == id_)) {
				if (general_handler_) {
					general_handler_(cmd, cmd.payload_length ? pool->real_pointer<void>((void*)cmd.payload_offset) : nullptr);
				}
				if (handlers_.find(cmd.type) != handlers_.end()) {
					handlers_[cmd.type](cmd, cmd.payload_length ? pool->real_pointer<void>((void*)cmd.payload_offset) : nullptr);
				}
			}
		}
	}
}

void client::send_message(const char *data_small, unsigned target, unsigned type, const void* payload, unsigned payload_length)
{
    auto lock = shmutex::shmutex::guard(*mutex_);
	command_data& cmd = memory_->commands[++memory_->command_count % max_commands];
	if (cmd.payload_size) {
		pool_->free(pool_->real_pointer<void>((void*)cmd.payload_offset));
	}
    memset(&cmd, 0, sizeof(cmd));
	if (data_small)
		memcpy(cmd.data, data_small, sizeof(cmd.data));
	if (payload_length) {
		void *block = pool_->alloc(payload_length);
		memcpy(block, payload, payload_length);
		cmd.payload_offset = (unsigned long)pool_->pool_pointer<void>(block);
		cmd.payload_length = payload_length;
	}
	cmd.type = command_type;
	cmd.sender = id_;
	cmd.target = target;
	cmd.number = memory->command_count;
}

unsigned client::free_slot() const
{
}

}