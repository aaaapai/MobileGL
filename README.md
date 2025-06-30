# MobileGL

> [!NOTE]
> **Important Note**: This project will undergo a complete redesign and rewrite in several weeks. The current implementation will be replaced completely.

## Third party components

**SPIRV-Cross** by **KhronosGroup** - [Apache License 2.0](https://github.com/KhronosGroup/SPIRV-Cross/blob/master/LICENSE): [github](https://github.com/KhronosGroup/SPIRV-Cross)

**glslang** by **KhronosGroup** - [Various Licenses](https://github.com/KhronosGroup/glslang/blob/main/LICENSE.txt): [github](https://github.com/KhronosGroup/glslang)

*(Unused Currently)* **GlslOptimizerV2** by **aiekick** - [Apache License 2.0](https://github.com/aiekick/GlslOptimizerV2/blob/master/LICENSE): [github](https://github.com/aiekick/GlslOptimizerV2)

**unordered_dense** by **Martin Leitner-Ankerl** - [MIT License](https://github.com/martinus/unordered_dense/blob/main/LICENSE): [github](https://github.com/martinus/unordered_dense)

**OpenGL Mathematics (*GLM*)** by **G-Truc Creation** - [MIT License](https://github.com/g-truc/glm/blob/master/copying.txt): [github](https://github.com/g-truc/glm)

## Progress

**Target**: **OpenGL 3.2 Core Profile**

| Component                      | Development Progress |
|--------------------------------|----------------------|
| **OpenGL State Manager**       | **60 ~ 70%**         |
| **Abstraction Layer (MG_RHI)** | **0%**               |
| **OpenGL ES Backend**          | **N/A** (of MG_RHI)  |
| **Vulkan Backend**             | **planned**          |

---

**Target**: **OpenGL implementation compatible with Minecraft 1.17 to 1.21.5**

| Component                      | Development Progress* |
|--------------------------------|-----------------------|
| **OpenGL State Manager**       | **65 / 65**           |
| **Abstraction Layer (MG_RHI)** | **0 / 61**            |
| **OpenGL ES Backend**          | **N/A** (of MG_RHI)   |
| **Vulkan Backend**             | **planned**           |

* *The number of functions that have been implemented*
