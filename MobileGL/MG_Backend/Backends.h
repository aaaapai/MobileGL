#pragma once

#define MOBILEGL_BACKEND_TYPE_UNKNOWN  0
#define MOBILEGL_BACKEND_TYPE_DILIGENT 1
#define MOBILEGL_BACKEND_TYPE_MRHI     2
#define MOBILEGL_BACKEND_TYPE_DIRECT   3 // the previous "ShittilyDraw"

namespace MobileGL {
	namespace MG_Backend {
		namespace Unknown {
			const RendererInfo RendererInfoUnknown = {
				"<unknown renderer of MobileGL>",
				"<unknown backend>",
				", <unknown>",
				{
					{ 0, 0, 0 },
					{ 0, 0, 0 },
					{ },
				},
				{ }
			};
		}
		namespace Diligent {
			enum class SpecificBackendType {
				Vulkan,
				Metal
			};

			const RendererInfo RendererInfoVulkan = {
				"MobileGlued-vk",
				"Diligent Engine (Vulkan)",
				Nullopt,
				{
					{ 3, 2, 0 },
					{ 4, 5, 0 },
					{ V_OpenGL30, V_OpenGL31, V_OpenGL32 },
					false
				},
				{ }
			};

			const RendererInfo RendererInfoMetal = {
				"MobileGlued-mtl",
				"Diligent Engine (Metal)",
				Nullopt,
				{
					{ 3, 2, 0 },
					{ 4, 5, 0 },
					{ V_OpenGL30, V_OpenGL31, V_OpenGL32 },
				},
				{ }
			};
		}

		void Init();
	}
}