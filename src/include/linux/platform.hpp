#pragma once

#include <cstdint>
#include <pthread.h>

#include "../mutex.hpp"
#include "../shmem.hpp"

namespace cat_ipc
{

typedef uint32_t process_time_t;

struct mutex_data
{
    pthread_mutex_t mtx;
};

class mutex : public mutex_interface 
{
public:
    virtual void init(_PLATFORM_ mutex_data *data) = 0;
    virtual void destroy() = 0;
    virtual void lock() = 0;
    virtual void unlock() = 0;
    virtual bool is_locked() = 0;
protected:
    _PLATFORM_ mutex_data *data_;
};

class shared_memory : public shared_memory_interface
{
protected:
    std::string name_;
};

}