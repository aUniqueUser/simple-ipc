#include "xshmutex.hpp"

#include <random>

const char alphanum_list[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

namespace xshmutex
{

xshmutex::xshmutex(xshmutex::shared_data *data_)
{
    shared_data_ = data;
}

xshmutex::~xshmutex()
{
}

void xshmutex::init()
{
    char random_name[17];
    random_name[16] = '\0';
    memset(shared_data_->mutex_name, 0, xshmutex::max_name_length);
    std::random_device rd;
    std::mt19937 rnd(rd());
    std::uniform_int_distribution<int> distr(0, sizeof(alphanum_list) - 1);
    for (int i = 0; i < 16; i++)
    {
        random_name[i] = alphanum_list[distr(rnd)];
    }
    snprintf(shared_data_->mutex_name, xshmutex::max_name_length, "xshmutex_a_%s", random_name);
}

void xshmutex::destroy()
{
}

void xshmutex::lock()
{
    if (handle_ == INVALID_HANDLE_VALUE)
    {
        connect();
    }

    
}

void xshmutex::unlock()
{
    if (handle_ == INVALID_HANDLE_VALUE)
    {
        connect();
    }
}

bool xshmutex::is_locked()
{
    if (handle_ == INVALID_HANDLE_VALUE)
    {
        connect();
    }
}

void xshmutex::connect()
{
    
}

}
