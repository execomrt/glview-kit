#include "RenderingTester.h"
#include <Windows.h>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <stdexcept>
#include <cstdio>

#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    bool noPause = false;
    bool vulkan = false;
    bool both = true;
    std::string resource;
    for (int i = 1; i < __argc; ++i) {
        const std::string_view arg = __argv[i];
        if (arg == "--no-pause") noPause = true;
        else if (arg == "--opengl") { vulkan = false; both = false; }
        else if (arg == "--vulkan13") { vulkan = true; both = false; }
        else if (arg == "--rmx" && i + 1 < __argc) resource = __argv[++i];
        else {
            MessageBoxA(nullptr, "Usage: GLViewWin32Project [--opengl | --vulkan13] [--no-pause] [--rmx path]",
                "GLVIEW Kit", MB_OK | MB_ICONINFORMATION);
            return 2;
        }
    }
    AllocConsole();
    FILE* stream = nullptr;
    freopen_s(&stream, noPause ? "glviewkit.log" : "CONOUT$", "w", stdout);
    freopen_s(&stream, noPause ? "glviewkit-error.log" : "CONOUT$", "w", stderr);
    if (!noPause) freopen_s(&stream, "CONIN$", "r", stdin);
    int status = 0;
    try {
        wchar_t executable[32768] = {};
        if (!GetModuleFileNameW(nullptr, executable, 32768)) throw std::runtime_error("Cannot locate executable");
        const auto directory = std::filesystem::path(executable).parent_path();
        if (resource.empty()) resource = (directory / "GLVIEW.RMX").string();
        RenderTest::RenderingTester tester(directory.string(), resource);
        if (!tester.Initialize()) throw std::runtime_error(tester.GetLastError());
        std::cout << "CPU: " << tester.GetCPUInfo() << '\n';
        auto run = [&](RenderTest::RendererType renderer) {
            auto config = RenderTest::RenderingTester::CreateDefaultConfig();
            config.renderer = renderer;
            config.scene = RenderTest::SceneType::SingleCube;
            config.width = 640; config.height = 480;
            config.fullscreen = false; config.vsync = true;
            config.testDurationSeconds = 5;
            const auto result = tester.RunSingleTest(config);
            std::cout << RenderTest::Utils::RendererTypeToString(renderer) << ": "
                << (result.passed ? "passed" : result.errorMessage)
                << " (" << result.averageFPS << " FPS)\n";
            if (!result.passed) status = 1;
        };
        if (both || !vulkan) run(RenderTest::RendererType::OpenGL_3_2);
        if (both || vulkan) run(RenderTest::RendererType::Vulkan_1_3);
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        status = 1;
    }
    if (!noPause) { std::cout << "Press Enter to exit..."; std::cin.get(); }
    return status;
}
