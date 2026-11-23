// Linux (X11/GLX) triangle test. Same drawing/readback/JSON as the Win32 harness, so the shared code is
// verified on Mesa/NVIDIA Linux. wgfx_impl_ogl_init.cpp is WGL-only and is NOT exercised here.
// UNTESTED: written without a Linux host to build on.
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glew.h>
#include <GL/glx.h>
#include <cstdio>
#include "triangle_common.hpp"

typedef GLXContext (*CreateContextAttribsFn)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

int main(int argc, char** argv)
{
    tri::Report r;
    if (!tri::ParseArgs(argc, argv, r.args))
    {
        fprintf(stderr, "usage: ogl_init_triangle [--gl M.m] [--msaa N] [--debug] [--batch]\n"
                        "  default: window loop until closed; --batch: one frame, verify, JSON, exit\n");
        return 2;
    }
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy)
    {
        r.error = "XOpenDisplay failed (no DISPLAY; try xvfb-run)";
        tri::PrintJson(r);
        return 1;
    }
    int attrs[] = {GLX_X_RENDERABLE, True, GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT, GLX_RENDER_TYPE, GLX_RGBA_BIT,
                   GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8, GLX_ALPHA_SIZE, 8,
                   GLX_DOUBLEBUFFER, True, GLX_SAMPLE_BUFFERS, r.args.msaa > 1 ? 1 : 0,
                   GLX_SAMPLES, r.args.msaa > 1 ? r.args.msaa : 0, None};
    int n = 0;
    GLXFBConfig* cfgs = glXChooseFBConfig(dpy, DefaultScreen(dpy), attrs, &n);
    if (!cfgs || n == 0)
    {
        r.error = "glXChooseFBConfig returned nothing";
        tri::PrintJson(r);
        return 1;
    }
    GLXFBConfig cfg = cfgs[0];
    XFree(cfgs);
    XVisualInfo* vi = glXGetVisualFromFBConfig(dpy, cfg);
    XSetWindowAttributes swa = {};
    swa.colormap = XCreateColormap(dpy, RootWindow(dpy, vi->screen), vi->visual, AllocNone);
    swa.event_mask = StructureNotifyMask | KeyPressMask;
    Window win = XCreateWindow(dpy, RootWindow(dpy, vi->screen), 0, 0, tri::kWindowWidth, tri::kWindowHeight, 0,
                               vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    XFree(vi);
    XStoreName(dpy, win, tri::kWindowTitle);
    Atom wmDelete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpy, win, &wmDelete, 1);
    XMapWindow(dpy, win);

    auto createAttribs = (CreateContextAttribsFn)glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");
    GLXContext ctx = nullptr;
    if (createAttribs)
    {
        int ca[] = {GLX_CONTEXT_MAJOR_VERSION_ARB, r.args.major, GLX_CONTEXT_MINOR_VERSION_ARB, r.args.minor,
                    GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
                    GLX_CONTEXT_FLAGS_ARB, r.args.debug ? GLX_CONTEXT_DEBUG_BIT_ARB : 0, None};
        ctx = createAttribs(dpy, cfg, nullptr, True, ca);
    }
    r.contextCreated = ctx != nullptr;
    int rcode = 1;
    if (ctx)
    {
        glXMakeCurrent(dpy, win, ctx);
        glewExperimental = GL_TRUE;
        glewInit();
        glGetError();
        tri::QueryInfo(r);
        tri::Triangle triangle;
        if (triangle.Init(r))
        {
            XWindowAttributes wa;
            XGetWindowAttributes(dpy, win, &wa);
            triangle.Draw(wa.width, wa.height);
            triangle.Verify(r, wa.width, wa.height);
            glXSwapBuffers(dpy, win);
            rcode = r.pixelPass ? 0 : 1;
            tri::PrintJson(r);
            for (bool run = !r.args.batch; run;)
            {
                while (XPending(dpy))
                {
                    XEvent e;
                    XNextEvent(dpy, &e);
                    if (e.type == ClientMessage && (Atom)e.xclient.data.l[0] == wmDelete)
                        run = false;
                    if (e.type == KeyPress && XLookupKeysym(&e.xkey, 0) == XK_Escape)
                        run = false;
                }
                XGetWindowAttributes(dpy, win, &wa);
                triangle.Draw(wa.width, wa.height);
                glXSwapBuffers(dpy, win);
            }
            triangle.Shutdown();
        }
        else
        {
            tri::PrintJson(r);
        }
        glXMakeCurrent(dpy, None, nullptr);
        glXDestroyContext(dpy, ctx);
    }
    else
    {
        r.error = "glXCreateContextAttribsARB failed or missing";
        tri::PrintJson(r);
    }
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
    return rcode;
}
