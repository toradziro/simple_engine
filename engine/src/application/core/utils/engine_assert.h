#pragma once

#include <print>
#include <string_view>
#include <exception>

#ifdef _DEBUG
inline void engineAssert(bool val, const std::string_view message = "")
{
	if (!val)
	{
		std::println("[ASSERT]: {}", message);
		__debugbreak();
	}
}
#else
inline void engineAssert(bool val, const std::string_view message = "")
{
	if (!val)
	{
		std::println("[ASSERT]: {}", message);
		std::terminate();
	}
}
#endif
