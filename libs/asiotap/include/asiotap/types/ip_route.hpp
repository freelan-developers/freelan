/*
 * libasiotap - A portable TAP adapter extension for Boost::ASIO.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libasiotap.
 *
 * libasiotap is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libasiotap is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 *
 * If you intend to use libasiotap in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file ip_route.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An IP route class.
 */

#ifndef ASIOTAP_IP_ROUTE_HPP
#define ASIOTAP_IP_ROUTE_HPP

#include "ip_network_address.hpp"

#include <boost/optional.hpp>

namespace asiotap
{
	/**
	 * \brief A generic IP network address template class.
	 */
	template <typename AddressType>
	class base_ip_route
	{
		public:

			/**
			 * \brief Get a null IP route.
			 * \return A null IP route.
			 */
			static base_ip_route null()
			{
				return base_ip_route();
			}

			/**
			 * \brief The address type.
			 */
			typedef AddressType address_type;

			/**
			 * \brief The network address type.
			 */
			typedef base_ip_network_address<address_type> ip_network_address_type;

			/**
			 * \brief Create an IP route.
			 */
			base_ip_route() : m_network_address(), m_gateway() {};

			/**
			 * \brief Create an IP route.
			 * \param _network_address The network address.
			 */
			base_ip_route(const ip_network_address_type& _network_address) : m_network_address(_network_address), m_gateway() {};

			/**
			 * \brief Create an IP network address.
			 * \param _network_address The network address.
			 * \param _gateway The gateway.
			 */
			base_ip_route(const ip_network_address_type& _network_address, boost::optional<address_type> _gateway) : m_network_address(_network_address), m_gateway(_gateway) {};

			/**
			 * \brief Check if the instance is null.
			 * \return true if the instance is null.
			 */
			bool is_null() const
			{
				return (*this == null());
			}

			/**
			 * \brief Get the network address.
			 * \return The network address.
			 */
			const ip_network_address_type& network_address() const
			{
				return m_network_address;
			}

			/**
			 * \brief Get the gateway.
			 * \return The gateway.
			 */
			const boost::optional<address_type>& gateway() const
			{
				return m_gateway;
			}

			/**
			 * \brief Check if the specified address belongs to the route.
			 * \param addr The address to check.
			 * \return true if addr belongs to the route, false otherwise.
			 */
			bool has_address(const address_type& addr) const
			{
				return m_network_address.has_address(addr);
			}

			/**
			 * \brief Check if the specified network address is a subnet of the current route.
			 * \param addr The network address to check.
			 * \return true if addr belongs to the route, false otherwise.
			 */
			template <typename AddressType2>
			bool has_network(const base_ip_network_address<AddressType2>& addr) const
			{
				return m_network_address.has_network(addr);
			}

			/**
			 * \brief Check if the specified network address represents an unicast address.
			 * \return true if the specified network address represents an unicast address.
			 */
			bool is_unicast() const
			{
				return m_network_address.is_unicast();
			}

		private:

			ip_network_address_type m_network_address;
			boost::optional<address_type> m_gateway;

		public:

			friend bool operator==(const base_ip_route& lhs, const base_ip_route& rhs)
			{
				return (lhs.network_address() == rhs.network_address()) && (lhs.gateway() == rhs.gateway());
			}

			friend bool operator!=(const base_ip_route& lhs, const base_ip_route& rhs)
			{
				return !(lhs == rhs);
			}

			friend bool operator<(const base_ip_route& lhs, const base_ip_route& rhs)
			{
				if (lhs.network_address() == rhs.network_address())
				{
					return (lhs.gateway() < rhs.gateway());
				}
				else
				{
					return (lhs.network_address() < rhs.network_address());
				}
			}
	};

	/**
	 * \brief Write an IP route to an output stream.
	 * \tparam AddressType The address type.
	 * \param os The output stream.
	 * \param value The value.
	 * \return os.
	 */
	template <typename AddressType>
	std::ostream& operator<<(std::ostream& os, const base_ip_route<AddressType>& value);

	/**
	 * \brief Read an IP route from an input stream.
	 * \tparam AddressType The address type.
	 * \param is The input stream.
	 * \param value The value.
	 * \return is.
	 */
	template <typename AddressType>
	std::istream& operator>>(std::istream& is, base_ip_route<AddressType>& value);

	/**
	 * \brief The IPv4 instantiation.
	 */
	typedef base_ip_route<boost::asio::ip::address_v4> ipv4_route;

	/**
	 * \brief The IPv6 instantiation.
	 */
	typedef base_ip_route<boost::asio::ip::address_v6> ipv6_route;

	/**
	 * \brief The generic IP type.
	 */
	typedef boost::variant<ipv4_route, ipv6_route> ip_route;

	/**
	 * \brief An IPv4 route list type.
	 */
	typedef std::vector<ipv4_route> ipv4_route_list;

	/**
	 * \brief An IPv6 route list type.
	 */
	typedef std::vector<ipv6_route> ipv6_route_list;

	/**
	 * \brief A generic IP route list type.
	 */
	typedef std::vector<ip_route> ip_route_list;

	/**
	 * \brief Convert an IP address into an IP route.
	 * \param addr The address.
	 * \return The IP route.
	 */
	inline ip_route to_ip_route(const boost::asio::ip::address& addr)
	{
		if (addr.is_v4())
		{
			return ip_network_address(addr.to_v4());
		}
		else
		{
			return ip_network_address(addr.to_v6());
		}
	}

	/**
	 * \brief Convert an IP address into an IP route.
	 * \param addr The address.
	 * \param prefix_len The prefix length.
	 * \return The IP route.
	 */
	inline ip_route to_ip_route(const boost::asio::ip::address& addr, unsigned int prefix_len)
	{
		if (addr.is_v4())
		{
			return ipv4_network_address(addr.to_v4(), prefix_len);
		}
		else
		{
			return ipv6_network_address(addr.to_v6(), prefix_len);
		}
	}

	/**
	 * \brief Convert an IP address into an IP route.
	 * \param addr The address.
	 * \param prefix_len The prefix length.
	 * \param gateway The gateway.
	 * \return The IP route.
	 */
	inline ip_route to_ip_route(const boost::asio::ip::address& addr, unsigned int prefix_len, const boost::asio::ip::address& gateway)
	{
		// Make sure the address and the gateway belong to the same family.
		assert(addr.is_v4() == gateway.is_v4());

		if (addr.is_v4())
		{
			return ipv4_route(ipv4_network_address(addr.to_v4(), prefix_len), gateway.to_v4());
		}
		else
		{
			return ipv6_route(ipv6_network_address(addr.to_v6(), prefix_len), gateway.to_v6());
		}
	}

	/**
	 * \brief Convert an IP address into an IP route.
	 * \param addr The address.
	 * \param prefix_len The prefix length.
	 * \param gateway The gateway.
	 * \return The IP route.
	 */
	inline ip_route to_ip_route(const ip_network_address& ina, const boost::optional<boost::asio::ip::address>& gateway)
	{
		const auto addr = get_network_address(ina);
		const auto prefix_len = to_prefix_length(ina);

		if (gateway)
		{
			return to_ip_route(addr, prefix_len, *gateway);
		}
		else
		{
			return to_ip_route(addr, prefix_len);
		}
	}

	/**
	 * \brief A visitor that writes ip_route to output streams.
	 */
	class ip_route_output_visitor : public boost::static_visitor<std::ostream&>
	{
		public:

			/**
			 * \brief Create a new ip_network_address_output_visitor.
			 * \param os The output stream.
			 */
			ip_route_output_visitor(result_type os) : m_os(os) {}

			/**
			 * \brief Write the specified ip_route.
			 * \tparam T The type of the ip_route.
			 * \param ir The ip_route.
			 * \return os.
			 */
			template <typename T>
			result_type operator()(const T& ir) const
			{
				return m_os << ir;
			}

		private:

			result_type m_os;
	};

	/**
	 * \brief Write an ip_route to an output stream.
	 * \param os The output stream.
	 * \param value The value.
	 * \return os.
	 */
	inline std::ostream& operator<<(std::ostream& os, const ip_route& value)
	{
		return boost::apply_visitor(ip_route_output_visitor(os), value);
	}

	/**
	 * \brief Read an ip_route from an input stream.
	 * \param is The input stream.
	 * \param value The value.
	 * \return is.
	 */
	std::istream& operator>>(std::istream& is, ip_route& value);

	/**
	 * \brief A visitor that checks if the ip_route contains an address.
	 */
	class ip_route_has_address_visitor : public boost::static_visitor<bool>
	{
		public:

			/**
			 * \brief Create a new ip_route_has_address_visitor.
			 * \param addr The address.
			 */
			ip_route_has_address_visitor(const boost::asio::ip::address& addr) : m_addr(addr) {}

			/**
			 * \brief Check if the ip_route contains an address.
			 * \param ir The ipv4_route.
			 * \return os.
			 */
			result_type operator()(const ipv4_route& ir) const
			{
				return (m_addr.is_v4() && ir.has_address(m_addr.to_v4()));
			}

			/**
			 * \brief Check if the ip_route contains an address.
			 * \param ir The ipv6_route.
			 * \return os.
			 */
			result_type operator()(const ipv6_route& ir) const
			{
				return (m_addr.is_v6() && ir.has_address(m_addr.to_v6()));
			}

		private:

			boost::asio::ip::address m_addr;
	};

	/**
	 * \brief Check if an ip_route contains an address.
	 * \param ir The base_ip_route.
	 * \param addr The address.
	 * \return true if addr is contained in ir.
	 */
	template <typename AddressType>
	inline bool has_address(const base_ip_route<AddressType>& ir, const AddressType& addr)
	{
		return ir.has_address(addr);
	}

	/**
	 * \brief Check if an ip_route contains an address.
	 * \param ir The ip_route.
	 * \param addr The address.
	 * \return true if addr is contained in ir.
	 */
	template <typename AddressType>
	inline bool has_address(const ip_route& ir, const AddressType& addr)
	{
		return boost::apply_visitor(ip_route_has_address_visitor(addr), ir);
	}

	/**
	 * \brief A visitor that checks if the ip_route contains a network.
	 */
	class ip_route_has_network_visitor : public boost::static_visitor<bool>
	{
		private:
			template <typename AddressType>
			class ip_route_belongs_to_network_visitor : public boost::static_visitor<bool>
			{
				public:

					ip_route_belongs_to_network_visitor(const base_ip_route<AddressType>& ir) : m_ir(ir) {}

					template <typename Any>
					result_type operator()(const Any&) const
					{
						return false;
					}

					result_type operator()(const base_ip_network_address<AddressType>& addr) const
					{
						return m_ir.has_network(addr);
					}

				private:

					base_ip_route<AddressType> m_ir;
			};

		public:

			/**
			 * \brief Create a new ip_route_has_network_visitor.
			 * \param addr The address.
			 */
			ip_route_has_network_visitor(const ip_network_address& addr) : m_addr(addr) {}

			/**
			 * \brief Default implementation.
			 * \return true if m_addr belongs to ir.
			 */
			template <typename AddressType>
			result_type operator()(const base_ip_route<AddressType>& ir) const
			{
				return boost::apply_visitor(ip_route_belongs_to_network_visitor<AddressType>(ir), m_addr);
			}

		private:

			ip_network_address m_addr;
	};

	/**
	 * \brief Check if an ip_route contains a network.
	 * \param ir The ip_route.
	 * \param addr The network address.
	 * \return true if addr is contained in ir.
	 */
	template <typename AddressType>
	inline bool has_network(const base_ip_route<AddressType>& ir, const base_ip_network_address<AddressType>& addr)
	{
		return ir.has_network(addr);
	}

	/**
	 * \brief Check if an ip_network_address contains a network.
	 * \param ir The ip_network_address.
	 * \param addr The network address.
	 * \return true if addr is contained in ir.
	 */
	inline bool has_network(const ip_route& ir, const ip_network_address& addr)
	{
		return boost::apply_visitor(ip_route_has_network_visitor(addr), ir);
	}

	/**
	 * \brief A visitor that checks if the ip_route is an unicast address.
	 */
	class ip_route_is_unicast_visitor : public boost::static_visitor<bool>
	{
		public:

			/**
			 * \brief Default implementation.
			 * \return false.
			 */
			template <typename AddressType>
			result_type operator()(const base_ip_route<AddressType>& ir) const
			{
				return ir.is_unicast();
			}
	};

	/**
	 * \brief Check if an ip_route is an unicast address.
	 * \param ir The ip_route.
	 * \return true if ir is an unicast address.
	 */
	template <typename AddressType>
	inline bool is_unicast(const base_ip_route<AddressType>& ir)
	{
		return ir.is_unicast();
	}

	/**
	 * \brief Check if an ip_route is an unicast address.
	 * \param ir The ip_route.
	 * \return true if ir is an unicast address.
	 */
	inline bool is_unicast(const ip_route& ir)
	{
		return boost::apply_visitor(ip_route_is_unicast_visitor(), ir);
	}

	/**
	 * \brief A visitor that gets the network address.
	 */
	class ip_route_network_address_visitor : public boost::static_visitor<ip_network_address>
	{
		public:

			/**
			 * \brief Get the network address of the specified address.
			 * \param ir The base_ip_route instance.
			 * \return The network address.
			 */
			template <typename AddressType>
			result_type operator()(const base_ip_route<AddressType>& ir) const
			{
				return ir.network_address();
			}
	};

	/**
	 * \brief Get the network address associated to an IP route.
	 * \param ir The ip_route.
	 * \return The network address.
	 */
	inline ip_network_address network_address(const ip_route& ir)
	{
		return boost::apply_visitor(ip_route_network_address_visitor(), ir);
	}

	/**
	 * \brief A visitor that gets the gateway.
	 */
	class ip_route_gateway_visitor : public boost::static_visitor<boost::optional<boost::asio::ip::address>>
	{
		public:

			/**
			 * \brief Get the gateway of the specified ip_route.
			 * \tparam T The type of the ip_route.
			 * \param ir The ip_route.
			 * \return The gateway.
			 */
			template <typename T>
			result_type operator()(const T& ir) const
			{
				if (ir.gateway())
				{
					return boost::asio::ip::address(*ir.gateway());
				}
				else
				{
					return boost::none;
				}
			}
	};

	/**
	 * \brief Get the gateway of a given IP route.
	 * \param ir the ip_route.
	 * \return The gateway.
	 */
	inline boost::optional<boost::asio::ip::address> gateway(const ip_route& ir)
	{
		return boost::apply_visitor(ip_route_gateway_visitor(), ir);
	}

	/**
	 * \brief Check if the given IP route has a gateway.
	 * \param ir the ip_route.
	 * \return True if the IP route has a gateway.
	 */
	inline bool has_gateway(const ip_route& ir)
	{
		return static_cast<bool>(gateway(ir));
	}

	/**
	 * \brief A route set type.
	 */
	typedef std::set<ip_route> ip_route_set;

	/**
	 * \brief Output the routes to a stream.
	 * \param os The output stream.
	 * \param routes The routes to output.
	 * \return os.
	 */
	std::ostream& operator<<(std::ostream& os, const ip_route_set& routes);
}

#endif /* ASIOTAP_IP_ROUTE_HPP */
