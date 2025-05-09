# glview-kit

SDK for accessing **GLView rendering tests**.

This SDK integrates with the [OpenGL Extensions Viewer](https://www.realtech-vr.com/glview) — a free utility for exploring OpenGL capabilities across a wide range of systems.

---

## 📦 Installation

1. Download and install the **OpenGL Extensions Viewer** from:
   [https://www.realtech-vr.com/glview](https://www.realtech-vr.com/glview)

2. Copy the following required files into the appropriate application or SDK directory:

   * `*.RMX`
   * `infogl.dll`
   * `v3x.dll`

3. Follow any additional instructions provided to run the rendering tests.

---

## 🛠️ Building the Project

To build the SDK from source:

```bash
./generate_vs
```

This script requires **CMake** to be installed and will generate a Visual Studio solution in the `./build` directory.

---

Let me know if you'd like to include usage examples, licensing info, or platform compatibility badges.
