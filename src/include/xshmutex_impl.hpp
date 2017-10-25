#include "xshmutex.hpp"

#include <stdexcept>
#include <iostream>

#if defined(__linux__)

#   include <unistd.h>
#   include <errno.h>
#   include <fcntl.h>
#   include <sys/stat.h>
#   include <assert.h>

namespace xshmutex
{

void xshmutex::_open()
{
    if (open_mode_ & force_create)
    {
        _init();
    }
    std::string fifo_name = "/tmp/.xshmutex_fifo_" + name_;
    data_.fd = open(fifo_name.c_str(), O_RDWR);
    if (data_.fd < 0)
    {
        if ((open_mode_ & open_create) && errno == ENOENT)
        {
            _init();        
        }
        throw std::runtime_error("could not init xshmutex: fifo open error " + std::to_string(errno));
    }
}

void xshmutex::_close()
{
    close(data_.fd);
}

void xshmutex::_init()
{
    std::string fifo_name = "/tmp/.xshmutex_fifo_" + name_;
    // Delete FIFO before proceeding
    unlink(fifo_name.c_str());
    int omask = umask(0);
    if (mkfifo(fifo_name.c_str(), 0666) == -1)
    {
        throw std::runtime_error("could not init xshmutex: fifo error " + std::to_string(errno));
    }
    data_.fd = open(fifo_name.c_str(), O_RDWR);
    if (data_.fd < 0)
    {
        throw std::runtime_error("could not init xshmutex: fifo open error " + std::to_string(errno));
    }
    unlock();
    umask(omask);
}

void xshmutex::_destroy()
{
    std::string fifo_name = "/tmp/.xshmutex_fifo_" + name_;
    unlink(fifo_name.c_str());
}

void xshmutex::lock()
{
    char buf[1];
    while (1 != read(data_.fd, buf, 1))
    {
        usleep(10000);
    }
}

void xshmutex::unlock()
{
    while (1 != write(data_.fd, "1", 1))
    {
        usleep(10000);
    }
}

}

#elif defined(WIN32)

#   include <Windows.h>

namespace xshmutex
{

void xshmutex::_open()
{
    // Open Windows Mutex
    std::string mtx_name = "Global\\xshmutex_" + name_;
    handle_ = CreateMutexA(NULL, FALSE, mtx_name.c_str());
    if (handle_ == NULL)
    {
        throw std::runtime_error("xshmutex: failed to create mutex");
    }
    
}

void xshmutex::_close()
{
    // Close Windows Mutex
    CloseHandle(handle_);
}

void xshmutex::_init()
{
    // Create Windows Mutex
    _open();
}

void xshmutex::_destroy()
{
    // Destroy Windows Mutex
    CloseHandle(handle_);
}

void xshmutex::lock()
{
    // Lock Windows Mutex
    WaitForSingleObject(handle_, INFINITE);
}

void xshmutex::unlock()
{
    // Unlock Windows Mutex
    ReleaseMutex(handle_);
}

}

#endif

