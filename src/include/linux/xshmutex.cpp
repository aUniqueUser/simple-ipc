#include "../xshmutex.hpp"

namespace xshmutex
{

xshmutex::xshmutex(bool owner)
    : is_owner_(owner)
{
}

xshmutex::~xshmutex()
{
    if (is_owner_)
    {
        if (shared_data_ != nullptr)
        {
            destroy();
        }
    }
}

void xshmutex::init()
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, 1);
    pthread_mutex_init(&shared_data_->mutex, &attr);
    pthread_mutexattr_destroy(&attr);
}

void xshmutex::destroy()
{
    pthread_mutex_destroy(&shared_data_->mutex);
}

void xshmutex::lock()
{
    pthread_mutex_lock(&shared_data_->mutex);
}

void xshmutex::unlock()
{
    pthread_mutex_unlock(&shared_data_->mutex);
}

void xshmutex::connect_shared(shared_data *data)
{
    shared_data_ = data;
    if (is_owner_)
    {
        init();
    }
}

}
