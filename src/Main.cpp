/**
 * @file RenderingTester.cpp
 * @brief Implementation of the OpenGL/Vulkan Rendering Performance Tester
 * @version 7.2 (2025-09-18) https://www.realtech-vr.com/glview
 */

#include "RenderingTester.h"
#include <Windows.h>
#include <ShellScalingAPI.h>
#include <VersionHelpers.h>
#include <CommCtrl.h>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <iomanip>

#pragma comment(lib, "Shcore.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "Comctl32.lib")
#pragma comment(linker, \
    "\"/manifestdependency:type='Win32' "\
    "name='Microsoft.Windows.Common-Controls' "\
    "version='6.0.0.0' "\
    "processorArchitecture='*' "\
    "publicKeyToken='6595b64144ccf1df' "\
    "language='*'\"")

using namespace RenderTest;

/**
 * @brief Simple example - Run a basic OpenGL test
 */
void Example1_BasicTest() {
    std::cout << "\n=== Example 1: Basic OpenGL Test ===\n";

    RenderingTester tester;

    if (!tester.Initialize()) {
        std::cout << "Failed to initialize tester: " << tester.GetLastError() << std::endl;
        return;
    }

    std::cout << "CPU: " << tester.GetCPUInfo() << std::endl;

    // Create a simple test configuration
    TestConfig config = RenderingTester::CreateDefaultConfig();
    config.renderer = RendererType::OpenGL_4_5;
    config.scene = SceneType::SingleCube;
    config.testDurationSeconds = 5;

    std::cout << "Running " << Utils::RendererTypeToString(config.renderer)
        << " test with " << Utils::SceneTypeToString(config.scene) << "...\n";

    TestResult result = tester.RunSingleTest(config);

    if (result.passed) {
        std::cout << "Test passed! Average FPS: " << std::fixed << std::setprecision(2)
            << result.averageFPS << std::endl;
    }
    else {
        std::cout << "Test failed: " << result.errorMessage << std::endl;
    }
}

void Example1_VulkanTest() {
    std::cout << "\n=== Example 1: Basic OpenGL Test ===\n";

    RenderingTester tester;

    if (!tester.Initialize()) {
        std::cout << "Failed to initialize tester: " << tester.GetLastError() << std::endl;
        return;
    }

    std::cout << "CPU: " << tester.GetCPUInfo() << std::endl;

    // Create a simple test configuration
    TestConfig config = RenderingTester::CreateDefaultConfig();
    config.renderer = RendererType::Vulkan_1_2;
    config.scene = SceneType::ManyCubes;
    config.testDurationSeconds = 5;

    std::cout << "Running " << Utils::RendererTypeToString(config.renderer)
        << " test with " << Utils::SceneTypeToString(config.scene) << "...\n";

    TestResult result = tester.RunSingleTest(config);

    if (result.passed) {
        std::cout << "Test passed! Average FPS: " << std::fixed << std::setprecision(2)
            << result.averageFPS << std::endl;
    }
    else {
        std::cout << "Test failed: " << result.errorMessage << std::endl;
    }
}


/**
 * @brief Performance comparison between different OpenGL versions
 */
void Example2_OpenGLVersionComparison() {
    std::cout << "\n=== Example 2: OpenGL Version Comparison ===\n";

    RenderingTester tester;
    if (!tester.Initialize()) {
        std::cout << "Failed to initialize tester: " << tester.GetLastError() << std::endl;
        return;
    }

    // Test different OpenGL versions
    std::vector<RendererType> renderers = {
        RendererType::OpenGL_3_3,
        RendererType::OpenGL_4_0,
        RendererType::OpenGL_4_2,
        RendererType::OpenGL_4_5
    };

    std::vector<TestConfig> configs;
    for (auto renderer : renderers) {
        TestConfig config = RenderingTester::CreatePerformanceConfig();
        config.renderer = renderer;
        config.scene = SceneType::ManyCubes;
        config.testDurationSeconds = 10;
        configs.push_back(config);
    }

    // Run tests with progress callback
    auto progressCallback = [](int current, int total, const TestResult& result) {
        std::cout << "Progress: " << current << "/" << total;
        if (result.passed) {
            std::cout << " - Last test: " << result.averageFPS << " FPS";
        }
        std::cout << std::endl;
        };

    TestResults results = tester.RunMultipleTests(configs, progressCallback);

    // Display results
    std::cout << "\nResults Summary:\n";
    std::cout << std::string(50, '-') << std::endl;
    for (size_t i = 0; i < results.size(); ++i) {
        const auto& result = results[i];
        std::cout << std::setw(15) << Utils::RendererTypeToString(renderers[i]) << ": ";
        if (result.passed) {
            std::cout << std::fixed << std::setprecision(2) << result.averageFPS << " FPS";
        }
        else {
            std::cout << "FAILED (" << result.errorMessage << ")";
        }
        std::cout << std::endl;
    }
}

/**
 * @brief Quality settings comparison
 */
void Example3_QualityComparison() {
    std::cout << "\n=== Example 3: Quality Settings Impact ===\n";

    RenderingTester tester;
    if (!tester.Initialize()) {
        std::cout << "Failed to initialize tester: " << tester.GetLastError() << std::endl;
        return;
    }

    struct QualityTest {
        std::string name;
        TestConfig config;
    };

    // Create different quality configurations
    std::vector<QualityTest> qualityTests = {
        {
            "Low Quality",
            []() {
                TestConfig config = RenderingTester::CreatePerformanceConfig();
                config.multisampleCount = 0;
                config.maxAnisotropy = 0;
                config.fog = false;
                config.transparency = false;
                return config;
            }()
        },
        {
            "Medium Quality",
            []() {
                TestConfig config = RenderingTester::CreateDefaultConfig();
                config.multisampleCount = 4;
                config.maxAnisotropy = 4;
                config.fog = true;
                return config;
            }()
        },
        {
            "High Quality",
            []() {
                TestConfig config = RenderingTester::CreateQualityConfig();
                config.multisampleCount = 8;
                config.maxAnisotropy = 16;
                config.fog = true;
                config.transparency = true;
                config.fbFormat = FramebufferFormat::HDR;
                return config;
            }()
        }
    };

    std::cout << "Testing quality impact on " << Utils::SceneTypeToString(SceneType::Character) << " scene...\n";

    for (const auto& test : qualityTests) {
        std::cout << "\nRunning " << test.name << " test...";
        TestResult result = tester.RunSingleTest(test.config);

        if (result.passed) {
            std::cout << " " << std::fixed << std::setprecision(2) << result.averageFPS << " FPS";
        }
        else {
            std::cout << " FAILED: " << result.errorMessage;
        }
    }
    std::cout << std::endl;
}

/**
 * @brief Vulkan vs OpenGL performance comparison
 */
void Example4_VulkanVsOpenGL() {
    std::cout << "\n=== Example 4: Vulkan vs OpenGL Comparison ===\n";

    RenderingTester tester;
    if (!tester.Initialize()) {
        std::cout << "Failed to initialize tester: " << tester.GetLastError() << std::endl;
        return;
    }

    // Create comparable configurations
    std::vector<TestConfig> configs;

    // OpenGL 4.5 test
    TestConfig glConfig = RenderingTester::CreatePerformanceConfig();
    glConfig.renderer = RendererType::OpenGL_4_5;
    glConfig.scene = SceneType::ManyCharacters;
    glConfig.testDurationSeconds = 15;
    configs.push_back(glConfig);

    // Vulkan 1.2 test
    TestConfig vkConfig = glConfig;
    vkConfig.renderer = RendererType::Vulkan_1_2;
    configs.push_back(vkConfig);

    TestResults results = tester.RunMultipleTests(configs);

    std::cout << "\nAPI Performance Comparison:\n";
    std::cout << std::string(30, '=') << std::endl;

    if (results.size() >= 2) {
        if (results[0].passed && results[1].passed) {
            double glFPS = results[0].averageFPS;
            double vkFPS = results[1].averageFPS;
            double improvement = ((vkFPS - glFPS) / glFPS) * 100.0;

            std::cout << "OpenGL 4.5: " << std::fixed << std::setprecision(2) << glFPS << " FPS\n";
            std::cout << "Vulkan 1.2: " << std::fixed << std::setprecision(2) << vkFPS << " FPS\n";
            std::cout << "Improvement: " << std::showpos << std::fixed << std::setprecision(1)
                << improvement << "%" << std::noshowpos << std::endl;
        }
        else {
            std::cout << "One or more tests failed:" << std::endl;
            if (!results[0].passed) std::cout << "OpenGL: " << results[0].errorMessage << std::endl;
            if (!results[1].passed) std::cout << "Vulkan: " << results[1].errorMessage << std::endl;
        }
    }
}

/**
 * @brief Resolution scaling test
 */
void Example5_ResolutionScaling() {
    std::cout << "\n=== Example 5: Resolution Scaling Test ===\n";

    RenderingTester tester;
    if (!tester.Initialize()) {
        std::cout << "Failed to initialize tester: " << tester.GetLastError() << std::endl;
        return;
    }

    struct ResolutionTest {
        int width, height;
        std::string name;
    };

    std::vector<ResolutionTest> resolutions = {
        {1280, 720, "720p"},
        {1920, 1080, "1080p"},
        {2560, 1440, "1440p"},
        {3840, 2160, "4K"}
    };

    TestConfig baseConfig = RenderingTester::CreatePerformanceConfig();
    baseConfig.scene = SceneType::ManyCubes;
    baseConfig.testDurationSeconds = 8;

    std::cout << "Testing resolution impact with " << Utils::RendererTypeToString(baseConfig.renderer) << ":\n";

    for (const auto& res : resolutions) {
        TestConfig config = baseConfig;
        config.width = res.width;
        config.height = res.height;

        std::cout << std::setw(8) << res.name << " (" << res.width << "x" << res.height << "): ";

        TestResult result = tester.RunSingleTest(config);
        if (result.passed) {
            std::cout << std::fixed << std::setprecision(1) << result.averageFPS << " FPS";
        }
        else {
            std::cout << "FAILED";
        }
        std::cout << std::endl;
    }
}

/**
 * @brief Comprehensive benchmark suite
 */
void Example6_ComprehensiveBenchmark() {
    std::cout << "\n=== Example 6: Comprehensive Benchmark Suite ===\n";

    RenderingTester tester;
    if (!tester.Initialize()) {
        std::cout << "Failed to initialize tester: " << tester.GetLastError() << std::endl;
        return;
    }

    std::cout << "System: " << tester.GetCPUInfo() << std::endl;
    std::cout << "\nRunning comprehensive benchmark...\n";

    std::vector<TestConfig> configs;

    // Different scenes with OpenGL 4.5
    std::vector<SceneType> scenes = {
        SceneType::SingleCube,
        SceneType::ManyCubes,
        SceneType::Character,
        SceneType::ManyCharacters
    };

    for (auto scene : scenes) {
        TestConfig config = RenderingTester::CreateDefaultConfig();
        config.renderer = RendererType::OpenGL_4_5;
        config.scene = scene;
        config.testDurationSeconds = 12;
        config.width = 1920;
        config.height = 1080;
        configs.push_back(config);
    }

    // Progress tracking
    auto progressCallback = [&scenes](int current, int total, const TestResult& result) {
        if (current > 0 && current <= static_cast<int>(scenes.size())) {
            std::cout << "Completed: " << Utils::SceneTypeToString(scenes[current - 1]);
            if (result.passed) {
                std::cout << " - " << std::fixed << std::setprecision(1) << result.averageFPS << " FPS";
            }
            else {
                std::cout << " - FAILED";
            }
            std::cout << " [" << current << "/" << total << "]" << std::endl;
        }
        };

    TestResults results = tester.RunMultipleTests(configs, progressCallback);

    // Generate report
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "             BENCHMARK REPORT" << std::endl;
    std::cout << std::string(60, '=') << std::endl;

    double totalScore = 0;
    int validTests = 0;

    for (size_t i = 0; i < results.size() && i < scenes.size(); ++i) {
        const auto& result = results[i];
        std::cout << std::setw(20) << Utils::SceneTypeToString(scenes[i]) << ": ";

        if (result.passed) {
            std::cout << std::setw(8) << std::fixed << std::setprecision(1)
                << result.averageFPS << " FPS";
            totalScore += result.averageFPS;
            validTests++;
        }
        else {
            std::cout << std::setw(8) << "FAILED";
        }
        std::cout << std::endl;
    }

    if (validTests > 0) {
        double averageScore = totalScore / validTests;
        std::cout << std::string(60, '-') << std::endl;
        std::cout << std::setw(20) << "Average Score" << ": "
            << std::setw(8) << std::fixed << std::setprecision(1)
            << averageScore << " FPS" << std::endl;
    }

    std::cout << std::string(60, '=') << std::endl;
}


/**
 * @brief Main function demonstrating all examples
 */
int main() {
	std::cout << "RenderingTester Library - Usage Examples" << std::endl;
	std::cout << std::string(50, '=') << std::endl;

	try {
        Example1_VulkanTest();

	} 
	catch (const RenderTestException& e) {
		std::cerr << "RenderTest Exception: " << e.what() << std::endl;
		return -1;
	}
	catch (const std::exception& e) {
		std::cerr << "Standard Exception: " << e.what() << std::endl;
		return -1;
	}

	std::cout << "\nAll examples completed successfully!" << std::endl;
	return 0;
}

/**
 * @brief Alternative WinMain entry point for Windows applications
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow) {
	// Allocate console for output
	AllocConsole();
	freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
	freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);
	freopen_s((FILE**)stdin, "CONIN$", "r", stdin);

	int result = main();

	std::cout << "\nPress any key to exit...";
	std::cin.get();

	FreeConsole();
	return result;
}