# vk-pipe-demo

A minimal, low-level Vulkan demo that sets up a custom graphics pipeline and showcases the **Dirac delta (plate) trick** implemented in GLSL shaders.

![vk-pipe-demo](https://github.com/user-attachments/assets/7d5e3ba3-8634-47a3-a950-79cc0525e578)
> Example fragment shader demonstrating the [Plate/Delta Trick](https://en.wikipedia.org/wiki/Plate_trick).

---

## 🔧 Features

- Minimal Vulkan setup with explicit graphics pipeline configuration
- Custom GLSL vertex and fragment shaders
- Implementation of the Dirac delta trick for impulse-like rendering effects
- Clean structure suitable for further expansion or educational use

---

## 🛠 Build Instructions

### ✅ Prerequisites
- CMake
- VCPKG (for dependency management)
- Vulkan SDK
- A C++20-capable compiler

### Windows

```shell
cmake -S . -B build `
  -DCMAKE_TOOLCHAIN_FILE="$env:USERPROFILE\vcpkg\scripts\buildsystems\vcpkg.cmake" `
  -DVCPKG_TARGET_TRIPLET=x64-windows `

cmake --build build --config debug
```

-DCMAKE_TOOLCHAIN_FILE=C:/Users/voids/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows 