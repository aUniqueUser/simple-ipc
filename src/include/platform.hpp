#pragma once

#define _PLATFORM_

#if defined(WINDOWS)
#define WIN32_ONLY(x) x
#define LINUX_ONLY(x)
#define XMEMBER(win32, linux) win32
#include "win32/platform.hpp"
#elif defined(LINUX)
#define WIN32_ONLY(x)
#define LINUX_ONLY(x)
#define XMEMBER(win32, linux) linux
#include "linux/platform.hpp"
#else
#error Unsupported platform
#endif