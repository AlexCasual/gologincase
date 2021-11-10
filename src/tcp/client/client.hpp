#pragma once
#include <core/core.hpp>
#include <tcp/core/core.hpp>

namespace gologin
{
	namespace tcp
	{
		class client : public client_base
		{
		public:
		  client(std::shared_ptr<gologin::core::dispatcher> disp, std::shared_ptr<gologin::core::logger> logger) noexcept;
		  ~client() override;

		  client_traits::status connect(const std::string& host, uint16_t port) noexcept;
		  client_traits::status disconnect() noexcept;
		  
		private:
		  bool recv(gologin::core::types::buffer_t& _buff) override;
		  bool recv(gologin::tcp::packet_header_t& _header, gologin::core::types::buffer_t& _buff);
		
		private:
			std::shared_ptr<gologin::core::dispatcher> m_disp;
			std::shared_ptr<gologin::core::logger> m_logger;
			std::thread m_handler_thread;
			#ifdef _WIN32
				WSAData m_wsadata;
			#endif
		};
	}
}