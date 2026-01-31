#include "DirectVulkan.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    UniquePtr<VulkanRenderer> pVulkanRenderer = nullptr;

    void Clear(GLbitfield mask) {}

    void DrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) {
        pVulkanRenderer->RenderFrame();
    }
} // namespace MobileGL::MG_Backend::DirectVulkan