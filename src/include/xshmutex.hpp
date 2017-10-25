#pragma once

#include "platform.hpp"

#include <string>
#include <cstdint>

#if defined(WINDOWS)
#include <Windows.h>
#endif

// Cross-Platform Shared Mutex

namespace xshmutex
{

constexpr uint32_t open_create = (1 << 0);
constexpr uint32_t delete_on_close = (1 << 1);
constexpr uint32_t force_create = (1 << 2);

class xshmutex
{
public:
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
    xshmutex(std::string name, uint32_t mode)
        : name_(name), open_mode_(mode)
    {
        _open();
    }
    ~xshmutex()
    {
        if (open_mode_ & delete_on_close)
        {
            _destroy();
        }
        else
        {
            _close();
        }
    }
    
    void lock();
    void unlock();
protected:
    void _open();
    void _close();
    void _init();
    void _destroy();
protected:
    const std::string name_ {};
    const uint32_t open_mode_ { 0 };
    WIN32_ONLY(HANDLE handle_ { INVALID_HANDLE_VALUE });
    LINUX_ONLY(linux_xshmutex_data data_);
};

}

#include "xshmutex_impl.hpp"