#pragma once

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
	template<typename T, typename... Args>
	inline SharedPtr<T> MakeShared(Args&&... args) {
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
	template<typename T, typename... Args>
	inline UniquePtr<T> MakeUnique(Args&&... args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}
	using SizeT = std::size_t;
	template <typename T, SizeT N>
	using Array = std::array<T, N>;
	template <typename Key, typename Value>
	using UnorderedMap = std::unordered_map<Key, Value>;
	template<typename T>
	inline constexpr std::remove_reference_t<T>&& Move(T&& t) noexcept {
		return static_cast<std::remove_reference_t<T>&&>(t);
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
		Fixed32 
	};

	struct Range1D {
		const SizeT maxEnd;
		const SizeT minStart;
		SizeT start;
		SizeT end;

		Range1D(SizeT minStart_, SizeT maxEnd_)
			: minStart(minStart_), maxEnd(maxEnd_), start(minStart_), end(minStart_) {
			if (minStart_ >= maxEnd_) {
				throw RuntimeError("Invalid range: minStart must be less than maxEnd");
			}
		}

		void Extend(SizeT newStart, SizeT newEnd) {
			if (newStart < minStart || newEnd > maxEnd) {
				throw RuntimeError("Range exceeds bounds");
			}
			start = std::min(start, newStart);
			end = std::max(end, newEnd);
		}
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
}