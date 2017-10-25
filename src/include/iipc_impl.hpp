/*
 * iipc_impl.hpp
 *
 *  Created on: Oct 24, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include <string.h>

namespace cat_ipc
{

template<typename S, typename U>
void shared<S, U>::process_new_commands()
{
    for (unsigned i = 0; i < max_commands; ++i)
    {
        internal::command_data& cmd = memory_->commands[i];
        if (cmd.number > last_command_)
        {
            last_command_ = cmd.number;
            if (cmd.sender != id_ && (cmd.target == ghost_id || cmd.target == id_))
            {
                if (general_handler_)
                {
                    general_handler_(cmd, cmd.payload_length ? pool_.real_pointer<uint8_t>(cmd.payload_offset) : nullptr);
                }
                if (specialized_handlers_.find(cmd.type) != specialized_handlers_.end())
                {
                    specialized_handlers_[cmd.type](cmd, cmd.payload_length ? pool_.real_pointer<uint8_t>(cmd.payload_offset) : nullptr);
                }
            }
        }
    }
}

template<typename S, typename U>
void shared<S, U>::send_message(uint32_t target, uint32_t type, const uint8_t *data_small, uint32_t data_small_length, const uint8_t* payload, uint32_t payload_length)
{
    {
        // Critical Section
        xshmutex::xshmutex::guard lock(mutex_);

        internal::command_data& cmd = memory_->commands[++memory_->command_count % max_commands];
        if (cmd.payload_length)
        {
            pool_.free(pool_.real_pointer<void>(cmd.payload_offset));
        }
        memset(&cmd, 0, sizeof(cmd));
        if (data_small && data_small_length)
        {
            memcpy(cmd.data, data_small, std::min(short_command_length, data_small_length));
        }
        if (payload && payload_length)
        {
            void *block = pool_.alloc(payload_length);
            memcpy(block, payload, payload_length);
            cmd.payload_length = payload_length;
            cmd.payload_offset = pool_.pool_pointer(block);
        }
        cmd.number = memory_->command_count;
        cmd.sender = id_;
        cmd.target = target;
        cmd.type = type;
    }
}

template<typename S, typename U>
void client<S, U>::send_message(uint32_t target, uint32_t type, const uint8_t *data_small, uint32_t data_small_length, const uint8_t* payload, uint32_t payload_length)
{
    if (!connected_ || type_ == client_type::ghost)
    {
        return;
    }
    shared<S, U>::send_message(target, type, data_small, data_small_length, payload, payload_length);
}

template<typename S, typename U>
void client<S, U>::process_new_commands()
{
    if (!connected_ || type_ == client_type::writer)
    {
        return;
    }
    shared<S, U>::process_new_commands();
}

template<typename S, typename U>
void client<S, U>::_store_data()
{
    if (type_ == client_type::ghost)
    {
        return;
    }
    {
        // Critical section
        xshmutex::xshmutex::guard lock();

        client_data_->free = false;
        xstoredata(*client_data_);
    }
}

template<typename S, typename U>
bool client<S, U>::_check_memory()
{
    // Endianness is different from server or memory is corrupted
    if (this->memory_->magic != layout<S, U>::magic_ideal ||
        this->memory_->magic2 != layout<S, U>::magic2_ideal)
    {
        return false;
    }
    if (this->memory_->size_server_data != sizeof(S) ||
        this->memory_->size_client_data != sizeof(U))
    {
        return false;
    }
    return true;
}

template<typename S, typename U>
client<S, U>::client(std::string name, client_type type)
    : shared<S, U>(false, name), type_(type)
{
}

template<typename S, typename U>
client<S, U>::~client()
{
}

template<typename S, typename U>
bool client<S, U>::connect()
{
    this->mutex_.connect();
    this->shmem_.connect();
    this->memory_ = nullptr;
    if (!_check_memory())
    {
        return false;
    }
    if (type_ == client_type::normal)
    {
        this->id_ = free_slot();
        if (this->id_ == this->ghost_id)
        {
            return false;
        }
        _store_data();
    }
    this->last_command_ = this->memory_->command_count;
    connected_ = true;
    return true;
}

template<typename S, typename U>
uint32_t client<S, U>::free_slot() const
{
    for (uint32_t i = 0; i < max_clients; ++i)
    {
        if (this->memory_->clients[i].free)
        {
            return i;
        }
    }
    return this->ghost_id;
}

template<typename S, typename U>
void server<S, U>::_store_magic_data()
{
    this->memory_->magic = layout<S, U>::magic_ideal;
    this->memory_->magic2 = layout<S, U>::magic2_ideal;

    this->memory_->size_server_data = sizeof(S);
    this->memory_->size_client_data = sizeof(U);
}

template<typename S, typename U>
server<S, U>::server(std::string name)
    : shared<S, U>(true, name)
{
    _init();
}

template<typename S, typename U>
server<S, U>::~server()
{
    _destroy();
}

template<typename S, typename U>
void server<S, U>::_init()
{
    this->id_ = this->server_id;
    this->last_command_ = 0;
    _store_magic_data();
    for (uint32_t i = 0; i < max_clients; ++i)
    {
        this->memory_->clients[i].free = true;
    }
}

template<typename S, typename U>
void server<S, U>::_destroy()
{

}

template<typename S, typename U>
void server<S, U>::update()
{
    _remove_dead_clients();
}

template<typename S, typename U>
void server<S, U>::_remove_dead_clients()
{
    for (uint32_t i = 0; i < max_clients; ++i)
    {
        internal::client_data& client = this->memory_->clients[i];
        if (xcheckdead(client))
        {
            client.free = true;
        }
    }
}

}


