#include "../xshmutex.hpp"
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

namespace xshmutex
{

xshmutex::xshmutex(std::string name, bool owner)
    : name_(name), is_owner_(owner)
{
}

xshmutex::~xshmutex()
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
    data_.fd = open(fifo_name.c_str(), O_RDWR, O_CREAT | O_TRUNC);
    write(data_.fd, "1", 1);
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
    read(data_.fd, buf, 1);
}

void xshmutex::unlock()
{
    write(data_.fd, "1", 1);
}

}
