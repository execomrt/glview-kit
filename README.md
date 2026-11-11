# GLVIEW Kit

Windows C++ wrapper and sample for the OpenGL Extensions Viewer rendering runtime.
The wrapper loads `infogl.dll` dynamically; no import library, Vulkan SDK, private
RenderCore checkout, or network access is required to build this kit.

## Requirements

- Windows 10 or later, Visual Studio 2022 with Desktop development with C++.
- CMake 3.21 or later; the targets use C++20.
- A matching x64 GLVIEW runtime: `infogl.dll`, `GLVIEW.RMX`, and its companion files.
- A GPU/driver supporting the requested API. Windows Vulkan tests use the
  RenderCore backend and require Vulkan 1.3, including the legacy Vulkan 1.0 ID.

This refresh uses GLVIEW SDK commit `c533cd1b52c4e4b8d7ab2703044b99945558b8c0`.
Runtime binaries are supplied separately and are not committed here. Keep the DLL
and resource archive from the same runtime build; an older archive may lack shaders.

## Build

From the kit root:

```powershell
cmake --preset vs2022-x64
cmake --build --preset debug
cmake --build --preset release
```

`generate_vs.bat` runs the configure preset and forwards additional arguments.
The existing `GLViewWin32Project` executable target is retained.

To stage a runtime beside the executable, configure with:

```powershell
cmake --preset vs2022-x64 -DGLVIEW_RUNTIME_DIR=C:/Home/code/GLVIEW/bin
cmake --build --preset debug
```

The runtime copy target runs on each sample build, including when the executable
is already up to date. Required files are checked at configure time; known optional
companions and databases are copied when present. Reconfigure after adding a new
companion file. Without this setting, compilation still works; supply runtime files
beside the executable before running it.

For a private local preset, create ignored `CMakeUserPresets.json`:

```json
{
  "version": 3,
  "configurePresets": [{
    "name": "local",
    "inherits": "vs2022-x64",
    "cacheVariables": { "GLVIEW_RUNTIME_DIR": "C:/Home/code/GLVIEW/bin" }
  }],
  "buildPresets": [{
    "name": "local-debug",
    "configurePreset": "local",
    "configuration": "Debug"
  }]
}
```

Use `cmake --preset local` and `cmake --build --preset local-debug`.
Machine paths belong in user presets, not shared presets.

## Run

```powershell
Push-Location build/vs2022-x64/Debug
.\GLViewWin32Project.exe --opengl --no-pause
.\GLViewWin32Project.exe --vulkan13 --no-pause
Pop-Location
```

The sample opens a 640x480 window for five seconds per test. With no API selection
it runs OpenGL 3.2 then Vulkan 1.3. Interactive runs wait for Enter; `--no-pause`
writes `glviewkit.log` and `glviewkit-error.log` in the working directory and exits.
Exit codes: 0 success, 1 initialization/test failure, 2 invalid arguments.
PowerShell callers automating a GUI executable should use `Start-Process -Wait
-PassThru` to wait for completion and read `ExitCode`.

The sample resolves the DLL and default archive beside its executable. Override
the archive with `--rmx path`. Run from the output directory so companion databases
and logs also resolve consistently. GPU tests require a desktop/display and are
not headless unit tests. The larger internal recording showcase is not included.

## Integrate

```cmake
add_subdirectory(path/to/glview-kit)
target_link_libraries(my_app PRIVATE GLViewKit::RenderingTester)
```

```cpp
#include "RenderingTester.h"

RenderTest::RenderingTester tester("C:/runtime", "C:/runtime/GLVIEW.RMX");
if (!tester.Initialize()) {
    // Inspect tester.GetLastError().
}
auto config = RenderTest::RenderingTester::CreateDefaultConfig();
config.renderer = RenderTest::RendererType::OpenGL_4_6;
config.fullscreen = false;
config.testDurationSeconds = 5;
auto result = tester.RunSingleTest(config);
```

The constructor's first argument is a directory containing `infogl.dll`; the second
is the archive filename. Empty arguments retain the wrapper's DLL-search and
working-directory defaults. The CMake library target supplies include directories,
C++20, and the Windows libraries `opengl32`, `Shcore`, and `Comctl32`.
For manual integration, compile `src/RenderingTester.cpp`, add `src` and `include`
to include paths, and link those system libraries.

`TestConfig` covers renderer/legacy version, scenes, fullscreen mode, framebuffer,
MSAA, anisotropy, fog, transparency, clip plane, and test duration. Availability
depends on the runtime and driver; enum presence does not guarantee support.
`RunMultipleTests` accepts configurations and an optional progress callback.
Renderer IDs cross the DLL ABI: Vulkan 1.3 is 17; 16 remains reserved for Metal.

## Refresh the SDK

```powershell
./scripts/Sync-Sdk.ps1 -GLViewRoot C:/Home/code/GLVIEW
```

This explicitly copies `SDK/RenderingTester.h`, `SDK/RenderingTester.cpp`, and
`SDK/include/oevSDK.h`, adapting the implementation's SDK include path. It does not
copy the internal sample or runtime binaries. Review the diff, update the source
commit recorded above, build Debug/Release, and smoke-test both APIs with a matching
runtime before releasing. Preserve the public header's ABI values.

## Layout and license

- `src/RenderingTester.*`: C++ wrapper.
- `src/Main.cpp`: standalone sample and command-line options.
- `include/oevSDK.h`: public C API and renderer IDs.
- `CMakePresets.json`: shared Visual Studio x64 configure/build presets.
- `scripts/Sync-Sdk.ps1`: repeatable refresh from the GLVIEW checkout.

See `LICENSE` and the copyright/permission notices in the SDK sources.
