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

	std::list<auth_client> provider::get_clients(const client_status& _status)
	{
		std::list<auth_client> _clients;

		std::shared_lock<std::shared_mutex> _lock(m_clients_mutex);
		for (const auto& _c : m_clients)
		{
			if (_c.second._status == _status)
			{
				_clients.emplace_back(_c.second);
			}
		}

		return _clients;
	}

	client_status provider::status_client(std::uint64_t _client_id, const std::string& _client_session)
	{
		std::shared_lock<std::shared_mutex> _lock(m_clients_mutex);
		if (auto _c = m_clients.find(_client_id); _c != m_clients.end())
		{
			return _c->second._status;
		}

		return client_status::not_found;
	}
		
	client_status provider::add_client(std::uint64_t _client_id, const core::types::client_cookies_t& _cookies,
		const std::string& _client_login, const std::string& _client_pass, client_session_id_t& _session)
	{
		if (_client_login != m_client_login || _client_pass != m_client_pass)
		{
			return client_status::rejected;
		}

		std::lock_guard<std::shared_mutex> _lock(m_clients_mutex);
		if (auto _c = m_clients.find(_client_id); _c == m_clients.end())
		{
			//generate session id
			{
				std::random_device _rd;
				std::mt19937 _mt(_rd());
				std::uniform_int_distribution<int> dist('a', 'z'); 
				std::generate_n(std::back_inserter(_session), 32, [&]{return dist(_mt);});
			}

			m_clients.emplace(std::piecewise_construct,
				std::forward_as_tuple(_client_id),
				std::forward_as_tuple(_cookies, _client_id, _session, m_client_login, client_status::accepted));

			return client_status::accepted;
		}
		else
		{
			_c->second._status;
		}

		return client_status::removed;
	}

	client_status provider::rem_client(std::uint64_t _client_id, const std::string& _client_session)
	{
		std::lock_guard<std::shared_mutex> _lock(m_clients_mutex);
		if (auto _c = m_clients.find(_client_id); _c != m_clients.end())
		{
			m_clients.erase(_c);
			return client_status::removed;
		}

		return client_status::not_found;
	}
}