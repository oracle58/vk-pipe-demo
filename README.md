# vk-pipe-demo

A minimal, low-level Vulkan demo that sets up a custom graphics pipeline and showcases the *Dirac Plate trick* implemented in GLSL shaders.

![vk-pipe-demo](https://github.com/user-attachments/assets/7d5e3ba3-8634-47a3-a950-79cc0525e578)
> Example fragment shader demonstrating the [Plate Trick](https://en.wikipedia.org/wiki/Plate_trick).

---

## Features

- Minimal Vulkan setup with explicit graphics pipeline configuration
- Custom GLSL vertex and fragment shaders
- Implementation of the Dirac delta trick for impulse-like rendering effects
- Clean structure suitable for further expansion or educational use

---

## 🛠 Build Instructions

### Prerequisites
- CMake
- VCPKG (for dependency management) 
- Vulkan SDK
- A C++20-capable compiler

### Windows

```shell
# configure and resolve dependencies
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="$env:USERPROFILE\vcpkg\scripts\buildsystems\vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows
# compile and link
cmake --build build --config debug
# run
./bin/debug/demo
```

### Linux 
> Only tested on arch so far.

#### 1. Install system dependencies

**Ubuntu:**
```shell
sudo apt update
sudo apt install build-essential cmake git pkg-config \
    libvulkan-dev vulkan-validationlayers-dev \
    libglfw3-dev libglm-dev libfmt-dev
```

**Arch Linux:**
```shell
sudo pacman -Syu --needed base-devel cmake pkgconf \
    vulkan-icd-loader vulkan-validation-layers \
    glfw-x11 glm fmt nvidia nvidia-utils libglvnd \
    lib32-nvidia-utils vulkan-driver vulkan-tools \
```

- For **AMD GPUs**: replace `nvidia nvidia-utils lib32-nvidia-utils` with `vulkan-radeon lib32-vulkan-radeon`.
- For **Intel GPUs**: replace with `vulkan-intel lib32-vulkan-intel`.
- `vulkan-tools` provides `vulkaninfo` for troubleshooting.
- `libglvnd` ensures proper OpenGL/Vulkan dispatch.
- Make sure your user is in the `video` group:  `sudo usermod -aG video $USER`
- Reboot after installing drivers.

#### 2. Install Vulkan SDK 

- Download from [LunarG Vulkan SDK](https://vulkan.lunarg.com/sdk/home)
- Extract and follow their setup instructions (add to `PATH` etc.)

#### 3. (Optional) Install VCPKG and dependencies via VCPKG

```shell
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg install glfw3 glm fmt imgui volk vulkan
```

#### 4. Build the project

```shell
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build build --config debug
./build/demo
```

- If you installed dependencies via your package manager, you can omit the `-DCMAKE_TOOLCHAIN_FILE=...` argument.

---

