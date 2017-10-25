#include "xshmutex.hpp"

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
