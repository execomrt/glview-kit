// Stand-in for RenderCore's gfx_debug.hpp. Included by wgfx_impl_ogl_init.cpp after <windows.h>.
#pragma once
#include "ogl_init_shim.hpp"

#define LogE(...) ::ogl_init_test::Log('E', __VA_ARGS__)
#define LogV(...) ::ogl_init_test::Log('V', __VA_ARGS__)

// Count the calls under test without editing wgfx_impl_ogl_init.cpp.
#define SetPixelFormat ::ogl_init_test::CountedSetPixelFormat
#define wglCreateContext ::ogl_init_test::CountedCreateContext
