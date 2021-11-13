#pragma once

#include <core/dispatcher.hpp>

namespace gologin
{
	namespace tcp
	{		
		template <class packer>
		class dispatcher : public gologin::core::dispatcher
		{
			using handler_id_t = std::size_t;

		public:
			dispatcher(std::shared_ptr<gologin::core::logger> logger)
			:m_logger(logger)
			{

			}

			~dispatcher() override
			{

			}

			bool dispatch(std::string& _cmd, const core::types::client_cookies_t& _cookies, const core::types::buffer_t& in, core::types::buffer_t& out) override
			{
				auto _handler_id = std::hash<std::string_view>{}(_cmd);

				auto const iter = m_handlers.find(_handler_id);
				if (iter != m_handlers.end())
				{
					try
					{
						iter->second(_cmd, _cookies, in, out);
						return true;
					}
					catch (const std::exception& ex)
					{
						m_logger->error("dispatcher::dispatch => handler [{}] exception !{}!", _cmd, ex.what());
					}
				}

				return false;
			}

			template <typename TFunc>
			void handle(std::string_view _cmd, TFunc f)
			{
				   std::function func = std::forward<TFunc>(f);
				   handle_(std::hash<std::string_view>{}(_cmd), std::move(func));
			}

		private:
			template<class R, class ...ars>
			void handle_(handler_id_t id, std::function<R(ars...)> func)
			{
				if (m_handlers.find(id) != m_handlers.end())
				{
					throw std::invalid_argument{ "[" + std::string{__func__ } + "] failed to add handler. "
							"The id \"" + std::to_string(id) + "\" already exists." };
				}

				auto wrapper = [f = std::move(func)](std::string& _cmd, const core::types::client_cookies_t& _cookies, const core::types::buffer_t& in, core::types::buffer_t& out)
				{
					using arguments_tuple_type = std::tuple<std::decay_t<ars> ... >;

					auto _arguments_count = std::tuple_size_v<std::tuple<ars... >>;

					//if constexpr(_arguments_count == 2) //! surprise - dont work !
					if constexpr(std::tuple_size_v<std::tuple<ars... >> == 3)
					{
						typename std::tuple_element<1, arguments_tuple_type>::type _req;

						packer::from(in, _req);

						typename std::tuple_element<2, arguments_tuple_type>::type _resp;

						f(_cookies, _req, _resp);

						_cmd = _resp[_message_field_("command")];

						packer::to(_resp, out);
					}

					if constexpr(std::tuple_size_v<std::tuple<ars... >> == 2)
					{
						typename std::tuple_element<1, arguments_tuple_type>::type _req;

						packer::from(in, _req);

						f(_cookies, _req);
					}
				};

				m_handlers.emplace(std::move(id), std::move(wrapper));
			}

			using handler_type = std::function<void(std::string&, const core::types::client_cookies_t&, const core::types::buffer_t& , core::types::buffer_t& )>;
			using handlers_type = std::map<handler_id_t, handler_type>;

			handlers_type m_handlers;
			std::shared_ptr<gologin::core::logger> m_logger;
		};
	}
}