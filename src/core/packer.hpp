#pragma once

#include <core/headers.hpp>

namespace gologin
{
	namespace core
	{
		struct packer
		{
			template<class msg>
			static void from(const std::vector<char>& /*_buff*/, msg& /*_msg*/)
			{
			}

			template<class msg>
			static void to(const msg& /*_msg*/, std::vector<char>& /*_buff*/)
			{
			}
		};
	}
}