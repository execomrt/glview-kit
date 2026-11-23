#pragma once
/*
 * -----------------------------------------------------------------------------
 *  File:        wgfx_impl_ogl_init.h
 *  Project:     RenderCore — unified multi-backend graphics API
 *  Version:     1.0
 *  Date:        2026-11-23
 *
 *  Description:
 *      Declarations for the Win32 WGL init: wglewContext (requested framebuffer
 *      format, GL version, debug flags) and the context/pixel-format entry points.
 *      Same OpenGL init used by GLView.
 *
 *  Author:      Stéphane Denis
 *  Copyright:   (c) 2026 Stéphane Denis - www.v3x.net
 *
 *  License:     MIT License
 *
 *  Portions adapted from GLEW (The OpenGL Extension Wrangler Library):
 *      Copyright (C) 2008-2015 Nigel Stewart, 2002-2008 Milan Ikits,
 *      2002-2008 Marcelo E. Magallon, 2002 Lev Povalahev. Modified BSD / MIT.
 */


enum wglewFrameBufferFormat
{
    WGLEW_FRAMEBUFFER_LINEAR,
    WGLEW_FRAMEBUFFER_sRGB,
    WGLEW_FRAMEBUFFER_HDR
};

typedef struct wglewContext
{
    PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
    int colorBits;
    int depthBits;
    int stencilBits;
    int sampleCount;
    int stereo;
    enum wglewFrameBufferFormat fbFormat;
    int noAcceleration;
    int majorVersion;
    int minorVersion;
    int debugBits;
    int pixelFormatIndex;
    int fastInit;
    int supportsCompatibility;
}wglewContext;

#ifdef __cplusplus
extern "C" {
#endif

int wglewDescribePixelFormat(wglewContext* context, HDC hDC);
HGLRC wglewCreateContext(wglewContext* context, HDC hDC);
int wglewInitContext(wglewContext* context);
void wglewInitDebug(wglewContext* context);

#ifdef __cplusplus
}
#endif
