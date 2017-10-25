#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>

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
    uint8_t  free;
    uint32_t process;
    XMEMBER(
    /* WIN32 */ FILETIME start_time,
    /* LINUX */ uint32_t start_time
    );
};

struct command_data
{
    uint32_t number;
    uint32_t sender;
    uint32_t target;
    uint32_t type;
    uint32_t payload_offset;
    uint32_t payload_length;
    uint8_t  data[short_command_length];
};

}

template<typename S, typename U>
struct layout
{
    static_assert(std::is_pod<S>::value, "Server data struct must be POD");
    static_assert(std::is_pod<U>::value, "Custom data struct must be POD");

    constexpr static uint32_t magic_ideal = 0xca75ca75;
    constexpr static uint32_t magic2_ideal = 0x0deadca7;

    // 0xca75ca75
    uint32_t magic;
    uint32_t size_server_data;
    uint32_t size_client_data;

    uint32_t client_count;
    uint32_t command_count;
    
    internal::client_data  clients[max_clients];
    internal::command_data commands[max_commands];
    
    uint8_t pool[pool_size];
    
    S user_server_data;
    U user_client_data[max_clients];

    // 0x0deadca7
    uint32_t magic2;
};

template<typename S, typename U>
class shared
{
public:
    typedef layout<S, U> memory_t;
    typedef std::function<void(internal::command_data&, const uint8_t *)> callback_t;
    // Special ID used by ghost (read-only) clients
    // When in `target': command can be handled by every client
    constexpr static uint32_t ghost_id  = unsigned(-1);
    // Special ID used by server
    constexpr static uint32_t server_id = unsigned(-2);
    // Special ID used by write-only clients
    constexpr static uint32_t writer_id = unsigned(-3);
public:
    inline shared(bool owner, std::string server)
        : server_name_(server), shmem_(server, owner, sizeof(memory_t)), mutex_(server, owner)
    {
    }
    inline void setup_general_handler(callback_t callback)
    {
        general_handler_ = callback;
    }
    inline bool has_new_commands() const
    {
        return last_command_ != memory_->command_count;
    }
    void setup_specialized_handler(callback_t callback, unsigned type)
    {
        if (specialized_handlers_.find(type) != specialized_handlers_.end())
        {
            throw std::logic_error("single command type can't have multiple callbacks");
        }
        specialized_handlers_.emplace(type, callback);
    }
    uint32_t client_count() const
    {
        return memory_->client_count;
    }
    memory_t *memory() const
    {
        return memory_;
    }
    void process_new_commands();
    void send_message(uint32_t target, uint32_t type, const uint8_t *data_small, uint32_t data_small_length, const uint8_t* payload, uint32_t payload_length);
protected:
    const std::string server_name_ { "" };
    uint32_t last_command_ { 0 };
    uint32_t id_ { ghost_id };

    callback_t general_handler_ { nullptr };
    std::unordered_map<unsigned, callback_t> specialized_handlers_ {};

    CatMemoryPool pool_;
    memory_t *memory_;
    xshmem::xshmem shmem_;
    xshmutex::xshmutex mutex_;
};

template<typename S, typename U>
class client : public shared<S, U>
{
public:
    enum class client_type
    {
        normal,
        ghost,
        writer
    };
public:
    client(std::string name, client_type type);
    ~client();
    
    bool     connect();
    uint32_t free_slot() const;
    bool     is_connected() const
    {
        return connected_;
    }
    // Overrides
    void send_message(uint32_t target, uint32_t type, const uint8_t *data_small, uint32_t data_small_length, const uint8_t* payload, uint32_t payload_length);
    void process_new_commands();
    U *client_data() const
    {
        return &this->memory_->user_client_data[this->id_];
    }
protected:
    void _store_data();
    bool _check_memory();
protected:
    const client_type type_;
    bool connected_ { false };
    internal::client_data *client_data_;
};

template<typename S, typename U>
class server : public shared<S, U>
{
public:
    server(std::string name);
    ~server();

    void update();
public:
    void _init();
    void _destroy();
    void _store_magic_data();
    void _remove_dead_clients();
};

void _PLATFORM_ xstoredata(internal::client_data& data);
bool _PLATFORM_ xcheckdead(internal::client_data& data);

}

#include "iipc_impl.hpp"
