#pragma once
#include "MG_Util/Types.h"

namespace MobileGL {
	enum class BufferTarget {
		Vertex,
		Index,
		Uniform,
		CopyRead,
		CopyWrite,
		PixelPack,
		PixelUnpack,
		Query,
		Texture,
		TransformFeedback,
		AtomicCounter,
		DispatchIndirect,
		DrawIndirect,
		ShaderStorage,
        BufferTargetCount,
		Unknown = -1
	};

	enum class BufferUsage {
		StreamDraw,
		StreamRead,
		StreamCopy,
		StaticDraw,
		StaticRead,
		StaticCopy,
		DynamicDraw,
		DynamicRead,
		DynamicCopy,
		Unknown = -1
	};

	enum class BufferMappingAccessBit : Uint {
		Null = 0x00,
		Read = 0x01,
		Write = 0x02,
		InvalidateRange = 0x04,
		InvalidateBuffer = 0x08,
		FlushExplicit = 0x10,
		Unsynchronized = 0x20,
		Persistent = 0x40,
		Coherent = 0x80
	};

	// inline Flags<BufferMappingAccessBit> operator|(BufferMappingAccessBit a, const BufferMappingAccessBit b) {
		// return Flags(a) | b;
	// }
	//
	// inline BufferMappingAccessBit& operator|=(BufferMappingAccessBit& a, BufferMappingAccessBit b) {
	// 	a = a | b;
	// 	return a;
	// }

	// inline Flags<BufferMappingAccessBit> operator&(const BufferMappingAccessBit a, const BufferMappingAccessBit b) {
	// 	return Flags(a) & b;
	// }

	// inline bool Any(BufferMappingAccessBit a) {
	// 	using T = std::underlying_type_t<BufferMappingAccessBit>;
	// 	return static_cast<T>(a) != 0;
	// }

	namespace MG_State {
		namespace GLState {
			class BufferObject {
			public:
				using TargetEnum = BufferTarget;
				
				BufferObject();

				void Resize(SizeT size);
				void UploadData(DataPtr data, SizeT atOffset);
				void SetUsage(BufferUsage usage);
				void* AcquireMemory(Bool markMapped, Bool read, Bool write);
				void* AcquireMemoryRange(Range1D range, Flags<BufferMappingAccessBit> access);
				void ReleaseMemory();
				void FlushMemoryRange(SizeT offset, SizeT length);
				void UploadSubData(DataPtr data, SizeT atOffset);
				void CopyDataFrom(const SharedPtr<BufferObject>& src, SizeT srcOffset, SizeT dstOffset, SizeT size);
				void ClearDirty();
				
				Bool IsMapped() const;
				SizeT GetSize() const;
				BufferUsage GetUsage() const;
				Range1D GetDirtyRange() const;
				Range1D GetMappedRange() const;
				Flags<BufferMappingAccessBit> GetMappingAccess() const;
			private:
				Int m_id = 0;
				SizeT m_size = 0;
				BufferUsage m_usage = BufferUsage::StaticDraw;
				Data m_data;
				Bool m_isMapped;
				Flags<BufferMappingAccessBit> m_mappingAccess;
				Range1D m_dirtyRange;
				Range1D m_mappedRange;
				std::vector<Uint8> m_stagingData;
				bool m_ownsStagingData;
			};
		}
	}
}
