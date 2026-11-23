// Win32 triangle test: drives wgfx_impl_ogl_init.cpp exactly like RenderCore's GLBackend::Create does.
// Window creation mirrors vkew tests/vkTest.cpp (800x600 outer size, CW_USEDEFAULT position).
#include <windows.h>
#include <GL/glew.h>
#include <GL/wglew.h>
#include "wgfx_impl_ogl_init.h"
#include "ogl_init_shim.hpp"
#include "triangle_common.hpp"

static LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    switch (m)
    {
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    case WM_KEYDOWN:
        if (w == VK_ESCAPE)
            PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(h, m, w, l);
}

int main(int argc, char** argv)
{
    tri::Report r;
    if (!tri::ParseArgs(argc, argv, r.args))
    {
        fprintf(stderr, "usage: ogl_init_triangle [--gl M.m] [--fb linear|srgb|hdr] [--msaa N] [--debug] "
                        "[--allow-legacy] [--echo] [--batch]\n"
                        "  default: window loop until closed (Esc quits); --batch: one frame, verify, JSON, exit\n");
        return 2;
    }
    ogl_init_test::SetEcho(r.args.echo);

    WNDCLASSA wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = "OglInitTriangle";
    RegisterClassA(&wc);
    HWND hwnd = CreateWindowExA(0, wc.lpszClassName, tri::kWindowTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                                CW_USEDEFAULT, tri::kWindowWidth, tri::kWindowHeight, nullptr, nullptr,
                                wc.hInstance, nullptr);
    if (!hwnd)
    {
        r.error = "CreateWindow failed";
        tri::PrintJson(r);
        return 1;
    }
    ShowWindow(hwnd, SW_SHOW);

    // Same construction as GLBackend::Create (gfx_backend_ogl.hpp).
    wglewContext ctx = {};
    ctx.colorBits = 32;
    ctx.depthBits = 0;
    ctx.sampleCount = r.args.msaa;
    ctx.fbFormat = r.args.fb == tri::Fb::Srgb  ? WGLEW_FRAMEBUFFER_sRGB
                   : r.args.fb == tri::Fb::Hdr ? WGLEW_FRAMEBUFFER_HDR
                                               : WGLEW_FRAMEBUFFER_LINEAR;
    ctx.majorVersion = r.args.major;
    ctx.minorVersion = r.args.minor;
    ctx.debugBits = r.args.debug ? 1 : 0;
#ifdef OGL_INIT_HAS_ALLOW_LEGACY
    ctx.allowLegacyFallback = r.args.allowLegacy ? 1 : 0;
#endif

    HDC hdc = GetDC(hwnd);
    ogl_init_test::ResetCounters();
    r.initRet = wglewInitContext(&ctx);
    HGLRC rcx = wglewCreateContext(&ctx, hdc);
    r.contextCreated = rcx != nullptr;
    r.supportsCompatibility = ctx.supportsCompatibility;
    auto& c = ogl_init_test::GetCounters();
    r.setPixelFormatCalls = c.setPixelFormat;
    r.setPixelFormatFailed = c.setPixelFormatFailed;
    r.legacyCreateCalls = c.createContext;
    r.log = ogl_init_test::GetLog();

    int rcode = 1;
    if (rcx)
    {
        wglMakeCurrent(hdc, rcx);
        glewExperimental = GL_TRUE;
        glewInit();
        glGetError(); // glewInit leaves GL_INVALID_ENUM on core profiles
        if (r.args.debug)
            wglewInitDebug(&ctx);
        tri::QueryInfo(r);

        tri::Triangle triangle;
        if (triangle.Init(r))
        {
            RECT cr;
            GetClientRect(hwnd, &cr);
            triangle.Draw(cr.right, cr.bottom);
            triangle.Verify(r, cr.right, cr.bottom);
            SwapBuffers(hdc);
            rcode = r.pixelPass ? 0 : 1;
            tri::PrintJson(r);

            if (!r.args.batch)
            {
                // Window loop like vkew's RunMessageLoop: render until closed (Esc or the close button).
                MSG msg = {};
                bool run = true;
                while (run)
                {
                    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
                    {
                        if (msg.message == WM_QUIT)
                            run = false;
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                    GetClientRect(hwnd, &cr);
                    if (cr.right > 0 && cr.bottom > 0)
                    {
                        triangle.Draw(cr.right, cr.bottom);
                        SwapBuffers(hdc);
                    }
                }
            }
            triangle.Shutdown();
        }
        else
        {
            tri::PrintJson(r);
        }
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(rcx);
    }
    else
    {
        r.error = "context creation failed (last error " + std::to_string(GetLastError()) + ")";
        tri::PrintJson(r);
    }
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);
    return rcode;
}
