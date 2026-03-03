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
static_assert(__cplusplus >= 202002L || _MSVC_LANG >= 202002L, "ShiggyGameCore requires C++20 or later");

} // namespace sgc
