#pragma once

#include "platform.hpp"

#if defined(WINDOWS)
#include <Windows.h>
#elif defined(LINUX)
#include <pthread.h>
#endif

// Cross-Platform Shared Mutex

namespace xshmutex
{

class xshmutex
{
public:
    constexpr unsigned max_name_length = 64;
#ifdef LINUX
    struct shared_data
    {
        pthread_mutex_t mutex;
    };
#endif
    class guard
    {
    public:
        guard(const lock&) = delete;
        guard(xshmutex& xshm) : xshm_(xshm) 
        {
            xshm_.lock();
        };
        ~guard()
        {
            xshm_.unlock();
        };
    protected:
        xshmutex& xshm_;
    };
public:
    XMEMBER(
    /* WIN32 */ xshmutex(std::string name),
    /* LINUX */ xshmutex(shared_data *data_)
    );
    ~xshmutex();
    
    void init();
    void destroy();
    
    void lock();
    void unlock();
    bool is_locked();
protected:
    shared_data *shared_data_;
    WIN32_ONLY(HANDLE handle_{INVALID_HANDLE_VALUE});
};

}