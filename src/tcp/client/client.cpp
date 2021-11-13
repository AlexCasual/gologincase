#include "client.hpp"

namespace gologin::tcp
{
	client::client(std::shared_ptr<gologin::core::dispatcher> disp, std::shared_ptr<gologin::core::logger> logger) noexcept
		:m_disp(disp), m_logger(logger)
	{
		m_type = socket_traits::type::client;
		m_status = client_traits::status::disconnected;
	}

	client::~client()
	{
		disconnect();

		if (m_handler_thread.joinable())
		{
			 m_handler_thread.join();
		}

	  _WINDOWS_PLATFORM_(::WSACleanup();)
	}

	client_traits::status client::connect(const std::string& host, uint16_t port) noexcept 
	{
		if (m_status == client_traits::status::connected)
		{
			return m_status;
		}

	  _WINDOWS_PLATFORM_(if(::WSAStartup(MAKEWORD(2, 2), &m_wsadata) != 0) {})

	  if((m_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) _CROSS_PLATFORM_(== INVALID_SOCKET, < 0))
		  return m_status = client_traits::status::init_error;

	  //new(&m_socket) sockaddr_in_t;

	  int sd, err = 0;
	  struct addrinfo _addrinfo = {}, *_paddrinfo;    
	  _addrinfo.ai_family = AF_INET;
	  _addrinfo.ai_socktype = SOCK_STREAM;
	  _addrinfo.ai_protocol = IPPROTO_TCP;

	  auto _port = std::to_string(port); 
	  auto _err = ::getaddrinfo(host.c_str(), _port.c_str(), &_addrinfo, &_paddrinfo);
	  if (_err == 0)
	  {
		  m_address.sin_port = port;

		  if(::connect(m_socket,  _paddrinfo->ai_addr ,  _paddrinfo->ai_addrlen) _CROSS_PLATFORM_(== SOCKET_ERROR,!= 0)) 
		  {
			  auto _le = get_last_error();

			_CROSS_PLATFORM_(::closesocket(m_socket) , ::close(m_socket));
		
			return m_status = client_traits::status::connect_error;
		  }
	  }

	  m_status = client_traits::status::connected;

	  m_handler_thread = std::thread([this]()
		  {
			  std::size_t _client_id = static_cast<std::size_t>(m_socket);

				while(m_status == client_traits::status::connected)
				{
					core::types::buffer_t _in;
					core::types::buffer_t _out;

					core::types::packet_header_t _header;

					if (recv(_header, _in))
					{
						std::string _cmd{_header.type};
						core::types::client_cookies_t _cookie{_client_id};
						m_disp->dispatch(_cmd,_cookie, _in, _out);
					}
				}
		  });
	  
	  return m_status;
	}

	client_traits::status client::disconnect() noexcept 
	{
		if(m_status != client_traits::status::connected)
			return m_status;
		
	  ::shutdown(m_socket, SD_BOTH);
	  
	  _CROSS_PLATFORM_(::closesocket(m_socket), ::close(m_socket));
	  
	  m_status = client_traits::status::disconnected;
	  
	  m_handler_thread.join();
	  
	  return m_status;
	}
	
	bool client::recv(core::types::buffer_t& _buff)
	{
		core::types::packet_header_t _header;

		auto _ret = recv(_header, _buff);

		return _ret;
	}

	bool client::recv(core::types::packet_header_t& _header, core::types::buffer_t& _buff)
	{	
		auto _res = ::recv(m_socket, reinterpret_cast<char*>(&_header), sizeof(_header), 0);
		if (_res)
		{
			if(_header.size) 
			{
				_buff.resize(_header.size);

				auto _res = ::recv(m_socket, reinterpret_cast<char*>(_buff.data()), _buff.size(), 0);

				return (_res >= 0);
			}
		}
	  
		return false;
	}
}