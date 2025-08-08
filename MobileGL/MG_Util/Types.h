#pragma once

#define GL_UNKNOWN_MGL 0

namespace MobileGL {
	using String = std::string;
	using StringStream = std::stringstream;
	using Int8 = int8_t;
	using Uint8 = uint8_t;
	using Int16 = int16_t;
	using Uint16 = uint16_t;
	using Int32 = int32_t;
	using Uint32 = uint32_t;
	using Int = Int32;
	using Uint = Uint32;
	using Int64 = int64_t;
	using Uint64 = uint64_t;	
	using Bool = bool;
	using Float = float;
	using Double = double;
	template <typename T>
	using Vector = std::vector<T>;
	template <typename T, typename N>
	using Pair = std::pair<T, N>;
	template <typename T>
	using SharedPtr = std::shared_ptr<T>;
	template <typename T>
	using UniquePtr = std::unique_ptr<T>;
	template <typename T, typename... Args>
	inline SharedPtr<T> MakeShared(Args&&... args) {
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
	template <typename T, typename... Args>
	inline UniquePtr<T> MakeUnique(Args&&... args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}
	using SizeT = std::size_t;
	template <typename T, SizeT N>
	using Array = std::array<T, N>;
	template <typename Key, typename Value>
	using UnorderedMap = std::unordered_map<Key, Value>;
	template <typename T>
	inline constexpr std::remove_reference_t<T>&& Move(T&& t) noexcept {
		return static_cast<std::remove_reference_t<T>&&>(t);
	}
	template <typename T>
	inline constexpr void Copy(const T* src, T* dest, SizeT count) {
		std::copy(src, src + count, dest);
	}
	template<typename... Ts>
	constexpr auto ToArray(Ts&&... elems) {
		using E = std::common_type_t<Ts...>;
		return std::array<E, sizeof...(Ts)>{ { std::forward<Ts>(elems)... } };
	}
	class RuntimeError : public std::runtime_error {
	public:
		using std::runtime_error::runtime_error;
	};
	template <typename T>
	using Optional = std::optional<T>;
	using NulloptT = std::nullopt_t;
	const NulloptT Nullopt = std::nullopt;

    using std::format;

	using Data = Vector<Uint8>;
	struct DataPtr {
		void* data;
		SizeT size;
	};

	enum class DataType {
		Int8,
		Uint8,
		Int16,
		Uint16,
		Int32,
		Uint32,
		Float16,
		Float32,
		Float64,
		Fixed32,
		Unknown = -1
	};

	// represents a range of [start, end)
	struct Range1D {
		SizeT start = 0;
		SizeT end = 0;

		void Update(SizeT newStart, SizeT newEnd) {
			assert(newStart <= newEnd);
			start = newStart;
			end = newEnd;
		}

		void UnionUpdate(SizeT newStart, SizeT newEnd) {
			assert(newStart <= newEnd);
			if (start == end) {
				Update(newStart, newEnd);
				return;
			}
			start = std::min(start, newStart);
			end = std::max(end, newEnd);
		}

		void IntersectionUpdate(SizeT newStart, SizeT newEnd) {
			assert(newStart <= newEnd);
			start = std::max(start, newStart);
			end = std::min(end, newEnd);
			assert(start <= end);
		}
	};

	template <typename ObjectType>
	class BindingSlot {
	public:
		using TargetEnum = typename ObjectType::TargetEnum;

        BindingSlot(): m_target((TargetEnum)0), m_boundObject(nullptr) {}

		explicit BindingSlot(TargetEnum target)
			: m_target(target), m_boundObject(nullptr) {
		}

		void Bind(SharedPtr<ObjectType> object) {
			m_boundObject = object;
		}

		SharedPtr<ObjectType> GetBoundObject() const {
			return m_boundObject;
		}

		TargetEnum GetTarget() const {
			return m_target;
		}

	private:
		TargetEnum m_target;
		SharedPtr<ObjectType> m_boundObject;
	};

	struct Version {
		Int Major;
		Int Minor;
		Int Patch;
		Optional<String> Suffix;

		String toString(Pair<Bool, Bool> dots = { true, true }, Bool useSuffix = true) const {
			StringStream result;
			result << Major << (dots.first ? "." : "") << Minor << (dots.second ? "." : "") << Patch << 
				(useSuffix && Suffix.has_value() ? *Suffix : "");
			return result.str();
		}
	};

	struct BackendCap {

	};

	struct GLInfo {
		Version TargetGLVersion;
		Version TargetGLSLVersion;
		Vector<GLExtension> Extensions;
		Bool IsCompatibilityProfile;
	};

	struct RendererInfo {
		String RendererName;
		String BackendName;
		Optional<String> ExtraVendor;
		GLInfo RendererGLInfo;
		BackendCap BackendCapability;
	};

	template <typename Bit, typename Underlying =
			std::enable_if<std::is_scoped_enum_v<Bit>, std::underlying_type_t<Bit>>
	>
	class Flags {
	public:
		Flags() = default;

		Flags(Bit b): flags(static_cast<typename Underlying::type>(b)) {}

		Flags(typename Underlying::type b): flags(b) {}

		// Flags - Bit
		Flags operator|(const Bit b) const {
			return Flags(flags | static_cast<typename Underlying::type>(b));
		}

		Flags operator&(const Bit b) const {
			return Flags(flags & static_cast<typename Underlying::type>(b));
		}

		Flags& operator|=(const Bit b) {
			flags |= static_cast<typename Underlying::type>(b);
			return *this;
		}

		Flags& operator&=(const Bit b) {
			flags &= static_cast<typename Underlying::type>(b);
			return *this;
		}

		bool operator==(const Bit b) const {
			return flags == static_cast<typename Underlying::type>(b);
		}

		bool operator!=(const Bit b) const {
			return !(*this == b);
		}

		// Flags - Flags
		Flags operator|(const Flags b) const {
			return Flags(flags | b.flags);
		}

		Flags operator&(const Flags b) const {
			return Flags(flags & b.flags);
		}

		Flags& operator|=(Flags b) {
			flags |= b.flags;
			return *this;
		}

		Flags& operator&=(Flags b) {
			flags &= b.flags;
			return *this;
		}

		bool operator==(const Flags b) const {
			return flags == b.flags;
		}

		bool operator!=(const Flags b) const {
			return !(*this == b);
		}

		operator bool() const {
			return Any();
		}

	private:
		bool Any() const {
			return static_cast<typename Underlying::type>(flags) != 0;
		}

		typename Underlying::type flags = 0;
	};

	template<typename Bit, typename = std::enable_if_t<std::is_scoped_enum_v<Bit>>>
	Flags<Bit> operator|(const Bit lhs, const Bit rhs) {
		return Flags<Bit>(lhs) | rhs;
	}

	template<typename Bit, typename = std::enable_if_t<std::is_scoped_enum_v<Bit>>>
	Flags<Bit> operator&(const Bit lhs, const Bit rhs) {
		return Flags<Bit>(lhs) & rhs;
	}
}

#include "../MG_Util/ShaderTranspiler/Types.h"
