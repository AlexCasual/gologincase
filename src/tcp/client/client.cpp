#include "client.hpp"

namespace gologin::tcp
{
	client::client(std::shared_ptr<gologin::core::dispatcher> disp, std::shared_ptr<gologin::core::logger> logger) noexcept
		:m_disp(disp), m_logger(logger)
	{
		m_type = socket_traits::type::client;
		m_status = client_traits::status::disconnected;

		m_handler_thread = std::thread([this]()
		{
			while(m_status == client_traits::status::connected)
			{
				gologin::core::types::buffer_t _in;
				gologin::core::types::buffer_t _out;

				packet_header_t _header;

				if (recv(_header, _in))
				{
					m_disp->dispatch(_header.type, _in, _out);
				}
			}
		});
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
	  _WINDOWS_PLATFORM_(if(::WSAStartup(MAKEWORD(2, 2), &m_wsadata) != 0) {})

	  if((m_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) _CROSS_PLATFORM_(== INVALID_SOCKET, < 0))
		  return m_status = client_traits::status::init_error;

	  new(&m_socket) sockaddr_in_t;
	  m_address.sin_family = AF_INET;
	  m_address.sin_addr.s_addr = ::inet_addr(host.c_str());
	  m_address.sin_port = ::htons(port);
	  _CROSS_PLATFORM_(m_address.sin_addr.S_un.S_addr = ::inet_addr(host.c_str()) , m_address.sin_addr.s_addr = ::inet_addr(host.c_str()));

	  if(::connect(m_socket, (sockaddr *)&m_address, sizeof(m_address)) _CROSS_PLATFORM_(== SOCKET_ERROR,!= 0)) 
	  {
		_CROSS_PLATFORM_(::closesocket(m_socket) , ::close(m_socket));
		
		return m_status = client_traits::status::connect_error;
	  }
	  
	  return m_status = client_traits::status::connected;
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
	
	bool client::recv(gologin::core::types::buffer_t& _buff)
	{
		packet_header_t _header;

		auto _ret = recv(_header, _buff);

		return _ret;
	}

	bool client::recv(gologin::tcp::packet_header_t& _header, gologin::core::types::buffer_t& _buff)
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