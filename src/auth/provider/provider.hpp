#pragma once 
#include <core/core.hpp>
#include <tcp/core/core.hpp>

namespace gologin
{	
	namespace auth
	{
		using client_session_id_t = gologin::tcp::client_traits::session_id_t;

		enum class status
		{
			success = 0,
			fail
		};

		class provider
		{
			public:
			provider(const std::string& _client_login, const std::string& _client_pass, std::shared_ptr<gologin::core::logger> _logger);
			~provider();
		
			status check_client(std::uint64_t _client_id, const std::string& _client_session);
			status rem_client(std::uint64_t _client_id, const std::string& _client_session);
			status add_client(std::uint64_t _client_id, const std::string& _client_login, const std::string& _client_pass, client_session_id_t& _session);
		
			private:
			std::string m_client_login;
			std::string m_client_pass;
			std::shared_ptr<gologin::core::logger> m_logger;
		};
	}
}
