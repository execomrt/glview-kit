cmake_minimum_required(VERSION 3.21)
project(GLViewKit VERSION 7.4.4 LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "GLViewKit currently supports Windows only")
endif()

add_library(RenderingTester STATIC src/RenderingTester.cpp src/RenderingTester.h)
add_library(GLViewKit::RenderingTester ALIAS RenderingTester)
target_compile_features(RenderingTester PUBLIC cxx_std_20)
target_include_directories(RenderingTester PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src" "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(RenderingTester PUBLIC opengl32 Shcore Comctl32)

# Retain the existing sample target name for consumers of the old kit.
add_executable(GLViewWin32Project WIN32 src/Main.cpp)
target_link_libraries(GLViewWin32Project PRIVATE GLViewKit::RenderingTester)
set_target_properties(GLViewWin32Project PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:GLViewWin32Project>")
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT GLViewWin32Project)

set(GLVIEW_RUNTIME_DIR "" CACHE PATH "Directory containing a matching GLVIEW runtime")
if(GLVIEW_RUNTIME_DIR)
    foreach(required IN ITEMS infogl.dll GLVIEW.RMX)
        if(NOT EXISTS "${GLVIEW_RUNTIME_DIR}/${required}")
            message(FATAL_ERROR "GLVIEW_RUNTIME_DIR is missing ${required}")
        endif()
    endforeach()
    # Explicit allowlist: never copy logs, backups, or arbitrary executables.
    set(runtime_files infogl.dll GLVIEW.RMX sentry-native.dll crashpad_handler.exe
        amd_ags_x64.dll amd_ags_x86.dll V3X.dll cpudatabase.xml gldatabase.db
        vkdatabase.db glcore.xml Localizable.strings AppFeatures.json)
    add_custom_target(glviewkit_runtime ALL
        COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:GLViewWin32Project>")
    foreach(file IN LISTS runtime_files)
        if(EXISTS "${GLVIEW_RUNTIME_DIR}/${file}")
            add_custom_command(TARGET glviewkit_runtime POST_BUILD
                COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                    "${GLVIEW_RUNTIME_DIR}/${file}" "$<TARGET_FILE_DIR:GLViewWin32Project>"
                VERBATIM)
        endif()
    endforeach()
    add_dependencies(GLViewWin32Project glviewkit_runtime)
else()
    message(STATUS "Runtime staging disabled; provide infogl.dll and GLVIEW.RMX before running")
endif()

