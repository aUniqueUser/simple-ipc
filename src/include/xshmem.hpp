#pragma once

#include "platform.hpp"

#ifdef WIN32
#   include <Windows.h>
#endif

#include <string>
#include <cstdint>

// Cross-Platform Shared Memory

namespace xshmem
{

constexpr uint32_t open_create = (1 << 0);
constexpr uint32_t delete_on_close = (1 << 1);

class xshmem
{
public:
    xshmem(std::string name, uint32_t mode, size_t size)
        : open_mode_(mode), name_(name), size_(size)
    {
        _open();
    }
    ~xshmem()
    {
        if (data_ != nullptr)
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
    }

    void *get() const
    {
        return data_;
    }
protected:
    void _open();
    void _close();
    void _init();
    void _destroy();
protected:
    void *data_ { nullptr };
    const uint32_t open_mode_;
    const std::string name_;
    const size_t size_;
    WIN32_ONLY(HANDLE handle_);
};

}

#include "xshmem_impl.hpp"
