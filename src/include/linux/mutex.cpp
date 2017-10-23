#include "../mutex.hpp"

namespace cat_ipc
{

void mutex::init(_PLATFORM_ mutex_data *data)
{
    data_ = data;
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_setpshared(&attr, 1);
	pthread_mutex_init(&data->mtx, &attr);
	pthread_mutexattr_destroy(&attr);
}

void mutex::destroy()
{
    pthread_mutex_destroy(&data_->mtx);   
}

void mutex::lock()
{
    pthread_mutex_lock(&data_->mtx);
}

void mutex::unlock()
{
    pthread_mutex_unlock(&data_->mtx);
}

bool mutex::is_locked()
{
    return false;
}

}