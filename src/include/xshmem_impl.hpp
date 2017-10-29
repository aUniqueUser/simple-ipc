#include "xshmem.hpp"

#include <stdexcept>

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
}

void xshmem::_destroy()
{
    munmap(data_, size_);
    shm_unlink(name_.c_str());
}

void xshmem::_open()
{
    std::string shm_name = "xshmem_" + name_;
    int omask = umask(0);
    int flags = O_RDWR;
    if (open_mode_ & open_create)
    {
        flags |= O_CREAT;
    }
    int fd = shm_open(shm_name.c_str(), flags, S_IRWXU | S_IRWXG | S_IRWXO);
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
#	include <Sddl.h>

namespace xshmem
{

void xshmem::_init()
{
    // Init Windows Shared Memory
	SECURITY_ATTRIBUTES security;
	ZeroMemory(&security, sizeof(security));
	security.nLength = sizeof(security);
	ConvertStringSecurityDescriptorToSecurityDescriptorA(
			"D:P(A;OICI;GA;;;SY)(A;OICI;GA;;;BA)(A;OICI;GA;;;IU)",
        SDDL_REVISION_1,
        &security.lpSecurityDescriptor,
        NULL);
    handle_ = CreateFileMappingA(INVALID_HANDLE_VALUE, &security, PAGE_READWRITE, 0, size_, ("Global\\xshmem_" + name_).c_str());
    if (handle_ == NULL)
    {
        throw std::runtime_error("xshmem: could not create file mapping: " + std::to_string(GetLastError()));
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
	if (open_mode_ & open_create)
	{
		_init();
		return;
	}
    // Open Windows Shared Memory
    // must set data_
    handle_ = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, ("Global\\xshmem_" + name_).c_str());
    if (handle_ == NULL)
    {
        throw std::runtime_error("xshmem: could not open file mapping: " + std::to_string(GetLastError()));
    }
    data_ = (uint8_t *) MapViewOfFile(handle_, FILE_MAP_ALL_ACCESS, 0, 0, size_);
    if (data_ == NULL)
    {
        throw std::runtime_error("xshmem: could not map view of file: " + std::to_string(GetLastError()));
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
