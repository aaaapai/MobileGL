//
// Created by BZLZHH on 2025/3/15.
//

#include "../../../../Includes.h"

namespace MG_GL::Getter {
    const std::string GetBackendName() {
#if BACKEND_TYPE == BACKEND_VULKAN
        return "Vulkan";
#elif BACKEND_TYPE == BACKEND_METAL
        return "Metal";
#elif BACKEND_TYPE == BACKEND_GLES
        return "OpenGL ES";
#elif BACKEND_TYPE == BACKEND_DILIGENT
        switch (MG_Diligent::DILIGENT_BACKEND_TYPE) {
            case MG_Constants::Backend::BACKEND_DILIGENT_VULKAN:
                return "DiligentEngine (Vulkan)";
            case MG_Constants::Backend::BACKEND_DILIGENT_METAL:
                return "DiligentEngine (Metal)";
            case MG_Constants::Backend::BACKEND_DILIGENT_OPENGL:
                return "DiligentEngine (OpenGL)";
            default:
                return "DiligentEngine (Unknown Backend)";
        }
#else
        return "<Unknown Backend>";
#endif
    }

    const std::string GetMGName() {
#if BACKEND_TYPE == BACKEND_VULKAN
        return "MobileGluvk";
#elif BACKEND_TYPE == BACKEND_METAL
        return "MobileGlumtl";
#elif BACKEND_TYPE == BACKEND_GLES
        return "MobileGlues";
#elif BACKEND_TYPE == BACKEND_DILIGENT
        switch (MG_Diligent::DILIGENT_BACKEND_TYPE) {
            case MG_Constants::Backend::BACKEND_DILIGENT_VULKAN:
                return "MobileGlued-vk";
            case MG_Constants::Backend::BACKEND_DILIGENT_METAL:
                return "MobileGlued-mtl";
            case MG_Constants::Backend::BACKEND_DILIGENT_OPENGL:
                return "MobileGlued-gl";
            default:
                return "MobileGlued-unknown";
        }
#else
        return "<Unknown MobileGL Version>";
#endif
    }

    const std::string GetBackendGfxAPIName() {
#if BACKEND_TYPE == BACKEND_VULKAN
        return "Vulkan";
#elif BACKEND_TYPE == BACKEND_METAL
        return "Metal";
#elif BACKEND_TYPE == BACKEND_GLES
        return "OpenGL ES";
#elif BACKEND_TYPE == BACKEND_DILIGENT
        switch (MG_Diligent::DILIGENT_BACKEND_TYPE) {
            case MG_Constants::Backend::BACKEND_DILIGENT_VULKAN:
                return "Vulkan";
            case MG_Constants::Backend::BACKEND_DILIGENT_METAL:
                return "Metal";
            case MG_Constants::Backend::BACKEND_DILIGENT_OPENGL:
                return "OpenGL";
            default:
                return "<unknown>";
        }
#else
        return "<Unknown Backend>";
#endif
    }

    const std::string GetGPUName() {
#if BACKEND_TYPE == BACKEND_DILIGENT
        if (MG_Diligent::g_pDevice)
            return MG_Diligent::g_pDevice->GetAdapterInfo().Description;

        return "<not set yet>";
#else
        return "<unknown GPU>";
#endif
    }

    const uint32_t GetBackendGfxAPIVersionMajor() {
#if BACKEND_TYPE == BACKEND_DILIGENT
        if (MG_Diligent::g_pDevice)
            return MG_Diligent::g_pDevice->GetDeviceInfo().APIVersion.Major;

        return 0;
#else
        return 0;
#endif
    }

    const uint32_t GetBackendGfxAPIVersionMinor() {
#if BACKEND_TYPE == BACKEND_DILIGENT
        if (MG_Diligent::g_pDevice)
            return MG_Diligent::g_pDevice->GetDeviceInfo().APIVersion.Minor;

        return 0;
#else
        return 0;
#endif
    }

    void LogMGInfo() {
        std::string MGName = MG_GL::Getter::GetMGName() + " (MobileGL Core)";

        std::string MGVersion = std::to_string(MG_Global::MG::VersionMajor) + "."
                                +  std::to_string(MG_Global::MG::VersionMinor) + "."
                                +  std::to_string(MG_Global::MG::VersionRevision);
        if (MG_Global::MG::VersionPatch != 0)
            MGVersion += "." + std::to_string(MG_Global::MG::VersionPatch);
        MGVersion += MG_Global::MG::VersionSuffix;

        std::string GLVersion = std::to_string(MG_Global::GL::GLVersionMajor) + "."
                                +  std::to_string(MG_Global::GL::GLVersionMinor) + "."
                                +  std::to_string(MG_Global::GL::GLVersionRevision);

        std::string backendName = MG_GL::Getter::GetBackendName();
        
        std::string backendVersion;
#if BACKEND_TYPE == BACKEND_VULKAN
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Vulkan Version Query";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;
        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        VkInstance instance;
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan instance!");
        }
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            throw std::runtime_error("No Vulkan devices found!");
        }
        VkPhysicalDevice physicalDevice;
        vkEnumeratePhysicalDevices(instance, &deviceCount, &physicalDevice);
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
        uint32_t major = VK_VERSION_MAJOR(deviceProperties.apiVersion);
        uint32_t minor = VK_VERSION_MINOR(deviceProperties.apiVersion);
        uint32_t patch = VK_VERSION_PATCH(deviceProperties.apiVersion);
        vkDestroyInstance(instance, nullptr);
        
        backendVersion = "Vulkan " + std::to_string(major) + "." + 
                std::to_string(minor)+"." + std::to_string(patch);
#elif BACKEND_TYPE == BACKEND_GLES
        backendVersion = std::string((const char*)::GLES::glGetString(GL_VERSION));
#elif BACKEND_TYPE == BACKEND_DILIGENT
        backendVersion = "Diligent";
#else
        backendVersion = "<Unknown Backend>";
#endif

        MG_Util::Debug::LogI("MobileGL Renderer Info:");
        MG_Util::Debug::LogI("----%s:", MGName.c_str());
        MG_Util::Debug::LogI("--------MobileGL Version: %s", MGVersion.c_str());
        MG_Util::Debug::LogI("--------Target OpenGL Implementation Version: %s", GLVersion.c_str());
        MG_Util::Debug::LogI("--------Backend: %s", backendName.c_str());
        MG_Util::Debug::LogI("--------Backend Version: %s", backendVersion.c_str());
    }
}