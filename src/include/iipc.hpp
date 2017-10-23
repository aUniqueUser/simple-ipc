#pragma once

#include <cstdint>

namespace cat_ipc
{

constexpr unsigned max_clients          = 32;
constexpr unsigned max_commands         = 64;
constexpr unsigned pool_size            = 2 * 1024 * 1024; // 2 MB
constexpr unsigned short_command_length = 64;

namespace internal
{

struct client_data
{
    bool free;
    int  process;
    _PLATFORM_ process_time_t start_time;
};

struct command_data
{
    unsigned number;
    unsigned sender;
    unsigned target;
    unsigned user_type;
    uint8_t  data[short_command_length];
    uintptr_t payload_offset;
    uintptr_t payload_length;
};

}


}