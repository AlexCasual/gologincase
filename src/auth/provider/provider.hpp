#pragma once 
#include <core/core.hpp>
#include <tcp/core/core.hpp>

namespace gologin
{	
	namespace auth
	{
		using client_session_id_t = gologin::tcp::client_traits::session_id_t;

		enum class client_status
		{
			added = 0,
			removed,
			accepted,
			rejected,
			not_found
		};

		using auth_client = struct
		{
			core::types::client_cookies_t	_cookie;
			std::uint64_t					_id;
			std::string						_session;
			std::string						_login;
			client_status					_status;
		};

		class provider
		{
			public:
			provider(const std::string& _client_login, const std::string& _client_pass, std::shared_ptr<gologin::core::logger> _logger);
			~provider();
		
			std::list<auth_client> get_clients(const client_status& _status);
			client_status status_client(std::uint64_t _client_id, const std::string& _client_session);
			client_status rem_client(std::uint64_t _client_id, const std::string& _client_session);
			client_status add_client(std::uint64_t _client_id, const core::types::client_cookies_t& _cookies, 
				const std::string& _client_login, const std::string& _client_pass, client_session_id_t& _session);
		
			private:
			std::string m_client_login;
			std::string m_client_pass;
			std::shared_mutex m_clients_mutex;
			std::unordered_map<std::uint64_t, auth_client> m_clients;
			std::shared_ptr<gologin::core::logger> m_logger;
		};
	}
}
