#include "app.hpp"
#include <messages/json/packer.hpp>
#include <tcp/core/dispatcher.hpp>

namespace detail
{
	static const std::string _logger_name("gologin_server");

	static const std::string _logger_file("gologin_server.log");
	static const std::string _http_server_property("server_addrr");
	static const std::string _logger_level_property("logger_level");
	static const std::string _logger_sink_property("logger_sink");
	static const std::string _client_login_property("client_login");
	static const std::string _client_pass_property("client_pass");
			
	using packer_t = gologin::json::packer;
	using dispacher_t = gologin::tcp::dispatcher<packer_t>;
	
	std::shared_ptr<dispacher_t> create_dispatcher(std::shared_ptr<gologin::core::logger> _logger)
	{
		return std::make_shared<dispacher_t>(_logger);
	}
	
	std::shared_ptr<gologin::tcp::server> create_tcp_server(const std::string& _addrr, std::shared_ptr<dispacher_t> _disp, std::shared_ptr<gologin::core::logger> _logger)
	{
		auto _saddrr = Poco::Net::SocketAddress(_addrr);
		
		return std::make_shared<gologin::tcp::server>(_saddrr.port(), _disp, _logger);
	}
			
	std::shared_ptr<gologin::auth::provider> create_auth_provider(const std::string& _client_login, const std::string& _client_pass, std::shared_ptr<gologin::core::logger> _logger)
	{
		return std::make_shared<gologin::auth::provider>(_client_login, _client_pass, _logger);
	}
}

namespace gologin
{
	app::app()
	{
	}
	
	app::~app()
	{
	}
	
	void app::startup()
	{
		m_config._srv_addrr = config().getString(detail::_http_server_property, "localhost:10052");
		m_config._client_pass = config().getString(detail::_client_pass_property, ""),
		m_config._client_login = config().getString(detail::_client_login_property, ""),
			
		m_config._data_folder = config().getString("application.dataDir", "");
		
		std::string _logger_path(m_config._data_folder);
		_logger_path += "\\";
		_logger_path += detail::_logger_file;
		
		auto _logger_level = config().getString(detail::_logger_level_property, "debug");
		auto _logger_sink = config().getString(detail::_logger_sink_property, "console");

		if (!std::filesystem::exists(m_config._data_folder))
		{
			auto _res = std::filesystem::create_directories(m_config._data_folder);
		}
		
		m_logger = gologin::core::create_logger(_logger_level, _logger_sink, detail::_logger_name, _logger_path);
	}

	int app::main(const std::vector<std::string>& /*args*/)
	{
		try
		{	
			startup();
		
			m_main_thread = std::jthread([&] (std::stop_token stoken)
			{
				main();
				
				std::mutex mutex;
				std::unique_lock lock(mutex);
					
				std::condition_variable_any().wait(lock, stoken, [&stoken] { return false; });
					
				if(stoken.stop_requested()) 
				{
					m_logger->info("app::main => exit from main thread by request.");
					return;
				}
			});	
			
			exit();
		}
		catch (const Poco::Exception& ex)
		{
			m_logger->error("app::main => exception !{}!", ex.displayText().c_str());
		}
		catch (const std::exception& ex)
		{
			m_logger->error("app::main => exception !{}!", ex.what());
		}
		catch (...)
		{
			m_logger->error("app::main => exception!");
		}

		m_logger->info("app::main => exit.");

		return Application::EXIT_OK;
	}
	
	void app::initialize(Poco::Util::Application& self)
	{
		loadConfiguration();
		ServerApplication::initialize(self);
	}
	
	void app::uninitialize()
	{
		ServerApplication::uninitialize();
	}
	
	void app::defineOptions(Poco::Util::OptionSet& options)
	{
		ServerApplication::defineOptions(options);
	}
	
	void app::handleOption(const std::string& name, const std::string& value)
	{		
		ServerApplication::handleOption(name, value);
	}
	
	void app::exit()
	{
		//todo : add termination logic;
		
		waitForTerminationRequest();
		
		m_logger->info("app::exit => receive stop request.");
		
		m_main_thread.request_stop();
		
		m_logger->info("app::exit => stopping main thread...");
		
		m_main_thread.join();
		
		m_logger->info("app::exit => main thread has been stopped.");
	}
	
	void app::main()
	{			
		m_logger->info("app::main => app_folder : {}", m_config._data_folder);
		m_logger->info("app::main => srv_addrr : {}", m_config._srv_addrr);
		m_logger->info("app::main => clinet : [ {}:{} ]", m_config._client_login, m_config._client_pass);

		m_auth_provider = detail::create_auth_provider(m_config._client_login, m_config._client_pass, m_logger);
                				
		auto _disp = detail::create_dispatcher(m_logger);
		_disp->handle(messages::commands::hello,[&](const core::types::client_cookies_t& _cookies, const messages::client::hello::request& _req, messages::server::hello::response& _res)
			{
				return client_hello_packet_handler(_cookies, _req, _res);
			});

		_disp->handle(messages::commands::ping,[&](const core::types::client_cookies_t& _cookies, const messages::client::ping::request& _req, messages::server::auth::response& _res)
			{
				return client_ping_packet_handler(_cookies, _req, _res);
			});

		_disp->handle(messages::commands::auth,[&](const core::types::client_cookies_t& _cookies, const messages::client::auth::request& _req, messages::server::auth::response& _res)
			{
				return client_login_packet_handler(_cookies, _req, _res);
			});

		_disp->handle(messages::commands::bye,[&](const core::types::client_cookies_t& _cookies, const messages::client::bye::request& _req, messages::server::bye::response& _res)
			{
				return client_logout_packet_handler(_cookies, _req, _res);
			});

		_disp->handle(messages::commands::msg,[&](const core::types::client_cookies_t& _cookies, const messages::client::msg::request& _req, messages::server::msg::response& _res)
			{
				return client_message_packet_handler(_cookies, _req, _res);
			});

		_disp->handle(messages::commands::msg_reply,[&](const core::types::client_cookies_t& _cookies, const messages::client::msg::response& _res)
			{
				return client_message_reply_packet_handler(_cookies, _res);
			});
		
		m_tcp_server = detail::create_tcp_server(m_config._srv_addrr, _disp, m_logger);	
				
		m_tcp_server->start();

		m_logger->info("app::main => ok");
	}
		
	void app::client_hello_packet_handler(const core::types::client_cookies_t& _cookies, const messages::client::hello::request& request, messages::server::hello::response& responce)
	{
		auto _id = request[_message_field_("id")];
		
		responce[_message_field_("id")] = _id;
		responce[_message_field_("command")] = messages::commands::hello;
		responce[_message_field_("auth_method")]= "plain-text";

		m_logger->debug("app::client_hello_packet_handler => client id : {}", _id);
	}

	void app::client_ping_packet_handler(const core::types::client_cookies_t& _cookies, const messages::client::ping::request& request, messages::server::auth::response& responce)
	{
		auto _id = request[_message_field_("id")];
		auto _session = request[_message_field_("session")];
		
		responce[_message_field_("id")] = _id;
		responce[_message_field_("command")] = messages::commands::ping_reply;

		auto _auth = m_auth_provider->status_client(_id, _session);
		if (_auth == auth::client_status::accepted)
		{
			responce[_message_field_("status")] = "ok";
			responce[_message_field_("message")]= "";
		}
		else
		{
			responce[_message_field_("status")] = "failed";
			responce[_message_field_("message")]= "client not registred";
		}

		m_logger->debug("app::client_ping_packet_handler => client [id: {}, session: {}] auth status: {}", _id, _session, _auth);
	}

	void app::client_login_packet_handler(const core::types::client_cookies_t& _cookies, const messages::client::auth::request& request, messages::server::auth::response& responce)
	{
		auto _id = request[_message_field_("id")];
		auto _login = request[_message_field_("login")];
		auto _password = request[_message_field_("password")];
		
		responce[_message_field_("id")] = _id;
		responce[_message_field_("command")] = messages::commands::auth;

		std::string _session;
		auto _auth = m_auth_provider->add_client(_id, _cookies, _login, _password, _session);
		if (_auth == auth::client_status::accepted)
		{
			responce[_message_field_("status")] = "ok";
			responce[_message_field_("session")] = _session;
			responce[_message_field_("message")]= "";
		}
		else
		{
			responce[_message_field_("status")] = "failed";
			responce[_message_field_("message")]= "wrong auth login or pass";
			responce[_message_field_("session")] = _session;
		}

		m_logger->debug("app::client_login_packet_handler => client [id: {}, login: {}, password: {}] auth status: {} --> session: {}", _id, _login, _password, _auth, _session);
	}

	void app::client_logout_packet_handler(const core::types::client_cookies_t& _cookies, const messages::client::bye::request& request, messages::server::bye::response& responce)
	{
		auto _id = request[_message_field_("id")];
		auto _session = request[_message_field_("session")];

		responce[_message_field_("id")] = _id;
		responce[_message_field_("command")] = messages::commands::bye_reply;

		auto _auth = m_auth_provider->rem_client(_id, _session);
		if (_auth == auth::client_status::removed)
		{
			responce[_message_field_("status")] = "ok";
		}
		else
		{
			responce[_message_field_("status")] = "failed";
		}

		m_logger->debug("app::client_logout_packet_handler => client [id: {}, _session: {}] auth status: {}", _id, _session, _auth);
	}

	void app::client_message_packet_handler(const core::types::client_cookies_t& _cookies, const messages::client::msg::request& request, messages::server::msg::response& responce)
	{
		auto _id = request[_message_field_("id")];
		auto _session = request[_message_field_("session")];
		auto _body = request[_message_field_("body")];

		responce[_message_field_("id")] = _id;
		responce[_message_field_("command")] = messages::commands::msg_reply;

		std::string _client_id;
		auto _auth = m_auth_provider->status_client(_id, _session);
		if (_auth == auth::client_status::accepted)
		{
			auto _auth_clients = m_auth_provider->get_clients(auth::client_status::accepted);
			for (const auto& _cid : _auth_clients)
			{
				messages::server::msg::request _client_request;
				_client_request[_message_field_("id")] = _cid._id;
				_client_request[_message_field_("sender_login")] = _cid._login;
				_client_request[_message_field_("session")] = _cid._session;
				_client_request[_message_field_("body")] = _body;
				_client_request[_message_field_("command")] = messages::commands::msg;

				core::types::buffer_t _buff;
				detail::packer_t::to(_client_request, _buff);
				m_tcp_server->send_to(_cid._cookie, messages::commands::msg, _buff);
			}		
		}

		if (_auth == auth::client_status::accepted)
		{
			responce[_message_field_("status")] = "ok";
			responce[_message_field_("client_id")] = _id;
			responce[_message_field_("message")] = "";
		}
		else
		{
			responce[_message_field_("status")] = "failed";
			responce[_message_field_("client_id")] = _id;
			responce[_message_field_("message")] = "client not registered";
		}

		m_logger->debug("app::client_message_packet_handler => client [id: {}, _session: {}] auth status: {}", _id, _session, _auth);
	}

	void app::client_message_reply_packet_handler(const core::types::client_cookies_t& _cookies, const messages::client::msg::response& responce)
	{
		auto _id = responce[_message_field_("id")];
		auto _status = responce[_message_field_("status")];
		auto _client_id = responce[_message_field_("client_id")];
		auto _message = responce[_message_field_("message")];

		m_logger->debug("app::client_message_reply_packet_handler => client [id: {}, _client_id: {}] status: {}, message: {}", _id, _client_id, _status, _message);
	}
}