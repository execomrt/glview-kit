/****************************************************************************
; *
; * 	File		:	oevSDK.h
; *
; * 	Description :	GLView SDK
; *
; * 	Copyright (C) Realtech VR 2000 - 2020 - http://www.realtech-vr.com/glview
; *
; * 	Permission to use, copy, modify, distribute and sell this software
; * 	and its documentation for any purpose is hereby granted without fee,
; * 	provided that the above copyright notice appear in all copies and
; * 	that both that copyright notice and this permission notice appear
; * 	in supporting documentation.  Realtech makes no representations
; * 	about the suitability of this software for any purpose.
; * 	It is provided "as is" without express or implied warranty.
; *
; ***************************************************************************/
// OpenGL Extensions Viewer headers.
#pragma once

#ifdef _WIN32
#include <Windows.h>

#ifdef INFOGL_EXPORTS
#define _OEV_EXPORTFUNC __declspec(dllexport)
#else
#define _OEV_EXPORTFUNC __declspec(dllimport)
#endif
#else

#define _OEV_EXPORTFUNC 

#endif

struct gvCpuid
{
	char* Name;
	char* CodeName;
	char* Package;
	char* VendorName;
	char* Specification;
	char* Technology;
	char* Instructions;
	int MaxTDP;
	int Stepping;
	int Revision;
	int Model;
	int Family;
	int ExtModel;
	int ExtFamily;
	int Signature;
	int FreqGhz;
	int FreqMhz;

	int RAMb;
	int Cores;
	int Threads;
};

struct gvRenderingTestResult
{
	int structSize;
	int index;
	int duration;
	float fps;
	const char* result;
	struct gvRenderingTestResult* next;
};


// Renderer list
enum ovRenderer
{
	RENDERER_GDI,
	RENDERER_GL2_0,
	RENDERER_GL3_0,
	RENDERER_GL3_1,
	RENDERER_GL3_2,
	RENDERER_GL3_3,
	RENDERER_GL4_0,
	RENDERER_GL4_1,
	RENDERER_GL4_2,
	RENDERER_GL4_3,
	RENDERER_GL4_4,
	RENDERER_GL4_5,
	RENDERER_GL4_6,
	RENDERER_VK1,
	MAX_RENDERER
};

// Options 
#define WGLDIAG_OPTION_ANISO 1
#define WGLDIAG_OPTION_MULTISAMPLE (1UL<<1)
#define WGLDIAG_OPTION_PBUFFER (1UL<<2)
#define WGLDIAG_OPTION_MULTISAMPLEHINT (1UL<<3)
#define WGLDIAG_OPTION_LODBIAS (1UL<<4)
#define WGLDIAG_OPTION_FOG (1UL<<5)
#define WGLDIAG_OPTION_FBO (1UL<<6)
#define WGLDIAG_OPTION_ARGB (1UL<<7)
#define WGLDIAG_OPTION_FB_FLOAT32 (1UL<<8)
#define WGLDIAG_OPTION_CLIP_PLANE (1UL<<10)
#define WGLDIAG_OPTION_FS (1UL<<11)
#define WGLDIAG_OPTION_VSYNC (1UL<<12)
#define WGLDIAG_OPTION_TRANSPARENCY (1UL<<13)
#define WGLDIAG_OPTION_MIPMAP (1UL<<14)
#define WGLDIAG_OPTION_RETINADISPLAY (1UL<<15)
#define WGLDIAG_OPTION_MPENGINE (1UL<<16)
#define WGLDIAG_OPTION_INSTANCING (1UL<<17)
#define WGLDIAG_OPTION_VBUM (1UL<<18)
#define WGLDIAG_OPTION_DEBUG (1UL << 20)


typedef int (*PFNOEVREADCPUID)(struct gvCpuid*);
typedef int (*PFNOEVINITWAD)(const char*);
typedef struct gvRenderingTestResult* (*PNFNOEVRUNRENDERINGTESTS)(const char*);


#ifdef __cplusplus
extern "C" {
#endif
	_OEV_EXPORTFUNC const char* gvReportGetLogFile(void);

	// Get version from renderer
	_OEV_EXPORTFUNC const char* oevDiagGetVersion(enum ovRenderer renderer);

	// Get renderer name
	_OEV_EXPORTFUNC const char* oevDiagGetRendererName(enum ovRenderer renderer);

	// Get vendor name
	_OEV_EXPORTFUNC const char* oevDiagGetVendorName(enum ovRenderer renderer);

	// Check if has extension
	_OEV_EXPORTFUNC int oevDiagHasExtension(enum ovRenderer renderer, const char* szFormat, ...);

	// Rescan viewer information. Must be called before access oevDiagGetVersion
	_OEV_EXPORTFUNC int oevScanRenderer(enum ovRenderer renderer, int debugMode);

	_OEV_EXPORTFUNC struct gvRenderingTestResult* oevRunRenderingTests(const char* szXml);

	// Wad
	_OEV_EXPORTFUNC int oevInitWad(const char* path);

	// Cpuid
	_OEV_EXPORTFUNC void oevReadCpuid(struct gvCpuid* pxSystemCaps);


#ifdef __cplusplus
}
#endif
