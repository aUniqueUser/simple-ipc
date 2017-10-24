/*
 * iipc_impl.hpp
 *
 *  Created on: Oct 24, 2017
 *      Author: nullifiedcat
 */

#pragma once

namespace cat_ipc
{

template<typename S, typename U>
void shared<S, U>::process_new_commands()
{
    for (unsigned i = 0; i < max_commands; ++i)
    {
        internal::command_data& cmd = shmem_.get<memory_t>()->commands[i];
        if (cmd.number > last_command_)
        {
            last_command_ = cmd.number;
            if (cmd.sender != id_ && (cmd.target == ghost_id || cmd.target == id_))
            {
                if (general_handler_)
                {
                    general_handler_(cmd, cmd.payload_length ? pool_.real_pointer<void>(cmd.payload_offset) : nullptr);
                }
                if (specialized_handlers_.find(cmd.type) != specialized_handlers_.end())
                {
                    specialized_handlers_[cmd.type](cmd, cmd.payload_length ? pool_.real_pointer<void>(cmd.payload_offset) : nullptr);
                }
            }
        }
    }
}

template<typename S, typename U>
void shared<S, U>::send_message(unsigned target, unsigned type, const char *data_small, size_t data_small_length, const void *payload, size_t payload_length)
{
    auto _lock = xshmutex::xshmutex::guard(mutex_);
    internal::command_data& cmd = memory_->commands[++memory_->command_count % max_commands];
    if (cmd.payload_length)
    {
        pool_.free(pool_.real_pointer<void>(cmd.payload_offset));
    }
}

template<typename S, typename U>
void client<S, U>::_store_data()
{
    if (ghost_)
    {
        return;
    }
    auto _lock = xshmutex::xshmutex::guard();
    client_data_->free = false;
    xstoredata(*client_data_);
}

template<typename S, typename U>
bool client<S, U>::_check_memory()
{
    // Endianness is different from server or memory is corrupted
    if (memory_->magic != layout<S, U>::magic_ideal ||
        memory_->magic2 != layout<S, U>::magic2_ideal)
    {
        return false;
    }
}

template<typename S, typename U>
client<S, U>::~client()
{

}


}


