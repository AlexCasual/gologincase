#include "core.hpp"
#include <core/dispatcher.hpp>

namespace gologin::tcp
{
	std::vector<char> create_packet(gologin::core::types::packet_type _type, const gologin::core::types::buffer_t& _buff)
	{
		std::vector<char> _packet;

		if (!_buff.empty())
		{
			if(_type.size() < sizeof(packet_header_t::type))
			{
				size_t _size = _buff.size() + sizeof(packet_header_t);

				_packet.resize(_size);

				auto _header = reinterpret_cast<packet_header_t*>(_packet.data());

				_header->size = _size;

				std::memcpy(_header->type, _type.data(), _type.size());
			}
		}

		return _packet;
	}

	uint32_t client_base::host() const
	{
		return _CROSS_PLATFORM_(m_address.sin_addr.S_un.S_addr, m_address.sin_addr.s_addr);
	}

	uint16_t client_base::port() const
	{
		return m_address.sin_port;
	}

	client_traits::status client_base::status() const
	{
		return m_status.load();
	}

	client_traits::session_id_t client_base::session_id() const
	{
		return m_id;
	}

	socket_traits::type client_base::type() const
	{
		return m_type;
	}

	bool client_base::send(gologin::core::types::packet_type _type, const gologin::core::types::buffer_t& _buff) const
	{
		if(!_buff.empty())
		{
			auto _packet = create_packet(_type, _buff);
			if (!_packet.empty())
			{
				auto _res = (::send(m_socket, _packet.data(), _packet.size(), 0) >= 0);
				return _res;
			}
		}

		return false;
	}
}