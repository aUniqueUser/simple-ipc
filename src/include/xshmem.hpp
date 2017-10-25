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
            init();
        }
        else
        {
            connect();
        }
    }
    ~xshmem()
    {
        if (is_owner_ && data_ != nullptr)
        {
            destroy();
        }
    }

    void  connect();
    void *get() const
    {
        return data_;
    }
protected:
    void init();
    void destroy();
protected:
    void *data_ { nullptr };
    const bool is_owner_;
    const std::string name_;
    const size_t size_;
};

}
