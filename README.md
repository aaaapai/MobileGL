# MobileGL

MobileGL is a *free (libre)* and *open-source* project that implements a desktop **OpenGL** API. The goal is to provide a complete OpenGL implementation with a state management layer and multiple backends.

> **Status:** In development. Parts of the codebase are incomplete. Current short-term target: **OpenGL 3.3 Core Profile**.

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
   git clone https://github.com/your/project.git
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
   
   Alternatively, you can use platform-specific build commands as needed.

## Notice

* MobileGL is **not** production-ready currently.
* Many features required by a full OpenGL 3.3 implementation are still missing or incomplete.
* Performance optimizatins are now almost missing.

## License

This project is distributed under **GNU LGPL v2.1**. See the `LICENSE` file in the repository for the full text.
