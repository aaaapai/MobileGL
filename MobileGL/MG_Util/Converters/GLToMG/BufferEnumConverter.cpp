// MobileGL - MobileGL/MG_Util/Converters/GLToMG/BufferEnumConverter.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "BufferEnumConverter.h"

namespace MobileGL {
    namespace MG_Util {
        BufferTarget ConvertGLEnumToBufferTarget(GLenum bufferTarget) {
            switch (bufferTarget) {
            case GL_ARRAY_BUFFER:
                return BufferTarget::Vertex;
            case GL_ELEMENT_ARRAY_BUFFER:
                return BufferTarget::Index;
            case GL_UNIFORM_BUFFER:
                return BufferTarget::Uniform;
            case GL_COPY_READ_BUFFER:
                return BufferTarget::CopyRead;
            case GL_COPY_WRITE_BUFFER:
                return BufferTarget::CopyWrite;
            case GL_PIXEL_PACK_BUFFER:
                return BufferTarget::PixelPack;
            case GL_PIXEL_UNPACK_BUFFER:
                return BufferTarget::PixelUnpack;
            case GL_QUERY_BUFFER:
                return BufferTarget::Query;
            case GL_TEXTURE_BUFFER:
                return BufferTarget::Texture;
            case GL_TRANSFORM_FEEDBACK_BUFFER:
                return BufferTarget::TransformFeedback;
            case GL_ATOMIC_COUNTER_BUFFER:
                return BufferTarget::AtomicCounter;
            case GL_DISPATCH_INDIRECT_BUFFER:
                return BufferTarget::DispatchIndirect;
            case GL_DRAW_INDIRECT_BUFFER:
                return BufferTarget::DrawIndirect;
            case GL_PARAMETER_BUFFER_ARB:
                return BufferTarget::Parameter;
            case GL_SHADER_STORAGE_BUFFER:
                return BufferTarget::ShaderStorage;
            case GL_UNKNOWN_MGL:
            default:
                return BufferTarget::Unknown;
            }
        }

        BufferUsage ConvertGLEnumToBufferUsage(GLenum usage) {
            switch (usage) {
            case GL_STREAM_DRAW:
                return BufferUsage::StreamDraw;
            case GL_STREAM_READ:
                return BufferUsage::StreamRead;
            case GL_STREAM_COPY:
                return BufferUsage::StreamCopy;
            case GL_STATIC_DRAW:
                return BufferUsage::StaticDraw;
            case GL_STATIC_READ:
                return BufferUsage::StaticRead;
            case GL_STATIC_COPY:
                return BufferUsage::StaticCopy;
            case GL_DYNAMIC_DRAW:
                return BufferUsage::DynamicDraw;
            case GL_DYNAMIC_READ:
                return BufferUsage::DynamicRead;
            case GL_DYNAMIC_COPY:
                return BufferUsage::DynamicCopy;
            default:
                return BufferUsage::Unknown;
            }
        }

        Flags<BufferMappingAccessBit> ConvertGLEnumToBufferMappingAccess(GLbitfield access) {
            Flags result = BufferMappingAccessBit::Null;
            if (access & GL_MAP_READ_BIT) result |= BufferMappingAccessBit::Read;
            if (access & GL_MAP_WRITE_BIT) result |= BufferMappingAccessBit::Write;
            if (access & GL_MAP_INVALIDATE_RANGE_BIT) result |= BufferMappingAccessBit::InvalidateRange;
            if (access & GL_MAP_INVALIDATE_BUFFER_BIT) result |= BufferMappingAccessBit::InvalidateBuffer;
            if (access & GL_MAP_FLUSH_EXPLICIT_BIT) result |= BufferMappingAccessBit::FlushExplicit;
            if (access & GL_MAP_UNSYNCHRONIZED_BIT) result |= BufferMappingAccessBit::Unsynchronized;
            if (access & GL_MAP_PERSISTENT_BIT) result |= BufferMappingAccessBit::Persistent;
            if (access & GL_MAP_COHERENT_BIT) result |= BufferMappingAccessBit::Coherent;
            return result;
        }
    } // namespace MG_Util
} // namespace MobileGL
