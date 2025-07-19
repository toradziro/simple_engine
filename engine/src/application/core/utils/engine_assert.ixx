export module engine_assert;

import <print>;
import <string_view>;
import <exception>;

#ifdef _DEBUG
export void engineAssert(bool val, const std::string_view message = "")
{
	if (!val)
	{
		std::println("[ASSERT]: {}", message);
		__debugbreak();
	}
}
#else
export void engineAssert(bool val, const std::string_view message = "")
{
	if (!val)
	{
		std::println("[ASSERT]: {}", message);
		std::terminate();
	}
}
#endif
