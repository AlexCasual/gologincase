#pragma once 
#include <core/core.hpp>
#include <auth/provider/provider.hpp>
#include <tcp/server/server.hpp>
#include <messages/messages.hpp>

namespace gologin
{
	class app final : public Poco::Util::ServerApplication
	{		
		using app_config = struct
		{
			std::string _srv_addrr;
			std::string _data_folder;
			std::string _client_pass;
			std::string _client_login;
		};	
		
	public:
		explicit app();
		~app();

		app(const app&) = delete;
		app& operator = (const app&) = delete;

	private:
		void startup();
		void main();
		void exit();
		int  main(const std::vector<std::string>& args) override final;
		void initialize(Poco::Util::Application& self) override final;
		void uninitialize() override final;
		void defineOptions(Poco::Util::OptionSet& options) override final;
		void handleOption(const std::string& name, const std::string& value) override final;
				
	private:
		void client_hello_packet_handler(const core::types::client_cookies_t& _cookies, const gologin::messages::client::hello::request& request, gologin::messages::server::hello::response& responce);	
		void client_ping_packet_handler(const core::types::client_cookies_t& _cookies, const gologin::messages::client::ping::request& request, gologin::messages::server::auth::response& responce);
		void client_login_packet_handler(const core::types::client_cookies_t& _cookies, const gologin::messages::client::auth::request& request, gologin::messages::server::auth::response& responce);
		void client_logout_packet_handler(const core::types::client_cookies_t& _cookies, const gologin::messages::client::bye::request& request, gologin::messages::server::bye::response& responce);
		void client_message_packet_handler(const core::types::client_cookies_t& _cookies, const gologin::messages::client::msg::request& request, gologin::messages::server::msg::response& responce);	
		void client_message_reply_packet_handler(const core::types::client_cookies_t& _cookies, const gologin::messages::client::msg::response& responce);

	private:
		app_config m_config;
		std::shared_ptr<gologin::core::logger> m_logger;
		std::shared_ptr<gologin::tcp::server> m_tcp_server;
		std::shared_ptr<gologin::auth::provider> m_auth_provider;
		std::jthread m_main_thread;
		std::mutex m_lock;
	};
}