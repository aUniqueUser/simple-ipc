#pragma once

#include "platform.hpp"

namespace cat_ipc
{

class mutex_interface
{
public:
    virtual void init(_PLATFORM_ mutex_data *data) = 0;
    virtual void destroy() = 0;
    virtual void lock() = 0;
    virtual void unlock() = 0;
    virtual bool is_locked() = 0;
};

class lock_guard
{
public:
    lock_guard(_PLATFORM_ mutex& mtx) : mtx_(mtx)
    {
        mtx.lock();
    }
    ~lock_guard()
    {
        mtx.unlock();
    }
protected:
    _PLATFORM_ mutex& mtx_;
};

}