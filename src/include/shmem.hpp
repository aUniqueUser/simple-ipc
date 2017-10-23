#pragma once

#include "platform.hpp"

#include <cstdint>
#include <string>

namespace cat_ipc
{

class shared_memory_interface
{
    virtual void init(std::string name, uint32_t size);
    virtual void connect(std::string name, uint32_t size);
    
    virtual bool exists(const std::string& name);
    virtual void destroy();
    virtual void *get();
};

}