/*
 * -----------------------------------------------------------------------------
 *  File:        wgfx_impl_ogl_init.cpp
 *  Project:     RenderCore — unified multi-backend graphics API
 *  Version:     1.0
 *  Date:        2026-11-23
 *
 *  Description:
 *      Win32 WGL context and pixel-format initialisation for the OpenGL backend:
 *      creates a temporary context to load WGL extensions, chooses an ARB/EXT/legacy
 *      pixel format (linear, sRGB or HDR, optional MSAA), then creates the final
 *      core context via wglCreateContextAttribsARB. This is the same OpenGL init
 *      used by GLView.
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
#ifndef NOMINMAX
#define NOMINMAX // windows.h min/max macros break std::max
#endif
#include <algorithm>
#include <windows.h>
#include "GL/glew.h"
#include "GL/wglew.h"
#include "wgfx_impl_ogl_init.h"
#include "gfx_debug.hpp"
#pragma comment(lib, "opengl32.lib")
static void CALLBACK _wglewCallbackDebug(GLenum source, GLenum type, GLuint id, GLenum severity,
                                         GLsizei length, const GLchar* message, const void* userParam)
{
    
    if (type == GL_DEBUG_TYPE_PERFORMANCE || type == GL_DEBUG_TYPE_OTHER)
        return;

    (void)userParam;
    (void)length;
    LogV("message: %s", message);
    LogV("source: %d", source);
    LogV("\ttype: ");
    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
        LogV("\tERROR");
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        LogV("\tDEPRECATED_BEHAVIOR");
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        LogV("\tUNDEFINED_BEHAVIOR");
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        LogV("\tPORTABILITY");
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        LogV("\tPERFORMANCE");
        break;
    case GL_DEBUG_TYPE_OTHER:
        LogV("\tOTHER");
        break;
    }
    LogV("\t\tid: %d", id);
    LogV("\t\tseverity: ");
    switch (severity)
    {
    case GL_DEBUG_SEVERITY_LOW:
        LogV("LOW");
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        LogV("MEDIUM");
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        LogV("HIGH");
        break;
    default:
        LogV("OTHER");
        break;
    }
    LogV("---------------------opengl-callback-end--------------");
}

static void CALLBACK _wglewCallbackDebugAMD(GLuint id, GLenum category, GLenum severity, GLsizei length,
                                            const GLchar* message, GLvoid* userParam)
{
    _wglewCallbackDebug(0, 0, 0, 0, length, message, userParam);
}

static int wglewChoosePixelFormat(wglewContext* context, HDC hDC)
{
    GLint iCompatiblesFormats[32] = {0};

    {
        int tryCount = 0;
#define ADD_ATTRIBi(a, b) iAttrbList[iAttrbCount++] = a; iAttrbList[iAttrbCount++] = b;
#define ADD_ATTRIBf(a, b) fAttrbList[iAttrbCount++] = a; fAttrbList[iAttrbCount++] = b;
        GLfloat fAttrbList[32] = {0};
        GLint iAttrbList[32] = {0};

        for (;;)
        {
            GLint iAttrbCount = 0;
            int sampleCountMax = 0;
            int iNumPixelFormats = 0;


            if (WGLEW_ARB_pixel_format)
            {
                GLint values[2];
                iAttrbList[0] = WGL_NUMBER_PIXEL_FORMATS_ARB;
                iAttrbList[1] = 0;
                wglGetPixelFormatAttribivARB(hDC, 0, 0, 1, iAttrbList, values);
                iNumPixelFormats = values[0];
                for (int i = 0; i < iNumPixelFormats; i++)
                {
                    iAttrbList[0] = WGL_SAMPLES_ARB;
                    iAttrbList[1] = 0;
                    wglGetPixelFormatAttribivARB(hDC, i + 1, 0, 1, iAttrbList, values);
                    sampleCountMax = std::max(sampleCountMax, values[0]);
                }
            }
            GLuint numFormats = 0;
            if (WGLEW_ARB_pixel_format && tryCount < 5)
            {
                ADD_ATTRIBi(WGL_SUPPORT_OPENGL_ARB, GL_TRUE);
                ADD_ATTRIBi(WGL_DRAW_TO_WINDOW_ARB, GL_TRUE);
                // ADD_ATTRIBi(WGL_DOUBLE_BUFFER_ARB, GL_TRUE);
                ADD_ATTRIBi(WGL_ACCELERATION_ARB,
                            context->noAcceleration ? WGL_NO_ACCELERATION_ARB : WGL_FULL_ACCELERATION_ARB);
                if (context->fbFormat == WGLEW_FRAMEBUFFER_HDR)
                {
                    ADD_ATTRIBi(WGL_COLOR_BITS_ARB, 16);
                }
                else
                {
                    ADD_ATTRIBi(WGL_COLOR_BITS_ARB,
                                context->colorBits);
                }
                ADD_ATTRIBi(WGL_DEPTH_BITS_ARB, context->depthBits);
                if (context->stencilBits != 0)
                {
                    ADD_ATTRIBi(WGL_STENCIL_BITS_ARB, context->stencilBits);
                }
                if (context->stereo != 0)
                {
                    ADD_ATTRIBi(WGL_STEREO_ARB, GL_TRUE);
                }

                if (context->sampleCount > 1 && context->sampleCount <= sampleCountMax)
                {
                    ADD_ATTRIBi(WGL_SAMPLE_BUFFERS_ARB, GL_TRUE);
                    ADD_ATTRIBi(WGL_SAMPLES_ARB, context->sampleCount);
                }
                else
                {
                    ADD_ATTRIBi(WGL_SAMPLE_BUFFERS_ARB, GL_FALSE);
                    ADD_ATTRIBi(WGL_SAMPLES_ARB, 1);
                }
                if (context->fbFormat == WGLEW_FRAMEBUFFER_HDR && WGLEW_ARB_pixel_format_float)
                {
                    ADD_ATTRIBi(WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_FLOAT_ARB);
                }
                else
                {
                    ADD_ATTRIBi(WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB);
                    if (context->fbFormat == WGLEW_FRAMEBUFFER_sRGB && WGLEW_ARB_framebuffer_sRGB)
                    {
                        ADD_ATTRIBi(WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE);
                    }
                    else if (context->fbFormat == WGLEW_FRAMEBUFFER_sRGB && WGLEW_EXT_colorspace)
                    {
                        ADD_ATTRIBi(WGL_COLORSPACE_EXT, WGL_COLORSPACE_SRGB_EXT);
                    }
                }
                if (wglChoosePixelFormatARB(hDC,
                                            iAttrbList,
                                            fAttrbList,
                                            1,
                                            iCompatiblesFormats,
                                            &numFormats) == GL_TRUE && numFormats > 0)
                {
                    break;
                }
            }
            else if (WGLEW_EXT_pixel_format)
            {
                ADD_ATTRIBi(WGL_DOUBLE_BUFFER_EXT, GL_TRUE);
                ADD_ATTRIBi(WGL_DRAW_TO_WINDOW_EXT, GL_TRUE);
                ADD_ATTRIBi(WGL_ACCELERATION_EXT,
                            context->noAcceleration ? WGL_NO_ACCELERATION_EXT : WGL_ACCELERATION_EXT);
                ADD_ATTRIBi(WGL_COLOR_BITS_EXT, context->colorBits);
                ADD_ATTRIBi(WGL_DEPTH_BITS_EXT, context->depthBits);
                if (context->sampleCount > 1)
                {
                    ADD_ATTRIBi(WGL_SAMPLE_BUFFERS_EXT, GL_TRUE);
                    ADD_ATTRIBi(WGL_SAMPLES_EXT, context->sampleCount);
                }
                if (wglChoosePixelFormatEXT(hDC,
                                            iAttrbList,
                                            fAttrbList,
                                            1,
                                            iCompatiblesFormats,
                                            &numFormats) == GL_TRUE && numFormats > 0)
                {
                    break;
                }
            }
            else
            {
                // back in the days ...
                PIXELFORMATDESCRIPTOR pfd = // pfd Tells Windows How We Want Things To Be
                {
                    sizeof(PIXELFORMATDESCRIPTOR), // Size Of This Pixel Format Descriptor
                    1, // Version Number
                    PFD_SUPPORT_OPENGL |
                    PFD_DRAW_TO_WINDOW |
                    PFD_GENERIC_ACCELERATED |
                    PFD_DOUBLEBUFFER, // Must Support Double Buffering
                    PFD_TYPE_RGBA, // Request An RGBA Format
                    static_cast<BYTE>(context->colorBits), // Select Our Color Depth
                    0, 0, 0, 0, 0, 0, // Color Bits Ignored
                    0, // No Alpha Buffer
                    0, // Shift Bit Ignored
                    0, // No Accumulation Buffer
                    0, 0, 0, 0, // Accumulation Bits Ignored
                    static_cast<BYTE>(context->depthBits), // 16Bit Z-Buffer (Depth Buffer)
                    static_cast<BYTE>(context->stencilBits), // No Stencil Buffer
                    0, // No Auxiliary Buffer
                    PFD_MAIN_PLANE, // Main Drawing Layer
                    0, // Reserved
                    0, 0, 0 // Layer Masks Ignored
                };
                iCompatiblesFormats[0] = ChoosePixelFormat(hDC, &pfd);
                if (iCompatiblesFormats[0])
                {
                    context->pixelFormatDescriptor = pfd;
                    break;
                }
            }
            if (context->stereo)
            {
                context->stereo = 0;
            }
            else if (context->sampleCount)
            {
                context->sampleCount = 0;
            }
            else if (context->colorBits == 32)
            {
                context->colorBits = 16;
            }
            else if (context->depthBits)
            {
                context->depthBits = 0;
            }
            tryCount++;
            if (tryCount > 10)
            {
                return 0;
            }
        }
    }
    return iCompatiblesFormats[0];
}

int wglewDescribePixelFormat(wglewContext* context, HDC hDC)
{
    DescribePixelFormat(hDC, context->pixelFormatIndex, sizeof(PIXELFORMATDESCRIPTOR), &context->pixelFormatDescriptor);
    return 0;
}


HGLRC wglewCreateContext(wglewContext* context, HDC hDC)
{
    HGLRC ret = nullptr;
     
    if (context->majorVersion >= 3)
    {
        if (!SetPixelFormat(hDC, context->pixelFormatIndex, &context->pixelFormatDescriptor))
        // Are We Able To Set The Pixel Format?
        {
            LogE("SetPixelFormat returned NULL");
            goto onError;
        }
        auto hGLRCTemp = wglCreateContext(hDC);
        if (hGLRCTemp == nullptr)
        {
            LogE("wglCreateContext returned NULL");
            return nullptr;
        }
        wglMakeCurrent(hDC, hGLRCTemp);

        const int attribs_Debug[32] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, context->majorVersion,
            WGL_CONTEXT_MINOR_VERSION_ARB, context->minorVersion,
            WGL_CONTEXT_FLAGS_ARB,
            WGL_CONTEXT_DEBUG_BIT_ARB |
            WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
            context->majorVersion >= 4 ? WGL_CONTEXT_PROFILE_MASK_ARB : 0,
            context->majorVersion >= 4 ? WGL_CONTEXT_CORE_PROFILE_BIT_ARB : 0,
        };
        const int attribs[32] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, context->majorVersion,
            WGL_CONTEXT_MINOR_VERSION_ARB, context->minorVersion,
            WGL_CONTEXT_FLAGS_ARB,
            WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
            WGL_CONTEXT_PROFILE_MASK_ARB,
            WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0
        };

        const int attribsCompat[32] = {
           WGL_CONTEXT_MAJOR_VERSION_ARB, context->majorVersion,
           WGL_CONTEXT_MINOR_VERSION_ARB, context->minorVersion,
           WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB, // Request a core profile

           0
        };

        if (WGLEW_ARB_create_context)
        {
            context->supportsCompatibility = 1;
            ret = wglCreateContextAttribsARB(hDC, nullptr, context->debugBits ? attribs_Debug : attribs);
            if (ret == nullptr)
            {
                DWORD lastError = GetLastError() & 0xFFFF;
                const char* details = "";
                if (lastError == ERROR_INVALID_VERSION_ARB)
                {
                    details = "Invalid Version";
                }
                else if (lastError == ERROR_INVALID_PROFILE_ARB)
                {
                    details = "Invalid Profile";
                }

                context->supportsCompatibility = 0;
                LogE("wglCreateContextAttribsARB returned NULL when trying context %d.%d (Error 0h%x - %s)",
                                   context->majorVersion, context->minorVersion, lastError, details);

                ret = wglCreateContextAttribsARB(hDC, nullptr, attribsCompat);
                if (ret == nullptr)
                {
                    LogE("wglCreateContextAttribsARB also returned NULL with minimal attributes when trying context %d.%d (Error 0h%x)",
                        context->majorVersion, context->minorVersion, GetLastError());
                }
            }
        }
        else
        {
            ret = nullptr;
        }
        wglDeleteContext(hGLRCTemp);
    }
    else
    {
        context->pixelFormatIndex = context->pixelFormatIndex;
    }

    if (ret == nullptr)
    {
        if (!SetPixelFormat(hDC, context->pixelFormatIndex, &context->pixelFormatDescriptor))
        // Are We Able To Set The Pixel Format?
        {
            LogE("SetPixelFormat returns NULL");
            goto onError;
        }
        ret = wglCreateContext(hDC);
    }
    return ret;
onError:
    return nullptr;
}


void wglewInitDebug(wglewContext* context)
{
    if (GLEW_KHR_debug || GLEW_VERSION_4_3)
    {
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(_wglewCallbackDebug, nullptr);
        GLuint unusedIds = 0;
        glDebugMessageControl(
            GL_DONT_CARE,
            GL_DONT_CARE,
            GL_DONT_CARE,
            0,
            &unusedIds,
            true);
    }
    else if (GLEW_ARB_debug_output)
    {
        glDebugMessageCallbackARB(_wglewCallbackDebug, nullptr);
    }
    else if (GLEW_AMD_debug_output)
    {
        glDebugMessageCallbackAMD(_wglewCallbackDebugAMD, nullptr);
    }
}

/// <summary>
/// 
/// </summary>
/// <param name="context"></param>
/// <returns></returns>
int wglewInitContext(wglewContext* context)
{
    // Create dummy windows and class. Access to WGL extensions
    WNDCLASS wcTemp;
    HWND hWndTemp = nullptr;
    HDC hDCTemp = nullptr;
    HGLRC hGLRCTemp;
    int pfdTemp;
    ZeroMemory(&wcTemp, sizeof(wcTemp));
    wcTemp.hInstance = GetModuleHandle(nullptr);
    wcTemp.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcTemp.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    wcTemp.lpfnWndProc = DefWindowProc;
#ifdef UNICODE
    wcTemp.lpszClassName = L"DummyGL";
#else
	wcTemp.lpszClassName = "DummyGL";
#endif
    if (!RegisterClass(&wcTemp))
    {
        UnregisterClass(wcTemp.lpszClassName, wcTemp.hInstance);
        RegisterClass(&wcTemp);
    }
    hWndTemp = CreateWindowEx(0, wcTemp.lpszClassName, wcTemp.lpszClassName,
                              WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, 0, 1, 1,
                              nullptr, nullptr, wcTemp.hInstance, nullptr);
    if (!hWndTemp)
    {
        return -2;
    }
    hDCTemp = GetDC(hWndTemp);
    if (!hWndTemp)
    {
        if (!UnregisterClass(wcTemp.lpszClassName, wcTemp.hInstance))
        {
        }
        DestroyWindow(hWndTemp);
        return -3;
    }
    pfdTemp = ChoosePixelFormat(hDCTemp, &context->pixelFormatDescriptor);
    if (pfdTemp == 0)
    {
        ReleaseDC(hWndTemp, hDCTemp);
        if (!UnregisterClass(wcTemp.lpszClassName, wcTemp.hInstance))
        {
        }
        DestroyWindow(hWndTemp);
        return -4;
    }
    if (!SetPixelFormat(hDCTemp, pfdTemp, &context->pixelFormatDescriptor))
    {
        ReleaseDC(hWndTemp, hDCTemp);
        if (!UnregisterClass(wcTemp.lpszClassName, wcTemp.hInstance))
        {
        }
        DestroyWindow(hWndTemp);
        return -5;
    }
    hGLRCTemp = wglCreateContext(hDCTemp);
    wglMakeCurrent(hDCTemp, hGLRCTemp);
    glewInit();

    context->pixelFormatIndex = wglewChoosePixelFormat(context, hDCTemp);
    if (context->pixelFormatIndex < 0)
    {
        wglDeleteContext(hGLRCTemp);
        ReleaseDC(hWndTemp, hDCTemp);
        if (!UnregisterClass(wcTemp.lpszClassName, wcTemp.hInstance))
        {
        }
        DestroyWindow(hWndTemp);
        return -6;
    }
    wglMakeCurrent(nullptr, nullptr);
    ReleaseDC(hWndTemp, hDCTemp);
    wglDeleteContext(hGLRCTemp);
    if (!UnregisterClass(wcTemp.lpszClassName, wcTemp.hInstance))
    {
    }
    DestroyWindow(hWndTemp);

    return 0;
}
#if 0
// Choose best pixel format
GLint wglCoreChoosePixelFormat(HDC pHdc, GLint* pBpp, GLint* pDepth, GLint* pStencil, GLint* pDbl, GLint* pAcc, GLint* pDib)
{
	GLint wbpp = pBpp == NULL ? -1 : *pBpp;
	GLint wdepth = pDepth == NULL ? 16 : *pDepth;
	GLint wstencil = pStencil == NULL ? 0 : *pDepth;
	GLint wdbl = pDbl == NULL ? -1 : *pDbl;
	GLint wacc = pAcc == NULL ? 1 : *pAcc;
	GLint wdib = pDib == NULL ? -1 : *pDib;
	GLint num;
	GLuint maxqual = 0;
	GLint maxindex = 0;
	GLint max_bpp, max_depth, max_dbl, max_acc, max_dib, max_stencil;
	GLint i;
	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	num = DescribePixelFormat(pHdc, 1, sizeof(pfd), &pfd);
	if (num == 0)
		return 0;
	for (i = 1; i <= num; i++)
	{
		ZeroMemory(&pfd, sizeof(pfd));
		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		DescribePixelFormat(pHdc, i, sizeof(pfd), &pfd);
		{
			GLint bpp = pfd.cColorBits;
			GLint depth = pfd.cDepthBits;
			GLint stencil = pfd.cStencilBits;
			GLboolean pal = !!(pfd.iPixelType == PFD_TYPE_COLORINDEX);
			GLboolean mcd = !!((pfd.dwFlags & PFD_GENERIC_FORMAT) && (pfd.dwFlags & PFD_GENERIC_ACCELERATED));
			GLboolean soft = !!((pfd.dwFlags & PFD_GENERIC_FORMAT) && !(pfd.dwFlags & PFD_GENERIC_ACCELERATED));
			GLboolean icd = (!(pfd.dwFlags & PFD_GENERIC_FORMAT) && !(pfd.dwFlags & PFD_GENERIC_ACCELERATED));
			GLboolean opengl = !!(pfd.dwFlags & PFD_SUPPORT_OPENGL);
			GLboolean window = !!(pfd.dwFlags & PFD_DRAW_TO_WINDOW);
			GLboolean bitmap = !!(pfd.dwFlags & PFD_DRAW_TO_BITMAP);
			GLboolean dbuff = !!(pfd.dwFlags & PFD_DOUBLEBUFFER);
			GLuint q = 0;
			if (opengl && window)
				q += 0x10000;
			if (wdepth == -1 || (wdepth > 0 && depth > 0))
				q += 0x8000;
			if (wstencil == -1 || (wstencil > 0 && stencil > 0))
				q += 0x4000;
			if (wdbl == -1 || (wdbl == 0 && !dbuff) || (wdbl == 1 && dbuff))
				q += 0x2000;
			if (wacc == -1 || (wacc == 0 && soft) || (wacc == 1 && (mcd || icd)))
				q += 0x1000;
			if (mcd || icd)
				q += 0x0040;
			if (icd)
				q += 0x0002;
			if (wbpp == -1 || (wbpp == bpp))
				q += 0x0800;
			if ((wdib == -1) || (wdib == 0 && !bitmap) || (wdbl == 1 && bitmap))
				q += 0x10000;
			if (bpp >= 16)
				q += 0x0020;
			if (bpp == 16)
				q += 0x0008;
			if (wdepth == -1 || (wdepth == depth))
				q += 0x0400;
			if (depth >= 16)
				q += 0x0010;
			if (depth == 16)
				q += 0x0004;
			if (!pal)
				q += 0x0080;
			if (q > maxqual)
			{
				maxqual = q;
				maxindex = i;
				max_bpp = bpp;
				max_depth = depth;
				max_stencil = stencil;
				max_dbl = dbuff ? 1 : 0;
				max_acc = soft ? 0 : 1;
				max_dib = bitmap ? 1 : 0;
			}
		}
	}
	if (maxindex == 0)
		return maxindex;
	if (pBpp != NULL)
		*pBpp = max_bpp;
	if (pDepth != NULL)
		*pDepth = max_depth;
	if (pStencil != NULL)
		*pStencil = max_stencil;
	if (pDbl != NULL)
		*pDbl = max_dbl;
	if (pAcc != NULL)
		*pAcc = max_acc;
	if (pDib != NULL)
		*pDib = max_dib;
	return maxindex;
}
#endif
