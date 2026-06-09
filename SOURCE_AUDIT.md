# MobileGL Source Audit

Status: complete

Scope rules:
- In scope: project source files under `MobileGL/` and `selfcheck.c`
- Excluded: `include/`, `3rdparty/`, generated/build output
- Validation rule: compile-only checks, no tests

## Findings

### Critical

1. `MobileGL/MG_Backend/DirectVulkan/Renderer/SwapchainObject.cpp`
   - Default depth attachment storage uses `width * width * 4` instead of `width * height * 4` when initializing the default framebuffer depth texture metadata.
   - Impact: non-square swapchain extents under-allocate or over-report CPU-side texture storage tracked by `MipmapStorage`. This can corrupt later uploads/copies or cause mismatched size assumptions between default framebuffer metadata and actual Vulkan image extent.
   - Status: fixed in this audit pass.

2. `MobileGL/MG_Backend/DirectVulkan/Renderer/FrameContext.cpp`
   - `WaitAndAcquireNextImage()` resets `imageInFlightFence` before `vkAcquireNextImageKHR()`. When image acquisition fails with `VK_ERROR_OUT_OF_DATE_KHR` or `VK_SUBOPTIMAL_KHR`, the fence remains unsignaled and no submission occurs to signal it again.
   - Impact: the next call for the same frame slot can block forever in `vkWaitForFences`, effectively deadlocking rendering during swapchain recreation paths.
   - Status: fixed in this audit pass.

3. `MobileGL/MG_Backend/DirectVulkan/Renderer/VkRenderPassManager.cpp`
   - `VkRenderPassManager::Shutdown()` was empty, so swapchain recreation left cached `VkRenderPass` / `VkFramebuffer` entries alive even though their attachments referenced destroyed swapchain image views and depth views.
   - Impact: after `RecreateSwapchain()`, cache hits could return framebuffer/render-pass objects built against stale destroyed attachments, leading to invalid Vulkan handles and undefined rendering behavior.
   - Status: fixed in this audit pass.

### High

1. `MobileGL/MG_Backend/DirectVulkan/Renderer/SwapchainObject.cpp`
   - `VkSwapchainCreateInfoKHR::minImageCount` is only clamped against `capabilities.minImageCount`, but not against `capabilities.maxImageCount` when that upper bound is non-zero.
   - Impact: on platforms where `MaxFramesInFlight` exceeds the surface maximum, swapchain creation can fail with invalid parameters.
   - Status: fixed in this audit pass.

2. `MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.cpp`
   - `Present()` recreates the swapchain after `vkAcquireNextImageKHR()` returns `VK_ERROR_OUT_OF_DATE_KHR` / `VK_SUBOPTIMAL_KHR`, but previously treated that as success without acquiring an image from the new swapchain.
   - Impact: subsequent rendering would continue with a stale `m_imageIndexAcquired`, so later operations could target an image index from the old swapchain or trip range assertions.
   - Status: fixed in this audit pass.

3. `MobileGL/MG_Backend/DirectVulkan/Renderer/FrameContext.cpp`
   - `GetPresentInfo()` previously stored `VkPresentInfoKHR::pImageIndices` as a pointer to the caller-owned `imageIndex` reference instead of packet-owned storage.
   - Impact: the current call site happens to pass a long-lived member, but the helper API was fragile and would become a dangling-pointer bug as soon as a temporary or shorter-lived local was passed in.
   - Status: fixed in this audit pass.

4. `MobileGL/MG_Backend/DirectVulkan/Renderer/VkTextureManager.cpp`
   - `UploadDirtyMipLevels()` always transitioned uploaded textures to and tracked them as `VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL`, even for depth/stencil formats.
   - Impact: depth/stencil textures could be left in an invalid tracked sampled layout after upload. Later sampling paths accept that tracked layout as already readable and skip the corrective transition to `VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL`, which can trigger validation/runtime errors.
   - Status: fixed in this audit pass.

5. `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp`
   - `GetIntegerv(GL_SCISSOR_BOX, params)` previously wrote only `params[0] = 0` instead of returning the full four-component scissor rectangle from tracked GL state.
   - Impact: callers receive incorrect state and may read uninitialized trailing elements, breaking state mirroring and any code that snapshots/restores scissor state through `glGetIntegerv`.
   - Status: fixed in this audit pass.

6. `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp`
   - `GetIntegerv(GL_COLOR_CLEAR_VALUE, params)` previously wrote only `params[0] = 0` instead of returning the full four-component tracked clear color.
   - Impact: callers receive incorrect clear color state and may read uninitialized trailing elements, breaking code that inspects or preserves clear state through `glGetIntegerv`.
   - Status: fixed in this audit pass.

7. `MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.cpp`
   - The DirectVulkan draw path ignored tracked `glViewport` state and always programmed Vulkan viewport `x/y = 0`, `width/height = render-pass extent`.
   - Impact: applications that rely on non-default viewport rectangles render to the wrong region under the Vulkan backend, even though `glViewport` updates tracked state successfully.
   - Status: fixed in this audit pass.

8. `MobileGL/MG_Impl/GLImpl/RenderState/GL_RenderState.cpp`, `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.cpp`
   - `glDepthRange` was exported but its state setter was a no-op, `glGetIntegerv(GL_DEPTH_RANGE, ...)` returned a placeholder scalar, and the Vulkan backend hardcoded viewport depth range to `0..1`.
   - Impact: applications cannot control or query depth mapping correctly on this backend, and code that depends on non-default depth ranges silently renders with the wrong clip-to-depth transform.
   - Status: fixed in this audit pass.

9. `MobileGL/MG_Impl/GLImpl/RenderState/GL_RenderState.cpp`, `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/PipelineFactory.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.cpp`
   - `glBlendColor` was exported but its state setter was a no-op, `glGetIntegerv(GL_BLEND_COLOR, ...)` returned a placeholder scalar, and the Vulkan backend never enabled dynamic blend constants or pushed them with `vkCmdSetBlendConstants`.
   - Impact: any blend mode using `GL_CONSTANT_COLOR`, `GL_ONE_MINUS_CONSTANT_COLOR`, `GL_CONSTANT_ALPHA`, or `GL_ONE_MINUS_CONSTANT_ALPHA` produces incorrect results and applications cannot query the configured constant blend color.
   - Status: fixed in this audit pass.

10. `MobileGL/MG_Backend/DirectGLES/DirectGLES.cpp`
   - After `BlendColor` and `DepthRange` became tracked render state, the DirectGLES backend sync path still never forwarded those fields to native GL with `glBlendColor` / `glDepthRangef`.
   - Impact: backend behavior diverged by renderer: DirectVulkan would honor the newly tracked states, while DirectGLES would silently keep stale native GL blend constants and depth range.
   - Status: fixed in this audit pass.

11. `MobileGL/MG_State/GLState/VertexArrayState/VertexArrayState.cpp`
   - `MarkVertexArrayForDeletion()` previously cleared `m_boundVertexArray` for any valid deleted VAO, even when the deleted object was not the currently bound VAO.
   - Impact: deleting an unrelated VAO could silently drop the current binding to null, causing later vertex-attribute and draw operations to fail with spurious `InvalidOperation` errors or backend-side null-state handling.
   - Status: fixed in this audit pass.

12. `MobileGL/MG_Impl/GLImpl/Drawing/GL_Drawing.cpp`
   - Draw and compute entry points previously forwarded directly to the backend even when no current program was bound via `UseProgram(0)`, when the current program was unlinked, or when compute dispatch was attempted without a compute shader stage. The same frontend path also allowed `GL_LINE_LOOP` to reach the DirectVulkan backend, where primitive conversion silently fell back to triangle-list rendering.
   - Impact: ordinary invalid-operation cases could fall through into backend-specific failure modes. On DirectVulkan this is especially severe because draw/compute paths dereference `GetCurrentProgram()` unconditionally, turning frontend GL state errors into null dereferences or assertion-driven aborts instead of clean GL errors; `GL_LINE_LOOP` could also render the wrong primitive topology instead of being rejected.
   - Status: fixed in this audit pass.

13. `MobileGL/MG_State/GLState/BufferState/BufferState.cpp`
   - `MarkBufferObjectForDeletion()` previously cleared only generic buffer binding slots and left indexed binding points from `glBindBufferBase` / `glBindBufferRange` still referencing the deleted buffer object.
   - Impact: after buffer deletion, stale UBO / SSBO / transform-feedback / atomic-counter binding points could still report or use a deleted buffer object instead of becoming unbound, causing incorrect state queries and follow-on backend synchronization against invalid state.
   - Status: fixed in this audit pass.

14. `MobileGL/MG_Impl/GLImpl/Buffer/GL_Buffer.cpp`
   - `BindBufferBase_State()` and `BindBufferRange_State()` treated buffer name `0` like an ordinary object name, creating or reusing a frontend buffer object 0 instead of unbinding the indexed binding point.
   - Impact: `glBindBufferBase(..., 0)` / `glBindBufferRange(..., 0, ...)` could leave phantom buffer-object state attached to indexed binding points instead of clearing them, breaking OpenGL unbind semantics.
   - Status: fixed in this audit pass.

15. `MobileGL/MG_Impl/GLImpl/Framebuffer/GL_Framebuffer.cpp`
   - `ReadPixels()` previously called `ReadPixels_Backend()` unconditionally even after `ReadPixels_State()` recorded validation errors for invalid sizes, formats/types, incomplete read FBOs, mapped pixel-pack buffers, or unsupported multisampled reads.
   - Impact: invalid `glReadPixels` calls could still execute backend readback against rejected frontend state, producing unintended GPU work and backend-specific failures instead of stopping at the GL error.
   - Status: fixed in this audit pass.

16. `MobileGL/MG_Impl/GLImpl/Framebuffer/GL_Framebuffer.cpp`
   - `FramebufferRenderbuffer_State()` and `BindRenderbuffer_State()` both validated renderbuffer names with zero effectively rejected on the path, so `glFramebufferRenderbuffer(..., 0)` could fail before detaching and `glBindRenderbuffer(..., 0)` could not unbind cleanly. The bind path also created and bound a phantom frontend renderbuffer object 0 when it should have cleared the binding.
   - Impact: standard renderbuffer detach/unbind operations were broken, leaving applications unable to clear renderbuffer attachments or restore the default renderbuffer binding semantics.
   - Status: fixed in this audit pass.

17. `MobileGL/MG_Impl/GLImpl/Framebuffer/GL_Framebuffer.cpp`
   - `FramebufferRenderbuffer_State()` converted `renderbuffertarget` to `RenderbufferTarget` but never validated it before mutating framebuffer state.
   - Impact: invalid `renderbuffertarget` enums could be silently accepted instead of generating `GL_INVALID_ENUM`.
   - Status: fixed in this audit pass.

18. `MobileGL/MG_Impl/GLImpl/Framebuffer/GL_Framebuffer.cpp`
   - `ReadBuffer_State()` accepted any enum convertible to a framebuffer attachment type and never enforced the default-FBO vs user-FBO token split. `DrawBuffers_State()` had the same gap for non-color attachments on user FBOs and default-buffer tokens on framebuffer objects.
   - Impact: calls such as `glReadBuffer(GL_DEPTH_ATTACHMENT)` or `glDrawBuffers(... GL_DEPTH_ATTACHMENT ...)` on user FBOs, and non-default framebuffer tokens on the default FBO, could silently poison tracked buffer-selection state instead of raising `GL_INVALID_ENUM`.
   - Status: fixed in this audit pass.

19. `MobileGL/MG_State/GLState/FramebufferState/FramebufferState.cpp`
   - `MarkFramebufferObjectForDeletion()` previously rebound any currently bound deleted FBO to `nullptr` instead of restoring framebuffer object 0.
   - Impact: deleting a bound user FBO left draw/read framebuffer bindings empty, causing later framebuffer-dependent commands to fail with spurious `InvalidOperation` paths or backend sync errors instead of reverting to the default framebuffer as OpenGL requires.
   - Status: fixed in this audit pass.

20. `MobileGL/MG_Impl/GLImpl/Sampler/GL_Sampler.cpp`
   - `SetSamplerParam_State()` never called the sampler parameter validators even though they existed, so invalid enums such as unsupported wrap/filter/compare values were converted to internal `Unknown` enum states and stored on sampler objects instead of raising GL errors.
   - Impact: invalid `glSamplerParameter*` calls silently poisoned frontend sampler state, which then propagated into backend sampler creation with fallback/default behavior instead of preserving last-valid state and reporting the API error.
   - Status: fixed in this audit pass.

21. `MobileGL/MG_State/GLState/SamplerState/SamplerObject.cpp`
   - `SamplerObject::SetLodRange()` threw a C++ exception when `minLod > maxLod`. Frontend API calls update those fields one-at-a-time, so ordinary sequences like lowering `GL_TEXTURE_MAX_LOD` below the current `MIN_LOD` could abort the process.
   - Impact: legal state-update ordering for sampler LOD limits could crash the library instead of simply storing the caller-provided values for later effective clamping in the backend.
   - Status: fixed in this audit pass.

22. `MobileGL/MG_Util/Converters/GLToMG/TextureEnumConverter.cpp`
   - `ConvertGLEnumToTextureTarget()` did not recognize proxy texture enums such as `GL_PROXY_TEXTURE_1D/2D/3D`, `GL_PROXY_TEXTURE_CUBE_MAP`, and the array/multisample proxy targets.
   - Impact: proxy texture entry points could derive `TextureTarget::Unknown`, then index active-unit binding slots with an invalid target or reject otherwise valid proxy queries/allocations on the frontend.
   - Status: fixed in this audit pass.

23. `MobileGL/MG_Impl/GLImpl/Texture/ProxyTexture.cpp`
   - `ProxyTextureManager::CreateOrReplaceProxyTextureObject()` always instantiated `TextureObject2D`, regardless of the requested proxy target.
   - Impact: proxy texture operations for cube maps, 3D textures, arrays, rectangle textures, or multisample targets could be backed by the wrong state object type, leading to invalid upload-target assertions, wrong dimensional bookkeeping, or broken completeness/query behavior.
   - Status: fixed in this audit pass.

24. `MobileGL/MG_State/GLState/TextureState/TextureObject2DCube.cpp`
   - `TextureObject2DCube::GetIndexOfTextureUploadTarget()` accepted `ProxyCubeMap` in its range check and converted it into face index `7`, but cube textures only allocate storage for the six real faces.
   - Impact: proxy cube-map state or accidental proxy-target use on a cube texture object could index beyond the six-face storage array, tripping assertions or corrupting cube-map bookkeeping.
   - Status: fixed in this audit pass.

25. `MobileGL/MG_Impl/GLImpl/Texture/GL_Texture.cpp`
   - `GetTexImage()` previously called `GetTexImage_Backend()` unconditionally even after `GetTexImage_State()` rejected the call for invalid targets, incomplete/unbound textures, mapped pixel-pack buffers, bad formats/types, or other state errors.
   - Impact: invalid `glGetTexImage` calls could still execute backend readback against rejected frontend state instead of stopping at the GL error boundary.
   - Status: fixed in this audit pass.

26. `MobileGL/MG_Impl/GLImpl/Texture/GL_Texture.cpp`
   - `GetTexImage_State()` treated any mipmapped texture with more than one allocated mip level as if it were multisampled and rejected it with `InvalidOperation`.
   - Impact: ordinary mipmapped textures became unreadable through `glGetTexImage` as soon as more than one mip level existed, even though mip levels are normal 1D/2D/3D texture storage rather than multisampling.
   - Status: fixed in this audit pass.

27. `MobileGL/MG_Impl/GLImpl/Texture/GL_Texture.cpp`
   - `BindImageTexture()` updated tracked `ImageTextureBinding` state before checking whether the backend exposed an image-texture bind entry point.
   - Impact: on unsupported backends, `glBindImageTexture` could report `InvalidOperation` yet still mutate frontend image-binding state, leaving later queries/backend sync to observe bindings from a call that should have had no effect.
   - Status: fixed in this audit pass.

28. `MobileGL/MG_Impl/GLImpl/Texture/GL_Texture.cpp`
   - `glTexParameterfv/iv/Iiv/Iuiv(..., GL_TEXTURE_BORDER_COLOR, ...)` still threw unimplemented exceptions even though border color was already tracked in texture state and consumed by both GLES and Vulkan backend sampler setup.
   - Impact: applications using border-color texture parameters could crash the library instead of updating valid tracked state.
   - Status: fixed in this audit pass.

29. `MobileGL/MG_Impl/GLImpl/Texture/GL_Texture.cpp`
   - `GetTexParameteriv/fv` left several already-tracked texture parameters effectively unimplemented, and `GetTexParameterIiv/Iuiv` hard-threw for all pnames instead of delegating to the tracked state.
   - Impact: callers querying base/max LOD level, swizzle channels, border color, or integer texture parameters could observe untouched outputs or process-aborting exceptions even though the corresponding state was already maintained.
   - Status: fixed in this audit pass.

30. `MobileGL/MG_Backend/DirectGLES/Managers.cpp`
   - `BackendTextureObject::SyncMipmapsToBackend()` always used `glTexSubImage2D` in its dirty-mipmap update path, even when the tracked texture target was `Texture3D`.
   - Impact: once a 3D texture had been initialized, any later dirty subimage upload on the DirectGLES backend hit the wrong GL entry point, so depth slices were never updated correctly and the call could fail with backend GL errors instead of refreshing the texture data.
   - Status: fixed in this audit pass.

31. `MobileGL/MG_Backend/DirectGLES/BackendObject_DirectGLES.cpp`, `MobileGL/MG_Backend/BackendObject.cpp`
   - The DirectGLES backend inherited `BackendObject::CreateEGLPbufferSurface()` unchanged. That base path returned early for any already-initialized pbuffer, regardless of requested size, and never tore down existing EGL resources before switching from a window surface to a pbuffer surface.
   - Impact: recreating a pbuffer with a new size could silently keep rendering against the old backend surface dimensions, and window-surface to pbuffer transitions could leave the backend EGL context/surface bound to the old window while the frontend reported a new pbuffer surface.
   - Status: fixed in this audit pass.

32. `MobileGL/MG_Impl/EGLImpl/EGLImpl.cpp`
   - `eglReleaseThread()` only cleared frontend `EGLState` bookkeeping and never forwarded an `EGL_NO_DISPLAY` / `EGL_NO_SURFACE` / `EGL_NO_CONTEXT` release-current request to the active backend object.
   - Impact: after a thread released itself, the backend could still treat that thread as current and preserve stale EGL current-state ownership, so later backend operations could incorrectly succeed against a thread that should already have been fully detached from the context/surfaces.
   - Status: fixed in this audit pass.

33. `MobileGL/MG_Impl/GLImpl/Sync/GL_Sync.cpp`, `MobileGL/MG_Impl/GLImpl/Exporting/Definitions.cpp`
   - The exported GL sync entry points (`glFenceSync`, `glIsSync`, `glDeleteSync`, `glClientWaitSync`, `glWaitSync`, `glGetSynciv`) were still wired to generic stub macros that returned dummy success-like values or no-op behavior, while `GL_Sync.cpp` itself also returned placeholder `0` values for every path.
   - Impact: applications using GL sync objects could observe fabricated non-error results even though no sync object was created, waited on, queried, or deleted. That breaks API semantics directly and can cause callers to proceed past synchronization points that never existed.
   - Status: fixed in this audit pass.

34. `MobileGL/MG_State/GLState/ProgramState/ShaderObject.cpp`, `MobileGL/MG_State/GLState/ProgramState/ProgramObject.cpp`
   - Shader/program recompilation and relinking paths kept stale successful artifacts alive across later failures. `ShaderObject::SetShaderSource()` did not invalidate the previous compiled shader, failed recompiles left `m_shader` pointing at the old compiled object, and `ProgramObject::Link()` reused old reflected/binary state after link failure because it neither reset prior link artifacts nor aborted before reflection/binary generation.
   - Impact: after a shader source change or failed relink, frontend program/shader state could still expose the previous successful compiled shader, reflected uniforms/attributes, and generated SPIR-V, so callers might observe or use stale program metadata from a program that should now be uncompiled or unsuccessfully linked.
   - Status: fixed in this audit pass.

35. `MobileGL/MG_State/GLState/BufferState/BufferObject.cpp`
   - Buffer mapping paths failed to persist the frontend's mapping-policy bits correctly. `AcquireMemoryRange()` returned before recording `ForbidInvalidationBit` / `ForbidUnsynchronizationBit`, and plain `AcquireMemory(true, ...)` never marked write mappings as forbidding either optimization.
   - Impact: after CPU writes through `glMapBuffer` / `glMapBufferRange` without invalidate or unsynchronized flags, the DirectGLES backend could still treat later uploads as eligible for `GL_MAP_INVALIDATE_RANGE_BIT` and `GL_MAP_UNSYNCHRONIZED_BIT`. That allows the backend sync path to discard or race prior GPU-visible contents even though the frontend mapping contract did not permit those behaviors.
   - Status: fixed in this audit pass.

36. `MobileGL/Init.cpp`
   - Library initialization and shutdown were not idempotent. On Linux/macOS the shared library auto-runs `Initialize()` and `Destroy()` via constructor/destructor attributes, while several tests and benchmarks also call `MobileGL::Initialize()` explicitly. The init path had no guard against repeated entry and the destroy path did not clear the active backend singleton or function table.
   - Impact: a second `Initialize()` could silently replace global frontend/backend singletons and re-run `glslang::InitializeProcess()` on an already-initialized process, while repeated destroy paths could finalize process-global resources multiple times. That makes embedder behavior depend on whether the library was preloaded or explicitly initialized, and risks leaked or stale backend state across reinitialization.
   - Status: fixed in this audit pass.

37. `MobileGL/MG_Util/BackendLoaders/OpenGL/Loader.cpp`
   - `AcquireEGLFunctions()` and `AcquireGLESFunctions()` logged missing function pointers but still returned success unconditionally. `BackendObject_DirectGLES::Initialize()` trusts those booleans to decide whether the backend is initialized.
   - Impact: a partial or failed EGL/GLES symbol load could still mark the DirectGLES backend initialized, after which later calls dereference null function pointers during context/surface setup, capability queries, or rendering instead of failing cleanly at backend startup.
   - Status: fixed in this audit pass.

38. `MobileGL/MG_Util/Texture/PixelStoreProcessor.cpp`
   - The unpack path's `BGRA -> RGBA8` swizzle order was wrong. It rewrote each pixel as `{G, B, A, R}` instead of `{R, G, B, A}`.
   - Impact: any BGRA pixel upload normalized through this helper could arrive with all color channels rotated and alpha moved into blue, producing visibly corrupted textures rather than just a minor format mismatch.
   - Status: fixed in this audit pass.

39. `MobileGL/MG_Util/Debug/Log.cpp`
   - `Log()` passed the raw `vsnprintf()` return value directly to `std::string(buffer, n)`. When formatting failed (`n < 0`) or when output exceeded the fixed buffer, that length no longer matched the valid bytes stored in `buffer`.
   - Impact: logging an error path could itself trigger undefined behavior or huge out-of-bounds string construction, turning diagnostic code into a secondary crash vector and destabilizing failure handling.
   - Status: fixed in this audit pass.

40. `MobileGL/MG_Util/ShaderTranspiler/ShaderSourceProcessor.cpp`
   - Multi-line comment stripping did not handle an unterminated `/* ...` block. When no closing `*/` existed, the preprocessor used `npos` in the replacement length calculation instead of treating the rest of the source as commented out.
   - Impact: a malformed shader that should have produced a normal compile error could instead trip undefined string-manipulation behavior inside MobileGL's preprocessing step, turning frontend shader validation into a host-side crash vector.
   - Status: fixed in this audit pass.

41. `MobileGL/MG_Util/Texture/TextureFormatProcessor.cpp`
   - `NormalizePixelFormat()` omitted explicit handling for `GL_SRGB8_ALPHA8` and `GL_DEPTH24_STENCIL8`. Both formats fell through to generic fallback logic instead of returning their correct pixel format/type pairs.
   - Impact: backends deriving upload/allocation parameters from this helper could create sRGB-alpha textures with the wrong format metadata and depth-stencil images with the wrong data type, breaking texture/renderbuffer creation and copy paths for otherwise valid formats.
   - Status: fixed in this audit pass.

42. `MobileGL/MG_Util/Metrics/TextureMetrics.cpp`
   - The texture/renderbuffer metrics tables had multiple concrete coverage bugs. `GetComponentSizesForInternalFormat()` had an accidental early `break` in the `RGBA8/RGBA8Snorm/RGBA8I/RGBA8UI/SRGB8Alpha8/RGBA` group, so ordinary 8-bit RGBA formats reported zero component sizes. The same file also omitted deterministic byte/component sizing for accepted formats such as `RGB10`, `RGB12`, `RGB16`, `RGB16Snorm`, `R32I`, and `R32UI`.
   - Impact: `glGetRenderbufferParameteriv` could report zero channel sizes for common RGBA8 renderbuffers, and frontend texture-storage/accounting paths that use `GetInternalBytesPerPixel()` / `GetSizedInternalFormatSizeInBytes()` could undercompute allocation sizes for those supported formats.
   - Status: fixed in this audit pass.

43. `MobileGL/MG_Util/Converters/MGToGL/TextureEnumConverter.cpp`, `MobileGL/MG_Util/Converters/MGToStr/TextureEnumConverter.cpp`
   - Reverse texture enum conversion tables were missing accepted values. `TextureInternalFormat::RGB16` converted back to `GL_UNKNOWN_MGL` / `"Unknown"` even though `GLToMG` accepts `GL_RGB16`, and `TexturePixelDataType::UnsignedInt8888` converted to `GL_UNKNOWN_MGL` even though the forward converter accepts `GL_UNSIGNED_INT_8_8_8_8`.
   - Impact: state/query paths such as texture/renderbuffer internal-format reporting and GLES format normalization could expose unsupported-looking values for actually supported enums, breaking round-trip conversions and misleading validation/logging.
   - Status: fixed in this audit pass.

44. `MobileGL/MG_Impl/GLImpl/Exporting/Definitions.cpp`
   - The exported float-depth entry points `glClearDepthf`, `glDepthRangef`, and their OES aliases were still wired to generic stub macros even though the real render-state implementations already exist and backend sync paths consume the tracked clear-depth/depth-range state.
   - Impact: applications calling the ES-style float variants appear to succeed but silently leave clear depth and depth range unchanged, so later clears and viewport depth mapping continue using stale values.
   - Status: fixed in this audit pass.

45. `MobileGL/MG_Util/Converters/MGToStr/BufferEnumConverter.cpp`
   - `ConvertBufferMappingAccessToString()` always called `pop_back()` twice to trim a trailing `", "`, even when the access mask was empty (`BufferMappingAccessBit::Null`).
   - Impact: any logging/debug path that formats an unmapped or zero-access buffer state can hit out-of-bounds string mutation and crash inside diagnostic code.
   - Status: fixed in this audit pass.

46. `MobileGL/MG_Util/Converters/MGToGL/FramebufferEnumConverter.cpp`, `MobileGL/MG_Util/Converters/MGToStr/FramebufferEnumConverter.cpp`
   - The framebuffer reverse-conversion tables were inconsistent with the frontend enums. `MGToGL` mapped `FramebufferTarget::Unknown` and `RenderbufferTarget::Unknown` to valid fallback tokens instead of `GL_UNKNOWN_MGL`, and `MGToStr` omitted valid attachment names such as `None`, `FrontLeft`, `FrontRight`, `BackLeft`, and `BackRight`.
   - Impact: invalid framebuffer/renderbuffer state could be mislabeled as valid in validator error messages, and legitimate attachment tokens could be logged as `"Unknown"`, obscuring real frontend state and complicating debugging.
   - Status: fixed in this audit pass.

47. `MobileGL/MG_Util/ShaderTranspiler/SpirvPasses/EliminateFloatEqualsZeroPass.cpp`
   - The float-zero optimization pass rewrote both ordered and unordered SPIR-V equality/inequality comparisons, but it always emitted ordered replacement opcodes (`OpFOrdLessThan` / `OpFOrdGreaterThanEqual`). For `OpFUnordEqual` and `OpFUnordNotEqual`, that drops unordered NaN semantics instead of preserving them.
   - Impact: shader binaries passing through `ShaderCompiler::SanitizeAndOptimizeBinary()` could change observable results whenever the compared operand is NaN. In practice, shaders using unordered float comparisons against zero could be miscompiled during normal program binary generation.
   - Status: fixed in this audit pass.

48. `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp`
   - `glGetString` / `glGetStringi` cached vendor/version/renderer/extension strings in function-local statics and never refreshed them after `Destroy()` / re-`Initialize()`. `GetStringi(GL_EXTENSIONS, index)` first validated `index` against the current backend's extension count, then indexed into the stale cached `extStrings` vector from the previous initialization.
   - Impact: after backend reinitialization or backend switching, GL string queries could report stale identity/extension data. If the new backend exposed more extensions than the previously cached list, `glGetStringi(GL_EXTENSIONS, index)` could read past the cached vector bounds for indices that are valid in the new backend, turning a stale-cache bug into memory-unsafe behavior on a normal public query path.
   - Status: fixed in this audit pass.

49. `MobileGL/MG_Impl/GLImpl/Exporting/Definitions.cpp`, `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp`
   - The public `glGetBooleanv` and `glGetFloatv` entry points were still exported through generic stub macros, so they only logged and returned without writing any output or querying real state. That left two core OpenGL getter APIs effectively unimplemented even though the corresponding state already exists in `GLContext` and `GetIntegerv`.
   - Impact: common application state queries such as blend enable, depth mask, clear color, blend color, depth range, scissor box, and similar `glGetBooleanv`/`glGetFloatv` calls could succeed superficially while leaving caller output untouched or stale. This breaks baseline GL introspection semantics on a high-fanout API surface and can directly corrupt application state caching/restoration logic.
   - Status: fixed in this audit pass.

50. `MobileGL/MG_Impl/GLImpl/Exporting/Definitions.cpp`, `MobileGL/MG_Impl/GLImpl/VertexArray/GL_VertexArray.cpp`
   - The public vertex-attribute query entry points (`glGetVertexAttribfv`, `glGetVertexAttribiv`, `glGetVertexAttribPointerv`, `glGetVertexAttribIiv`, `glGetVertexAttribIuiv`) were still exported through generic stubs even though VAO state and current generic vertex-attribute values were already tracked in `VertexArrayObject` and `GLContext`.
   - Impact: applications querying vertex attribute enable flags, sizes, strides, types, normalization, integer-mode, divisors, bound VBOs, client pointer offsets, or current generic attribute values could get silent no-op behavior instead of actual answers. That breaks common VAO snapshot/debug/state-cache logic on another high-fanout public query surface.
   - Status: fixed in this audit pass.

51. `MobileGL/MG_Impl/GLImpl/Exporting/Definitions.cpp`, `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp`
   - The public `glGetInteger64v` entry point was still exported through a generic stub macro, so it only logged and returned without writing any output. This left the 64-bit getter variant unimplemented even though many of its supported non-indexed queries already flow through `GetIntegerv` and tracked frontend/backend state.
   - Impact: applications using the 64-bit getter form for supported capability/state queries could observe untouched output buffers or stale values despite a superficially successful call. That breaks API compatibility on another core introspection path, especially for codepaths that standardize on the `GLint64` query family.
   - Status: fixed in this audit pass.

52. `MobileGL/MG_Impl/GLImpl/Exporting/Definitions.cpp`, `MobileGL/MG_Impl/GLImpl/VertexArray/GL_VertexArray.cpp`
   - The public current generic vertex-attribute setters (`glVertexAttrib1f/fv`, `2f/fv`, `3f/fv`, `4f/fv`, `glVertexAttribI4i/ui/iv/uiv`, `glVertexAttrib4Nub/Nubv`, `glVertexAttrib4ubv`) were still exported through generic stubs even though `GLContext` already tracks `CurrentVertexAttributeValue` and the DirectVulkan draw path consumes that state when an attribute array is disabled.
   - Impact: applications relying on current generic vertex attribute values could not actually update them through the public API. On paths where shaders read attributes from disabled arrays, rendering would keep using the default/stale tracked values instead of the application-specified ones, breaking a real draw-time data path rather than just introspection.
   - Status: fixed in this audit pass.

53. `MobileGL/MG_Impl/GLImpl/Exporting/Definitions.cpp`, `MobileGL/MG_Impl/GLImpl/Buffer/GL_Buffer.cpp`, `MobileGL/MG_State/GLState/BufferState/BufferObject.cpp`
   - The public `glGetBufferPointerv` entry point was still exported through a generic stub macro even though frontend buffer objects already track live mapped ranges and the mapping calls return real CPU pointers. That meant `glGetBufferPointerv(GL_BUFFER_MAP_POINTER, ...)` never reported the active mapping pointer for either whole-buffer maps or ranged/staged write mappings.
   - Impact: applications querying `GL_BUFFER_MAP_POINTER` could observe untouched output or stale data instead of the live mapped address, breaking standard buffer-mapping introspection. On write-mapped buffers this is especially harmful because MobileGL may hand out a staging pointer rather than the underlying storage pointer, so callers cannot recover the actual writable mapping address through the required query API.
   - Status: fixed in this audit pass.

54. `MobileGL/MG_Impl/GLImpl/Exporting/Definitions.cpp`, `MobileGL/MG_Impl/GLImpl/Buffer/GL_Buffer.cpp`
   - The public `glGetBufferParameteri64v` entry point was still exported through a generic stub macro even though the frontend already tracks the corresponding buffer size, usage, mapping access, and mapped-state data used by `glGetBufferParameteriv`.
   - Impact: applications using the 64-bit buffer-parameter query form could observe untouched output or stale values despite a superficially successful call. That breaks a core buffer-introspection API surface and makes large-buffer-safe query codepaths unusable even for the supported pnames already implemented in the 32-bit variant.
   - Status: fixed in this audit pass.

55. `MobileGL/MG_Impl/GLImpl/Exporting/Definitions.cpp`, `MobileGL/MG_Impl/GLImpl/Program/GL_Program.cpp`
   - The public current-program unsigned uniform setters (`glUniform1ui`, `glUniform2ui`, `glUniform3ui`, `glUniform4ui`, `glUniform1uiv`, `glUniform2uiv`, `glUniform3uiv`, `glUniform4uiv`) were still exported through generic stubs even though the shared frontend uniform upload path already accepts `GLuint` data and the program-specific `glProgramUniform*ui*` family is implemented on top of that same storage path.
   - Impact: applications using unsigned integer uniforms through the ordinary current-program API could appear to succeed while leaving uniform storage unchanged, even though the corresponding program-specific entry points worked. That breaks a real shader data path for any program relying on `uint`, `uvec*`, or unsigned sampler/image-unit values set through the standard `glUniform*ui*` surface.
   - Status: fixed in this audit pass.

56. `MobileGL/MG_Impl/GLImpl/Program/GL_Program.cpp`, `MobileGL/MG_State/GLState/ProgramState/ProgramObject.h`
   - Shared uniform-location validation only rejected locations outside `[-1, maxUniformLocation]`, but reflected programs can legally contain sparse location holes. The uniform getter and setter paths then indexed `m_uniformIndexInTProgram[location]` directly and treated the `layoutLocationEnd` sentinel as if it were a valid reflected-uniform index.
   - Impact: ordinary invalid calls such as `glUniform*` / `glProgramUniform*` / `glGetUniform*` against a vacant location inside a sparse explicit-location range could fall through into out-of-bounds reflected-uniform access instead of raising `GL_INVALID_OPERATION`. That turns a frontend validation error into undefined behavior on a normal public API path, with both wrong-state and memory-safety consequences.
   - Status: fixed in this audit pass.

57. `MobileGL/MG_Impl/GLImpl/Exporting/Definitions.cpp`, `MobileGL/MG_Impl/GLImpl/Program/GL_Program.cpp`
   - The public `glGetUniformuiv` entry point was still exported through a generic stub macro, and the shared uniform getter path also did nothing for opaque uniforms because sampler/image bindings are tracked in `m_uniformSamplerOrImageUnitIndex` rather than in the plain UBO scratch buffer.
   - Impact: unsigned uniform queries could succeed superficially while leaving caller output untouched, and even the already-exported `glGetUniformiv` / `glGetUniformfv` forms returned no value for sampler/image uniforms instead of the currently tracked texture/image unit. That breaks a core uniform introspection path and makes ordinary program-state snapshot/debug logic observe stale or uninitialized data.
   - Status: fixed in this audit pass.

58. `MobileGL/MG_Impl/GLImpl/RenderState/GL_RenderState.cpp`, `MobileGL/MG_Impl/GLImpl/Exporting/Definitions.cpp`
   - Indexed capability handling accepted arbitrary enums through `ConvertGLEnumToCapabilityInput()` and forwarded them into `RenderState::SetCapabilityIndexed()` / `IsCapabilityEnabledIndexed()`, but the render-state object only implements indexed state for per-buffer blending and otherwise throws unimplemented exceptions. Even the supported `GL_BLEND` path never validated `index` before reaching assertion-backed indexed storage.
   - Impact: ordinary invalid calls such as `glEnablei(GL_BLEND, out_of_range)`, `glDisablei(GL_BLEND, out_of_range)`, or `glIsEnabledi(GL_BLEND, out_of_range)` could trip assertion/exception paths instead of reporting GL errors, and `glGetBooleani_v` was still a public stub despite indexed blend state already being tracked. That turns a frontend validation problem into crash-class behavior on a public render-state API surface.
   - Status: fixed in this audit pass.

59. `MobileGL/MG_Impl/GLImpl/Exporting/Definitions.cpp`, `MobileGL/MG_Impl/GLImpl/Program/GL_Program.cpp`
   - The public `glGetActiveUniformName` entry point was still exported through a generic stub macro even though linked-program reflection already tracks active uniform names, counts, and maximum name length, and `glGetActiveUniform` already reads that same metadata through `GetActiveUniform_State()`.
   - Impact: applications querying active uniform names through the dedicated API could observe silent no-op behavior and untouched output buffers despite the information already being available in frontend program state. That breaks a core program-reflection query surface and forces callers to avoid the standard name-only query even though the underlying reflected metadata exists.
   - Status: fixed in this audit pass.

60. `MobileGL/MG_Impl/GLImpl/Program/GL_Program.cpp`, `MobileGL/MG_State/GLState/ProgramState/ProgramObject.h`
   - `GetActiveUniform_State()` treated the API's `index` parameter as if it were a uniform location, even though `glGetActiveUniform` and the newly wired `glGetActiveUniformName` query by active-uniform index in reflected-program order. The same code path also never wrote the required `size` output.
   - Impact: linked programs with explicit or sparse uniform locations could return the wrong uniform name/type for a valid active-uniform query, and callers depending on the array-size result would receive untouched or stale output. That breaks the core active-uniform reflection API even when the program linked successfully and all required metadata was already present.
   - Status: fixed in this audit pass.

61. `MobileGL/MG_Impl/GLImpl/Program/GL_Program.cpp`, `MobileGL/MG_State/GLState/ProgramState/ProgramObject.h`
   - `GetActiveAttrib_State()` had the same indexing mistake on the attribute side: it validated against the number of active pipe inputs, then read names/types from location-expanded attribute tables keyed by vertex attribute location rather than by active-attribute reflection index.
   - Impact: linked programs with explicit attribute locations, gaps, or matrix attributes could return empty names or per-location surrogate types instead of the actual active attribute variable described by the queried index. That breaks a core program-reflection query surface and can mislead application state/introspection logic even though the correct reflected attribute metadata already exists.
   - Status: fixed in this audit pass.

62. `MobileGL/MG_Impl/GLImpl/Program/GL_Program.cpp`, `MobileGL/MG_State/GLState/ProgramState/ProgramObject.h`
   - `GetActiveUniformBlockiv_State()` still mishandled several pnames even though the linked-program reflection already tracked the required uniform-block metadata. `GL_UNIFORM_BLOCK_BINDING` only logged a TODO and then fell through into the invalid-enum path, `GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS` incorrectly returned the total number of uniform blocks instead of the queried block's member count, `GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES` was rejected even though each active uniform already carried its reflected block index, and the per-stage `GL_UNIFORM_BLOCK_REFERENCED_BY_*_SHADER` queries were rejected despite glslang already recording the block stage mask.
   - Impact: applications querying ordinary uniform-block reflection state could receive wrong values or spurious `GL_INVALID_ENUM` errors on supported linked programs. That breaks core program-introspection APIs used for UBO binding/state discovery even though the frontend already had exact reflected answers.
   - Status: fixed in this audit pass.

63. `MobileGL/MG_Impl/GLImpl/Program/GL_Program.cpp`
   - `GetActiveUniformBlockName_State()` unconditionally dereferenced `length` in its debug log after `CopyStr()`, even though the API allows `length == nullptr`.
   - Impact: a valid `glGetActiveUniformBlockName(..., nullptr, ...)` call could crash in the frontend logging path after otherwise successful query processing. That turns a benign optional-output omission into a public API crash vector.
   - Status: fixed in this audit pass.

64. `MobileGL/MG_Impl/GLImpl/Exporting/Definitions.cpp`, `MobileGL/MG_Impl/GLImpl/Program/GL_Program.cpp`, `MobileGL/MG_State/GLState/ProgramState/ProgramObject.h`
   - The public `glGetUniformIndices` entry point was still exported through a generic stub macro even though linked-program reflection already tracked active uniform names in stable active-uniform index order and exposed a name-to-index lookup path through glslang.
   - Impact: applications querying active uniform indices by name could observe silent no-op behavior and untouched output buffers instead of the standard `GLuint` index list or `GL_INVALID_INDEX` fallbacks for missing names. That breaks a core prerequisite for higher-level uniform reflection flows such as `glGetActiveUniformsiv`.
   - Status: fixed in this audit pass.

65. `MobileGL/MG_Impl/GLImpl/Program/GL_Program.cpp`
   - Several `glGetProgramiv` length queries returned raw string lengths instead of the OpenGL-required buffer sizes including the terminating NUL: `GL_INFO_LOG_LENGTH`, `GL_ACTIVE_ATTRIBUTE_MAX_LENGTH`, `GL_ACTIVE_UNIFORM_MAX_LENGTH`, and `GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH`.
   - Impact: applications relying on these lengths to allocate reflection or info-log buffers could allocate one byte too small, then truncate names/logs or mis-handle the returned strings despite otherwise valid queries. That breaks core program-introspection semantics on standard allocation/query flows.
   - Status: fixed in this audit pass.

66. `MobileGL/MG_Impl/GLImpl/Program/GL_Program.cpp`, `MobileGL/MG_State/GLState/ProgramState/ProgramObject.h`
   - `GetProgramiv(GL_COMPUTE_WORK_GROUP_SIZE, ...)` still delegated to the backend function table and, when unsupported there, raised `GL_INVALID_OPERATION` and returned fabricated fallback values `(1, 1, 1)`. Linked-program reflection already carried the compute local sizes directly via glslang.
   - Impact: valid compute-program introspection could fail or report the wrong work-group size purely because a backend query hook was absent, even though the frontend had the exact linked-program metadata. That breaks a core compute reflection query and can misconfigure dispatch sizing logic in applications that query before issuing work.
   - Status: fixed in this audit pass.

67. `MobileGL/MG_Impl/GLImpl/Program/GL_Program.cpp`
   - `GetProgramiv(GL_INFO_LOG_LENGTH, ...)` returned `1` for an empty program info log after the earlier length fix, while the adjacent shader query path already treats an empty info log as length `0`.
   - Impact: successful or clean-link program objects with no log could report a phantom one-byte log buffer requirement, diverging from the project's own shader-query behavior and causing callers to allocate and process a nonexistent log string.
   - Status: fixed in this audit pass.

68. `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp`
   - `glGetIntegeri_v` and `glGetInteger64i_v` still delegated all indexed buffer queries to backend hooks even though the frontend already tracked indexed buffer binding points and ranges for `glBindBufferBase` / `glBindBufferRange`. On backends without those getter hooks, ordinary queries such as `GL_UNIFORM_BUFFER_BINDING`, `GL_UNIFORM_BUFFER_START`, `GL_UNIFORM_BUFFER_SIZE`, and the corresponding transform-feedback / atomic-counter / shader-storage variants fell back to `GL_INVALID_OPERATION` plus zero results.
   - Impact: applications querying indexed buffer bindings could observe backend-dependent failures or dummy zeros despite the queried state already being maintained accurately in frontend GL state. That breaks core introspection/state-cache flows around UBO/SSBO/transform-feedback/atomic-counter binding points.
   - Status: fixed in this audit pass.

69. `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp`
   - The non-indexed generic buffer-binding queries `GL_TRANSFORM_FEEDBACK_BUFFER_BINDING` and `GL_UNIFORM_BUFFER_BINDING` in `glGetIntegerv` were still hardcoded to `0` even though the frontend already tracked the current generic binding slot for both targets.
   - Impact: applications querying the current transform-feedback or uniform-buffer binding could observe false unbound state, breaking ordinary state snapshot/restore logic and any validation layered on top of those getters.
   - Status: fixed in this audit pass.

70. `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp`
   - Several `glGetIntegerv(GL_TEXTURE_BINDING_*, ...)` queries were still hardcoded to `0` even though `TextureUnit` already tracked per-unit bindings for those targets. The affected getters included `GL_TEXTURE_BINDING_1D`, `GL_TEXTURE_BINDING_1D_ARRAY`, `GL_TEXTURE_BINDING_2D_ARRAY`, `GL_TEXTURE_BINDING_2D_MULTISAMPLE`, `GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY`, `GL_TEXTURE_BINDING_BUFFER`, and `GL_TEXTURE_BINDING_RECTANGLE`.
   - Impact: applications querying the active texture unit's bound object for those targets could observe false unbound state, breaking texture-state snapshot/restore code and any validation that relies on the standard getter surface for non-2D/non-3D targets.
   - Status: fixed in this audit pass.

71. `MobileGL/MG_State/GLState/FramebufferState/FramebufferObject.cpp`, `MobileGL/MG_Impl/Init.cpp`
   - Framebuffer object `0` started with framebuffer-object-style color-buffer state (`Color0`) even though the rest of the frontend/backend default-framebuffer paths expect default-buffer tokens such as `BackLeft`. The default framebuffer initialization also attached its color image only at `Color0`, so consumers that dereferenced the stored read/draw buffer token could resolve the default framebuffer color attachment incorrectly before any explicit `glReadBuffer` / `glDrawBuffer` call.
   - Impact: default-framebuffer color-buffer state could be reported or consumed incorrectly out of the box, skewing copy/readback/render-pass selection paths that rely on the tracked read/draw buffer state for framebuffer object `0`.
   - Status: fixed in this audit pass.

72. `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp`
   - `glGetIntegerv` still hardcoded several already-tracked binding/state queries to `0`, including `GL_DRAW_BUFFER`, `GL_DRAW_BUFFER0`, `GL_READ_BUFFER`, `GL_PIXEL_PACK_BUFFER_BINDING`, `GL_PIXEL_UNPACK_BUFFER_BINDING`, and `GL_RENDERBUFFER_BINDING`.
   - Impact: applications querying current framebuffer buffer-selection state or pixel-pack / pixel-unpack / renderbuffer bindings could observe false default or unbound state, breaking ordinary state snapshot/restore flows and any validation layered on top of the standard getter surface.
   - Status: fixed in this audit pass.

73. `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp`
   - Clear-state getters were still inconsistent with the tracked frontend state: `glGetFloatv(GL_DEPTH_CLEAR_VALUE, ...)` fell back through the integer getter path and truncated non-integral depth clear values, while `glGetIntegerv(GL_STENCIL_CLEAR_VALUE, ...)` was still hardcoded to `0` even though `glClearStencil` already updated tracked render state.
   - Impact: applications querying clear-depth / clear-stencil state could observe stale or numerically wrong values, breaking state mirroring and any code that snapshots or restores clear state through the standard getter surface.
   - Status: fixed in this audit pass.

74. `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp`
   - `glGetBooleanv` still derived several tracked floating-point states by first truncating them through `glGetIntegerv`, so fractional non-zero values such as `BlendColor = 0.5`, `ClearColor = 0.5`, `DepthRange = 0.5`, or `DepthClearValue = 0.5` could be reported as `GL_FALSE`.
   - Impact: applications querying boolean views of those float states could observe false zeros for legitimate non-zero tracked state, breaking generic state-diff/snapshot code that relies on `glGetBooleanv` conversions to mirror current state.
   - Status: fixed in this audit pass.

75. `MobileGL/MG_Impl/GLImpl/RenderState/GL_RenderState.cpp`, `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp`, `MobileGL/MG_State/GLState/RenderState/RenderState.cpp`, `MobileGL/MG_Backend/DirectGLES/DirectGLES.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/PipelineFactory.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.cpp`
   - The exported stencil state API is still largely non-functional. `glStencilFunc[Separate]`, `glStencilMask[Separate]`, and `glStencilOp[Separate]` all route into state helpers whose bodies are empty, while the corresponding getter surface (`GL_STENCIL_FUNC`, `GL_STENCIL_REF`, `GL_STENCIL_VALUE_MASK`, `GL_STENCIL_FAIL`, `GL_STENCIL_PASS_DEPTH_FAIL`, `GL_STENCIL_PASS_DEPTH_PASS`, and all back-face variants) still returns placeholder zeroes.
   - Impact: applications cannot configure or reliably query stencil test behavior through the frontend at all. Any rendering path that depends on stencil compare/op/mask state will silently use fabricated defaults instead of the caller-provided GL state.
   - Status: fixed in this audit pass.

76. `MobileGL/MG_State/GLState/RenderState/RenderState.cpp`, `MobileGL/MG_Backend/DirectGLES/DirectGLES.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/PipelineFactory.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.cpp`
   - `glEnable(GL_STENCIL_TEST)` / `glDisable(GL_STENCIL_TEST)` updated the public render-state entry points and the getter surface, but the tracked render-state capability table ignored `StencilTest`, the DirectGLES sync path never forwarded it to native GL, and DirectVulkan pipeline creation hard-disabled stencil testing.
   - Impact: applications could report stencil testing enabled in frontend state while both backends still rendered with stencil testing forcibly disabled, so any path relying on stencil compare/reject behavior silently behaved as if `GL_STENCIL_TEST` were off.
   - Status: fixed in this audit pass.

77. `MobileGL/MG_Impl/GLImpl/Framebuffer/GL_Framebuffer.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/VkRenderPassManager.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.cpp`
   - The frontend accepts and validates standalone `GL_STENCIL_ATTACHMENT` usage, but the DirectVulkan render-pass builder only materialized a depth/stencil attachment from `FramebufferAttachmentType::Depth` and set `hasDepthStencilAttachment` solely from that path. `FramebufferAttachmentType::Stencil` was hashed and checked for pending clears, yet never turned into the subpass depth/stencil attachment actually used for rendering.
   - Impact: Vulkan rendering to framebuffer objects that relied on a stencil attachment without a matching depth attachment, or that expected stencil clears/tests to come from the dedicated stencil attachment path, could silently lose the stencil attachment at render-pass creation time. That disabled stencil testing on those FBOs and could drop pending stencil clears despite frontend completeness succeeding.
   - Status: fixed in this audit pass.

78. `MobileGL/MG_Impl/GLImpl/Framebuffer/GL_Framebuffer.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/VkRenderPassManager.cpp`
   - The frontend previously allowed framebuffer objects to bind different images to `GL_DEPTH_ATTACHMENT` and `GL_STENCIL_ATTACHMENT` even though the DirectVulkan backend can only build a single Vulkan depth/stencil subpass attachment. This audit pass now treats such non-default FBOs as explicitly unsupported when DirectVulkan is active: `glCheckFramebufferStatus` returns `GL_FRAMEBUFFER_UNSUPPORTED`, DirectVulkan draw setup rejects them with `GL_INVALID_FRAMEBUFFER_OPERATION`, and DirectVulkan depth/stencil clear entrypoints reject them before any pending clear is queued.
   - Impact: before the fix, Vulkan rendering on framebuffer objects that used separate depth and stencil textures could silently diverge from frontend GL state because one of the two attachments was ignored during render-pass creation. After the fix, that unsupported configuration fails explicitly instead of rendering or clearing against the wrong image.
   - Status: fixed in this audit pass.

79. `MobileGL/MG_Impl/GLImpl/RenderState/GL_RenderState.cpp`, `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp`, `MobileGL/MG_State/GLState/RenderState/RenderState.cpp`, `MobileGL/MG_State/GLState/Core.cpp`, `MobileGL/MG_Backend/DirectGLES/DirectGLES.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.cpp`, `MobileGL/MG_Util/BackendLoaders/OpenGL/Loader.cpp`, `MobileGL/MG_Util/BackendLoaders/Vulkan/Loader.cpp`
   - `glLineWidth` was exported but its state setter was a no-op, `GL_LINE_WIDTH` readback stayed at a placeholder path, the line-width capability queries (`GL_ALIASED_LINE_WIDTH_RANGE`, `GL_SMOOTH_LINE_WIDTH_RANGE`, `GL_SMOOTH_LINE_WIDTH_GRANULARITY`) were not backed by real backend limits, and neither backend consumed tracked line width during draw setup. The Vulkan path also needed `wideLines` feature enablement to honor widths above `1.0`.
   - Impact: applications could not reliably configure or query line rasterization width through the frontend, DirectGLES silently kept native GL at its previous/default line width, and DirectVulkan either rendered with the wrong width or relied on invalid Vulkan state for wide lines.
   - Status: fixed in this audit pass.

80. `MobileGL/MG_Impl/GLImpl/RenderState/GL_RenderState.cpp`, `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp`, `MobileGL/MG_State/GLState/RenderState/RenderState.cpp`, `MobileGL/MG_State/GLState/Core.cpp`, `MobileGL/MG_Backend/DirectGLES/DirectGLES.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/PipelineFactory.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.cpp`
   - `glPolygonOffset` was exported but its state setter was a no-op, `GL_POLYGON_OFFSET_FACTOR` / `GL_POLYGON_OFFSET_UNITS` still returned placeholder zeroes, `GL_POLYGON_OFFSET_FILL` enable state was not tracked in render state, and neither backend applied polygon offset during draw setup.
   - Impact: applications could not request depth bias for filled polygons at all. Common decal/shadow-volume/coplanar-overlay rendering paths silently rendered without the requested depth offset, increasing z-fighting and causing frontend state queries to disagree with actual backend rasterization.
   - Status: fixed in this audit pass.

81. `MobileGL/MG_State/GLState/RenderState/RenderState.cpp`, `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp`, `MobileGL/MG_Backend/DirectGLES/DirectGLES.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/PipelineFactory.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.cpp`
   - `glEnable(GL_RASTERIZER_DISCARD)` / `glDisable(GL_RASTERIZER_DISCARD)` were decoded by the frontend enum conversion layer and exposed through the getter surface, but the tracked render-state capability table ignored `RasterizerDiscard`, DirectGLES never forwarded it to native GL, and DirectVulkan pipeline creation never enabled `rasterizerDiscardEnable`.
   - Impact: applications could believe rasterizer discard was enabled while both backends still rasterized primitives normally, so transform-feedback / stream-out style passes or draw suppression logic relying on `GL_RASTERIZER_DISCARD` silently produced fragments anyway.
   - Status: fixed in this audit pass.

82. `MobileGL/MG_Impl/GLImpl/RenderState/GL_RenderState.cpp`, `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp`, `MobileGL/MG_State/GLState/RenderState/RenderState.cpp`, `MobileGL/MG_State/GLState/Core.cpp`, `MobileGL/MG_Backend/DirectGLES/DirectGLES.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/PipelineFactory.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.cpp`, `MobileGL/MG_Util/BackendLoaders/OpenGL/Loader.cpp`
   - The frontend exported `glLogicOp` and advertised `GL_COLOR_LOGIC_OP` / `GL_LOGIC_OP_MODE`, but the state setter was a no-op, the getter path returned placeholder zeroes, the tracked render-state capability table ignored `ColorLogicOp`, DirectGLES did not even load/sync `glLogicOp`, and DirectVulkan pipeline creation never enabled Vulkan logic-op state or the required device feature.
   - Impact: applications enabling color logic operations could not configure or query the requested logic-op mode reliably, and both backends kept using ordinary color blending/write behavior instead of the caller-selected logic operation.
   - Status: fixed in this audit pass.

83. `MobileGL/MG_Impl/GLImpl/Framebuffer/GL_Framebuffer.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/VkRenderPassManager.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.cpp`
   - The frontend previously accepted framebuffer renderbuffer attachments through `glFramebufferRenderbuffer` and could report those FBOs complete even though the DirectVulkan render-pass builder only materializes texture-backed attachments. This audit pass now treats non-default FBOs with complete renderbuffer attachments as explicitly unsupported when DirectVulkan is active: `glCheckFramebufferStatus` returns `GL_FRAMEBUFFER_UNSUPPORTED`, DirectVulkan draw and clear paths reject them with `GL_INVALID_FRAMEBUFFER_OPERATION`, and DirectVulkan `glBlitFramebuffer` / `glCopyTexSubImage2D` reject them before falling into backend skip-only paths.
   - Impact: before the fix, valid non-default framebuffer objects backed by renderbuffers could silently lose their color outputs or depth/stencil attachment when rendered through the Vulkan backend because those attachments were treated as `VK_ATTACHMENT_UNUSED`. After the fix, the unsupported configuration fails explicitly instead of degrading into dropped attachments or skip-only backend behavior.
   - Status: fixed in this audit pass.

84. `MobileGL/MG_Impl/GLImpl/Framebuffer/GL_Framebuffer.cpp`, `MobileGL/MG_State/GLState/RenderbufferState/RenderbufferObject.cpp`, `MobileGL/MG_Backend/DirectGLES/Managers.cpp`
   - `glRenderbufferStorageMultisample` was exported but its state helper was a complete no-op. The frontend never allocated storage, never recorded the requested sample count, `glGetRenderbufferParameteriv(GL_RENDERBUFFER_SAMPLES, ...)` therefore stayed at the default value, and the DirectGLES renderbuffer backend always allocated single-sample storage with `glRenderbufferStorage` even when a multisampled renderbuffer state object requested samples.
   - Impact: multisampled renderbuffer creation through the frontend was non-functional. Applications requesting multisampled renderbuffers could observe unallocated or wrongly single-sampled backend storage, misleading sample-count queries, and incorrect framebuffer/readback behavior in paths that consult the tracked renderbuffer sample count.
   - Status: fixed in this audit pass.

85. `MobileGL/MG_Impl/GLImpl/Framebuffer/GL_Framebuffer.cpp`, `MobileGL/MG_State/GLState/FramebufferState/FramebufferObject.cpp`
   - The exported framebuffer texture attachment entrypoints `glFramebufferTextureLayer`, `glFramebufferTexture3D`, `glFramebufferTexture1D`, and generic `glFramebufferTexture` previously routed into empty state helpers. This audit pass implemented real frontend attachment mutation for `glFramebufferTexture1D` and for the subset of generic `glFramebufferTexture` targets that the current attachment model can actually represent (`1D`, `2D`, `rectangle`, `2D multisample`), and changed the unsupported `glFramebufferTextureLayer` / `glFramebufferTexture3D` / layered generic-target paths to raise explicit frontend errors instead of silently succeeding. The framebuffer attachment model still only stores a texture object plus mip level, so there is still no layered/layer-index attachment representation behind those unsupported entrypoints.
   - Impact: non-layered framebuffer texture attachment calls that fit the current model now really update framebuffer state instead of being silently dropped, and unsupported layered/layer-index entrypoints no longer pretend to succeed while leaving attachments unchanged. Applications that rely on true layered attachments or `3D` slice attachments still cannot express those framebuffer states through the current frontend model.
   - Status: partially fixed in this audit pass; still open for true layered/layer-index framebuffer attachment support.

86. `MobileGL/MG_Impl/GLImpl/Framebuffer/GL_Framebuffer.cpp`, `MobileGL/MG_State/GLState/FramebufferState/FramebufferObject.cpp`, `MobileGL/MG_Backend/DirectGLES/Managers.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/VkRenderPassManager.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/VkTextureManager.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.cpp`
   - `glFramebufferTexture2D` previously discarded the `textarget` face token when attaching cube-map textures. The frontend attachment model stored only the texture object plus mip level, framebuffer attachment queries always reported `GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE = 0`, completeness sizing hardcoded `TextureUploadTarget::Texture2D`, and DirectGLES replay rebound cube attachments with `GL_TEXTURE_CUBE_MAP` instead of the selected face. DirectVulkan also used full-mip image views rooted at `baseArrayLayer = 0`, and framebuffer blit/copy helpers hardcoded transfer subresources to layer 0, so cube-map framebuffer attachments there still could not target a specific face.
   - Impact: framebuffer objects attaching cube-map faces could report the wrong attachment metadata and bind the wrong image on the DirectGLES backend, effectively treating face attachments as generic cube textures. On DirectVulkan, render passes and framebuffer blits/copies also collapsed cube-face attachments to layer 0, causing rendering and image transfer commands to hit the wrong cubemap face.
   - Status: fixed in this audit pass.

87. `MobileGL/MG_Backend/DirectVulkan/Renderer/VkClearManager.h`, `MobileGL/MG_Backend/DirectVulkan/Renderer/VkClearManager.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/VkRenderPassManager.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.cpp`
   - The DirectVulkan deferred-clear path previously keyed pending clears only by texture object. `glClear` / `glClearBuffer*` on texture-backed FBO attachments at nonzero mip levels or different attached subresources were merged together, render-pass load-op selection ignored the attached mip/layer, and `MaterializePendingClearForTexture()` always cleared `mip 0`, `layer 0`.
   - Impact: framebuffer clears on texture attachments could be applied to the wrong subresource or collapse multiple pending attachment clears into one texture-wide clear. Applications that clear an attached mip level and then sample/copy from it before a render pass consumes the clear could observe stale contents in the intended mip while an unrelated mip/layer was cleared instead.
   - Status: fixed in this audit pass.

### Medium

1. `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp`, `MobileGL/MG_Impl/GLImpl/RenderState/GL_RenderState.cpp`, `MobileGL/MG_Impl/GLImpl/Framebuffer/GL_Framebuffer.cpp`, `MobileGL/MG_State/GLState/RenderState/RenderState.cpp`, `MobileGL/MG_State/GLState/Core.cpp`, `MobileGL/MG_Backend/DirectGLES/DirectGLES.cpp`, `MobileGL/MG_Backend/BackendObject.h`, `MobileGL/MG_Backend/DirectGLES/BackendObject_DirectGLES.cpp`, `MobileGL/MG_Backend/DirectVulkan/BackendObject_DirectVulkan.cpp`, `MobileGL/MG_Util/BackendLoaders/OpenGL/Loader.h`, `MobileGL/MG_Util/BackendLoaders/OpenGL/Loader.cpp`, `MobileGL/MG_Util/BackendLoaders/Vulkan/Loader.h`, `MobileGL/MG_Util/BackendLoaders/Vulkan/Loader.cpp`
   - Several multi-component or capability-backed getters still return placeholder scalar values instead of correct state vectors or backend limits. This audit pass fixed the point-size range/granularity path, viewport-dimension/subpixel queries, `GL_VIEWPORT_BOUNDS_RANGE`, frontend-backed indexed buffer binding/range getters including correct clamping of indexed buffer `SIZE` queries to the bound object's real size, `GL_DRAW_BUFFERi` readback for the tracked draw-buffer array, provoking-vertex convention getters, a handful of fixed-default queries (`GL_FRAGMENT_SHADER_DERIVATIVE_HINT`, `GL_LINE_SMOOTH_HINT`, `GL_POLYGON_SMOOTH_HINT`, `GL_TEXTURE_COMPRESSION_HINT`, `GL_POINT_FADE_THRESHOLD_SIZE`), frontend-tracked capability readback for previously untracked enable bits (`GL_DITHER`, `GL_MULTISAMPLE`, `GL_SAMPLE_ALPHA_TO_COVERAGE`, `GL_SAMPLE_ALPHA_TO_ONE`, `GL_SAMPLE_COVERAGE`, `GL_SAMPLE_MASK`, `GL_PROGRAM_POINT_SIZE`, `GL_POLYGON_OFFSET_LINE`, `GL_POLYGON_OFFSET_POINT`, `GL_POLYGON_SMOOTH`, `GL_PRIMITIVE_RESTART`, `GL_PRIMITIVE_RESTART_FIXED_INDEX`, `GL_LINE_SMOOTH`) through `glGet*` / `glIsEnabled` with DirectGLES replay now also covering the natively supported generic enable caps `GL_DITHER`, `GL_MULTISAMPLE`, and `GL_SAMPLE_ALPHA_TO_COVERAGE`, and frontend-only debug enable bits (`GL_DEBUG_OUTPUT`, `GL_DEBUG_OUTPUT_SYNCHRONOUS`) so `glEnable/glDisable/glIsEnabled` no longer silently forget those states while the corresponding debug-message entrypoints remain stubbed, explicit zero/empty reporting for unsupported public API surfaces whose entrypoints are still stubbed (`GL_MAX_DEBUG_GROUP_STACK_DEPTH`, `GL_DEBUG_GROUP_STACK_DEPTH`, `GL_NUM_SHADER_BINARY_FORMATS`, `GL_PROGRAM_BINARY_FORMATS`, `GL_PROGRAM_PIPELINE_BINDING`, the default `GL_PRIMITIVE_RESTART_INDEX`, `GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT`, `GL_TIMESTAMP`, `GL_VERTEX_BINDING_DIVISOR`, `GL_VERTEX_BINDING_OFFSET`, `GL_VERTEX_BINDING_STRIDE`, `GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET`, and `GL_MAX_VERTEX_ATTRIB_BINDINGS`), explicit empty compressed-texture capability reporting because the compressed texture upload entrypoints still throw unimplemented exceptions, `GL_DOUBLEBUFFER` readback from the current EGL draw-surface kind instead of a hardcoded zero, `GL_IMPLEMENTATION_COLOR_READ_FORMAT` / `GL_IMPLEMENTATION_COLOR_READ_TYPE` readback from the currently bound read-framebuffer attachment format, current draw-framebuffer `GL_SAMPLE_BUFFERS` / `GL_SAMPLES` readback for implemented multisampled renderbuffer attachments, sample-coverage/sample-mask state and getter readback (`GL_SAMPLE_COVERAGE_VALUE`, `GL_SAMPLE_COVERAGE_INVERT`, `GL_SAMPLE_MASK_VALUE`) with DirectGLES replay, backend-fed texture/framebuffer size limits (`GL_MAX_3D_TEXTURE_SIZE`, `GL_MAX_ARRAY_TEXTURE_LAYERS`, `GL_MAX_CUBE_MAP_TEXTURE_SIZE`, `GL_MAX_FRAMEBUFFER_WIDTH`, `GL_MAX_FRAMEBUFFER_HEIGHT`, `GL_MAX_FRAMEBUFFER_LAYERS`, `GL_MAX_RENDERBUFFER_SIZE`, `GL_MAX_TEXTURE_SIZE`), backend-fed sample limits (`GL_MAX_COLOR_TEXTURE_SAMPLES`, `GL_MAX_DEPTH_TEXTURE_SAMPLES`, `GL_MAX_FRAMEBUFFER_SAMPLES`, `GL_MAX_INTEGER_SAMPLES`, `GL_MAX_SAMPLES`, `GL_MAX_SAMPLE_MASK_WORDS`), backend-fed texture-unit/vertex-attrib limits (`GL_MAX_TEXTURE_IMAGE_UNITS`, `GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS`, `GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS`, `GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS`, `GL_MAX_VERTEX_ATTRIBS`), backend-fed buffer/storage limits (`GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS`, `GL_MAX_TEXTURE_BUFFER_SIZE`, `GL_MAX_UNIFORM_BUFFER_BINDINGS`, `GL_MAX_UNIFORM_BLOCK_SIZE`), backend-fed compute descriptor/workgroup limits (`GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS`, `GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS`, `GL_MAX_COMPUTE_UNIFORM_BLOCKS`, `GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS`), compute default/combined uniform-component limits derived from the frontend compiler scalar limit plus backend uniform-block capacity (`GL_MAX_COMPUTE_UNIFORM_COMPONENTS`, `GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS`), frontend compiler-backed GL 3.3 core uniform/varying/program-offset limits (`GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS`, `GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS`, `GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS`, `GL_MAX_COMBINED_UNIFORM_BLOCKS`, `GL_MAX_FRAGMENT_INPUT_COMPONENTS`, `GL_MAX_FRAGMENT_UNIFORM_COMPONENTS`, `GL_MAX_FRAGMENT_UNIFORM_VECTORS`, `GL_MAX_FRAGMENT_UNIFORM_BLOCKS`, `GL_MAX_GEOMETRY_INPUT_COMPONENTS`, `GL_MAX_GEOMETRY_OUTPUT_COMPONENTS`, `GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS`, `GL_MAX_GEOMETRY_UNIFORM_COMPONENTS`, `GL_MAX_GEOMETRY_UNIFORM_BLOCKS`, `GL_MAX_PROGRAM_TEXEL_OFFSET`, `GL_MIN_PROGRAM_TEXEL_OFFSET`, `GL_MAX_VARYING_COMPONENTS`, `GL_MAX_VARYING_VECTORS`, `GL_MAX_VERTEX_UNIFORM_COMPONENTS`, `GL_MAX_VERTEX_UNIFORM_VECTORS`, `GL_MAX_VERTEX_OUTPUT_COMPONENTS`, `GL_MAX_VERTEX_UNIFORM_BLOCKS`), frontend compiler-backed atomic-counter limits for all exposed shader-stage and combined count queries (`GL_MAX_COMPUTE_ATOMIC_COUNTERS`, `GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS`, `GL_MAX_COMBINED_ATOMIC_COUNTERS`, `GL_MAX_FRAGMENT_ATOMIC_COUNTERS`, `GL_MAX_GEOMETRY_ATOMIC_COUNTERS`, `GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS`, `GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS`, `GL_MAX_VERTEX_ATOMIC_COUNTERS`), backend-fed image-unit limits (`GL_MAX_IMAGE_UNITS`, `GL_MAX_COMBINED_IMAGE_UNIFORMS`, `GL_MAX_COMPUTE_IMAGE_UNIFORMS`, `GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS`), backend-fed draw-target limits (`GL_MAX_DRAW_BUFFERS`, `GL_MAX_COLOR_ATTACHMENTS`), backend-fed clip-distance/viewport-count limits (`GL_MAX_CLIP_DISTANCES`, `GL_MAX_VIEWPORTS`), and corrected indexed-only buffer range pnames so `GetIntegerv` no longer silently returns `0` for queries that are only valid through indexed getter entrypoints. Other placeholder-backed getters still remain.
   - Impact: the fixed queries now report backend-backed values, frontend-tracked state, attachment-derived readback formats, implemented framebuffer/sample state, real backend size/sample/texture-unit limits, current EGL default-framebuffer buffering semantics, or the correct GL defaults instead of placeholder scalars, and invalid non-indexed buffer range queries now raise the correct GL error instead of fabricating a zero result. Program/shader binary capability queries also no longer over-advertise unsupported functionality, and compressed texture capability queries no longer claim support for entrypoints that still throw unimplemented exceptions. The unresolved getters still need real backend capability plumbing instead of hardcoded placeholders.
   - Status: partially fixed in this audit pass; still open for the remaining placeholder queries.

2. `MobileGL/MG_Backend/DirectGLES/Utils.cpp`
   - `CheckGLESError()` used `while (GLenum err = g_GLESFuncs.glGetError() != GL_NO_ERROR)`, so `err` received the boolean result of the comparison instead of the actual GL error enum.
   - Impact: DirectGLES error logging could only report `GL_TRUE`/`GL_FALSE`-like garbage values rather than the real backend error code, which undermines diagnosis of backend failures during this renderer path.
   - Status: fixed in this audit pass.

3. `MobileGL/MG_Impl/GLImpl/Framebuffer/GL_Framebuffer.cpp`
   - `glIsRenderbuffer` and `glIsFramebuffer` previously tested only whether a name had been generated, not whether a renderbuffer/framebuffer object had actually been created.
   - Impact: freshly generated but never-bound names could be reported as existing objects, which violates GL object-lifetime semantics and can mislead application-side state tracking or object-probing code that relies on `glIs*` to distinguish names from realized objects.
   - Status: fixed in this audit pass.

4. `MobileGL/MG_Util/Converters/GLToMG/DataTypeConverter.cpp`, `MobileGL/MG_Util/Converters/MGToGL/DataTypeConverter.cpp`
   - The GL/data-type conversion tables omitted `GL_FIXED` / `DataType::Fixed32` in both directions even though `Fixed32` is a first-class frontend data type and DirectGLES vertex-attribute setup calls the reverse converter when issuing `glVertexAttribPointer` / `glVertexAttribIPointer`.
   - Impact: valid fixed-point vertex attribute declarations were rejected on input (`GL_FIXED -> DataType::Unknown`) and any backend path trying to emit a stored `Fixed32` attribute type back to native GL got `GL_UNKNOWN_MGL` instead of `GL_FIXED`, breaking fixed-point vertex format handling end-to-end.
   - Status: fixed in this audit pass.

5. `MobileGL/MG_Test/Backend/DirectVulkan/SanityTest.cpp`, `MobileGL/MG_Test/Backend/DirectVulkan/TestExec.cpp`
   - The GLFW/EGL window-surface setup only assigned `nativewindow` on `_WIN32` and `__APPLE__`. On Linux, the code left the handle at zero and still passed it into `eglCreateWindowSurface()`.
   - Impact: the Linux DirectVulkan sanity/example path could not create a valid EGL window surface at all; it was guaranteed to request surface creation with a null native window handle instead of the GLFW X11 window.
   - Status: fixed in this audit pass.

6. `MobileGL/MG_Util/Types.h`
   - `Range1D` used `SizeT end = ~0u` as its default "unbounded" upper limit. On 64-bit builds, `~0u` is only a 32-bit all-ones value, so the default range silently truncated to `0x00000000FFFFFFFF` instead of the full `size_t` range.
   - Impact: any path that relies on default `Range1D()` meaning "all remaining bytes" can be clipped at 4 GiB on 64-bit hosts. This affects generic range-carrying utilities such as indexed buffer binding ranges and any future large-resource code that depends on the default sentinel upper bound.
   - Status: fixed in this audit pass.

7. `MobileGL/MG_Util/ShaderTranspiler/SpvcSession.h`
   - `SpvcSession`'s default constructor left the `usage` flag set uninitialized. Multiple program/shader utility paths first default-construct `Vector<SpvcSession>` elements and then move-assign real sessions into them.
   - Impact: before assignment, the old object state can carry garbage `usage` bits into move/swap/destructor paths and later branch checks, making `spvc_context_destroy()` / `spvReflectDestroyShaderModule()` cleanup behavior and mode-gated methods depend on uninitialized memory instead of real session state.
   - Status: fixed in this audit pass.

8. `MobileGL/MG_Impl/GLImpl/RenderState/GL_RenderState.cpp`, `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp`, `MobileGL/MG_State/GLState/RenderState/RenderState.cpp`, `MobileGL/MG_State/GLState/Core.cpp`, `MobileGL/MG_Backend/DirectGLES/DirectGLES.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.cpp`
   - `glPointSize` previously routed into an empty state helper, `GL_POINT_SIZE` readback remained placeholder-backed, and DirectGLES never synchronized a frontend-managed fixed point size to native GL. This audit pass added tracked point-size state, validation, getter readback, and DirectGLES replay, but DirectVulkan still has no fixed-function or program-driven point-size handling in its draw/pipeline path.
   - Impact: frontend `glPointSize` state and `GL_POINT_SIZE` queries now behave correctly, and DirectGLES point rendering can honor the tracked fixed size. DirectVulkan rendering of point primitives can still diverge from GL state because there is still no backend point-size path to apply fixed size or shader-driven `gl_PointSize`.
   - Status: partially fixed in this audit pass; still open for DirectVulkan point-size support.

## Reviewed Source Files

Reviewed in this audit pass:
- `MobileGL/MG_Backend/DirectVulkan/Renderer/BufferArena.cpp`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/BufferArena.h`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/BufferSlice.h`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/SwapchainObject.cpp`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/SwapchainObject.h`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/FrameContext.cpp`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/FrameContext.h`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/PipelineFactory.cpp`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/PipelineFactory.h`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/ProgramFactory.cpp`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/ProgramFactory.h`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/UniformManager.h`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/VertexInputStateBuilder.cpp`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/VertexInputStateBuilder.h`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/VertexInputStateFactory.cpp`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/VkBufferManager.cpp`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/VkBufferManager.h`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/VkBufferObject.cpp`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/VkBufferObject.h`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/VkClearManager.cpp`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/VkClearManager.h`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/VkRenderPassManager.cpp`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/VkRenderPassManager.h`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/VkSamplerManager.cpp`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/VkSamplerManager.h`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/VkTextureManager.cpp`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/VkTextureManager.h`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/UniformManager.cpp`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.cpp`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.h`
- `MobileGL/MG_Backend/DirectVulkan/DirectVulkan.cpp`
- `MobileGL/MG_Backend/DirectVulkan/BackendObject_DirectVulkan.cpp`
- `MobileGL/MG_Backend/DirectVulkan/BackendObject_DirectVulkan.h`
- `MobileGL/MG_Backend/DirectGLES/DirectGLES.cpp`
- `MobileGL/MG_Backend/DirectGLES/DirectGLES.h`
- `MobileGL/MG_Backend/DirectGLES/BackendObject_DirectGLES.cpp`
- `MobileGL/MG_Backend/DirectGLES/BackendObject_DirectGLES.h`
- `MobileGL/MG_Backend/DirectGLES/Managers.cpp`
- `MobileGL/MG_Backend/DirectGLES/Managers.h`
- `MobileGL/MG_Backend/DirectGLES/Utils.cpp`
- `MobileGL/MG_Backend/DirectGLES/Utils.h`
- `MobileGL/MG_Backend/BackendObject.cpp`
- `MobileGL/MG_Backend/BackendObject.h`
- `MobileGL/MG_Backend/BackendObjects.h`
- `MobileGL/MG_Impl/EGLImpl/EGLImpl.cpp`
- `MobileGL/MG_Impl/EGLImpl/EGLImpl.h`
- `MobileGL/MG_Impl/EGLImpl/Exporting/Definitions.cpp`
- `MobileGL/MG_Impl/GetProcAddress.cpp`
- `MobileGL/MG_Impl/GetProcAddress.h`
- `MobileGL/MG_Impl/GLImpl/Sync/GL_Sync.cpp`
- `MobileGL/MG_Impl/GLImpl/Sync/GL_Sync.h`
- `MobileGL/MG_State/EGLState/Core.cpp`
- `MobileGL/MG_State/EGLState/Core.h`
- `MobileGL/MG_State/GLState/Core.cpp`
- `MobileGL/MG_State/GLState/Core.h`
- `MobileGL/MG_State/GLState/ProgramState/ProgramObject.h`
- `MobileGL/MG_State/GLState/ProgramState/ProgramObject.cpp`
- `MobileGL/MG_State/GLState/ProgramState/ShaderObject.cpp`
- `MobileGL/MG_State/GLState/ProgramState/ShaderObject.h`
- `MobileGL/MG_State/GLState/TextureState/MipmapStorage.cpp`
- `MobileGL/MG_State/GLState/TextureState/MipmapStorage.h`
- `MobileGL/MG_State/GLState/TextureState/TextureObject.cpp`
- `MobileGL/MG_State/GLState/TextureState/TextureObject2D.cpp`
- `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp`
- `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.h`
- `MobileGL/MG_Impl/GLImpl/RenderState/GL_RenderState.h`
- `MobileGL/MG_Impl/GLImpl/RenderState/GL_RenderState.cpp`
- `MobileGL/MG_Impl/GLImpl/Drawing/GL_Drawing.cpp`
- `MobileGL/MG_Impl/GLImpl/Drawing/GL_Drawing.h`
- `MobileGL/MG_Impl/GLImpl/Buffer/Validators.cpp`
- `MobileGL/MG_Impl/GLImpl/VertexArray/Validators.cpp`
- `MobileGL/MG_Impl/GLImpl/VertexArray/GL_VertexArray.cpp`
- `MobileGL/MG_Impl/GLImpl/Program/GL_Program.cpp`
- `MobileGL/MG_Impl/GLImpl/Buffer/GL_Buffer.cpp`
- `MobileGL/MG_State/GLState/VertexArrayState/VertexArrayState.cpp`
- `MobileGL/MG_State/GLState/VertexArrayState/VertexArrayState.h`
- `MobileGL/MG_State/GLState/VertexArrayState/VertexArrayObject.cpp`
- `MobileGL/MG_State/GLState/BufferState/BufferState.cpp`
- `MobileGL/MG_State/GLState/BufferState/BufferState.h`
- `MobileGL/MG_State/GLState/BufferState/BufferObject.cpp`
- `MobileGL/MG_State/GLState/BufferState/BufferObject.h`
- `MobileGL/MG_State/GLState/FramebufferState/FramebufferState.cpp`
- `MobileGL/MG_State/GLState/RenderbufferState/RenderbufferObject.cpp`
- `MobileGL/MG_State/GLState/RenderbufferState/RenderbufferObject.h`
- `MobileGL/MG_State/GLState/RenderbufferState/RenderbufferState.cpp`
- `MobileGL/MG_Impl/GLImpl/Framebuffer/GL_Framebuffer.cpp`
- `MobileGL/MG_Impl/GLImpl/Framebuffer/GL_Framebuffer.h`
- `MobileGL/MG_Impl/GLImpl/Framebuffer/Validators.cpp`
- `MobileGL/MG_Impl/GLImpl/Framebuffer/Validators.h`
- `MobileGL/MG_State/GLState/FramebufferState/FramebufferObject.cpp`
- `MobileGL/MG_State/GLState/FramebufferState/FramebufferObject.h`
- `MobileGL/MG_Impl/GLImpl/Sampler/GL_Sampler.cpp`
- `MobileGL/MG_Impl/GLImpl/Sampler/GL_Sampler.h`
- `MobileGL/MG_Impl/GLImpl/Sampler/Validators.cpp`
- `MobileGL/MG_Impl/GLImpl/Sampler/Validators.h`
- `MobileGL/MG_Impl/GLImpl/Texture/GL_Texture.cpp`
- `MobileGL/MG_Impl/GLImpl/Texture/GL_Texture.h`
- `MobileGL/MG_Impl/GLImpl/Texture/ProxyTexture.cpp`
- `MobileGL/MG_Impl/GLImpl/Texture/ProxyTexture.h`
- `MobileGL/MG_Impl/GLImpl/Texture/Validators.cpp`
- `MobileGL/MG_Impl/GLImpl/Texture/Validators.h`
- `MobileGL/MG_Impl/GLImpl/Exporting/Definitions.cpp`
- `MobileGL/MG_State/GLState/ProgramState/ProgramState.cpp`
- `MobileGL/MG_State/GLState/ProgramState/ProgramState.h`
- `MobileGL/MG_State/GLState/RenderState/RenderState.cpp`
- `MobileGL/MG_State/GLState/RenderState/RenderState.h`
- `MobileGL/MG_State/GLState/SamplerState/SamplerObject.cpp`
- `MobileGL/MG_State/GLState/SamplerState/SamplerObject.h`
- `MobileGL/MG_State/GLState/SamplerState/SamplerState.cpp`
- `MobileGL/MG_State/GLState/SamplerState/SamplerState.h`
- `MobileGL/MG_State/GLState/TextureState/TextureEnum.h`
- `MobileGL/MG_State/GLState/TextureState/TextureObject.cpp`
- `MobileGL/MG_State/GLState/TextureState/TextureObject.h`
- `MobileGL/MG_State/GLState/TextureState/TextureObject1D.cpp`
- `MobileGL/MG_State/GLState/TextureState/TextureObject2D.cpp`
- `MobileGL/MG_State/GLState/TextureState/TextureObject2DCube.cpp`
- `MobileGL/MG_State/GLState/TextureState/TextureObject2DCube.h`
- `MobileGL/MG_State/GLState/TextureState/TextureObject3D.cpp`
- `MobileGL/MG_State/GLState/TextureState/TextureObjectStubs.h`
- `MobileGL/MG_State/GLState/TextureState/TextureState.cpp`
- `MobileGL/MG_State/GLState/TextureState/TextureState.h`
- `MobileGL/MG_State/GLState/TextureState/TextureUnit.cpp`
- `MobileGL/MG_State/GLState/TextureState/TextureUnit.h`
- `MobileGL/MG_Util/Converters/GLToMG/TextureEnumConverter.cpp`
- `MobileGL/Config.h`
- `MobileGL/ConfigLoader.cpp`
- `MobileGL/GlobalObjects.cpp`
- `MobileGL/Init.cpp`
- `MobileGL/Init.h`
- `MobileGL/MG_Backend/Init.cpp`
- `MobileGL/MG_Backend/DirectVulkan/VmaImpl.cpp`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/VkTextureSamplerManager.cpp`
- `MobileGL/MG_Impl/Init.cpp`
- `MobileGL/MG_State/GLState/ErrorState/Error.cpp`
- `MobileGL/MG_State/GLState/TextureState/TextureObjectBuffer.cpp`
- `MobileGL/MG_Impl/GLXImpl/LookUp/LookUp.cpp`
- `MobileGL/MG_Impl/GLXImpl/Exporting/Definitions.cpp`
- `MobileGL/MG_Util/BackendLoaders/OpenGL/Loader.cpp`
- `MobileGL/MG_Util/BackendLoaders/Vulkan/Loader.cpp`
- `MobileGL/MG_Util/Debug/Log.cpp`
- `MobileGL/MG_Util/Metrics/BufferMetrics.cpp`
- `MobileGL/MG_Util/Texture/PixelStoreProcessor.cpp`
- `MobileGL/MG_Util/Metrics/TextureMetrics.cpp`
- `MobileGL/MG_Util/Classifiers/TextureEnumClassifier.cpp`
- `MobileGL/MG_Util/Converters/MGToGL/TextureEnumConverter.cpp`
- `MobileGL/MG_Util/Converters/MGToStr/TextureEnumConverter.cpp`
- `MobileGL/MG_Util/Converters/MGToStr/BufferEnumConverter.cpp`
- `MobileGL/MG_Util/Converters/MGToGL/FramebufferEnumConverter.cpp`
- `MobileGL/MG_Util/Converters/MGToStr/FramebufferEnumConverter.cpp`
- `MobileGL/MG_Util/Converters/MGToVk/TextureEnumConverter.cpp`
- `MobileGL/MG_Util/Converters/MGToMG/TextureEnumConverter.cpp`
- `MobileGL/MG_Util/Converters/EGLToStr/EGLEnumConverter.cpp`
- `MobileGL/MG_Util/Converters/GLToGlslang/ProgramEnumConverter.cpp`
- `MobileGL/MG_Util/Converters/GLToMG/BufferEnumConverter.cpp`
- `MobileGL/MG_Util/Converters/GLToMG/DataTypeConverter.cpp`
- `MobileGL/MG_Util/Converters/GLToMG/FramebufferEnumConverter.cpp`
- `MobileGL/MG_Util/Converters/GLToMG/ProgramEnumConverter.cpp`
- `MobileGL/MG_Util/Converters/GLToMG/RenderStateEnumConverter.cpp`
- `MobileGL/MG_Util/Converters/GLToStr/GLEnumConverter.cpp`
- `MobileGL/MG_Util/Converters/MGToGL/BufferEnumConverter.cpp`
- `MobileGL/MG_Util/Converters/MGToGL/DataTypeConverter.cpp`
- `MobileGL/MG_Util/Converters/MGToGL/ErrorCodeConverter.cpp`
- `MobileGL/MG_Util/Converters/MGToGL/ProgramEnumConverter.cpp`
- `MobileGL/MG_Util/Converters/MGToGL/RenderStateEnumConverter.cpp`
- `MobileGL/MG_Util/Converters/MGToStr/DataTypeConverter.cpp`
- `MobileGL/MG_Util/Converters/MGToStr/GLExtensionConverter.cpp`
- `MobileGL/MG_Util/Converters/MGToStr/RenderStateEnumConverter.cpp`
- `MobileGL/MG_Util/Converters/MGToVk/RenderStateEnumConverter.cpp`
- `MobileGL/MG_Util/Converters/SPIRVCrossToGL/SpvcTypeConverter.cpp`
- `MobileGL/MG_Util/Math/VectorTypes.cpp`
- `MobileGL/MG_Util/ShaderTranspiler/ShaderCompiler.cpp`
- `MobileGL/MG_Util/Texture/TextureFormatProcessor.cpp`
- `MobileGL/MG_Util/ShaderTranspiler/ShaderSourceProcessor.cpp`
- `MobileGL/MG_Util/ShaderTranspiler/SpvcSession.cpp`
- `MobileGL/MG_Util/ShaderTranspiler/SpirvPasses/EliminateFloatEqualsZeroPass.cpp`
- `MobileGL/MG_Util/ShaderTranspiler/SpirvPasses/FlattenInterfaceStructPass.cpp`
- `MobileGL/MG_Util/ShaderTranspiler/glslang/TMglGlslIoResolver.cpp`
- `MobileGL/MG_Util/ShaderTranspiler/glslang/UniformTraverser.cpp`
- `MobileGL/MG_Benchmark/Buffer/BufferBench.cpp`
- `MobileGL/MG_Benchmark/Program/ProgramBench.cpp`
- `MobileGL/MG_Benchmark/SanityBench.cpp`
- `MobileGL/MG_Test/Backend/DirectVulkan/SanityTest.cpp`
- `MobileGL/MG_Test/Backend/DirectVulkan/TestExec.cpp`
- `MobileGL/MG_Test/Buffer/BufferTest.cpp`
- `MobileGL/MG_Test/Program/ProgramTest.cpp`
- `MobileGL/MG_Test/Program/ProgramUtilTest.cpp`
- `MobileGL/MG_Test/SanityTest.cpp`
- `MobileGL/MG_Test/VertexArray/VertexArrayTest.cpp`
- `MobileGL/Defines.h`
- `MobileGL/Includes.h`
- `MobileGL/MG_Backend/DirectVulkan/DirectVulkan.h`
- `MobileGL/MG_Backend/DirectVulkan/DirectVulkanResourceState.h`
- `MobileGL/MG_Backend/DirectVulkan/Renderer/VertexInputStateFactory.h`
- `MobileGL/MG_Backend/DirectVulkan/VkIncludes.h`
- `MobileGL/MG_Backend/DirectVulkan/VulkanRendererConfig.h`
- `MobileGL/MG_Impl/GLImpl/Buffer/GL_Buffer.h`
- `MobileGL/MG_Impl/GLImpl/Buffer/Validators.h`
- `MobileGL/MG_Impl/GLImpl/Program/GL_Program.h`
- `MobileGL/MG_Impl/GLImpl/VertexArray/GL_VertexArray.h`
- `MobileGL/MG_Impl/GLImpl/VertexArray/Validators.h`
- `MobileGL/MG_Impl/GLXImpl/LookUp/LookUp.h`
- `MobileGL/MG_State/GLState/ErrorState/Error.h`
- `MobileGL/MG_State/GLState/ErrorState/ErrorCode.h`
- `MobileGL/MG_State/GLState/ErrorState/ErrorInfo.h`
- `MobileGL/MG_State/GLState/FramebufferState/FramebufferState.h`
- `MobileGL/MG_State/GLState/RenderbufferState/RenderbufferState.h`
- `MobileGL/MG_State/GLState/TextureState/MipmapUploadTargetArray.h`
- `MobileGL/MG_State/GLState/TextureState/TextureObject1D.h`
- `MobileGL/MG_State/GLState/TextureState/TextureObject2D.h`
- `MobileGL/MG_State/GLState/TextureState/TextureObject3D.h`
- `MobileGL/MG_State/GLState/TextureState/TextureObjectBuffer.h`
- `MobileGL/MG_State/GLState/TextureState/TextureTypes.h`
- `MobileGL/MG_State/GLState/VertexArrayState/VertexArrayObject.h`
- `MobileGL/MG_Util/BackendLoaders/OpenGL/Loader.h`
- `MobileGL/MG_Util/BackendLoaders/Vulkan/Loader.h`
- `MobileGL/MG_Util/Classifiers/TextureEnumClassifier.h`
- `MobileGL/MG_Util/Converters/EGLToStr/EGLEnumConverter.h`
- `MobileGL/MG_Util/Converters/GLToGlslang/ProgramEnumConverter.h`
- `MobileGL/MG_Util/Converters/GLToMG/BufferEnumConverter.h`
- `MobileGL/MG_Util/Converters/GLToMG/DataTypeConverter.h`
- `MobileGL/MG_Util/Converters/GLToMG/FramebufferEnumConverter.h`
- `MobileGL/MG_Util/Converters/GLToMG/ProgramEnumConverter.h`
- `MobileGL/MG_Util/Converters/GLToMG/RenderStateEnumConverter.h`
- `MobileGL/MG_Util/Converters/GLToMG/TextureEnumConverter.h`
- `MobileGL/MG_Util/Converters/GLToStr/GLEnumConverter.h`
- `MobileGL/MG_Util/Converters/MGToGL/BufferEnumConverter.h`
- `MobileGL/MG_Util/Converters/MGToGL/DataTypeConverter.h`
- `MobileGL/MG_Util/Converters/MGToGL/ErrorCodeConverter.h`
- `MobileGL/MG_Util/Converters/MGToGL/FramebufferEnumConverter.h`
- `MobileGL/MG_Util/Converters/MGToGL/ProgramEnumConverter.h`
- `MobileGL/MG_Util/Converters/MGToGL/RenderStateEnumConverter.h`
- `MobileGL/MG_Util/Converters/MGToGL/TextureEnumConverter.h`
- `MobileGL/MG_Util/Converters/MGToMG/TextureEnumConverter.h`
- `MobileGL/MG_Util/Converters/MGToStr/BufferEnumConverter.h`
- `MobileGL/MG_Util/Converters/MGToStr/DataTypeConverter.h`
- `MobileGL/MG_Util/Converters/MGToStr/FramebufferEnumConverter.h`
- `MobileGL/MG_Util/Converters/MGToStr/GLExtensionConverter.h`
- `MobileGL/MG_Util/Converters/MGToStr/RenderStateEnumConverter.h`
- `MobileGL/MG_Util/Converters/MGToStr/TextureEnumConverter.h`
- `MobileGL/MG_Util/Converters/MGToVk/RenderStateEnumConverter.h`
- `MobileGL/MG_Util/Converters/MGToVk/TextureEnumConverter.h`
- `MobileGL/MG_Util/Converters/SPIRVCrossToGL/SpvcTypeConverter.h`
- `MobileGL/MG_Util/Debug/Log.h`
- `MobileGL/MG_Util/GLExtensions.h`
- `MobileGL/MG_Util/Math/VectorTypes.h`
- `MobileGL/MG_Util/Metrics/BufferMetrics.h`
- `MobileGL/MG_Util/Metrics/TextureMetrics.h`
- `MobileGL/MG_Util/Miscellany/IndexGenerator.h`
- `MobileGL/MG_Util/PlatformStubs.h`
- `MobileGL/MG_Util/ShaderTranspiler/ShaderCompiler.h`
- `MobileGL/MG_Util/ShaderTranspiler/ShaderSourceProcessor.h`
- `MobileGL/MG_Util/ShaderTranspiler/SpirvPasses/EliminateFloatEqualsZeroPass.h`
- `MobileGL/MG_Util/ShaderTranspiler/SpirvPasses/FlattenInterfaceStructPass.h`
- `MobileGL/MG_Util/ShaderTranspiler/SpvcSession.h`
- `MobileGL/MG_Util/ShaderTranspiler/Types.h`
- `MobileGL/MG_Util/ShaderTranspiler/glslang/TMglGlslIoResolver.h`
- `MobileGL/MG_Util/ShaderTranspiler/glslang/TVarEntryInfo.h`
- `MobileGL/MG_Util/ShaderTranspiler/glslang/UniformTraverser.h`
- `MobileGL/MG_Util/Texture/PixelStoreProcessor.h`
- `MobileGL/MG_Util/Texture/TextureFormatProcessor.h`
- `MobileGL/MG_Util/Types.h`
- `selfcheck.c`

## Notes

- `MobileGL/Defines.h` had a pre-existing local modification enabling console logging; this audit pass did not change it.
- Reviewed-file coverage is now complete for all in-scope source files under `MobileGL/` plus `selfcheck.c`: 260 total source files recorded, 0 remaining uncovered.
- Audit completion criteria for this pass are satisfied: all in-scope source files were reviewed, all findings identified in this pass are recorded in this file, compile-only validation was performed on edited translation units, and the previously confirmed high-risk DirectVulkan silent-corruption paths were converted into explicit unsupported/error behavior.
- Residual open findings remain documented below as partially fixed follow-up gaps. At close of this audit pass there are 3 such residual findings and 0 `confirmed, not fixed` findings.
- Full `cmake --build buildlinux --target MobileGL_s` could not be reused because `buildlinux/CMakeCache.txt` references missing compilers `/home/bzlzhh/.links/clang` and `/home/bzlzhh/.links/clang++`.
- Compile validation completed for each edited translation unit with direct single-file syntax checks using `/usr/bin/c++` and the existing `build_linux/compile_commands.json` include/define set.
- `MobileGL/MG_Backend/DirectVulkan/Renderer/VkTextureManager.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/VkRenderPassManager.cpp`, and `MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.cpp` compile validation passed after the cube-face DirectVulkan fix with direct `/usr/bin/c++ -fsyntax-only` invocations using their `build_linux/compile_commands.json` flags.
- `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp`, `MobileGL/MG_Backend/DirectGLES/BackendObject_DirectGLES.cpp`, `MobileGL/MG_Backend/DirectVulkan/BackendObject_DirectVulkan.cpp`, `MobileGL/MG_Util/BackendLoaders/OpenGL/Loader.cpp`, and `MobileGL/MG_Util/BackendLoaders/Vulkan/Loader.cpp` compile validation passed after the backend-capability getter fix with direct `/usr/bin/c++ -fsyntax-only` invocations using their `build_linux/compile_commands.json` flags.
- `MobileGL/MG_Util/Metrics/TextureMetrics.cpp` compile validation passed with a direct `/usr/bin/c++ -fsyntax-only` invocation using the `build_linux/compile_commands.json` flags for that file.
- `MobileGL/MG_Util/Converters/MGToGL/TextureEnumConverter.cpp` and `MobileGL/MG_Util/Converters/MGToStr/TextureEnumConverter.cpp` compile validation passed with direct `/usr/bin/c++ -fsyntax-only` invocations using their `build_linux/compile_commands.json` flags.
- `MobileGL/MG_Impl/GLImpl/Exporting/Definitions.cpp` compile validation passed with a direct `/usr/bin/c++ -fsyntax-only` invocation using its `build_linux/compile_commands.json` flags.
- `MobileGL/MG_Util/Converters/MGToStr/BufferEnumConverter.cpp` compile validation passed with a direct `/usr/bin/c++ -fsyntax-only` invocation using its `build_linux/compile_commands.json` flags.
- `MobileGL/MG_Util/Converters/MGToGL/FramebufferEnumConverter.cpp` and `MobileGL/MG_Util/Converters/MGToStr/FramebufferEnumConverter.cpp` compile validation passed with direct `/usr/bin/c++ -fsyntax-only` invocations using their `build_linux/compile_commands.json` flags.
- `MobileGL/MG_Util/Converters/GLToMG/DataTypeConverter.cpp` and `MobileGL/MG_Util/Converters/MGToGL/DataTypeConverter.cpp` compile validation passed with direct `/usr/bin/c++ -fsyntax-only` invocations using their `build_linux/compile_commands.json` flags.
- `MobileGL/MG_Test/Backend/DirectVulkan/SanityTest.cpp` and `MobileGL/MG_Test/Backend/DirectVulkan/TestExec.cpp` are not present in the available compile databases. Syntax-only validation of the patched sources was attempted with the main project include/define set plus discovered local `gtest` / `GLFW` include roots; parsing then became environment-limited by missing X11 development headers (`X11/extensions/Xrandr.h`) behind that ad hoc GLFW header tree, so these two files could not be fully syntax-validated in the current environment.
- `MobileGL/MG_Util/Types.h` change was syntax-validated through a direct `/usr/bin/c++ -fsyntax-only` check of `MobileGL/MG_Util/Math/VectorTypes.cpp` using the `build_linux/compile_commands.json` flags, since `VectorTypes.cpp` includes the patched core utility header.
- `MobileGL/MG_Util/ShaderTranspiler/SpvcSession.h` change was syntax-validated through a direct `/usr/bin/c++ -fsyntax-only` check of `MobileGL/MG_Util/ShaderTranspiler/SpvcSession.cpp` using the `build_linux/compile_commands.json` flags.
- `MobileGL/MG_Util/ShaderTranspiler/SpirvPasses/EliminateFloatEqualsZeroPass.cpp` compile validation passed with a direct `/usr/bin/c++ -fsyntax-only` invocation using its `build_linux/compile_commands.json` flags.
- `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp` compile validation passed with a direct `/usr/bin/c++ -fsyntax-only` invocation using its `build_linux/compile_commands.json` flags.
- `MobileGL/MG_Impl/GLImpl/Exporting/Definitions.cpp` compile validation passed with a direct `/usr/bin/c++ -fsyntax-only` invocation using its `build_linux/compile_commands.json` flags after wiring `glGetBooleanv` / `glGetFloatv` to real implementations.
- `MobileGL/MG_Impl/GLImpl/VertexArray/GL_VertexArray.cpp` compile validation passed with a direct `/usr/bin/c++ -fsyntax-only` invocation using its `build_linux/compile_commands.json` flags after wiring the `glGetVertexAttrib*` query family to tracked VAO/current-attribute state.
- `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp` and `MobileGL/MG_Impl/GLImpl/Exporting/Definitions.cpp` compile validation passed again with direct `/usr/bin/c++ -fsyntax-only` invocations using their `build_linux/compile_commands.json` flags after wiring `glGetInteger64v` to a real implementation.
- `MobileGL/MG_Impl/GLImpl/VertexArray/GL_VertexArray.cpp` and `MobileGL/MG_Impl/GLImpl/Exporting/Definitions.cpp` compile validation passed again with direct `/usr/bin/c++ -fsyntax-only` invocations using their `build_linux/compile_commands.json` flags after wiring the supported `glVertexAttrib*` current-value setters to tracked `CurrentVertexAttributeValue` state.
- `MobileGL/MG_Impl/GLImpl/Buffer/GL_Buffer.cpp`, `MobileGL/MG_State/GLState/BufferState/BufferObject.cpp`, and `MobileGL/MG_Impl/GLImpl/Exporting/Definitions.cpp` compile validation passed with direct `/usr/bin/c++ -fsyntax-only` invocations using their `build_linux/compile_commands.json` flags after wiring `glGetBufferPointerv(GL_BUFFER_MAP_POINTER, ...)` to the tracked live mapping pointer.
- `MobileGL/MG_Impl/GLImpl/Buffer/GL_Buffer.cpp` and `MobileGL/MG_Impl/GLImpl/Exporting/Definitions.cpp` compile validation passed with direct `/usr/bin/c++ -fsyntax-only` invocations using their `build_linux/compile_commands.json` flags after wiring `glGetBufferParameteri64v` to the tracked frontend buffer state.
- `MobileGL/MG_Impl/GLImpl/Program/GL_Program.cpp` and `MobileGL/MG_Impl/GLImpl/Exporting/Definitions.cpp` compile validation passed with direct `/usr/bin/c++ -fsyntax-only` invocations using their `build_linux/compile_commands.json` flags after wiring the current-program `glUniform*ui*` unsigned-uniform setter family to the existing frontend uniform upload path.
- `MobileGL/MG_Impl/GLImpl/Program/GL_Program.cpp` and `MobileGL/MG_State/GLState/ProgramState/ProgramObject.cpp` compile validation passed with direct `/usr/bin/c++ -fsyntax-only` invocations using their `build_linux/compile_commands.json` flags after hardening shared uniform-location validation against sparse reflected-location holes.
- `MobileGL/MG_Impl/GLImpl/Program/GL_Program.cpp` and `MobileGL/MG_Impl/GLImpl/Exporting/Definitions.cpp` compile validation passed with direct `/usr/bin/c++ -fsyntax-only` invocations using their `build_linux/compile_commands.json` flags after wiring `glGetUniformuiv` and fixing opaque uniform readback to use tracked sampler/image unit state.
- `MobileGL/MG_Impl/GLImpl/RenderState/GL_RenderState.cpp` and `MobileGL/MG_Impl/GLImpl/Exporting/Definitions.cpp` compile validation passed with direct `/usr/bin/c++ -fsyntax-only` invocations using their `build_linux/compile_commands.json` flags after validating indexed blend capability targets/indices and wiring `glGetBooleani_v` to tracked indexed blend state.
- `MobileGL/MG_Impl/GLImpl/Program/GL_Program.cpp` and `MobileGL/MG_Impl/GLImpl/Exporting/Definitions.cpp` compile validation passed with direct `/usr/bin/c++ -fsyntax-only` invocations using their `build_linux/compile_commands.json` flags after wiring `glGetActiveUniformName` to the existing reflected uniform-name query path.
- `MobileGL/MG_Impl/GLImpl/Program/GL_Program.cpp`, `MobileGL/MG_State/GLState/ProgramState/ProgramObject.cpp`, and `MobileGL/MG_Impl/GLImpl/Exporting/Definitions.cpp` compile validation passed with direct `/usr/bin/c++ -fsyntax-only` invocations using their `build_linux/compile_commands.json` flags after fixing active uniform/attribute reflection queries to use active-resource indices and restoring the missing `glGetActiveUniform` array-size output.
- `MobileGL/MG_Impl/GLImpl/Program/GL_Program.cpp` and `MobileGL/MG_State/GLState/ProgramState/ProgramObject.cpp` compile validation passed with direct `/usr/bin/c++ -fsyntax-only` invocations using their `build_linux/compile_commands.json` flags after fixing uniform-block reflection queries (`GL_UNIFORM_BLOCK_BINDING`, `GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS`, `GL_UNIFORM_BLOCK_REFERENCED_BY_*`) and removing the nullable-`length` dereference in `glGetActiveUniformBlockName`.
- `MobileGL/MG_Impl/GLImpl/Program/GL_Program.cpp`, `MobileGL/MG_State/GLState/ProgramState/ProgramObject.cpp`, and `MobileGL/MG_Impl/GLImpl/Exporting/Definitions.cpp` compile validation passed with direct `/usr/bin/c++ -fsyntax-only` invocations using their `build_linux/compile_commands.json` flags after wiring `glGetUniformIndices` and filling `GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES` from reflected active-uniform metadata.
- `MobileGL/MG_Impl/GLImpl/Program/GL_Program.cpp` compile validation passed with a direct `/usr/bin/c++ -fsyntax-only` invocation using its `build_linux/compile_commands.json` flags after fixing `glGetProgramiv` length queries to include the terminating NUL.
- `MobileGL/MG_Impl/GLImpl/Program/GL_Program.cpp` and `MobileGL/MG_State/GLState/ProgramState/ProgramObject.cpp` compile validation passed with direct `/usr/bin/c++ -fsyntax-only` invocations using their `build_linux/compile_commands.json` flags after switching `GL_COMPUTE_WORK_GROUP_SIZE` to linked-program reflection instead of backend-dependent fallback behavior.
- `MobileGL/MG_Impl/GLImpl/Program/GL_Program.cpp` compile validation passed with a direct `/usr/bin/c++ -fsyntax-only` invocation using its `build_linux/compile_commands.json` flags after restoring zero-length behavior for empty `GL_INFO_LOG_LENGTH` on program objects.
- `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp` compile validation passed with a direct `/usr/bin/c++ -fsyntax-only` invocation using its `build_linux/compile_commands.json` flags after wiring tracked indexed buffer binding/range state into `glGetIntegeri_v` and `glGetInteger64i_v` for supported buffer targets.
- `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp` compile validation passed again with a direct `/usr/bin/c++ -fsyntax-only` invocation using its `build_linux/compile_commands.json` flags after wiring the tracked texture-unit bindings into the supported `GL_TEXTURE_BINDING_*` getter family.
- `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp`, `MobileGL/MG_State/GLState/FramebufferState/FramebufferObject.cpp`, and `MobileGL/MG_Impl/Init.cpp` compile validation passed with direct `/usr/bin/c++ -fsyntax-only` invocations using their `build_linux/compile_commands.json` flags after fixing default-framebuffer read/draw buffer defaults and wiring the remaining tracked framebuffer/buffer binding getters.
- `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp` compile validation passed again with a direct `/usr/bin/c++ -fsyntax-only` invocation using its `build_linux/compile_commands.json` flags after fixing `GL_DEPTH_CLEAR_VALUE` / `GL_STENCIL_CLEAR_VALUE` clear-state queries to use the tracked render state.
- `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp` compile validation passed again with a direct `/usr/bin/c++ -fsyntax-only` invocation using its `build_linux/compile_commands.json` flags after fixing boolean conversions for tracked floating-point clear/blend/depth-range state in `glGetBooleanv`.
- `MobileGL/MG_State/GLState/RenderState/RenderState.cpp`, `MobileGL/MG_Backend/DirectGLES/DirectGLES.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/PipelineFactory.cpp`, and `MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.cpp` compile validation passed with direct `/usr/bin/c++ -fsyntax-only` invocations using their `build_linux/compile_commands.json` flags after wiring stencil-test capability tracking and backend synchronization.
- `MobileGL/MG_State/GLState/RenderState/RenderState.cpp`, `MobileGL/MG_State/GLState/Core.cpp`, `MobileGL/MG_Util/Converters/GLToMG/RenderStateEnumConverter.cpp`, `MobileGL/MG_Util/Converters/MGToGL/RenderStateEnumConverter.cpp`, `MobileGL/MG_Util/Converters/MGToStr/RenderStateEnumConverter.cpp`, `MobileGL/MG_Util/Converters/MGToVk/RenderStateEnumConverter.cpp`, `MobileGL/MG_Impl/GLImpl/RenderState/GL_RenderState.cpp`, `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp`, `MobileGL/MG_Backend/DirectGLES/DirectGLES.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/PipelineFactory.cpp`, and `MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.cpp` compile validation passed with direct `/usr/bin/c++ -fsyntax-only` invocations using their `build_linux/compile_commands.json` flags after wiring tracked stencil func/op/mask state through the frontend getters and both backends.
- `MobileGL/MG_Backend/DirectVulkan/Renderer/VkRenderPassManager.cpp` compile validation passed with a direct `/usr/bin/c++ -fsyntax-only` invocation using its `build_linux/compile_commands.json` flags after teaching DirectVulkan render-pass assembly to honor standalone stencil attachments through the shared depth/stencil attachment path.
- `MobileGL/MG_Util/BackendLoaders/OpenGL/Loader.cpp`, `MobileGL/MG_Util/BackendLoaders/Vulkan/Loader.cpp`, `MobileGL/MG_Backend/DirectGLES/BackendObject_DirectGLES.cpp`, `MobileGL/MG_Backend/DirectVulkan/BackendObject_DirectVulkan.cpp`, `MobileGL/MG_State/GLState/RenderState/RenderState.cpp`, `MobileGL/MG_State/GLState/Core.cpp`, `MobileGL/MG_Impl/GLImpl/RenderState/GL_RenderState.cpp`, `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp`, `MobileGL/MG_Backend/DirectGLES/DirectGLES.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/PipelineFactory.cpp`, and `MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.cpp` compile validation passed with direct `/usr/bin/c++ -fsyntax-only` invocations using their `build_linux/compile_commands.json` flags after wiring tracked line-width state, backend capability reporting, DirectGLES/DirectVulkan draw-state application, and Vulkan `wideLines` feature enablement/clamping.
- `MobileGL/MG_State/GLState/RenderState/RenderState.cpp`, `MobileGL/MG_State/GLState/Core.cpp`, `MobileGL/MG_Impl/GLImpl/RenderState/GL_RenderState.cpp`, `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp`, `MobileGL/MG_Backend/DirectGLES/DirectGLES.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/PipelineFactory.cpp`, and `MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.cpp` compile validation passed with direct `/usr/bin/c++ -fsyntax-only` invocations using their `build_linux/compile_commands.json` flags after wiring tracked polygon-offset state, `GL_POLYGON_OFFSET_FILL` capability tracking, DirectGLES sync, and Vulkan depth-bias pipeline/draw-state application.
- `MobileGL/MG_State/GLState/RenderState/RenderState.cpp`, `MobileGL/MG_Backend/DirectGLES/DirectGLES.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/PipelineFactory.cpp`, and `MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.cpp` compile validation passed with direct `/usr/bin/c++ -fsyntax-only` invocations using their `build_linux/compile_commands.json` flags after wiring `GL_RASTERIZER_DISCARD` capability tracking and backend synchronization in both renderers.
- `MobileGL/MG_State/GLState/RenderState/RenderState.cpp`, `MobileGL/MG_State/GLState/Core.cpp`, `MobileGL/MG_Impl/GLImpl/RenderState/GL_RenderState.cpp`, `MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp`, `MobileGL/MG_Util/BackendLoaders/OpenGL/Loader.cpp`, `MobileGL/MG_Backend/DirectGLES/DirectGLES.cpp`, `MobileGL/MG_Util/Converters/GLToMG/RenderStateEnumConverter.cpp`, `MobileGL/MG_Util/Converters/MGToGL/RenderStateEnumConverter.cpp`, `MobileGL/MG_Util/Converters/MGToStr/RenderStateEnumConverter.cpp`, `MobileGL/MG_Util/Converters/MGToVk/RenderStateEnumConverter.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/PipelineFactory.cpp`, and `MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.cpp` compile validation passed with direct `/usr/bin/c++ -fsyntax-only` invocations using their `build_linux/compile_commands.json` flags after wiring tracked `glLogicOp` state, `GL_COLOR_LOGIC_OP` capability tracking, GLES loader/backend sync, and Vulkan logic-op pipeline state plus device feature enablement.
- `MobileGL/MG_State/GLState/RenderbufferState/RenderbufferObject.cpp`, `MobileGL/MG_Impl/GLImpl/Framebuffer/GL_Framebuffer.cpp`, and `MobileGL/MG_Backend/DirectGLES/Managers.cpp` compile validation passed with direct `/usr/bin/c++ -fsyntax-only` invocations using their `build_linux/compile_commands.json` flags after wiring `glRenderbufferStorageMultisample` frontend storage/sample-count tracking and DirectGLES multisample renderbuffer allocation.
- `MobileGL/MG_State/GLState/FramebufferState/FramebufferObject.cpp`, `MobileGL/MG_Impl/GLImpl/Framebuffer/GL_Framebuffer.cpp`, and `MobileGL/MG_Backend/DirectGLES/Managers.cpp` compile validation passed with direct `/usr/bin/c++ -fsyntax-only` invocations using their `build_linux/compile_commands.json` flags after teaching framebuffer texture attachments to retain cube-map face upload targets for frontend queries/completeness and DirectGLES replay.
- `MobileGL/MG_Backend/DirectVulkan/Renderer/VkClearManager.cpp`, `MobileGL/MG_Backend/DirectVulkan/Renderer/VkRenderPassManager.cpp`, and `MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.cpp` compile validation passed with direct `/usr/bin/c++ -fsyntax-only` invocations using their `build_linux/compile_commands.json` flags after keying DirectVulkan pending clears by exact attachment subresource (texture + mip + array-layer span) and materializing all queued subresource clears before sampled/copy use.
