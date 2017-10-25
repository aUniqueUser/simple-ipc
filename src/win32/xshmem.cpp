#include "xshmem.hpp"

#include <Windows.h>

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
