/****************************************************************************
; *
; * 	File		:	oevTest.cpp
; *
; * 	Description :	OpenGL Rendering Tests
; *
; * 	Copyright (C) Realtech VR 2000 - 2022 - https://www.realtech-vr.com/glview
; *
; * 	Permission to use, copy, modify, distribute and sell this software
; * 	and its documentation for any purpose is hereby granted without fee,
; * 	provided that the above copyright notice appear in all copies and
; * 	that both that copyright notice and this permission notice appear
; * 	in supporting documentation.  Realtech VR makes no representations
; * 	about the suitability of this software for any purpose.
; * 	It is provided "as is" without express or implied warranty.
; *
; ***************************************************************************/
// OpenGL Extensions Viewer headers.
#include <string>
#include <Windows.h>
#include <ShellScalingAPI.h>
#include <VersionHelpers.h>
#include <CommCtrl.h>
#pragma comment(lib, "Shcore.lib")
#pragma comment(lib, "Comctl32.lib")
#pragma comment(linker, \
	"\"/manifestdependency:type='Win32' "\
	"name='Microsoft.Windows.Common-Controls' "\
	"version='6.0.0.0' "\
	"processorArchitecture='*' "\
	"publicKeyToken='6595b64144ccf1df' "\
	"language='*'\"")
#include "include/oevSDK.h"
using namespace std;
class DebugLog
{
public:
	char szTextBuffer[4096];
	void v(const char* format, ...)
	{
		va_list argptr;
		va_start(argptr, format);
		vsnprintf(szTextBuffer, sizeof(szTextBuffer), format, argptr);
		va_end(argptr);
		OutputDebugStringA("[INFO] ");
		OutputDebugStringA(szTextBuffer);
		OutputDebugStringA("\n");
	}
	void e(const char* format, ...)
	{
		va_list argptr;
		va_start(argptr, format);
		vsnprintf(szTextBuffer, sizeof(szTextBuffer), format, argptr);
		va_end(argptr);
		OutputDebugStringA("[ERROR] ");
		OutputDebugStringA(szTextBuffer);
		OutputDebugStringA("\n");
	}
};
static DebugLog Log;
/// <summary>
/// 
/// </summary>
/// <param name="width"></param>
/// <param name="height"></param>
/// <returns></returns>
static int GetDisplayMode(int width, int height)
{
	int n = 0;
	DEVMODE  devmode = { 0 };
	devmode.dmSize = sizeof(DEVMODE);
	for (;;)
	{
		while (EnumDisplaySettings(nullptr, n, &devmode))
		{
			if (width == devmode.dmPelsWidth && height == devmode.dmPelsHeight)
			{
				return n;
			}
			n++;
		}
	}
	return -1;
}
/// <summary>
/// 
/// </summary>
/// <param name="name"></param>
/// <param name="value"></param>
/// <returns></returns>
static inline std::string CreateElement(const std::string& name, const std::string& value)
{
	return "<" + name + ">" + value + "</" + name + ">\n";
}
/// <summary>
/// 
/// </summary>
/// <param name="renderer"></param>
/// <param name="fullscreen"></param>
/// <param name="width"></param>
/// <param name="height"></param>
/// <param name="vsync"></param>
/// <param name="fog"></param>
/// <param name="transparency"></param>
/// <param name="user_clip_pane"></param>
/// <param name="multisampling"></param>
/// <param name="maxAnisotropy"></param>
/// <param name="texture_load"></param>
/// <param name="pixel_format"></param>
/// <param name="scene"></param>
/// <param name="fbformat"></param>
/// <param name="test_duration"></param>
/// <returns></returns>
static int run_rendering_test(ovRenderer renderer,
	int debug,
	int fullscreen,
	int width,
	int height,
	int vsync,
	int fog,
	int transparency,
	int user_clip_pane,
	const string& multisampling,
	const string& maxAnisotropy,
	const string& texture_load,
	const string& pixel_format,
	const string& scene,
	const string& fbformat,
	int test_duration)
{
	std::string szXml;
	int option = 0;
	if (fullscreen) {
		option |= (WGLDIAG_OPTION_FS | WGLDIAG_OPTIONS_FS_EX);
	}
	if (debug) {
		option |= (WGLDIAG_OPTION_DEBUG);
	}
	if (vsync) {
		option |= (WGLDIAG_OPTION_VSYNC);
	}
	if (fog) {
		option |= (WGLDIAG_OPTION_FOG);
	}
	if (transparency) {
		option |= (WGLDIAG_OPTION_TRANSPARENCY);
	}
	if (user_clip_pane) {
		option |= (WGLDIAG_OPTION_CLIP_PLANE);
	}
	szXml = szXml + CreateElement("option", std::to_string(option));
	szXml = szXml + CreateElement("fbformat", fbformat);
	szXml = szXml + CreateElement("duration", std::to_string(test_duration));
	szXml = szXml + CreateElement("multisample", multisampling);
	szXml = szXml + CreateElement("anisotropy", maxAnisotropy);
	szXml = szXml + CreateElement("texturelod", texture_load);
	szXml = szXml + CreateElement("displaymode", std::to_string(GetDisplayMode(width, height)));
	szXml = szXml + CreateElement("renderer", std::to_string(renderer));
	szXml = szXml + CreateElement("pixelformat", pixel_format);
	if (renderer == RENDERER_GDI || renderer == RENDERER_GL2_0)
	{
		szXml = szXml + CreateElement("test", "1.1;1.2;1.3;1.5;2.0");
	}
	else if (renderer >= RENDERER_GL3_0 && renderer <= RENDERER_GL4_6)
	{
		szXml = szXml + CreateElement("test", "3.0;3.1;3.2;3.3;4.0;4.1;4.2;4.3;4.4;4.5");
	}
	else if (renderer >= RENDERER_VK1_0)
	{
		szXml = szXml + CreateElement("test", "1.0");
	}
	szXml = szXml + CreateElement("fbenable", "Default");
	szXml = szXml + CreateElement("fbformat", fbformat);
	szXml = szXml + CreateElement("scene", scene);
	szXml = szXml + CreateElement("width", std::to_string(width));
	szXml = szXml + CreateElement("height", std::to_string(height));
	// Enable 
	if (IsWindows8OrGreater()) {
		auto monitor = MonitorFromWindow(GetActiveWindow(), MONITOR_DEFAULTTONEAREST);
		auto x = unsigned{};
		auto y = unsigned{};
		GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &x, &y);
		SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
	}
	szXml = CreateElement("root", szXml);
	//	szXml = "<root><option>1061920</option><duration>20</duration><sampleCount>8</sampleCount><maxAnisotropy>0</maxAnisotropy><texturelod>0</texturelod><displaymode>48</displaymode><renderer>12</renderer><pixelformat>1</pixelformat><test>4.2;4.3;4.4;4.5</tet><fbenable>Simple</fbenable><fbformat>RGB</fbformat><scene>0</scene><width>1920</width><height>1080</height></root>";
	//	szXml = "<root><test>3.0;3.1;3.2;3.3;4.0;4.1;4.2;4.3;4.4;4.5</test><option>0</option><maxAnisotropy>0</maxAnisotropy><sampleCount>0</sampleCount><texturelod>0</texturelod><renderer>12</renderer><pixelformat>1</pixelformat><fbenable>Simple</fbenable><duration>6</duration><scene>0</scene><fbformat>RGB</fbformat><displaymode>72</displaymode></root>";
	//	szXml = "<root><test>4.2;4.3;4.4;4.5</test><option>0</option><maxAnisotropy>0</maxAnisotropy><sampleCount>8</sampleCount><texturelod>0</texturelod><renderer>12</renderer><pixelformat>1</pixelformat><fbenable>Simple</fbenable><duration>6</duration><scene>0</scene><fbformat>RGB</fbformat><displaymode>72</displaymode><width>1920</width><height>1080</height></root>";
	//	szXml = "<root><test>3.0;3.1;3.2;3.3;4.0;4.1;4.2;4.3;4.4;4.5</test><option>1048576</option><maxAnisotropy>0</maxAnisotropy><sampleCount>0</sampleCount><texturelod>0</texturelod><renderer>12</renderer><pixelformat>1</pixelformat><fbenable>Simple</fbenable><duration>6</duration><scene>0</scene><fbformat>RGB</fbformat><displaymode>72</displaymode><width>1920</width><height>1080</height></root>";
	//	szXml = "<root><test>3.0;3.1;3.2;3.3;4.0;4.1;4.2;4.3;4.4;4.5</test><option>1061920</option><maxAnisotropy>0</maxAnisotropy><sampleCount>0</sampleCount><texturelod>0</texturelod><renderer>12</renderer><pixelformat>1</pixelformat><fbenable>Simple</fbenable><duration>6</duration><scene>0</scene><fbformat>RGB</fbformat><displaymode>72</displaymode><width>1920</width><height>1080</height></root>";
	//	szXml = "<root><option>1061920</option><duration>20</duration><sampleCount>8</sampleCount><maxAnisotropy>0</maxAnisotropy><texturelod>0</texturelod><displaymode>48</displaymode><renderer>12</renderer><pixelformat>1</pixelformat><test>3.0;3.1;3.2;3.3;4.04.1;4.2;4.3;4.4;4.5</test><fbenable>Simple</fbenable><fbformat>RGB</fbformat><scene>0</scene><width>1920</width><height>1080</height></root>";
	//	szXml = "<root><test>3.0;3.1;3.2;3.3;4.0;4.1;4.2;4.3;4.4;4.5</test><option>1061920</option><maxAnisotropy>0</maxAnisotropy><sampleCount>0</sampleCount><texturelod>0</texturelod><renderer>12</renderer><pixelformat>1</pixelformat><fbenable>Simple</fbenable><duration>6</duration><scene>0</scene><fbformat>RGB</fbformat><displaymode>72</displaymode><width>1920</width><height>1080</height></root>";
	auto library = LoadLibraryA("infogl.dll");
	if (library == NULL) {
		Log.e("Missing DLL infogl.dll");
		return -1;
	}
	auto funcInitWad = (PFNOEVINITWAD)GetProcAddress(library, "oevInitWad");
	auto funcCpuid = (PFNOEVREADCPUID)GetProcAddress(library, "oevReadCpuid");
	auto funcRunRenderingTests = (PNFNOEVRUNRENDERINGTESTS)GetProcAddress(library, "oevRunRenderingTests");
	struct gvCpuid processorInfo;
	funcCpuid(&processorInfo);
	Log.v("Starting test %s", processorInfo.Specification);
	if (funcInitWad("GLVIEW.RMX") >= 0)
	{
		Log.v("Payload: %s", szXml.c_str());
		auto lpResult = funcRunRenderingTests(szXml.c_str());
		while (lpResult)
		{
			if (!strcmp(lpResult->result, "OK"))
			{
				Log.v("Test '%d' passed, avg: %g fps.", lpResult->index, lpResult->fps);
			}
			else
			{
				Log.e("Test '%d' failed", lpResult->index);
			}
			lpResult = lpResult->next;
		}
		return 0;
	}
	else
	{
		Log.e("Missing package GLVIEW.RMX");
		return -2;
	}
}
/// <summary>
/// 
/// </summary>
/// <param name="hInstance"></param>
/// <param name="hPrevInstance"></param>
/// <param name="lpCmdLine"></param>
/// <param name="nCmdShow"></param>
/// <returns></returns>
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if (IsWindows8OrGreater()) {
		auto monitor = MonitorFromWindow(GetActiveWindow(), MONITOR_DEFAULTTONEAREST);
		auto x = unsigned{};
		auto y = unsigned{};
		GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &x, &y);
		SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
	}
	auto library = LoadLibraryA("infogl.dll");
	if (library == NULL) {
		return -3;
	}
	PFNOEVINITWAD funcInitWad = (PFNOEVINITWAD)GetProcAddress(library, "oevInitWad");
	PNFNOEVRUNRENDERINGTESTS funcRunRenderingTests = (PNFNOEVRUNRENDERINGTESTS)GetProcAddress(library, "oevRunRenderingTests");
	if (funcInitWad("GLVIEW.RMX") >= 0)
	{
		// @Note: Choose renderer from here
		//auto renderer = RENDERER_GDI;
		//auto renderer = RENDERER_GL2_0; 
		//auto renderer = RENDERER_GL4_6;
		auto renderer = RENDERER_VK1_2;
		return run_rendering_test(renderer,
			FALSE, // Debug
			FALSE, // fullscreen
			1920,
			1080,
			0, // vsync
			0, // fog
			0, // transp
			0, // user clip plane			
			"8", // multisampling
			"16", // maxAnisotropy
			"0", // texture_LOD
			"1", // pixel_format
			"0", // scene
			"sRGB",  // fbformat : "Linear", "sRGB", "HDR"
			20 // duration
		);
	}
	else
	{
		return -1;
	}
}
