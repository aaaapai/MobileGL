#pragma once

namespace MobileGL {
	using String = std::string;
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

	using Data = Vector<Uint8>;
	struct DataPtr {
		void* data;
		SizeT size;
	};

	struct Version {
		Int Major;
		Int Minor;
		Int Patch;
		Optional<String> Suffix;

		String toString() const {
			String versionStr = std::format("{}.{}.{}", Major, Minor, Patch);
			if (Suffix.has_value()) {
				versionStr += std::format("{}", Suffix.value());
			}
			return versionStr;
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