#include "xshmem.hpp"

#include <stdexcept>
#include <iostream>

#if defined(__linux__)

#   include <fcntl.h>
#   include <string.h>
#   include <sys/mman.h>
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <unistd.h>

namespace xshmem
{

void xshmem::_init()
{
    std::string shm_name = "xshmem" + name_;
    std::cout << "xshmem: initializing shared memory\n";
    int omask = umask(0);
    int fd = shm_open(shm_name.c_str(), O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
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

void xshmem::_destroy()
{
    munmap(data_, size_);
    shm_unlink(name_.c_str());
}

void xshmem::_open()
{
    std::string shm_name = "xshmem" + name_;
    int omask = umask(0);
    int fd = shm_open(shm_name.c_str(), O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
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

void xshmem::_close()
{
    munmap(data_, size_);
}

}

#elif defined(WIN32)

#   include <Windows.h>

namespace xshmem
{

void xshmem::_init()
{
    // Init Windows Shared Memory
    handle_ = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size_, ("Global\\xshmem_" + name_).c_str());
    if (handle_ == NULL)
    {
        throw std::runtime_error("xshmem: could not create file mapping");
    }
    data_ = (uint8_t *) MapViewOfFile(handle_, FILE_MAP_ALL_ACCESS, 0, 0, size_);
    if (data_ == NULL)
    {
        throw std::runtime_error("xshmem: could not map view of file");
    }
}

void xshmem::_destroy()
{
    // Destroy Windows Shared Memory
    _close();
}

void xshmem::_open()
{
    // Open Windows Shared Memory
    // must set data_
    handle_ = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, ("Global\\xshmem_" + name_).c_str());
    if (handle_ == NULL)
    {
        throw std::runtime_error("xshmem: could not open file mapping");
    }
    data_ = (uint8_t *) MapViewOfFile(handle_, FILE_MAP_ALL_ACCESS, 0, 0, size_);
    if (data_ == NULL)
    {
        throw std::runtime_error("xshmem: could not map view of file");
    }
}

void xshmem::_close()
{
    // Close Windows Shared Memory
    UnmapViewOfFile(data_);
    CloseHandle(handle_);
}

}

#endif
