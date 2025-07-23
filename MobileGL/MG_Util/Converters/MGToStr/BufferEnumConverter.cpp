#include "../../../Includes.h"

namespace MobileGL {
	namespace MG_Util {
		String ConvertBufferTargetToString(BufferTarget bufferTarget) {
			switch (bufferTarget) {
			case BufferTarget::Vertex: return "Vertex";
			case BufferTarget::Index: return "Index";
			case BufferTarget::Uniform: return "Uniform";
			case BufferTarget::CopyRead: return "CopyRead";
			case BufferTarget::CopyWrite: return "CopyWrite";
			case BufferTarget::PixelPack: return "PixelPack";
			case BufferTarget::PixelUnpack: return "PixelUnpack";
			case BufferTarget::Query: return "Query";
			case BufferTarget::Texture: return "Texture";
			case BufferTarget::TransformFeedback: return "TransformFeedback";
			case BufferTarget::AtomicCounter: return "AtomicCounter";
			case BufferTarget::DispatchIndirect: return "DispatchIndirect";
			case BufferTarget::DrawIndirect: return "DrawIndirect";
			case BufferTarget::ShaderStorage: return "ShaderStorage";
			case BufferTarget::Unknown:
			default: return "Unknown";
			}
		}

		String ConvertBufferUsageToString(BufferUsage usage) {
			switch (usage) {
			case BufferUsage::StreamDraw: return "StreamDraw";
			case BufferUsage::StreamRead: return "StreamRead";
			case BufferUsage::StreamCopy: return "StreamCopy";
			case BufferUsage::StaticDraw: return "StaticDraw";
			case BufferUsage::StaticRead: return "StaticRead";
			case BufferUsage::StaticCopy: return "StaticCopy";
			case BufferUsage::DynamicDraw: return "DynamicDraw";
			case BufferUsage::DynamicRead: return "DynamicRead";
			case BufferUsage::DynamicCopy: return "DynamicCopy";
			default: return "Unknown";
			}
		}

		String ConvertBufferMappingAccessToString(BufferMappingAccessBit access) {
			String result = "[";
			if (Any(access & BufferMappingAccessBit::Read)) result += "Read, ";
			if (Any(access & BufferMappingAccessBit::Write)) result += "Write, ";
			if (Any(access & BufferMappingAccessBit::InvalidateRange)) result += "InvalidateRange, ";
			if (Any(access & BufferMappingAccessBit::InvalidateBuffer)) result += "InvalidateBuffer, ";
			if (Any(access & BufferMappingAccessBit::FlushExplicit)) result += "FlushExplicit, ";
			if (Any(access & BufferMappingAccessBit::Unsynchronized)) result += "Unsynchronized, ";
			if (Any(access & BufferMappingAccessBit::Persistent)) result += "Persistent, ";
			if (Any(access & BufferMappingAccessBit::Coherent)) result += "Coherent, ";
			result.pop_back();
			result.pop_back();
			result += "]";
			return result.empty() ? "[]" : result;
		}
	}
}