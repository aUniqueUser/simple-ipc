#include "platform.hpp"

void shared_memory::init(std::string name, uint32_t size)
{
    name_ = name;
}

void shared_memory::connect(std::string name, uint32_t size)
{
    name = name_;
}

void shared_memory::destroy()
{

}

void *shared_memory::get()
{

}

bool shared_memory::exists(const std::string& name)
{
    
}