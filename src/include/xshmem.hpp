#pragma once

#include <string>

// Cross-Platform Shared Memory

namespace xshmem
{

class xshmem
{
public:
    xshmem(std::string name, bool owner, size_t size)
        : is_owner_(owner), name_(name), size_(size)
    {
        if (is_owner_)
        {
            _init();
        }
        else
        {
            _open();
        }
    }
    ~xshmem()
    {
        if (data_ != nullptr)
        {
            if (is_owner_)
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
    const bool is_owner_;
    const std::string name_;
    const size_t size_;
};

}
