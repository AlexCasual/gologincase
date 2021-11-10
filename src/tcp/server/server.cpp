#include "server.hpp"

namespace gologin::tcp
{
	//client
	server_client::server_client(socket_t socket, sockaddr_in_t address)
	{
		m_address = address;
		m_socket = socket;
		m_type = socket_traits::type::client;
		m_status = client_traits::status::connected;
	}

	server_client::~server_client()
	{
	  if(m_socket == _CROSS_PLATFORM_(INVALID_SOCKET, -1)) 
		  return;
	  
	  ::shutdown(m_socket, SD_BOTH);
	  
	  _CROSS_PLATFORM_(::closesocket(m_socket), ::close(m_socket));
	}
	
	void server_client::disconnect() 
	{
	  m_status = client_traits::status::disconnected;
	  
	  if(m_socket == _CROSS_PLATFORM_(INVALID_SOCKET, -1))
		  return;
	  
	 ::shutdown(m_socket, SD_BOTH);
	 
	  _CROSS_PLATFORM_(::closesocket, ::close)(m_socket);
	  
	  m_socket = _CROSS_PLATFORM_(INVALID_SOCKET, -1);
	}
	
	bool server_client::recv(gologin::core::types::buffer_t& _buff)
	{
		// Read data length in non-blocking mode
		_WINDOWS_PLATFORM_(if(u_long t = true; SOCKET_ERROR == ::ioctlsocket(m_socket, FIONBIO, &t)) return false;)

		packet_header_t _header;
		auto _recv = ::recv(m_socket, reinterpret_cast<char*>(&_header), sizeof(_header), _CROSS_PLATFORM_(0, MSG_DONTWAIT));
		if(!_recv) 
		{
			disconnect();
			return false;
		}
		else if(_recv == -1)
		{
			int _error = 0;

			_CROSS_PLATFORM_(_error = get_last_error();
			  if(!_error) 
			  {
				socklen_t len = sizeof (_error);
				::getsockopt (m_socket, SOL_SOCKET, SO_ERROR, _WINDOWS_PLATFORM_((char*))&_error, &len);
			  },

			  socklen_t len = sizeof (_error);
			  ::getsockopt (m_socket, SOL_SOCKET, SO_ERROR, _WINDOWS_PLATFORM_((char*))&_error, &len);
			  if(!_error) _error = _errorno;
			)

			_WINDOWS_PLATFORM_(if(u_long t = false; SOCKET_ERROR == ::ioctlsocket(m_socket, FIONBIO, &t)) return false;) 
			
			switch (_error)
			{
			  case 0: 
				break;
			  case EAGAIN:
				return false;
			  case ETIMEDOUT:
			  case ECONNRESET:
			  case EPIPE:
				disconnect();
			  default:
				disconnect();
			  return false;
			}
		}
		
		if(!_header.size)
		  return false;

		_buff.resize(_header.size);
				
		auto _res = ::recv(m_socket, reinterpret_cast<char*>(_buff.data()), _buff.size(), 0);
		
		return (_res >= 0);
	}

	server::server(uint16_t port, std::shared_ptr<gologin::core::dispatcher> disp, std::shared_ptr<gologin::core::logger> logger, keep_alive_config ka_conf)
	: m_server_port(port), m_disp(disp), m_logger(logger), m_ka_conf(ka_conf), m_status(server_traits::status::close)
	{}

	server::~server() 
	{
	  if(m_status == server_traits::status::up)
	  {
		stop();
	  }

	  _WINDOWS_PLATFORM_(::WSACleanup());
	}
	
	server_traits::status server::status() const
	{
		return m_status.load();
	}
	
	void server::send_to_all(gologin::core::types::packet_type _type, const gologin::core::types::buffer_t& _buff)
	{
		std::shared_lock<std::shared_mutex> _lock(m_client_list_mutex);
		for(auto& _c : m_client_list)
		{
			_c->send(_type, _buff);
		}
	}
	
	server_traits::status server::start()
	{
		int _flag = 1;
		
		if(m_status == server_traits::status::up) 
		{
			stop();
		}

		_WINDOWS_PLATFORM_(if(::WSAStartup(MAKEWORD(2, 2), &m_wsadata) == 0) {})
			
		sockaddr_in_t _address;
		_address.sin_port = ::htons(m_server_port);
		_address.sin_family = AF_INET;
		_address.sin_addr _CROSS_PLATFORM_(.S_un.S_addr, .s_addr) = INADDR_ANY;

		if((m_server_socket = ::socket(AF_INET, SOCK_STREAM, 0)) _CROSS_PLATFORM_(== INVALID_SOCKET, == -1))
		{
			return m_status = server_traits::status::init_error;
		}

		if((::setsockopt(m_server_socket, SOL_SOCKET, SO_REUSEADDR, _WINDOWS_PLATFORM_((char*))&_flag, sizeof(_flag)) == -1) ||
			(::bind(m_server_socket, (struct sockaddr*)&_address, sizeof(_address)) _CROSS_PLATFORM_(== SOCKET_ERROR, < 0)))
		{
			return m_status = server_traits::status::bind_error;
		}

		if(::listen(m_server_socket, SOMAXCONN) _CROSS_PLATFORM_(== SOCKET_ERROR, < 0))
		{
			return m_status = server_traits::status::listening_error;
		}
		
		m_status = server_traits::status::up;
		
		m_handler_thread = std::thread([this]{handler_thread_routine();});
		
		return m_status.load();
	}

	void server::stop() 
	{
	  m_status = server_traits::status::close;
	  
	  _CROSS_PLATFORM_(::closesocket, ::close)(m_server_socket);

	  m_handler_thread.join();
	  
	  {
		  std::lock_guard<std::mutex> _lock(m_client_handler_mutex);

		  for(auto& t : m_client_handler_threads)
		  {
			t.join();
		  }
	  }
	  
	  m_client_handler_threads.clear();
	  
	  m_client_list.clear();
	}

	void server::handler_thread_routine()
	{
	  socklen_t _addrlen = sizeof(sockaddr_in_t);
	  
	  while (m_status == server_traits::status::up)
	  {
		sockaddr_in_t _client_address;
		if (socket_t _client_socket = ::accept(m_server_socket, (struct sockaddr*)&_client_address, &_addrlen);
			_client_socket _CROSS_PLATFORM_(!= 0 , >= 0) && m_status == server_traits::status::up)
		{
		  if(_client_socket == _CROSS_PLATFORM_(INVALID_SOCKET, -1))
			  continue;

		  if(!set_keep_alive(_client_socket, true))
		  {
			::shutdown(_client_socket, 0);
			_CROSS_PLATFORM_(::closesocket, ::close)(_client_socket);
		  }

		  std::unique_ptr<server_client> _client(std::make_unique<server_client>(_client_socket, _client_address));
		  		  
		  {
			std::lock_guard<std::shared_mutex> _lock(m_client_list_mutex);

			m_client_list.emplace_back(std::move(_client));
		  }
		  
		  if(m_client_handler_threads.empty())
		  {
			  m_client_handler_threads.emplace_back(std::thread([this]{client_handler(m_client_list.begin());}));
		  }
		}
	  }
	}

	bool server::set_keep_alive(socket_t _socket, bool _alive) 
	{
	  int _flag = _alive ? 1 : 0;
	  
	#ifdef _WIN32
	  if (::setsockopt (_socket, SOL_SOCKET, SO_KEEPALIVE, (const char *) &_flag, sizeof(_flag)) != 0)
		  return false;	  
	  unsigned long _returned = 0;
	  tcp_keepalive _ka {1, m_ka_conf.ka_idle * 1000, m_ka_conf.ka_interval * 1000};
	  if(::WSAIoctl(_socket, SIO_KEEPALIVE_VALS, &_ka, sizeof (_ka), nullptr, 0, &_returned, 0, nullptr) != 0)
		  return false;	  
	#else
	  if(::setsockopt(_socket, SOL_SOCKET, SO_KEEPALIVE, &_flag, sizeof(_flag)) == -1) 
		  return false;
	  if(::setsockopt(_socket, IPPROTO_TCP, TCP_KEEPIDLE, &m_ka_conf.ka_idle, sizeof(m_ka_conf.ka_idle)) == -1) 
		  return false;
	  if(::setsockopt(_socket, IPPROTO_TCP, TCP_KEEPINTVL, &m_ka_conf.ka_interval, sizeof(m_ka_conf.ka_interval)) == -1) 
		  return false;
	  if(::setsockopt(_socket, IPPROTO_TCP, TCP_KEEPCNT, &m_ka_conf.ka_cnt, sizeof(m_ka_conf.ka_cnt)) == -1) 
		  return false;
	#endif
	
	  return true;
	}
	
	void server::packet_handler(const gologin::core::types::buffer_t& _buff, server_client& _client)
	{
		auto _header = header_packet(_buff);

		gologin::core::types::buffer_t _out;
		
		m_disp->dispatch(_header->type, _buff, _out);
		
		_client.send(_header->type, _out);
	}
	
	void server::client_handler(std::list<std::unique_ptr<server_client>>::iterator _cur) 
	{
	  const std::thread::id _thread_id = std::this_thread::get_id();

	  auto _next_client = [this, &_cur]()
	  {
		if(!*_cur)
			return false;
		
		std::mutex& _next_mtx = (*_cur)->m_move_mutex;				
		std::lock_guard<std::mutex> _lock(_next_mtx);

		if(++_cur == m_client_list.end()) 
		{
		  _cur = m_client_list.begin();
		  if(_cur == m_client_list.end()) 
		  { 
			return false;
		  }
		}

		return true;
	  };


	  auto _rem_thread = [&_thread_id, this]()
	  {
		std::lock_guard<std::mutex> _lock(m_client_handler_mutex);
		
		for(auto _it = m_client_handler_threads.begin(); _it != m_client_handler_threads.end(); ++_it)
		{
			if(_it->get_id() == _thread_id)
			{
				_it->detach();				
				m_client_handler_threads.erase(_it);								
				return;
			}
		}
	  };

	  auto _add_thread = [this, &_cur]() mutable
	  {				
		m_client_handler_threads.emplace_back([this, _cur]() mutable 
		{
			std::shared_lock<std::shared_mutex> _lock(m_client_list_mutex);
		  
			auto it = (++_cur == m_client_list.end())? m_client_list.begin() : _cur;

			client_handler(it);
		});
	  };


	  while(m_status == server_traits::status::up) 
	  {
		if(!*_cur)
		{ 
		  if(_next_client()) 
			  continue;
		  else 
		  { 
			_rem_thread(); 
			return; 
		  }
		} 
		else if(!(*_cur)->m_access_mutex.try_lock())
		{ 
		  if(_next_client())
			  continue;
		  else 
		  {
			_rem_thread(); 
			return; 
		  }
		}

		gologin::core::types::buffer_t _data;

		if((*_cur)->recv(_data))
		{
			if(!_data.empty())
			{
			  _add_thread();

			  packet_handler(_data, **_cur);

			  {
				std::lock_guard<std::shared_mutex> _lock(m_client_list_mutex);
		  
				m_client_list.splice(m_client_list.cend(), m_client_list, _cur);
		  
				(*_cur)->m_access_mutex.unlock();	  
			  }

			  _rem_thread();
		  
			  return;

			} 
			else
			{
			  if((*_cur)->m_status != client_traits::status::connected)
			  {			
				bool _move = false;
				{
					std::lock_guard<std::shared_mutex> _lock(m_client_list_mutex);

					// If m_client_list.length() == 1;
					if(m_client_list.begin() == --m_client_list.end()) 
					{
					  (*_cur)->m_access_mutex.unlock();
				  
					  m_client_list.erase(_cur);
				  
					  _rem_thread();
				  
					  return;
					}

					auto _prev = _cur;
					auto _last_cur = _cur;
				
					if(_prev == m_client_list.begin()) 
						_prev = --m_client_list.end();
					else 
						--_prev;

					_move = _next_client();
				
					(*_prev)->m_move_mutex.lock();
				
					(*_last_cur)->m_access_mutex.unlock();
					m_client_list.erase(_last_cur);
					(*_prev)->m_move_mutex.unlock();
				}
			
				if(_move)
					continue;
				else
				{
					_rem_thread(); 
					return;
				}
			
				return;
			  }

			  // Move to next client
			  (*_cur)->m_access_mutex.unlock();
		  
			  if(_next_client())
				  continue;
			  else 
			  {
				_rem_thread();
				return;
			  }
			}
		}
	  }
	}
}