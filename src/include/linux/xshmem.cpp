#include "../xshmem.hpp"

#include <fcntl.h>
#include <sys/mman.h>

namespace xshmem
{

xshmem::xshmem(std::string name, unsigned length)
{
    name_ = name;
    size_ = length;
}

xshmem::~xshmem()
{
    
}

int xshmem::init()
{
    int omask = umask(0);
    int fd = shm_open(name.c_str(), O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd < 0)
    {
        return -1;
    }
	ftruncate(fd, size_);
	umask(omask);
	data_ = mmap(0, size_, PROT_WRITE | PROT_READ | PROT_EXEC, MAP_SHARED, fd, 0);
    memset(data_, 0, size_);
	close(fd);   
    return 0;
}

void xshmem::destroy()
{
    shm_unlink(name_.c_str());
	munmap(data_, size_);
}

int xshmem::connect()
{
    int omask = umask(0);
    int fd = shm_open(name.c_str(), O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd < 0)
    {
        return -1;
    }
	ftruncate(fd, size_);
	umask(omask);
	data_ = mmap(0, size_, PROT_WRITE | PROT_READ | PROT_EXEC, MAP_SHARED, fd, 0);
	close(fd);
    return 0;
}

void *xshmem::get()
{
    return data_;
}

}