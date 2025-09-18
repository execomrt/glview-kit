/**
 * @file RenderingTester.h
 * @brief OpenGL/Vulkan Rendering Performance Tester
 *
 * A clean, modern C++ wrapper for testing OpenGL and Vulkan rendering performance.
 * Supports various rendering configurations, scenes, and measurement options.
 *
 * @author Realtech VR
 * @copyright Copyright (C) Realtech VR 2000-2025 - http://www.realtech-vr.com/glview
 * @version 7.2
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>

 // Forward declarations for Windows types
struct HINSTANCE__;
typedef HINSTANCE__* HINSTANCE;

namespace RenderTest {

    /**
     * @brief Supported rendering APIs
     */
    enum class RendererType {
        GDI,           // Software GDI renderer
        OpenGL_2_0,    // OpenGL 2.0
        OpenGL_3_0,    // OpenGL 3.0
        OpenGL_3_1,    // OpenGL 3.1
        OpenGL_3_2,    // OpenGL 3.2
        OpenGL_3_3,    // OpenGL 3.3
        OpenGL_4_0,    // OpenGL 4.0
        OpenGL_4_1,    // OpenGL 4.1
        OpenGL_4_2,    // OpenGL 4.2
        OpenGL_4_3,    // OpenGL 4.3
        OpenGL_4_4,    // OpenGL 4.4
        OpenGL_4_5,    // OpenGL 4.5
        OpenGL_4_6,    // OpenGL 4.6
        Vulkan_1_0,    // Vulkan 1.0
        Vulkan_1_1,    // Vulkan 1.1
        Vulkan_1_2     // Vulkan 1.2
    };

    /**
     * @brief Available test scenes
     */
    enum class SceneType {
        SingleCube,        // Simple single cube scene
        ManyCubes,         // Multiple cubes for stress testing
        Character,         // Single character model
        ManyCharacters,    // Multiple character models
        Raytracing         // Ray tracing test scene
    };

    /**
     * @brief Framebuffer formats
     */
    enum class FramebufferFormat {
        RGB_Linear,    // Standard RGB linear
        sRGB,          // sRGB color space
        HDR            // High Dynamic Range
    };

    /**
     * @brief Framebuffer types
     */
    enum class FramebufferType {
        Default,           // Default framebuffer
        PixelBuffer,       // Pixel buffer object
        FramebufferObject  // Framebuffer object (FBO)
    };

    /**
     * @brief Test configuration structure
     */
    struct TestConfig {
        // Rendering settings
        RendererType renderer = RendererType::OpenGL_4_6;

        // Display settings
        bool fullscreen = false;
        int width = 1920;
        int height = 1080;
        bool vsync = false;

        // Quality settings
        bool fog = false;
        bool transparency = false;
        bool userClipPlane = false;
        int multisampleCount = 0;      // 0 = disabled, 2/4/8/16 = MSAA samples
        int maxAnisotropy = 0;         // 0 = disabled, 1-16 = anisotropic filtering
        int textureLOD = 0;            // Texture level of detail bias

        // Framebuffer settings
        FramebufferFormat fbFormat = FramebufferFormat::RGB_Linear;
        FramebufferType fbType = FramebufferType::Default;

        // Test settings
        SceneType scene = SceneType::SingleCube;
        int testDurationSeconds = 10;
        bool enableDebugOutput = false;

        // Advanced settings
        int pixelFormat = 1;           // Pixel format index
    };

    /**
     * @brief Test result for a single rendering test
     */
    struct TestResult {
        int testIndex;                 // Test identifier
        bool passed;                   // Whether test completed successfully
        double averageFPS;             // Average frames per second
        std::string errorMessage;      // Error description if test failed

        TestResult() : testIndex(0), passed(false), averageFPS(0.0) {}
    };

    /**
     * @brief Collection of test results
     */
    using TestResults = std::vector<TestResult>;

    /**
     * @brief Callback function type for receiving test progress updates
     * @param testIndex Current test being executed
     * @param totalTests Total number of tests
     * @param currentResult Result of current test (may be incomplete during progress)
     */
    using ProgressCallback = std::function<void(int testIndex, int totalTests, const TestResult& currentResult)>;

    /**
     * @brief Main rendering tester class
     */
    class RenderingTester {
    public:
        /**
         * @brief Constructor
         * @param dllPath Path to the infogl.dll library (optional, defaults to current directory)
         * @param resourcePath Path to GLVIEW.RMX resource file (optional, defaults to current directory)
         */
        explicit RenderingTester(const std::string& dllPath = "", const std::string& resourcePath = "");

        /**
         * @brief Destructor
         */
        ~RenderingTester();

        /**
         * @brief Initialize the tester (loads required libraries and resources)
         * @return true if initialization successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Run a single rendering test with the specified configuration
         * @param config Test configuration
         * @return Test result
         */
        TestResult RunSingleTest(const TestConfig& config);

        /**
         * @brief Run multiple tests with different configurations
         * @param configs Vector of test configurations
         * @param progressCallback Optional callback for progress updates
         * @return Vector of test results
         */
        TestResults RunMultipleTests(const std::vector<TestConfig>& configs,
            ProgressCallback progressCallback = nullptr);

        /**
         * @brief Get the last error message
         * @return Error message string
         */
        const std::string& GetLastError() const { return m_lastError; }

        /**
         * @brief Check if the tester is properly initialized
         * @return true if initialized, false otherwise
         */
        bool IsInitialized() const { return m_initialized; }

        /**
         * @brief Get CPU information
         * @return CPU specification string
         */
        std::string GetCPUInfo() const;

        /**
         * @brief Create a default test configuration
         * @return Default TestConfig structure
         */
        static TestConfig CreateDefaultConfig();

        /**
         * @brief Create a performance test configuration for benchmarking
         * @return Performance-oriented TestConfig structure
         */
        static TestConfig CreatePerformanceConfig();

        /**
         * @brief Create a quality test configuration with all features enabled
         * @return Quality-oriented TestConfig structure
         */
        static TestConfig CreateQualityConfig();



    private:
        // Private implementation details
        class Impl;
        std::unique_ptr<Impl> m_pImpl;

        std::string m_dllPath;
        std::string m_resourcePath;
        std::string m_lastError;
        bool m_initialized;

        // Helper methods
        bool InitLibrary();
        void SetLastError(const std::string& error);
        std::string BuildXMLConfiguration(const TestConfig& config);
        std::string CreateXMLElement(const std::string& name, const std::string& value);
        std::string GetTestVersionString(RendererType renderer);
        std::string FramebufferFormatToString(FramebufferFormat format);
        std::string FramebufferTypeToString(FramebufferType type);
    };

    /**
     * @brief Utility functions
     */
    namespace Utils {
        /**
         * @brief Convert RendererType to string
         */
        std::string RendererTypeToString(RendererType type);

        /**
         * @brief Convert SceneType to string
         */
        std::string SceneTypeToString(SceneType type);

        /**
         * @brief Find available display modes for given resolution
         * @param width Desired width
         * @param height Desired height
         * @return Display mode index, -1 if not found
         */
        int FindDisplayMode(int width, int height);

        /**
         * @brief Setup DPI awareness for modern Windows versions
         */
        void SetupDPIAwareness();
    }

    /**
     * @brief Exception class for rendering test errors
     */
    class RenderTestException : public std::exception {
    public:
        explicit RenderTestException(const std::string& message) : m_message(message) {}
        const char* what() const noexcept override { return m_message.c_str(); }

    private:
        std::string m_message;
    };

} // namespace RenderTest