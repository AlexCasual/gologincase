#pragma once
#include <wx/wx.h>
#include <core/logger.hpp>
#include <tcp/client/client.hpp>
#include <messages/messages.hpp>
#include <client/frame.hpp>

namespace gologin
{
	namespace gui
	{
		class app : public wxApp
		{
			using app_config = struct
			{
				std::string _data_folder;
			};

			using app_cookie = struct
			{
				std::uint64_t _client_id;
				std::string _client_session;
				std::string _client_login;
			};
			
		public:
			bool OnInit() override;
			//int  OnRun() override;
			//int  OnExit() override;

		private:
			void init();
			int main();
			int exit();
			
			gologin::gui::frame* create_frame();

			void output_handler(const ui_action& act, const ui_status& st, const wxString& msg, const wxString& data); 

			void on_login_frame_callback(const wxString& data);
			void on_logout_frame_callback(const wxString& data);
			void on_connect_frame_callback(const wxString& data);
			void on_disconnect_frame_callback(const wxString& data);
			void on_send_frame_callback(const wxString& data);

			void server_message_packet_handler(const gologin::messages::server::msg::request& request, gologin::messages::client::msg::response& responce);
			void server_message_reply_packet_handler(const gologin::messages::server::msg::response& responce);
			void server_hello_reply_packet_handler(const gologin::messages::server::hello::response& responce);
			void server_auth_reply_packet_handler(const gologin::messages::server::auth::response& responce);
			void server_bye_reply_packet_handler(const gologin::messages::server::bye::response& responce);
			void server_ping_reply_packet_handler(const gologin::messages::server::ping::response& responce);

		private:
			app_config m_config;
			app_cookie m_cookie; 
			gologin::gui::frame* m_frame;
			ouput_handlers_t m_output_frame_handlers;
			std::shared_ptr<gologin::tcp::client> m_tcp_client;
			std::shared_ptr<gologin::core::logger> m_logger;
		};
	}
}