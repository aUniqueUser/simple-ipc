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
#if defined(WINDOWS)
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
#endif
    
}

void xshmutex::destroy()
{
}

void xshmutex::lock()
{
#if defined(WINDOWS)
    if (!handle_ == INVALID_HANDLE_VALUE)
    {
        connect();
    }
#endif

    
}

void xshmutex::unlock()
{
#if defined(WINDOWS)
    if (!handle_ == INVALID_HANDLE_VALUE)
    {
        connect();
    }
#endif


}

bool xshmutex::is_locked()
{
#if defined(WINDOWS)
    if (!handle_ == INVALID_HANDLE_VALUE)
    {
        connect();
    }
#endif

    
}

#if defined(WINDOWS)
void xshmutex::connect()
{
    
}
#endif

}
