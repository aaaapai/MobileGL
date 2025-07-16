#pragma once

namespace MobileGL {
	enum class BufferTarget {
		Vertex,
		Index,
		Uniform,
		Array,
		ElementArray,
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
		ShaderStorage
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
				void UploadData(DataPtr data);
				void SetUsage(BufferUsage usage);
				SizeT GetSize() const;
				BufferUsage GetUsage() const;
				Range1D GetDirtyRange() const;
				SharedPtr<Data> AquireMemory(Bool markMapped);
				SharedPtr<Data> AquireMemoryRange(Range1D range, BufferMappingAccessBit access);
				Bool IsMapped() const;

			private:
				Int m_id = 0;
				SizeT m_size = 0;
				BufferUsage m_usage = BufferUsage::StaticDraw;
				Data m_data;
				Bool m_isMapped;
				BufferMappingAccessBit m_mappingAccess;
				UniquePtr<Range1D> m_dirtyRange;
			};
		}
	}
}