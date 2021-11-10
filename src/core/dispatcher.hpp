#pragma once

#include <core/headers.hpp>
#include <core/types.hpp>
#include <core/logger.hpp>

namespace gologin
{
	namespace core
	{		
		class dispatcher
		{
		public:
			dispatcher() = default;
			
			virtual ~dispatcher() = default;

			virtual bool dispatch(gologin::core::types::packet_type tp, const types::buffer_t& in, types::buffer_t& out) = 0;
			
			protected:
			std::shared_ptr<gologin::core::logger> m_logger;
		};
		
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