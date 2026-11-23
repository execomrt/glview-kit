#include "ogl_init_shim.hpp"
#include <cstdarg>
#include <cstdio>

namespace ogl_init_test
{
static Counters g_counters;
static std::vector<std::string> g_log;
static bool g_echo = false;

Counters& GetCounters() { return g_counters; }
void ResetCounters() { g_counters = Counters{}; }
std::vector<std::string>& GetLog() { return g_log; }
void SetEcho(bool echo) { g_echo = echo; }

void Log(char level, const char* fmt, ...)
{
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    std::string line = std::string(1, level) + ": " + buf;
    g_log.push_back(line);
    if (g_echo)
        fprintf(stderr, "%s\n", line.c_str());
}

BOOL CountedSetPixelFormat(HDC hdc, int format, const PIXELFORMATDESCRIPTOR* pfd)
{
    ++g_counters.setPixelFormat;
    BOOL ok = ::SetPixelFormat(hdc, format, pfd);
    if (!ok)
        ++g_counters.setPixelFormatFailed;
    return ok;
}

HGLRC CountedCreateContext(HDC hdc)
{
    ++g_counters.createContext;
    return ::wglCreateContext(hdc);
}
} // namespace ogl_init_test
