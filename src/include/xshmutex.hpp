#pragma once

#include "platform.hpp"

#if defined(WINDOWS)
#include <Windows.h>
#endif

// Cross-Platform Shared Mutex

namespace xshmutex
{

class xshmutex
{
public:
    static constexpr unsigned max_name_length = 64;
#ifdef __linux__
    struct linux_xshmutex_data
    {
        int fd;
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
    xshmutex(std::string name, bool owner);
    ~xshmutex();
    
    void lock();
    void unlock();

protected:
    void _init();
    void _destroy();
protected:
    const std::string name_ {};
    bool is_owner_ { false };
    WIN32_ONLY(HANDLE handle_ { INVALID_HANDLE_VALUE });
    LINUX_ONLY(linux_xshmutex_data data_);
};

}
