#include "app.hpp"
#include <messages/json/packer.hpp>
#include <tcp/core/dispatcher.hpp>

namespace detail
{
	static const wxString 	 _frame_title(wxT("Gologin by a.zawadski [2021]"));
	static const std::string _logger_name("gologin_client");
	static const std::string _logger_file("gologin_client.log");
	static const std::string _logger_level("debug");
	static const std::string _logger_sink("file");
	
	using dispacher_t = gologin::tcp::dispatcher<gologin::json::packer>;
	
	std::shared_ptr<dispacher_t> create_dispatcher(std::shared_ptr<gologin::core::logger> _logger)
	{
		return std::make_shared<dispacher_t>(_logger);
	}
	
	std::shared_ptr<gologin::tcp::client> create_tcp_client(std::shared_ptr<dispacher_t> _disp, std::shared_ptr<gologin::core::logger> _logger)
	{
		return std::make_shared<gologin::tcp::client>(_disp, _logger);
	}
}

namespace gologin::gui
{
	bool app::OnInit()
	{
		init();

		main();

		return true;
	}
	
	/*int app::OnRun()
	{
		return main();
	}
	
	int app::OnExit()
	{
		return exit();
	}*/
	
	void app::init()
	{
		m_config._data_folder = wxGetHomeDir();
		
		std::string _logger_path(m_config._data_folder.c_str());
		_logger_path += "\\";
		_logger_path += "gologin";
		_logger_path += "\\";
		_logger_path += detail::_logger_file;

		if (!std::filesystem::exists(m_config._data_folder))
		{
			auto _res = std::filesystem::create_directories(m_config._data_folder);
		}
		
		m_logger = gologin::core::create_logger(detail::_logger_level, detail::_logger_sink, detail::_logger_name, _logger_path);
	}
	
	int app::main()
	{
		m_logger->info("app::main => app_folder : {}", m_config._data_folder);

		auto _disp = detail::create_dispatcher(m_logger);
		_disp->handle("message",[&](const gologin::messages::server::msg::request& _req, gologin::messages::client::msg::response& _res)
		{
			return server_message_packet_handler(_req, _res);
		});
		_disp->handle("message_reply",[&](const gologin::messages::server::msg::response& _res)
		{
			return server_message_reply_packet_handler(_res);
		});
		_disp->handle("hello",[&](const gologin::messages::server::hello::response& _res)
		{
			return server_hello_reply_packet_handler(_res);
		});
		_disp->handle("login",[&](const gologin::messages::server::auth::response& _res)
		{
			return server_auth_reply_packet_handler(_res);
		});
		_disp->handle("logout",[&](const gologin::messages::server::bye::response& _res)
		{
			return server_bye_reply_packet_handler(_res);
		});
		_disp->handle("ping",[&](const gologin::messages::server::ping::response& _res)
		{
			return server_ping_reply_packet_handler(_res);
		});

		m_tcp_client = detail::create_tcp_client(_disp, m_logger);
		
		m_frame = create_frame();

		m_frame->Show(true);

		m_logger->info("app::main => ok");

		return 1;
	}
	
	int app::exit()
	{
		m_logger->info("app::exit => receive stop request.");

		m_logger->info("app::exit => ok");

		return 0;
	}
	
	gologin::gui::frame* app::create_frame()
	{
		input_handlers_t _input_frame_callbacks;
		_input_frame_callbacks.emplace(ui_action::on_login, std::bind(&app::on_login_frame_callback, this, std::placeholders::_1));
		_input_frame_callbacks.emplace(ui_action::on_logout, std::bind(&app::on_logout_frame_callback, this, std::placeholders::_1));
		_input_frame_callbacks.emplace(ui_action::on_connect, std::bind(&app::on_connect_frame_callback, this, std::placeholders::_1));
		_input_frame_callbacks.emplace(ui_action::on_disconnect, std::bind(&app::on_disconnect_frame_callback, this, std::placeholders::_1));
		_input_frame_callbacks.emplace(ui_action::on_send, std::bind(&app::on_send_frame_callback, this, std::placeholders::_1));
		
		return new gologin::gui::frame(detail::_frame_title, _input_frame_callbacks, m_output_frame_handlers);
	}

	void app::output_handler(const ui_action& act, const ui_status& st, const wxString& msg, const wxString& data)
	{
		if (auto _h = m_output_frame_handlers.find(act); _h != m_output_frame_handlers.end())
		{
			return _h->second(st, msg, data);
		}
	}

	void app::on_login_frame_callback(const wxString& data)
	{
		std::string _credentials{data.c_str().AsChar()};
		
		std::string _login;
		std::string _pass;

		std::size_t _delim = _credentials.find(':');
		if(_delim != std::string::npos)
		{
			_login = _credentials.substr(0, _delim);
			_pass = _credentials.substr(++_delim , _credentials.size() - _delim);
		}

		//form : [user:pass]
		if (_login.empty() || _pass.empty())
		{
			output_handler(ui_action::on_login, ui_status::failed, _T("Invalid user or pass."), wxString{});
			return;
		}

		gologin::messages::client::auth::request _request;
		_request[_message_field_("command")] = gologin::messages::commands::auth;
		_request[_message_field_("id")] = m_cookie._client_id;
		_request[_message_field_("login")] = _login;
		_request[_message_field_("password")] = _pass;

		auto _buff = google::json::packer::to(_request);
		auto _ret = m_tcp_client->send(gologin::messages::commands::auth,_buff);

		_ret ? output_handler(ui_action::on_login, ui_status::success, _T("Login..."), wxString{}) :
			output_handler(ui_action::on_login, ui_status::failed, _T("Login failed."), wxString{});
	}

	void app::on_logout_frame_callback(const wxString& data)
	{
		gologin::messages::client::bye::request _request;
		_request[_message_field_("command")] = gologin::messages::commands::bye;
		_request[_message_field_("id")] = m_cookie._client_id;
		_request[_message_field_("session")] = m_cookie._client_session;

		auto _buff = google::json::packer::to(_request);
		auto _ret = m_tcp_client->send(gologin::messages::commands::bye, _buff);

		_ret ? output_handler(ui_action::on_logout, ui_status::success, _T("Logout..."), wxString{}) :
			output_handler(ui_action::on_logout, ui_status::failed, _T("Logout failed."), wxString{});
	}

	void app::on_connect_frame_callback(const wxString& data)
	{
		auto _saddrr = Poco::Net::SocketAddress(data.c_str().AsChar());
		auto _status = m_tcp_client->connect(_saddrr.host().toString(), _saddrr.port());
		if (_status != gologin::tcp::client_traits::status::connected)
		{
			output_handler(ui_action::on_connect, ui_status::failed, _T("Connect to server failed."), wxString{});
			return;
		}

		//generate client id
		{
			std::string _id;
			std::random_device _rd;
			std::mt19937 _mt(_rd());
			std::uniform_int_distribution<int> dist('a', 'z'); 
			std::generate_n(std::back_inserter(_id), 32, [&]{return dist(_mt);});

			m_cookie._client_id = std::atoll(_id.c_str());
		}


		gologin::messages::client::hello::request _request;
		_request[_message_field_("command")] = gologin::messages::commands::hello;
		_request[_message_field_("id")] = m_cookie._client_id;

		auto _buff = google::json::packer::to(_request);
		auto _ret = m_tcp_client->send(gologin::messages::commands::hello, _buff);

		_ret ? output_handler(ui_action::on_connect, ui_status::pending, _T("Connect..."), wxString{}) :
			output_handler(ui_action::on_connect, ui_status::failed, _T("Connect failed."), wxString{});
	}

	void app::on_disconnect_frame_callback(const wxString& /*data*/)
	{
		auto _status = m_tcp_client->disconnect();

		_status != gologin::tcp::client_traits::status::disconnected ? output_handler(ui_action::on_disconnect, ui_status::success, _T("Disconnected."), wxString{}) :
			output_handler(ui_action::on_disconnect, ui_status::failed, _T("Disconnection from server failed."), wxString{});
	}

	void app::on_send_frame_callback(const wxString& data)
	{
		gologin::messages::client::msg::request _request;
		_request[_message_field_("command")] = gologin::messages::commands::msg;
		_request[_message_field_("id")] = m_cookie._client_id;
		_request[_message_field_("session")] = m_cookie._client_session;
		_request[_message_field_("body")] = data.c_str().AsChar();

		auto _buff = google::json::packer::to(_request);
		auto _ret = m_tcp_client->send(gologin::messages::commands::msg, _buff);

		_ret ? output_handler(ui_action::on_send, ui_status::pending, _T("Mesaging..."), wxString{}) :
			output_handler(ui_action::on_send, ui_status::failed, _T("Mesaging failed."), wxString{});
	}

	void app::server_message_packet_handler(const gologin::messages::server::msg::request& request, gologin::messages::client::msg::response& responce)
	{
		responce[_message_field_("id")] = m_cookie._client_id;
		responce[_message_field_("command")] = gologin::messages::commands::msg;

		auto _sender_login = request[_message_field_("sender_login")];
		auto _sender_session = request[_message_field_("session")];
		auto _message = request[_message_field_("body")];

		if (_sender_login != m_cookie._client_login || _sender_session != m_cookie._client_session)
		{
			responce[_message_field_("status")] = "failed";
			responce[_message_field_("message")] = "wrong login or session";
		}
		else
		{
			responce[_message_field_("status")] = "ok";
			responce[_message_field_("message")] = "all good";

			output_handler(ui_action::on_recv, ui_status::success, _sender_login, _message);
		}
	}

	void app::server_message_reply_packet_handler(const gologin::messages::server::msg::response& responce)
	{
		auto _sender_id = responce[_message_field_("id")];
		auto _status = responce[_message_field_("status")];
		auto _client_id = responce[_message_field_("client_id")];
		auto _message = responce[_message_field_("message")];

		/*if (_client_id != m_cookie._client_id)
		{
			return 
		}*/

		_status == "ok" ? output_handler(ui_action::on_send, ui_status::success, _T("Message was sended."), wxString{}) :
			output_handler(ui_action::on_send, ui_status::failed, _message, wxString{});
	}

	void app::server_hello_reply_packet_handler(const gologin::messages::server::hello::response& responce)
	{
		auto _sender_id = responce[_message_field_("id")];
		auto _auth_method = responce[_message_field_("auth_method")];

		output_handler(ui_action::on_connect, ui_status::success, _T("Connection has been established."), wxString{});
	}

	void app::server_auth_reply_packet_handler(const gologin::messages::server::auth::response& responce)
	{
		auto _sender_id = responce[_message_field_("id")];
		auto _status = responce[_message_field_("status")];
		auto _client_id = responce[_message_field_("session")];
		auto _message = responce[_message_field_("message")];

		_status == "ok" ? output_handler(ui_action::on_login, ui_status::pending, _T("Login success."), wxString{}) :
			output_handler(ui_action::on_login, ui_status::failed, _message, wxString{});
	}

	void app::server_bye_reply_packet_handler(const gologin::messages::server::bye::response& responce)
	{
		auto _sender_id = responce[_message_field_("id")];
		auto _status = responce[_message_field_("status")];

		_status == "ok" ? output_handler(ui_action::on_logout, ui_status::pending, _T("Logout success."), wxString{}) :
			output_handler(ui_action::on_logout, ui_status::failed, _T("Logout failed."), wxString{});
	}

	void app::server_ping_reply_packet_handler(const gologin::messages::server::ping::response& responce)
	{
		auto _sender_id = responce[_message_field_("id")];
		auto _status = responce[_message_field_("status")];
		auto _message = responce[_message_field_("message")];

		_status == "ok" ? output_handler(ui_action::on_login, ui_status::success, _T("Ping success."), wxString{}) :
			output_handler(ui_action::on_login, ui_status::failed, _message, wxString{});
	}
}