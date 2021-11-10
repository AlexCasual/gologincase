#pragma once

#include <core/types.hpp>

#ifdef _WIN32
#else
#define SD_BOTH 0
#endif

#ifdef _WIN32
#include <WinSock2.h>
#include <mstcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#endif

#include <cstdint>
#include <cstring>
#include <cinttypes>
#include <malloc.h>
#include <functional>
#include <list>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <atomic>

#ifdef _WIN32
typedef int socklen_t;
typedef SOCKADDR_IN sockaddr_in_t;
typedef SOCKET socket_t;
typedef u_long ka_prop_t;
#else
typedef struct sockaddr_in sockaddr_in_t;
typedef int socket_t;
typedef int ka_prop_t;
#endif

#ifdef _WIN32
#define _WINDOWS_PLATFORM_(win) win
#else
#define _WINDOWS_PLATFORM_(win)
#endif

#ifdef _WIN32
#define _CROSS_PLATFORM_(win, unix) win
#else
#define _CROSS_PLATFORM_(win, unix) unix
#endif

namespace gologin
{	
	namespace tcp
	{
		using packet_header_t = struct
		{
			uint64_t sign;
			uint32_t size;
			char type[16];
		};
		
		namespace server_traits
		{	
		  enum class status : uint8_t 
		  {
			up = 0,
			init_error,
			bind_error,
			keep_alive_error,
			listening_error,
			close
		  };
		}
		
		namespace client_traits
		{	
		  	enum class status : uint8_t 
			{
			  connected = 0,
			  init_error,
			  bind_error,
			  connect_error,
			  disconnected
			};

			using session_id_t = std::string;
		}
		
		namespace socket_traits
		{			
			enum class type : uint8_t 
			{
				client = 0,
				server = 1
			};
		}

		class client_base
		{
		public:
		  
		  client_base() = default;
		  virtual ~client_base() = default;
		  
		  virtual bool recv(gologin::core::types::buffer_t& _buff) = 0;
		  virtual bool send(gologin::core::types::packet_type _type, const gologin::core::types::buffer_t& _buff) const;
		  virtual uint32_t host() const;
		  virtual uint16_t port() const;
		  virtual client_traits::status status() const;
		  virtual client_traits::session_id_t session_id() const;
		  virtual socket_traits::type type() const;

		protected:
			sockaddr_in_t m_address;
			socket_t m_socket;
			client_traits::session_id_t m_id;
			socket_traits::type m_type;
			std::atomic<client_traits::status> m_status;
		};

		static std::vector<char> create_packet(gologin::core::types::packet_type _type, const gologin::core::types::buffer_t& _buff);
		
		inline const packet_header_t* header_packet(const gologin::core::types::buffer_t& _buff)
		{
			return reinterpret_cast<const packet_header_t*>(_buff.data());
		}

		inline int get_last_error()
		{
		#ifdef _WIN32	
			switch (WSAGetLastError()) 
			{
			case 0:
				return 0;
			case WSAEINTR:
				return EINTR;
			case WSAEINVAL:
				return EINVAL;
			case WSA_INVALID_HANDLE:
				return EBADF;
			case WSA_NOT_ENOUGH_MEMORY:
				return ENOMEM;
			case WSA_INVALID_PARAMETER:
				return EINVAL;
			case WSAENAMETOOLONG:
				return ENAMETOOLONG;
			case WSAENOTEMPTY:
				return ENOTEMPTY;
			case WSAEWOULDBLOCK:
				return EAGAIN;
			case WSAEINPROGRESS:
				return EINPROGRESS;
			case WSAEALREADY:
				return EALREADY;
			case WSAENOTSOCK:
				return ENOTSOCK;
			case WSAEDESTADDRREQ:
				return EDESTADDRREQ;
			case WSAEMSGSIZE:
				return EMSGSIZE;
			case WSAEPROTOTYPE:
				return EPROTOTYPE;
			case WSAENOPROTOOPT:
				return ENOPROTOOPT;
			case WSAEPROTONOSUPPORT:
				return EPROTONOSUPPORT;
			case WSAEOPNOTSUPP:
				return EOPNOTSUPP;
			case WSAEAFNOSUPPORT:
				return EAFNOSUPPORT;
			case WSAEADDRINUSE:
				return EADDRINUSE;
			case WSAEADDRNOTAVAIL:
				return EADDRNOTAVAIL;
			case WSAENETDOWN:
				return ENETDOWN;
			case WSAENETUNREACH:
				return ENETUNREACH;
			case WSAENETRESET:
				return ENETRESET;
			case WSAECONNABORTED:
				return ECONNABORTED;
			case WSAECONNRESET:
				return ECONNRESET;
			case WSAENOBUFS:
				return ENOBUFS;
			case WSAEISCONN:
				return EISCONN;
			case WSAENOTCONN:
				return ENOTCONN;
			case WSAETIMEDOUT:
				return ETIMEDOUT;
			case WSAECONNREFUSED:
				return ECONNREFUSED;
			case WSAELOOP:
				return ELOOP;
			case WSAEHOSTUNREACH:
				return EHOSTUNREACH;
			default:
				return EIO;
			}
		#else
			return 0;
		#endif
		}
	}
}