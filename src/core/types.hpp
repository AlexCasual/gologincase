#pragma once

#include <vector>
#include <string_view>

namespace gologin
{
	namespace core
	{
		namespace types
		{
			using buffer_t = std::vector<char>;
			using packet_type = std::string_view;
		}
	}
}