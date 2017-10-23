#pragma once

#define _PLATFORM_

#if defined(WINDOWS)
#include "win32/platform.hpp"
#elif defined(LINUX)
#include "linux/platform.hpp"
#else
#error Unsupported platform
#endif