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

 // Include the original SDK header
#include "include/oevSDK.h"


namespace RenderTest {

	/**
	 * @brief Internal logging utility
	 */
	class Logger {
	public:
		static void Info(const char* format, ...) {
			va_list args;
			va_start(args, format);
			LogMessage("[INFO] ", format, args);
			va_end(args);
		}

		static void Error(const char* format, ...) {
			va_list args;
			va_start(args, format);
			LogMessage("[ERROR] ", format, args);
			va_end(args);
		}

	private:
		static void LogMessage(const char* prefix, const char* format, va_list args) {
			char buffer[4096];
			vsnprintf(buffer, sizeof(buffer), format, args);
			OutputDebugStringA(prefix);
			OutputDebugStringA(buffer);
			OutputDebugStringA("\n");

			// Also output to console if available
			std::cout << prefix << buffer << std::endl;
		}
	};

	/**
	 * @brief Private implementation class
	 */
	class RenderingTester::Impl {
	public:
		HMODULE hLibrary = nullptr;
		PFNOEVINITWAD funcInitWad = nullptr;
		PFNOEVREADCPUID funcReadCpuid = nullptr;
		PNFNOEVRUNRENDERINGTESTS funcRunRenderingTests = nullptr;

		bool LoadDLL(const std::string& dllPath) {
			std::string fullPath = dllPath.empty() ? "infogl.dll" : dllPath + "\\infogl.dll";

			hLibrary = LoadLibraryA(fullPath.c_str());
			if (!hLibrary) {
				return false;
			}

			funcInitWad = (PFNOEVINITWAD)GetProcAddress(hLibrary, "oevInitWad");
			funcReadCpuid = (PFNOEVREADCPUID)GetProcAddress(hLibrary, "oevReadCpuid");
			funcRunRenderingTests = (PNFNOEVRUNRENDERINGTESTS)GetProcAddress(hLibrary, "oevRunRenderingTests");

			return (funcInitWad && funcReadCpuid && funcRunRenderingTests);
		}

		~Impl() {
			if (hLibrary) {
				FreeLibrary(hLibrary);
			}
		}
	};

	// Constructor
	RenderingTester::RenderingTester(const std::string& dllPath, const std::string& resourcePath)
		: m_pImpl(std::make_unique<Impl>())
		, m_dllPath(dllPath)
		, m_resourcePath(resourcePath.empty() ? "GLVIEW.RMX" : resourcePath)
		, m_initialized(false) {
	}

	// Destructor
	RenderingTester::~RenderingTester() = default;

	// Initialize the tester
	bool RenderingTester::Initialize() {
		if (m_initialized) {
			return true;
		}

		// Setup DPI awareness
		Utils::SetupDPIAwareness();

		// Load the DLL
		if (!InitLibrary()) {
			SetLastError("Failed to load infogl.dll or required functions");
			return false;
		}

		// Initialize the resource package
		if (m_pImpl->funcInitWad(m_resourcePath.c_str()) < 0) {
			SetLastError("Failed to initialize resource package: " + m_resourcePath);
			return false;
		}

		m_initialized = true;
		Logger::Info("RenderingTester initialized successfully");
		return true;
	}

	// Run a single test
	TestResult RenderingTester::RunSingleTest(const TestConfig& config) {
		TestResult result;

		if (!m_initialized) {
			if (!Initialize()) {
				result.errorMessage = GetLastError();
				return result;
			}
		}

		// Build XML configuration
		std::string xmlConfig = BuildXMLConfiguration(config);

		Logger::Info("Running test with config: %s", xmlConfig.c_str());

		// Execute the test
		auto testResults = m_pImpl->funcRunRenderingTests(xmlConfig.c_str());

		// Process results
		if (testResults && testResults->result) {
			if (strcmp(testResults->result, "OK") == 0) {
				result.passed = true;
				result.averageFPS = testResults->fps;
				result.testIndex = testResults->index;
				Logger::Info("Test %d passed with %.2f FPS", result.testIndex, result.averageFPS);
			}
			else {
				result.passed = false;
				result.errorMessage = testResults->result;
				result.testIndex = testResults->index;
				Logger::Error("Test %d failed: %s", result.testIndex, result.errorMessage.c_str());
			}
		}
		else {
			result.errorMessage = "No test results returned";
			Logger::Error("Test execution failed - no results returned");
		}

		return result;
	}

	// Run multiple tests
	TestResults RenderingTester::RunMultipleTests(const std::vector<TestConfig>& configs,
		ProgressCallback progressCallback) {
		TestResults results;
		results.reserve(configs.size());

		for (size_t i = 0; i < configs.size(); ++i) {
			TestResult result = RunSingleTest(configs[i]);
			results.push_back(result);

			if (progressCallback) {
				progressCallback(static_cast<int>(i + 1), static_cast<int>(configs.size()), result);
			}
		}

		return results;
	}

	// Get CPU info
	std::string RenderingTester::GetCPUInfo() const {
		if (!m_initialized || !m_pImpl->funcReadCpuid) {
			return "CPU information not available";
		}

		struct gvCpuid cpuInfo;
		m_pImpl->funcReadCpuid(&cpuInfo);
		return std::string(cpuInfo.Specification);
	}

	// Create default configuration
	TestConfig RenderingTester::CreateDefaultConfig() {
		TestConfig config;
		config.renderer = RendererType::OpenGL_4_6;
		config.width = 1920;
		config.height = 1080;
		config.scene = SceneType::SingleCube;
		config.testDurationSeconds = 5;
		return config;
	}

	// Create performance configuration
	TestConfig RenderingTester::CreatePerformanceConfig() {
		TestConfig config;
		config.renderer = RendererType::OpenGL_4_6;
		config.width = 1920;
		config.height = 1080;
		config.scene = SceneType::ManyCubes;
		config.testDurationSeconds = 30;
		config.multisampleCount = 0; // Disable MSAA for performance
		config.maxAnisotropy = 0;    // Disable anisotropic filtering
		return config;
	}

	// Create quality configuration
	TestConfig RenderingTester::CreateQualityConfig() {
		TestConfig config;
		config.renderer = RendererType::OpenGL_4_6;
		config.width = 1920;
		config.height = 1080;
		config.scene = SceneType::Character;
		config.testDurationSeconds = 15;
		config.multisampleCount = 8;     // High MSAA
		config.maxAnisotropy = 16;       // Max anisotropic filtering
		config.fog = true;
		config.transparency = true;
		config.fbFormat = FramebufferFormat::HDR;
		return config;
	}

	// Private helper methods
	bool RenderingTester::InitLibrary() {
		return m_pImpl->LoadDLL(m_dllPath);
	}

	void RenderingTester::SetLastError(const std::string& error) {
		m_lastError = error;
		Logger::Error("%s", error.c_str());
	}

	std::string RenderingTester::BuildXMLConfiguration(const TestConfig& config) {
		std::ostringstream xml;

		// Build options bitmask
		int options = 0;
		if (config.fullscreen) options |= (WGLDIAG_OPTION_FS | WGLDIAG_OPTIONS_FS_EX);
		if (config.enableDebugOutput) options |= WGLDIAG_OPTION_DEBUG;
		if (config.vsync) options |= WGLDIAG_OPTION_VSYNC;
		if (config.fog) options |= WGLDIAG_OPTION_FOG;
		if (config.transparency) options |= WGLDIAG_OPTION_TRANSPARENCY;
		if (config.userClipPlane) options |= WGLDIAG_OPTION_CLIP_PLANE;

		// Convert enums to values
		int rendererValue = static_cast<int>(config.renderer);
		std::string sceneValue = std::to_string(static_cast<int>(config.scene));
		std::string fbFormatStr = FramebufferFormatToString(config.fbFormat);
		std::string fbTypeStr = FramebufferTypeToString(config.fbType);
		std::string testVersion = GetTestVersionString(config.renderer);

		// Build XML elements
		xml << CreateXMLElement("option", std::to_string(options));
		xml << CreateXMLElement("duration", std::to_string(config.testDurationSeconds));
		xml << CreateXMLElement("multisample", std::to_string(config.multisampleCount));
		xml << CreateXMLElement("anisotropy", std::to_string(config.maxAnisotropy));
		xml << CreateXMLElement("texturelod", std::to_string(config.textureLOD));
		xml << CreateXMLElement("displaymode", std::to_string(Utils::FindDisplayMode(config.width, config.height)));
		xml << CreateXMLElement("renderer", std::to_string(rendererValue));
		xml << CreateXMLElement("pixelformat", std::to_string(config.pixelFormat));
		xml << CreateXMLElement("test", testVersion);
		xml << CreateXMLElement("fbenable", fbTypeStr);
		xml << CreateXMLElement("fbformat", fbFormatStr);
		xml << CreateXMLElement("scene", sceneValue);
		xml << CreateXMLElement("width", std::to_string(config.width));
		xml << CreateXMLElement("height", std::to_string(config.height));

		// Wrap in root element
		return CreateXMLElement("root", xml.str());
	}

	std::string RenderingTester::CreateXMLElement(const std::string& name, const std::string& value) {
		return "<" + name + ">" + value + "</" + name + ">\n";
	}

	std::string RenderingTester::GetTestVersionString(RendererType renderer) {
		switch (renderer) {
		case RendererType::GDI:
		case RendererType::OpenGL_2_0: return "2.0";
		case RendererType::OpenGL_3_0: return "3.0";
		case RendererType::OpenGL_3_1: return "3.1";
		case RendererType::OpenGL_3_2: return "3.2";
		case RendererType::OpenGL_3_3: return "3.3";
		case RendererType::OpenGL_4_0: return "4.0";
		case RendererType::OpenGL_4_1: return "4.1";
		case RendererType::OpenGL_4_2: return "4.2";
		case RendererType::OpenGL_4_3: return "4.3";
		case RendererType::OpenGL_4_4: return "4.4";
		case RendererType::OpenGL_4_5: return "4.5";
		case RendererType::OpenGL_4_6: return "4.6";
		case RendererType::Vulkan_1_0:
		case RendererType::Vulkan_1_1:
		case RendererType::Vulkan_1_2: return "1.0";
		default: return "4.6";
		}
	}

	std::string RenderingTester::FramebufferFormatToString(FramebufferFormat format) {
		switch (format) {
		case FramebufferFormat::RGB_Linear: return "Linear";
		case FramebufferFormat::sRGB: return "sRGB";
		case FramebufferFormat::HDR: return "HDR";
		default: return "Linear";
		}
	}

	std::string RenderingTester::FramebufferTypeToString(FramebufferType type) {
		switch (type) {
		case FramebufferType::Default: return "Default";
		case FramebufferType::PixelBuffer: return "PixelBuffer";
		case FramebufferType::FramebufferObject: return "FrameBufferObject";
		default: return "Default";
		}
	}

	// Utility functions implementation
	namespace Utils {
		std::string RendererTypeToString(RendererType type) {
			switch (type) {
			case RendererType::GDI: return "GDI";
			case RendererType::OpenGL_2_0: return "OpenGL 2.0";
			case RendererType::OpenGL_3_0: return "OpenGL 3.0";
			case RendererType::OpenGL_3_1: return "OpenGL 3.1";
			case RendererType::OpenGL_3_2: return "OpenGL 3.2";
			case RendererType::OpenGL_3_3: return "OpenGL 3.3";
			case RendererType::OpenGL_4_0: return "OpenGL 4.0";
			case RendererType::OpenGL_4_1: return "OpenGL 4.1";
			case RendererType::OpenGL_4_2: return "OpenGL 4.2";
			case RendererType::OpenGL_4_3: return "OpenGL 4.3";
			case RendererType::OpenGL_4_4: return "OpenGL 4.4";
			case RendererType::OpenGL_4_5: return "OpenGL 4.5";
			case RendererType::OpenGL_4_6: return "OpenGL 4.6";
			case RendererType::Vulkan_1_0: return "Vulkan 1.0";
			case RendererType::Vulkan_1_1: return "Vulkan 1.1";
			case RendererType::Vulkan_1_2: return "Vulkan 1.2";
			default: return "Unknown";
			}
		}

		std::string SceneTypeToString(SceneType type) {
			switch (type) {
			case SceneType::SingleCube: return "Single Cube";
			case SceneType::ManyCubes: return "Many Cubes";
			case SceneType::Character: return "Character";
			case SceneType::ManyCharacters: return "Many Characters";
			case SceneType::Raytracing: return "Raytracing";
			default: return "Unknown";
			}
		}

		int FindDisplayMode(int width, int height) {
			int modeIndex = 0;
			DEVMODE displayMode = { 0 };
			displayMode.dmSize = sizeof(DEVMODE);

			while (EnumDisplaySettings(nullptr, modeIndex, &displayMode)) {
				if (width == static_cast<int>(displayMode.dmPelsWidth) &&
					height == static_cast<int>(displayMode.dmPelsHeight)) {
					return modeIndex;
				}
				modeIndex++;
			}

			return -1; // Mode not found
		}

		void SetupDPIAwareness() {
			if (IsWindows8OrGreater()) {
				auto monitor = MonitorFromWindow(GetActiveWindow(), MONITOR_DEFAULTTONEAREST);
				unsigned int dpiX = 0, dpiY = 0;
				GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);

				if (IsWindows10OrGreater()) {
					SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
				}
				else {
					SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
				}
			}
		}
	}

} // namespace RenderTest

