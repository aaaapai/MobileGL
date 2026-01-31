<h1 align="center">MobileGL</h1>

<p align="center">
  <img src="https://img.shields.io/badge/Language-C%2B%2B-00599c?style=flat&logo=c%2B%2B" alt="C++">
  <img src="https://img.shields.io/badge/License-GNU%20LGPL%203.0-00399c?style=flat" alt="GNU LGPL 3.0">
  <img src="https://img.shields.io/badge/Status-Development-0078d7?style=flat" alt="Development">
</p>

<p align="center"><em>
A desktop OpenGL implementation
</em></p>

MobileGL is a *free (libre)* and *open-source* project that implements a desktop **OpenGL** API. The goal is to provide a complete OpenGL implementation with a state management layer and multiple backends.

> [!NOTE]
>
> **Status:** In development. Parts of the codebase are incomplete. Current short-term target: **OpenGL 3.3 (Core Profile)**.

## Project positioning

MobileGL is a ground-up implementation of a desktop OpenGL library. It aims to provide:

* Full OpenGL state management.
* A front-end that exposes OpenGL functions.
* Pluggable backends so the same front-end and state machine can run on different graphics backends.

This project is intended as an implementation/translation layer; it is not a driver nor a GPU firmware replacement currently.

## Key components

The repository is organized into a small number of top-level modules:

1. **MG_State** — the OpenGL state machine and related state-management code.
2. **MG_Impl** — the front-end OpenGL function implementations that mainly use `MG_State` and `MG_Backend`.
3. **MG_Backend** — backend implementations that perform actual rendering. Example backend: `Direct (OpenGL ES)` backend.
4. **MG_Util** and other utility modules.

## Third-party components

MobileGL reuses several established open-source projects:

* **SPIRV-Cross** by **KhronosGroup** - [Apache License 2.0](https://github.com/KhronosGroup/SPIRV-Cross/blob/master/LICENSE): [github](https://github.com/KhronosGroup/SPIRV-Cross)
* **glslang** by **KhronosGroup** - [Various Licenses](https://github.com/KhronosGroup/glslang/blob/main/LICENSE.txt): [github](https://github.com/KhronosGroup/glslang)
* **DiligentCore** by **Diligent Graphics** - [Apache License 2.0](https://github.com/DiligentGraphics/DiligentCore/blob/master/License.txt): [github](https://github.com/DiligentGraphics/DiligentCore)

Refer to each component's repository for exact license texts. Any bundled third-party code in this repository is included under the upstream project's license.

## Compatibility & target

* **Short-term target:** Implement `OpenGL 3.3 (Core Profile)`.
* **Current development focus:** MG_State and MG_Impl for `OpenGL 3.3 (Core Profile)`, and a `Direct (OpenGL ES)` backend implementation. These components are incomplete.

## Build Instructions

We currently provide **no releases** and **no precompiled binaries**.  
If you want to try the project right now, you’ll need to build it yourself:

1. Clone the repository:

   ```sh
   git clone https://github.com/MobileGL-Dev/MobileGL.git
   ```

2. Initialize and update all submodules recursively:

   ```sh
   git submodule update --init --recursive
   ```

3. Follow glslang’s own documentation for its required initiation.

4. Configure and build the project with CMake:

   ```sh
   cmake -B build
   cmake --build build
   ```
   
   or do it in a modern way:
   
   ```sh
   cmake -S . -B build -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
   cmake --build build
   ```
   
   Alternatively, you can use platform-specific build commands as needed.

## Build Options

| Option                       | Description                                           | Default |
| ---------------------------- | ----------------------------------------------------- | ------- |
| `MOBILEGL_BUILD_TEST`        | Build MobileGL tests (requires Clang)                 | ON      |
| `MOBILEGL_BUILD_BENCHMARK`   | Build MobileGL benchmarks (requires Clang)            | ON      |
| `MOBILEGL_FORCE_RELEASE_OPT` | Enable O3 and LTO in Debug build                      | ON      |
| `MOBILEGL_ENABLE_TRACY`      | Enable Tracy profiler for performance analysis        | OFF     |

   **Notes:**

* The project requires C++23.
* `MG_Test` and `MG_Benchmark` can only be built with Clang, not GCC. To enforce Clang, use `-DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++`.
* On Android, tests and benchmarks are always disabled.

## Notice

* MobileGL is **not** production-ready currently.
* Many features required by a full OpenGL 3.3 implementation are still missing or incomplete.
* Performance optimizatins are now almost missing.

## License

This project is distributed under **GNU LGPL v3.0**. See the `LICENSE` file in the repository for detailed information.
