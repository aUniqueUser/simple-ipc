#include "iipc.hpp"

#include <Windows.h>

namespace cat_ipc
{

void _PLATFORM_ xstoredata(cat_ipc::internal::client_data& data)
{
    HANDLE current = GetCurrentProcess();
    data.process = GetProcessId(current);
    GetProcessTimes(current, &data.start_time, nullptr, nullptr, nullptr);
}

bool _PLATFORM_ xcheckdead(cat_ipc::internal::client_data& data)
{
    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, current.process);
    if (process == NULL)
    {
        return true;
    }
    FILETIME ft;
    GetProcessTimes(current, &ft, nullptr, nullptr, nullptr);
    return ft.dwLowDateTime == data.start_time.dwLowDateTime &&
           ft.dwHighDateTime == data.start_time.dwHighDateTime;
}

}