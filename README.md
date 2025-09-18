# RenderingTester Library v2.0

A modern, clean C++ library for testing OpenGL and Vulkan rendering performance. This is a complete rewrite of the original OpenGL Extensions Viewer test code, designed to be more maintainable, easier to use, and better structured.

## Features

- **Multiple Rendering APIs**: Support for OpenGL 2.0-4.6 and Vulkan 1.0-1.2
- **Flexible Configuration**: Easy-to-use configuration system for all test parameters
- **Multiple Test Scenes**: Various complexity levels from simple cubes to complex character models
- **Quality Options**: Configurable multisampling, anisotropic filtering, fog, transparency
- **Progress Tracking**: Callback system for monitoring test progress
- **Modern C++**: Clean C++17 code with RAII, smart pointers, and proper error handling
- **Easy Integration**: Simple header/implementation split for easy project integration

## Requirements

- Windows 10 or later
- Visual Studio 2019 or later (with C++17 support)
- OpenGL-capable graphics card
- `infogl.dll` and `GLVIEW.RMX` files (from original OpenGL Extensions Viewer)

## Project Structure

```
RenderingTester/
├── RenderingTester.h      # Main header file
├── RenderingTester.cpp    # Implementation
├── Examples.cpp           # Usage examples
├── CMakeLists.txt         # Build configuration
├── README.md             # This file
└── include/
    └── oevSDK.h          # Original SDK header (required)
```

## Quick Start

### 1. Basic Usage

```cpp
#include "RenderingTester.h"

int main() {
    RenderTest::RenderingTester tester;
    
    if (!tester.Initialize()) {
        std::cout << "Failed to initialize: " << tester.GetLastError() << std::endl;
        return -1;
    }
    
    // Create a simple test configuration
    auto config = RenderTest::RenderingTester::CreateDefaultConfig();
    config.renderer = RenderTest::RendererType::OpenGL_4_6;
    config.scene = RenderTest::SceneType::SingleCube;
    
    // Run the test
    auto result = tester.RunSingleTest(config);
    
    if (result.passed) {
        std::cout << "Test passed! FPS: " << result.averageFPS << std::endl;
    } else {
        std::cout << "Test failed: " << result.errorMessage << std::endl;
    }
    
    return 0;
}
```

### 2. Multiple Tests with Progress Tracking

```cpp
#include "RenderingTester.h"
#include <vector>

int main() {
    RenderTest::RenderingTester tester;
    tester.Initialize();
    
    // Create multiple test configurations
    std::vector<RenderTest::TestConfig> configs = {
        RenderTest::RenderingTester::CreateDefaultConfig(),
        RenderTest::RenderingTester::CreatePerformanceConfig(),
        RenderTest::RenderingTester::CreateQualityConfig()
    };
    
    // Progress callback
    auto progress = [](int current, int total, const RenderTest::TestResult& result) {
        std::cout << "Progress: " << current << "/" << total;
        if (result.passed) {
            std::cout << " - FPS: " << result.averageFPS;
        }
        std::cout << std::endl;
    };
    
    // Run all tests
    auto results = tester.RunMultipleTests(configs, progress);
    
    // Process results
    for (const auto& result : results) {
        // Handle each result...
    }
    
    return 0;
}
```

## Configuration Options

### Renderer Types
- `RendererType::GDI` - Software GDI renderer
- `RendererType::OpenGL_2_0` through `RendererType::OpenGL_4_6` - OpenGL versions
- `RendererType::Vulkan_1_0` through `RendererType::Vulkan_1_2` - Vulkan versions

### Scene Types
- `SceneType::SingleCube` - Simple single cube (fastest)
- `SceneType::ManyCubes` - Multiple cubes for stress testing
- `SceneType::Character` - Single character model
- `SceneType::ManyCharacters` - Multiple character models
- `SceneType::Raytracing` - Ray tracing test scene

### Quality Settings
- **Multisampling**: 0 (disabled), 2, 4, 8, 16 samples
- **Anisotropic Filtering**: 0 (disabled), 1-16x
- **Effects**: Fog, transparency, user clip planes
- **Framebuffer Formats**: Linear RGB, sRGB, HDR

## Example Configurations

### Performance Testing
```cpp
auto config = RenderTest::RenderingTester::CreatePerformanceConfig();
config.renderer = RenderTest::RendererType::OpenGL_4_6;
config.scene = RenderTest::SceneType::ManyCubes;
config.multisampleCount = 0;  // Disable MSAA for max performance
config.testDurationSeconds = 30;
```

### Quality Testing
```cpp
auto config = RenderTest::RenderingTester::CreateQualityConfig();
config.multisampleCount = 8;     // High MSAA
config.maxAnisotropy = 16;       // Max anisotropic filtering
config.fog = true;
config.transparency = true;
config.fbFormat = RenderTest::FramebufferFormat::HDR;
```

### API Comparison
```cpp
// OpenGL test
auto glConfig = RenderTest::RenderingTester::CreateDefaultConfig();
glConfig.renderer = RenderTest::RendererType::OpenGL_4_6;

// Vulkan test  
auto vkConfig = glConfig;
vkConfig.renderer = RenderTest::RendererType::Vulkan_1_2;

std::vector<RenderTest::TestConfig> configs = {glConfig, vkConfig};
auto results = tester.RunMultipleTests(configs);
```

## Building

### Using CMake (Recommended)

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Manual Build

1. Add `RenderingTester.h` and `RenderingTester.cpp` to your project
2. Link against: `opengl32.lib`, `Shcore.lib`, `Comctl32.lib`
3. Ensure `infogl.dll` and `GLVIEW.RMX` are in your output directory
4. Set C++17 standard in your compiler settings

## Error Handling

The library uses several mechanisms for error handling:

```cpp
RenderTest::RenderingTester tester;

// Check initialization
if (!tester.Initialize()) {
    std::cout << "Error: " << tester.GetLastError() << std::endl;
}

// Check test results
auto result = tester.RunSingleTest(config);
if (!result.passed) {
    std::cout << "Test failed: " << result.errorMessage << std::endl;
}

// Exception handling for critical errors
try {
    auto results = tester.RunMultipleTests(configs);
} catch (const RenderTest::RenderTestException& e) {
    std::cout << "Critical error: " << e.what() << std::endl;
}
```

## Migration from Original Code

If you're migrating from the original `oevTest.cpp`:

| Original | New Library |
|----------|-------------|
| `run_rendering_test()` | `tester.RunSingleTest(config)` |
| Hardcoded XML strings | `TestConfig` structure |
| Manual option bitflags | Boolean configuration options |
| Direct DLL calls | Wrapped in `RenderingTester` class |
| Global variables | RAII-managed resources |

## Dependencies

- **Required at runtime**: `infogl.dll`, `GLVIEW.RMX`
- **System libraries**: OpenGL, Windows API
- **C++ Standard**: C++17 or later

## Examples

See `Examples.cpp` for comprehensive usage examples including:
- Basic single tests
- OpenGL version comparisons  
- Quality impact analysis
- Vulkan vs OpenGL benchmarks
- Resolution scaling tests
- Full benchmark suites

## License

Copyright (C) Realtech VR 2000-2023  
Licensed under the same terms as the original OpenGL Extensions Viewer.

## Changes from Original

- ✅ **Clean C++ API**: Modern C++17 with proper RAII
- ✅ **Type Safety**: Enums instead of magic numbers
- ✅ **Error Handling**: Exceptions and error codes
- ✅ **Documentation**: Comprehensive comments and examples
- ✅ **Flexibility**: Easy configuration without XML strings
- ✅ **Progress Tracking**: Callback system for long-running tests
- ✅ **Memory Management**: Smart pointers, no memory leaks
- ✅ **Maintainability**: Separated concerns, testable code

The new library maintains full compatibility with the original `infogl.dll` while providing a much cleaner and easier-to-use interface.