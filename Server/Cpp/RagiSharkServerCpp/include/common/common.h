#pragma once

#if defined(_WINDOWS) || defined(_WIN32) || defined(_WIN64)

#define RAGII_WINDOWS

#define _WIN32_WINNT 0x0A00
#include <sdkddkver.h>

#define NOMINMAX
#include <Windows.h>

#endif
