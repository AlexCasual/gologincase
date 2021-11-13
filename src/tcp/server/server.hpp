#pragma once
#include <core/core.hpp>
#include <tcp/core/core.hpp>

namespace gologin
{
	namespace tcp
	{
		struct keep_alive_config
		{
		  ka_prop_t ka_idle = 120;
		  ka_prop_t ka_interval = 3;
		  ka_prop_t ka_cnt = 5;
		};

		//forward decl
		class server_client;

		using client_handler_t = std::function<void()>;
		using packet_handler_t = std::function<void(const core::types::packet_header_t&, const core::types::buffer_t&, core::types::buffer_t&)>;

		class server 
		{		  
		public:
		  server(uint16_t port, std::shared_ptr<gologin::core::dispatcher> disp, std::shared_ptr<gologin::core::logger> logger, keep_alive_config ka_conf = {});
		  ~server();

		  bool send_to(const core::types::client_cookies_t& _cookies, core::types::cmd_t _cmd, const core::types::buffer_t& _buff);
		  void handle(std::string_view _cmd, const packet_handler_t& _handler);

		  server_traits::status status() const;
		  server_traits::status start();
		  void stop();
		  
		private:
		  void handler_thread_routine();
		  bool set_keep_alive(socket_t socket, bool alive);
		  void client_handler(sockaddr_in_t _client_address, socket_t _client_socket);
		  void packet_handler(std::size_t _cid, const core::types::packet_header_t& _header, const core::types::buffer_t& _buff, server_client& _client);
			  
		private:
		  std::shared_ptr<gologin::core::dispatcher> m_disp;
		  std::shared_ptr<gologin::core::logger> m_logger;
		  std::atomic<server_traits::status> m_status;
		  socket_t m_server_socket;
		  uint16_t m_server_port;
		  keep_alive_config m_ka_conf;
		  std::thread m_handler_thread;
		  std::shared_mutex m_packet_handlers_mutex;
		  std::map<std::size_t, packet_handler_t> m_packet_handlers;
		  std::shared_mutex m_client_list_mutex;
		  std::map<std::size_t, std::shared_ptr<server_client>> m_client_list;
		  std::shared_mutex m_client_handler_mutex;
		  std::list<std::thread> m_client_handler_threads;
		#ifdef _WIN32
		  WSAData m_wsadata;
		#endif		  
		};

		class server_client : public client_base
		{
		  friend struct server;

		public:
		  server_client(socket_t socket, sockaddr_in_t address);
		  ~server_client() override;

		  bool recv(core::types::buffer_t& _buff) override;
		  bool recv(core::types::packet_header_t& _header, core::types::buffer_t& _buff);
		  
		private:
		  void disconnect();
		  
		private:
		  std::mutex m_access_mutex;
		  std::mutex m_move_mutex;
		};
	}
}