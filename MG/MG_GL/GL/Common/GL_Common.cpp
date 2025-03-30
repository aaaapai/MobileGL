//
// Created by BZLZHH on 2025/3/15.
//

#include "GL_Common.h"

namespace MG_GL::GL {
    void ClearDepth(GLdouble depth) {
        
    }
    
    void ClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
        EGLContext ctx = eglGetCurrentContext();
        if (ctx == EGL_NO_CONTEXT) return;

        VulkanContext& vkCtx = g_contexts[ctx];
        vkCtx.clearColor = VkClearColorValue{{red, green, blue, alpha}};
    }

    void Clear(GLbitfield mask) {
        if ((mask & GL_COLOR_BUFFER_BIT) == 0) return;

        EGLContext eglCtx = eglGetCurrentContext();
        if (eglCtx == EGL_NO_CONTEXT) return;

        VulkanContext& ctx = g_contexts[eglCtx];
        VulkanDisplay& disp = g_displays[eglGetCurrentDisplay()];
        EGLSurface drawSurf = eglGetCurrentSurface(EGL_DRAW);
        if (drawSurf == EGL_NO_SURFACE) return;

        VulkanSurface& surf = g_surfaces[drawSurf];
        if (surf.type != VulkanSurface::WINDOW) return;

        VkImage currentImage = surf.images[ctx.frameIndex];

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = currentImage;
        barrier.subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
        };
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        vkCmdPipelineBarrier(
                ctx.currentCmdBuffer,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
        );

        VkImageSubresourceRange range = barrier.subresourceRange;
        vkCmdClearColorImage(
                ctx.currentCmdBuffer,
                currentImage,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                &ctx.clearColor,
                1, &range
        );

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = 0;

        vkCmdPipelineBarrier(
                ctx.currentCmdBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
        );
    }
}