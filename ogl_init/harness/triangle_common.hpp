// Platform-neutral part of the triangle test: argument parsing, drawing, readback, JSON report.
// The platform file (triangle_win32.cpp / triangle_linux.cpp) creates the context and calls into this.
// The triangle matches vkew's Vulkan test (tests/vkTestGraphics.cpp): red top, green right, blue left,
// clear color (0.2, 0.3, 0.4), 800x600 window, +Y up.
#pragma once
#include <string>
#include <vector>

namespace tri
{
constexpr int kWindowWidth = 800;
constexpr int kWindowHeight = 600;
constexpr const char* kWindowTitle = "OGL Init Triangle Test";

enum class Fb { Linear, Srgb, Hdr };

struct Args
{
    int major = 4, minor = 4;
    Fb fb = Fb::Linear;
    int msaa = 0;
    bool debug = false;
    bool echo = false;        // forward init logs to stderr
    bool allowLegacy = false; // opt-in legacy wglCreateContext fallback (Win32 only)
    bool batch = false;       // draw one frame, verify, print JSON, exit (default: window loop until closed)
};

struct Report
{
    Args args;
    int initRet = 0;           // wglewInitContext return code (Win32)
    bool contextCreated = false;
    int setPixelFormatCalls = -1;
    int setPixelFormatFailed = -1;
    int legacyCreateCalls = -1;
    int supportsCompatibility = -1;
    std::string glVersion, glRenderer, glVendor;
    int profileMask = 0;       // GL_CONTEXT_PROFILE_MASK
    int gotMajor = 0, gotMinor = 0;
    bool legacyFallback = false; // got a compatibility context although core was requested
    bool versionMismatch = false;
    float pixel[4] = {0, 0, 0, 0};  // window centre, expected ~(0.5, 0.25, 0.25)
    float corner[4] = {0, 0, 0, 0}; // near bottom-left corner, expected clear color
    bool pixelPass = false;
    std::string error;
    std::vector<std::string> log;
};

bool ParseArgs(int argc, char** argv, Args& out);
// Needs a current context and glewInit() done. Fills gl strings/version/profile.
void QueryInfo(Report& r);

class Triangle
{
public:
    bool Init(Report& r); // compiles shaders, creates VAO; false + r.error on failure
    void Draw(int w, int h) const;
    void Verify(Report& r, int w, int h) const; // reads back the back buffer after Draw, before swap
    void Shutdown();

private:
    unsigned prog_ = 0, vao_ = 0;
};

// One JSON object on stdout.
void PrintJson(const Report& r);
} // namespace tri
