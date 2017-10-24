#pragma once

#include "platform.hpp"

#if defined(WINDOWS)
#include <Windows.h>
#elif defined(__linux__)
#include <pthread.h>
#endif

// Cross-Platform Shared Mutex

namespace xshmutex
{

class xshmutex
{
public:
    static constexpr unsigned max_name_length = 64;
#ifdef __linux__
    struct shared_data
    {
        pthread_mutex_t mutex;
    };
#endif
    class guard
    {
    public:
        guard(const guard&) = delete;
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
    xshmutex(bool owner);
    ~xshmutex();
    
    void lock();
    void unlock();

    XMEMBER(
    /* WIN32 */ void set_mutex_name(std::string name),
    /* LINUX */ void connect_shared(shared_data *data)
    );
protected:
    void init();
    void destroy();
protected:
    bool is_owner_ { false };
    XMEMBER(
    /* WIN32 */ HANDLE handle_ {INVALID_HANDLE_VALUE},
    /* LINUX */ shared_data *shared_data_ { nullptr }
    );
};

}
