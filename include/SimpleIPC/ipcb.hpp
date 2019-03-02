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

#include "util.hpp"
#include "cmp.hpp"

/* This implementation allows up to 32 clients (unsigned int) */
/* very strange mix of P2P and client-server */

namespace cat_ipc
{

constexpr unsigned max_peers      = 32;
constexpr unsigned command_buffer = 64;
constexpr unsigned pool_size      = command_buffer * 4096; // A lot of space.
constexpr unsigned command_data   = 64;                    // Guaranteed space that every command has

struct peer_data_s
{
    bool free;
    pid_t pid;
    unsigned long starttime;
};

struct command_s
{
    unsigned command_number;              // sequentional command number
    unsigned peer_mask;                   // bitfield with peer ID's which should process the message
    unsigned sender;                      // sender ID
    unsigned long payload_offset;         // offset from pool start, points to payload allocated in pool
    unsigned payload_size;                // size of payload
    unsigned cmd_type;                    // stores user-defined command ID
    unsigned char cmd_data[command_data]; // can be used to store command name or just smaller messages instead of pool
};

// S = struct for global data
// U = struct for peer data
template <typename S, typename U> struct ipc_memory_s
{
    static_assert(std::is_pod<S>::value, "Global data struct must be POD");
    static_assert(std::is_pod<U>::value, "Peer data struct must be POD");

    pthread_mutex_t mutex;              // IPC mutex, must be locked every time you access ipc_memory_s
    unsigned peer_count;                // count of alive peers, managed by "manager" (server)
    unsigned long command_count;        // last command number + 1
    peer_data_s peer_data[max_peers];   // state of each peer, managed by server
    command_s commands[command_buffer]; // command buffer, every peer can write/read here
    unsigned char pool[pool_size];      // pool for storing command payloads
    S global_data;                      // some data, struct is defined by user
    U peer_user_data[max_peers];        // some data for each peer, struct is defined by user
};

template <typename S, typename U> class Peer
{
public:
    typedef ipc_memory_s<S, U> memory_t;

    /*
     * name: IPC file name, will be used with shm_open
     * process_old_commands: if false, peer's last_command will be set to actual last command in memory to prevent processing outdated commands
     * manager: there must be only one manager peer in memory, if the peer is manager, it allocates/deallocates shared memory
     */
    Peer(std::string name, bool process_old_commands = true, bool manager = false, bool ghost = false) : name(name), process_old_commands(process_old_commands), is_manager(manager), is_ghost(ghost)
    {
    }

    ~Peer()
    {
        if (is_manager)
        {
            pthread_mutex_destroy(&memory->mutex);
            shm_unlink(name.c_str());
            munmap(memory, sizeof(memory_t));
            return;
        }
        if (is_ghost)
        {
            return;
        }
        if (memory)
        {
            MutexLock lock(this);
            memory->peer_data[client_id].free = true;
        }
    }

    typedef std::function<void(command_s &, void *)> CommandCallbackFn_t;

    // do MutexLock lock(this); in each function where shared memory is accessed
    // when the object goes out of scope, mutex is unlocked
    class MutexLock
    {
    public:
        MutexLock(Peer *parent) : parent(parent)
        {
            pthread_mutex_lock(&parent->memory->mutex);
        }
        ~MutexLock()
        {
            pthread_mutex_unlock(&parent->memory->mutex);
        }

        Peer *parent;
    };

    /*
     * Checks if peer has new commands to process (non-blocking)
     */
    bool HasCommands() const
    {
        return (last_command != memory->command_count);
    }

    /*
     * Actually connects to server
     */
    void Connect()
    {
        connected    = true;
        int old_mask = umask(0);
        int flags    = O_RDWR;
        if (is_manager)
            flags |= O_CREAT;
        int fd = shm_open(name.c_str(), flags, S_IRWXU | S_IRWXG | S_IRWXO);
        if (fd == -1)
        {
            throw std::runtime_error("server isn't running");
        }
        ftruncate(fd, sizeof(memory_t));
        umask(old_mask);
        memory = (memory_t *) mmap(0, sizeof(memory_t), PROT_WRITE | PROT_READ | PROT_EXEC, MAP_SHARED, fd, 0);
        close(fd);
        pool = new CatMemoryPool(&memory->pool, pool_size);
        if (is_manager)
        {
            InitManager();
        }
        if (not is_ghost)
        {
            client_id = FirstAvailableSlot();
            StorePeerData();
        }
        else
        {
            client_id = unsigned(-1);
        }
        if (!process_old_commands)
        {
            last_command = memory->command_count;
        }
    }

    /*
     * Checks every slot in memory->peer_data, throws runtime_error if there are no free slots
     */
    unsigned FirstAvailableSlot()
    {
        MutexLock lock(this);
        for (unsigned i = 0; i < max_peers; i++)
        {
            if (memory->peer_data[i].free)
            {
                return i;
            }
        }
        throw std::runtime_error("no available slots");
    }

    /*
     * Returns true if the slot can be marked free
     */
    bool IsPeerDead(unsigned id) const
    {
        if (memory->peer_data[id].free)
            return true;
        proc_stat_s stat;
        read_stat(memory->peer_data[id].pid, &stat);
        if (stat.starttime != memory->peer_data[id].starttime)
            return true;
        return false;
    }

    /*
     * Should be called only once in a lifetime of ipc instance.
     * this function initializes memory
     */
    void InitManager()
    {
        memset(memory, 0, sizeof(memory_t));
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, 1);
        pthread_mutex_init(&memory->mutex, &attr);
        pthread_mutexattr_destroy(&attr);
        for (unsigned i = 0; i < max_peers; i++)
            memory->peer_data[i].free = true;
        pool->init();
    }

    /*
     * Marks every dead peer as free
     */
    void SweepDead()
    {
        MutexLock lock(this);
        memory->peer_count = 0;
        for (unsigned i = 0; i < max_peers; i++)
        {
            if (IsPeerDead(i))
            {
                memory->peer_data[i].free = true;
            }
            if (!memory->peer_data[i].free)
            {
                memory->peer_count++;
            }
        }
    }

    /*
     * Stores data about this peer in memory
     */
    void StorePeerData()
    {
        if (is_ghost)
        {
            return;
        }
        MutexLock lock(this);
        proc_stat_s stat;
        read_stat(getpid(), &stat);
        memory->peer_data[client_id].free      = false;
        memory->peer_data[client_id].pid       = getpid();
        memory->peer_data[client_id].starttime = stat.starttime;
    }

    /*
     * A callback will be called every time peer gets a message
     * previously named: SetCallback(...)
     */
    void SetGeneralHandler(CommandCallbackFn_t new_callback)
    {
        callback = new_callback;
    }

    /*
     * Set a handler that will be fired when command with specified type will appear
     */
    void SetCommandHandler(unsigned command_type, CommandCallbackFn_t handler)
    {
        if (callback_map.find(command_type) != callback_map.end())
        {
            throw std::logic_error("single command type can't have multiple callbacks (" + std::to_string(command_type) + ")");
        }
        callback_map.emplace(command_type, handler);
    }

    /*
     * Processes every command with command_number higher than this peer's last_command
     */
    void ProcessCommands()
    {
        for (unsigned i = 0; i < command_buffer; i++)
        {
            command_s &cmd = memory->commands[i];
            if (cmd.command_number > last_command)
            {
                last_command = cmd.command_number;
                if (cmd.sender != client_id && (!cmd.peer_mask || ((1 << client_id) & cmd.peer_mask)))
                {
                    if (callback)
                    {
                        callback(cmd, cmd.payload_size ? pool->real_pointer<void>((void *) cmd.payload_offset) : nullptr);
                    }
                    if (callback_map.find(cmd.cmd_type) != callback_map.end())
                    {
                        callback_map[cmd.cmd_type](cmd, cmd.payload_size ? pool->real_pointer<void>((void *) cmd.payload_offset) : nullptr);
                    }
                }
            }
        }
    }

    /*
     * Posts a command to memory, increases command_count
     */
    void SendMessage(const char *data_small, unsigned peer_mask, unsigned command_type, const void *payload, size_t payload_size)
    {
        MutexLock lock(this);
        command_s &cmd = memory->commands[++memory->command_count % command_buffer];
        if (cmd.payload_size)
        {
            pool->free(pool->real_pointer<void>((void *) cmd.payload_offset));
            cmd.payload_offset = 0;
            cmd.payload_size   = 0;
        }
        if (data_small)
            memcpy(cmd.cmd_data, data_small, sizeof(cmd.cmd_data));
        if (payload_size)
        {
            void *block = pool->alloc(payload_size);
            memcpy(block, payload, payload_size);
            cmd.payload_offset = (unsigned long) pool->pool_pointer<void>(block);
            cmd.payload_size   = payload_size;
        }
        cmd.cmd_type       = command_type;
        cmd.sender         = client_id;
        cmd.peer_mask      = peer_mask;
        cmd.command_number = memory->command_count;
    }

    std::unordered_map<unsigned, CommandCallbackFn_t> callback_map{};
    bool connected{ false };
    unsigned client_id{ 0 };
    unsigned long last_command{ 0 };
    CommandCallbackFn_t callback{ nullptr };
    CatMemoryPool *pool{ nullptr };
    const std::string name;
    bool process_old_commands{ true };
    ipc_memory_s<S, U> *memory{ nullptr };
    const bool is_manager{ false };
    const bool is_ghost{ false };
};

} // namespace cat_ipc

#endif /* IPCB_HPP_ */
