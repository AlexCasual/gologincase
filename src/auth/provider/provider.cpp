#include "provider.hpp"

namespace gologin::auth
{
	provider::provider(const std::string& _client_login, const std::string& _client_pass, std::shared_ptr<gologin::core::logger> _logger)
	: m_client_login(_client_login), m_client_pass(_client_pass), m_logger(_logger)
	{
	}
	
	provider::~provider()
	{
	}

	status provider::check_client(std::uint64_t _client_id, const std::string& _client_session)
	{
		return status::success;
	}
		
	status provider::add_client(std::uint64_t _client_id, const std::string& _client_login, const std::string& _client_pass, client_session_id_t& _session)
	{
		return status::success;
	}

	status provider::rem_client(std::uint64_t _client_id, const std::string& _client_session)
	{
		return status::success;
	}
}