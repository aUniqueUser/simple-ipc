#pragma once

#include <cstdint>

#include "cmp.hpp"
#include "xshmutex.hpp"

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
    _PLATFORM_ process_time_t start_time;
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
class client
{
public:
    typedef layout<S, U> memory_t;
    typedef std::function<void(internal::command_data&, void *)> callback_t;
    constexpr unsigned ghost_id = unsigned(-1);
public:
    client(std::string name);
    ~client();
    
    bool connect(bool ghost);
    void setup_message_handler(unsigned type, callback_t callback);
    bool has_new_commands() const;
    void process_commands();
    void send_message(const char *data_small, unsigned target, unsigned type, const void* payload, unsigned payload_length);
    unsigned free_slot() const;
protected:
    bool ghost_ { false };
    bool connected_ { false };
    unsigned id_ { ghost_id };
    std::string name_ { "" };
    unsigned last_command_ { 0 };
    std::shared_ptr<shmutex::shmutex> mutex_;
    std::shared_ptr<CatMemoryPool> pool_;
    memory_t *memory { nullptr };
    callback_t general_handler_ { nullptr };
    std::unordered_map<unsigned, callback_t> handlers_ {};
};

template<typename S, typename U>
class server
{
public:
    typedef layout<S, U> memory_t;
    typedef std::function<void(internal::command_data&, void *)> callback_t;
public:
    server(std::string name);
    ~server();

    void update();
    bool is_client_dead(unsigned id);
    void remove_dead_clients();
    
    void setup_message_handler(unsigned type, callback_t callback);
    bool has_new_commands() const;
    void process_commands();
    void send_message(const char *data_small, unsigned target, unsigned type, const void* payload, unsigned payload_length);
protected:
    std::string name_ { "" };
    unsigned last_command_ { 0 };
    std::shared_ptr<shmutex::shmutex> mutex_;
    std::shared_ptr<CatMemoryPool> pool_;
    memory_t *memory { nullptr };
    callback_t general_handler_ { nullptr };
    std::unordered_map<unsigned, callback_t> handlers_ {};   
};

}