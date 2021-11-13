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
			using cmd_t = std::string_view;

			using packet_header_t = struct
			{
				uint64_t sign;
				uint32_t size;
				char type[16];
			};

			using client_cookies_t = struct
			{
				std::size_t _server_client_id;
			};
		}
	}
}