#pragma once

#include <cstdint>
#include <memory>

#include "cmp.hpp"
#include "xshmutex.hpp"
#include "xshmem.hpp"

namespace cat_ipc
{

constexpr unsigned max_clients          = 32;
constexpr unsigned max_commands         = 64;
constexpr unsigned pool_size            = 2 * 1024 * 1024; // 2 MB
constexpr unsigned short_command_length = 64;

namespace internal
{

struct client_data
{
    bool free;
    int  process;
    XMEMBER(
    /* WIN32 */ FILETIME start_time,
    /* LINUX */ uint32_t start_time
    );
};

struct command_data
{
    unsigned number;
    unsigned sender;
    unsigned target;
    unsigned user_type;
    uint8_t  data[short_command_length];
    uintptr_t payload_offset;
    uintptr_t payload_length;
};

}

template<typename S, typename U>
struct layout
{
    static_assert(std::is_pod<S>::value, "Server data struct must be POD");
    static_assert(std::is_pod<U>::value, "Custom data struct must be POD");

    LINUX_ONLY(xshmutex::xshmutex::shared_data shmutex);
    unsigned client_count;
    unsigned command_count;
    
    internal::client_data  clients[max_clients];
    internal::command_data commands[max_commands];
    
    uint8_t pool[pool_size];
    
    S user_server_data;
    U user_client_data;
};

template<typename S, typename U>
class shared
{
public:
    typedef layout<S, U> memory_t;
    typedef std::function<void(internal::command_data&, void *)> callback_t;
    constexpr unsigned ghost_id = unsigned(-1);
public:
    inline shared(std::string server)
        : server_name_(server)
    {
    }
    inline void setup_general_handler(callback_t callback)
    {
        general_handler_ = callback;
    }
    inline bool has_new_commands() const
    {
        return last_command_ != memory_.get<memory_t>()->command_count;
    }
    void setup_specialized_handler(callback_t callback, unsigned type)
    {
        if (specialized_handlers_.find(type) != specialized_handlers_.end())
        {
            throw std::logic_error("single command type can't have multiple callbacks");
        }
        specialized_handlers_.emplace(type, callback);
    }
    void process_new_commands();
    void send_message(unsigned target, unsigned type, const char *data_small, unsigned data_small_length, const void* payload, unsigned payload_length);
protected:
    const std::string server_name_ { "" };
    unsigned last_command_ { 0 };

    callback_t general_handler_ { nullptr };
    std::unordered_map<unsigned, callback_t> specialized_handlers_ {};

    CatMemoryPool pool_;
    xshmem::xshmem memory_;
    xshmutex::xshmutex mutex_;
};

template<typename S, typename U>
class client : public shared<S, U>
{
public:
    client(std::string name, bool ghost);
    ~client();
    
    bool     connect();
    unsigned free_slot() const;
protected:
    const bool ghost_;
    bool connected_ { false };
    unsigned id_ { ghost_id };
};

template<typename S, typename U>
class server : public shared<S, U>
{
public:
    server(std::string name);
    ~server();

    void update();
    bool is_client_dead(unsigned id);
    void remove_dead_clients();
};

}

#include "iipc_impl.hpp"
