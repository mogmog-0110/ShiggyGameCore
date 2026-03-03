#pragma once

#include <cstdint>

namespace sgc
{

/// Library version information
struct Version
{
	std::uint32_t major;
	std::uint32_t minor;
	std::uint32_t patch;
};

/// Current library version
inline constexpr Version LIBRARY_VERSION{0, 1, 0};

/// Toolchain verification: requires C++20
#ifdef _MSVC_LANG
static_assert(_MSVC_LANG >= 202002L, "ShiggyGameCore requires C++20 or later");
#else
static_assert(__cplusplus >= 202002L, "ShiggyGameCore requires C++20 or later");
#endif

} // namespace sgc
