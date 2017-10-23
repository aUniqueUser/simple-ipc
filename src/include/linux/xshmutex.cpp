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

bool xshmutex::is_locked()
{
    
}

}
