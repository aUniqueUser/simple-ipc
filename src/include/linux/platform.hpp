#pragma once

#include <cstdint>
#include <pthread.h>

#include "../mutex.hpp"

namespace cat_ipc
{

typedef uint32_t process_time_t;

struct mutex_data
{
    pthread_mutex_t mtx;
};

class mutex : public mutex_interface 
{
protected:
    _PLATFORM_ mutex_data *data_;
};

class shared_memory : public shared_memory_interface
{
protected:
    std::string name_;
};

}