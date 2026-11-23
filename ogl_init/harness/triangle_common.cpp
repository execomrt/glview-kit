#include "triangle_common.hpp"
#include <GL/glew.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace tri
{
bool ParseArgs(int argc, char** argv, Args& a)
{
    for (int i = 1; i < argc; ++i)
    {
        std::string s = argv[i];
        auto next = [&]() -> const char* { return (i + 1 < argc) ? argv[++i] : ""; };
        if (s == "--gl")
        {
            if (sscanf(next(), "%d.%d", &a.major, &a.minor) != 2)
                return false;
        }
        else if (s == "--fb")
        {
            std::string v = next();
            if (v == "linear") a.fb = Fb::Linear;
            else if (v == "srgb") a.fb = Fb::Srgb;
            else if (v == "hdr") a.fb = Fb::Hdr;
            else return false;
        }
        else if (s == "--msaa") a.msaa = atoi(next());
        else if (s == "--debug") a.debug = true;
        else if (s == "--echo") a.echo = true;
        else if (s == "--allow-legacy") a.allowLegacy = true;
        else if (s == "--batch") a.batch = true;
        else return false;
    }
    return true;
}

void QueryInfo(Report& r)
{
    auto str = [](GLenum e) {
        const GLubyte* p = glGetString(e);
        return std::string(p ? (const char*)p : "");
    };
    r.glVersion = str(GL_VERSION);
    r.glRenderer = str(GL_RENDERER);
    r.glVendor = str(GL_VENDOR);
    GLint maj = 0, min = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &maj);
    glGetIntegerv(GL_MINOR_VERSION, &min);
    if (glGetError() != GL_NO_ERROR || maj == 0) // pre-3.0 context
        sscanf(r.glVersion.c_str(), "%d.%d", &maj, &min);
    r.gotMajor = maj;
    r.gotMinor = min;
    if (maj >= 3)
    {
        GLint mask = 0;
        glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &mask);
        glGetError();
        r.profileMask = mask;
    }
    r.versionMismatch = (maj < r.args.major) || (maj == r.args.major && min < r.args.minor);
    // Core was requested for >= 3.2; a compatibility profile (or no profile bit) means a legacy fallback.
    r.legacyFallback = r.args.major >= 3 && !(r.profileMask & GL_CONTEXT_CORE_PROFILE_BIT);
}

static GLuint Compile(GLenum type, const char* src, std::string& err)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char buf[512];
        glGetShaderInfoLog(s, sizeof(buf), nullptr, buf);
        err += buf;
    }
    return s;
}

bool Triangle::Init(Report& r)
{
    // Same vertices and colors as vkew tests/vkTestGraphics.cpp.
    const char* vs = "#version 330 core\n"
                     "const vec2 pos[3] = vec2[3](vec2(0.0, 0.5), vec2(0.5, -0.5), vec2(-0.5, -0.5));\n"
                     "const vec4 col[3] = vec4[3](vec4(1,0,0,1), vec4(0,1,0,1), vec4(0,0,1,1));\n"
                     "out vec4 vColor;\n"
                     "void main(){ gl_Position = vec4(pos[gl_VertexID], 0.0, 1.0); vColor = col[gl_VertexID]; }\n";
    const char* fs = "#version 330 core\nin vec4 vColor;\nout vec4 c;\nvoid main(){ c = vColor; }\n";
    std::string err;
    GLuint v = Compile(GL_VERTEX_SHADER, vs, err);
    GLuint f = Compile(GL_FRAGMENT_SHADER, fs, err);
    prog_ = glCreateProgram();
    glAttachShader(prog_, v);
    glAttachShader(prog_, f);
    glLinkProgram(prog_);
    GLint linked = 0;
    glGetProgramiv(prog_, GL_LINK_STATUS, &linked);
    glDeleteShader(v);
    glDeleteShader(f);
    if (!linked)
    {
        r.error = "shader build failed: " + err;
        return false;
    }
    glGenVertexArrays(1, &vao_);
    return true;
}

void Triangle::Draw(int w, int h) const
{
    glViewport(0, 0, w, h);
    glClearColor(0.2f, 0.3f, 0.4f, 1.0f); // vkew clear color
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(prog_);
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Triangle::Verify(Report& r, int w, int h) const
{
    glReadBuffer(GL_BACK);
    glReadPixels(w / 2, h / 2, 1, 1, GL_RGBA, GL_FLOAT, r.pixel);
    glReadPixels(4, 4, 1, 1, GL_RGBA, GL_FLOAT, r.corner);
    GLenum e = glGetError();
    if (e != GL_NO_ERROR)
        r.error += " glError=0x" + std::to_string(e);
    auto within = [](float a, float b, float t) { return std::fabs(a - b) <= t; }; // not "near": Win32 macro
    // Centre is the barycentric mid-point of the base-to-apex line: red 0.5, green 0.25, blue 0.25.
    bool centre = within(r.pixel[0], 0.5f, 0.06f) && within(r.pixel[1], 0.25f, 0.06f) && within(r.pixel[2], 0.25f, 0.06f);
    bool bg = within(r.corner[0], 0.2f, 0.02f) && within(r.corner[1], 0.3f, 0.02f) && within(r.corner[2], 0.4f, 0.02f);
    r.pixelPass = centre && bg;
}

void Triangle::Shutdown()
{
    if (vao_) glDeleteVertexArrays(1, &vao_);
    if (prog_) glDeleteProgram(prog_);
    vao_ = prog_ = 0;
}

static std::string Esc(const std::string& s)
{
    std::string o;
    for (char c : s)
    {
        if (c == '"' || c == '\\') { o += '\\'; o += c; }
        else if (c == '\n' || c == '\r' || c == '\t') o += ' ';
        else o += c;
    }
    return o;
}

void PrintJson(const Report& r)
{
    const char* fb[] = {"linear", "srgb", "hdr"};
    printf("{\"req\":\"%d.%d\",\"fb\":\"%s\",\"msaa\":%d,\"debug\":%d,\"allowLegacy\":%d,"
           "\"initRet\":%d,\"context\":%d,\"got\":\"%d.%d\",\"profileMask\":%d,"
           "\"legacyFallback\":%d,\"versionMismatch\":%d,\"supportsCompat\":%d,"
           "\"setPixelFormat\":%d,\"setPixelFormatFailed\":%d,\"legacyCreate\":%d,"
           "\"pixelPass\":%d,\"pixel\":[%.2f,%.2f,%.2f,%.2f],\"corner\":[%.2f,%.2f,%.2f,%.2f],"
           "\"renderer\":\"%s\",\"vendor\":\"%s\",\"version\":\"%s\",\"error\":\"%s\",\"log\":[",
           r.args.major, r.args.minor, fb[(int)r.args.fb], r.args.msaa, r.args.debug, r.args.allowLegacy,
           r.initRet, r.contextCreated, r.gotMajor, r.gotMinor, r.profileMask,
           r.legacyFallback, r.versionMismatch, r.supportsCompatibility,
           r.setPixelFormatCalls, r.setPixelFormatFailed, r.legacyCreateCalls,
           r.pixelPass, r.pixel[0], r.pixel[1], r.pixel[2], r.pixel[3],
           r.corner[0], r.corner[1], r.corner[2], r.corner[3],
           Esc(r.glRenderer).c_str(), Esc(r.glVendor).c_str(), Esc(r.glVersion).c_str(),
           Esc(r.error).c_str());
    for (size_t i = 0; i < r.log.size(); ++i)
        printf("%s\"%s\"", i ? "," : "", Esc(r.log[i]).c_str());
    printf("]}\n");
    fflush(stdout);
}
} // namespace tri
