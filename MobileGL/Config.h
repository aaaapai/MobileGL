#pragma once

#define MOBILEGL_LOG_ACTIVE_LEVEL MOBILEGL_LOG_LEVEL_DEBUG

#define MOBILEGL_LOG_ENABLE_CONSOLE 0
#define MOBILEGL_LOG_ENABLE_FILE    1
#define MOBILEGL_LOG_ENABLE_ANDROID 1

#ifdef __ANDROID__
#define MOBILEGL_LOG_FILE_PATH "/sdcard/MG/latest.log"
#else
#define MOBILEGL_LOG_FILE_PATH ""
#endif

namespace MobileGL {
	namespace MG_Config {
		inline const String ProjectName = "MobileGL";
		inline const String CoreName = "MobileGL Core";
		inline const String CoreVendor = "MobileGL-Dev";
		inline const Version CoreVersion = { 1, 0, 0, "-Dev" };
		extern RendererInfo* RendererInfoPtr;

		namespace Backend {
#define MOBILEGL_BACKEND MOBILEGL_BACKEND_UNKNOWN

			namespace Diligent {
				inline const MG_Backend::Diligent::SpecificBackendType SpecificBackend = 
					MG_Backend::Diligent::SpecificBackendType::Vulkan;
			}
		}
	}
}