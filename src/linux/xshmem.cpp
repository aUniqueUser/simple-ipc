#include "xshmem.hpp"

#include <stdexcept>
#include <iostream>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace xshmem
{

void xshmem::init()
{
    std::cout << "xshmem: initializing shared memory\n";
    int omask = umask(0);
    int fd = shm_open(name_.c_str(), O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd < 0)
    {
        throw std::runtime_error("xshmem: could not create shared memory region");
    }
    if (ftruncate(fd, size_) != 0)
    {
        throw std::runtime_error("xshmem: internal error");
    }
    umask(omask);
    data_ = mmap(0, size_, PROT_WRITE | PROT_READ | PROT_EXEC, MAP_SHARED, fd, 0);
    if (data_ == (void *) -1)
    {
        throw std::runtime_error("xshmem: could not map shared memory region");
    }
    memset(data_, 0, size_);
    close(fd);
}

void xshmem::destroy()
{
    shm_unlink(name_.c_str());
    munmap(data_, size_);
}

void xshmem::connect()
{
    int omask = umask(0);
    int fd = shm_open(name_.c_str(), O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd < 0)
    {
        throw std::runtime_error("xshmem: could not open shared memory region");
    }
    if (ftruncate(fd, size_) != 0)
    {
        throw std::runtime_error("xshmem: internal error");
    }
    umask(omask);
    data_ = mmap(0, size_, PROT_WRITE | PROT_READ | PROT_EXEC, MAP_SHARED, fd, 0);
    if (data_ == (void *) -1)
    {
        throw std::runtime_error("xshmem: could not map shared memory region");
    }
    close(fd);
}

}
