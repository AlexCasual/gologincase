#pragma once
#include <google/tagged_json.hpp>
#include <core/types.hpp>

namespace gologin
{
	namespace json
	{
		struct packer
		{
			template<class msg>
			static void from(const core::types::buffer_t& _buff, msg& _msg)
			{
				google::json::packer::from(_buff, _msg);
			}

			template<class msg>
			static void to(const msg& _msg, core::types::buffer_t& _buff)
			{
				_buff = google::json::packer::to(_msg);
			}
		};
	}
}