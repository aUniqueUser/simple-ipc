/*
 * platform.cpp
 *
 *  Created on: Oct 24, 2017
 *      Author: nullifiedcat
 */

#include "../iipc.hpp"
#include "util.h"

#include <sys/types.h>

namespace cat_ipc
{

void _PLATFORM_ xstoredata(cat_ipc::internal::client_data& data)
{
    proc_stat_s stat;
    read_stat(getpid(), &stat);
    data.process = getpid();
    data.start_time = stat.starttime;
}

bool _PLATFORM_ xcheckdead(cat_ipc::internal::client_data& data)
{
    proc_stat_s stat;
    if (0 == read_stat(data.process, &stat))
    {
        return true;
    }
    return data.start_time != stat.starttime;
}

}
