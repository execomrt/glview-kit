# ogl_init: the WGL context/pixel-format init used by GLView (and RenderCore's OpenGL backend),
# isolated from RenderCore so it can be tested on its own. wgfx_impl_ogl_init.cpp is a copy of
# RenderCore src/backends/ogl/Win32/wgfx_impl_ogl_init.cpp; fixes are made here, then copied back.
# Targets: Win32 -> ogl_init (static), ogl_init_triangle. Linux -> ogl_init_triangle (GLX, shared code only).
include(FetchContent)

set(glew-cmake_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(glew-cmake_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(glew-cmake_BUILD_MULTI_CONTEXT OFF CACHE BOOL "" FORCE)
set(ONLY_LIBS ON CACHE BOOL "" FORCE)
FetchContent_Declare(glew_cmake
    GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
    GIT_TAG        glew-cmake-2.3.0
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(glew_cmake)

set(OGL_INIT_COMMON harness/triangle_common.cpp harness/triangle_common.hpp)

if(WIN32)
    add_library(ogl_init STATIC
        wgfx_impl_ogl_init.cpp wgfx_impl_ogl_init.h
        shim/ogl_init_shim.cpp shim/ogl_init_shim.hpp shim/gfx_debug.hpp)
    target_compile_features(ogl_init PUBLIC cxx_std_20)
    # shim/ resolves "gfx_debug.hpp" to the test shim, not RenderCore's.
    target_include_directories(ogl_init PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/shim")
    target_link_libraries(ogl_init PUBLIC libglew_static opengl32)

    add_executable(ogl_init_triangle harness/triangle_win32.cpp ${OGL_INIT_COMMON})
    target_link_libraries(ogl_init_triangle PRIVATE ogl_init)
    target_include_directories(ogl_init_triangle PRIVATE harness)
else()
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED)
    add_executable(ogl_init_triangle harness/triangle_linux.cpp ${OGL_INIT_COMMON})
    target_compile_features(ogl_init_triangle PRIVATE cxx_std_20)
    target_include_directories(ogl_init_triangle PRIVATE harness ${X11_INCLUDE_DIR})
    target_link_libraries(ogl_init_triangle PRIVATE libglew_static OpenGL::GL ${X11_LIBRARIES})
endif()

# Visual Studio solution folders.
foreach(t IN ITEMS ogl_init ogl_init_triangle)
    if(TARGET ${t})
        set_target_properties(${t} PROPERTIES FOLDER "OpenGL Init")
    endif()
endforeach()
foreach(t IN ITEMS libglew_static libglew_shared libglewmx_static libglewmx_shared)
    if(TARGET ${t})
        set_target_properties(${t} PROPERTIES FOLDER "Third-Party")
    endif()
endforeach()
