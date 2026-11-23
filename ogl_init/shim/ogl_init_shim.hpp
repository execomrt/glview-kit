// Test shim for wgfx_impl_ogl_init.cpp: replaces RenderCore's gfx_debug.hpp logging and
// counts the Win32 calls the test matrix cares about. No macros here; see gfx_debug.hpp.
#pragma once
#include <windows.h>
#include <string>
#include <vector>

namespace ogl_init_test
{
struct Counters
{
    int setPixelFormat = 0;
    int setPixelFormatFailed = 0;
    int createContext = 0; // legacy wglCreateContext calls
};

Counters& GetCounters();
void ResetCounters();

// Captured LogE/LogV output (also forwarded to stderr when echo is true).
std::vector<std::string>& GetLog();
void SetEcho(bool echo);
void Log(char level, const char* fmt, ...);

BOOL CountedSetPixelFormat(HDC hdc, int format, const PIXELFORMATDESCRIPTOR* pfd);
HGLRC CountedCreateContext(HDC hdc);
} // namespace ogl_init_test
