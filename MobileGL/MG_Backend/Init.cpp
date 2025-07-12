#include "../Includes.h"

namespace MobileGL {
	namespace MG_Config {
		RendererInfo* RendererInfoPtr = nullptr;
	}

	namespace MG_Backend {
		void Init() {
			MGLOG_D("Initializing MobileGL Backend...");

#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_DILIGENT
			switch (MG_Config::Backend::Diligent::SpecificBackend) {
			case MG_Backend::Diligent::SpecificBackendType::Vulkan:
				MG_Config::RendererInfoPtr = (RendererInfo*)&MG_Backend::Diligent::RendererInfoVulkan;
				break;
			case MG_Backend::Diligent::SpecificBackendType::Metal:
				MG_Config::RendererInfoPtr = (RendererInfo*)&MG_Backend::Diligent::RendererInfoMetal;
				break;
			default:
				throw RuntimeError("Unsupported renderer type");
			}
#else
			 MG_Config::RendererInfoPtr = (RendererInfo*)&RendererInfoUnknown;
#endif
		}
	}
}