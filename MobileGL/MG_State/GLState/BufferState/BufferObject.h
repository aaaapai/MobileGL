#pragma once

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
        BufferTargetCount
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
		DynamicCopy
	};

	enum class BufferMappingAccessBit : Uint {
		Null = 0x00,
		Read = 0x01,
		Write = 0x02,
		InvalidateRange = 0x04,
		InvalidateBuffer = 0x08,
		FlushExplicit = 0x10,
		Unsynchronized = 0x20
	};

	inline BufferMappingAccessBit operator|(BufferMappingAccessBit a, BufferMappingAccessBit b) {
		using T = std::underlying_type_t<BufferMappingAccessBit>;
		return static_cast<BufferMappingAccessBit>(static_cast<T>(a) | static_cast<T>(b));
	}

	inline BufferMappingAccessBit& operator|=(BufferMappingAccessBit& a, BufferMappingAccessBit b) {
		a = a | b;
		return a;
	}

	inline BufferMappingAccessBit operator&(BufferMappingAccessBit a, BufferMappingAccessBit b) {
		using T = std::underlying_type_t<BufferMappingAccessBit>;
		return static_cast<BufferMappingAccessBit>(static_cast<T>(a) & static_cast<T>(b));
	}

	inline bool any(BufferMappingAccessBit a) {
		using T = std::underlying_type_t<BufferMappingAccessBit>;
		return static_cast<T>(a) != 0;
	}

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
				void* AcquireMemoryRange(Range1D range, BufferMappingAccessBit access);
				void ReleaseMemory();
				void FlushMemoryRange(SizeT offset, SizeT length);
				void UploadSubData(SizeT offset, SizeT size, const void* data);
				void CopyDataFrom(const SharedPtr<BufferObject>& src, SizeT srcOffset, SizeT dstOffset, SizeT size);
				void ClearDirty();
				
				Bool IsMapped() const;
				SizeT GetSize() const;
				BufferUsage GetUsage() const;
				Range1D GetDirtyRange() const;

			private:
				Int m_id = 0;
				SizeT m_size = 0;
				BufferUsage m_usage = BufferUsage::StaticDraw;
				Data m_data;
				Bool m_isMapped;
				BufferMappingAccessBit m_mappingAccess;
				Range1D m_dirtyRange;
				Range1D m_mappedRange;
				std::vector<Uint8> m_stagingData;
				bool m_ownsStagingData;
			};
		}
	}
}