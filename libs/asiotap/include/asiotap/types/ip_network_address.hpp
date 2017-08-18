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
 * \file ip_network_address.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An IP network address class.
 */

#ifndef ASIOTAP_IP_NETWORK_ADDRESS_HPP
#define ASIOTAP_IP_NETWORK_ADDRESS_HPP

#include <vector>
#include <set>
#include <type_traits>
#include <tuple>

#include <boost/asio.hpp>
#include <boost/variant.hpp>

namespace asiotap
{
	/**
	 * \brief A generic IP network address template class.
	 */
	template <typename AddressType>
	class base_ip_network_address
	{
		public:

			/**
			 * \brief The address type.
			 */
			typedef AddressType address_type;

			/**
			 * \brief Get a null network address.
			 * \return A null network address.
			 */
			static base_ip_network_address null()
			{
				return base_ip_network_address();
			}

			/**
			* \brief Get a network address that represents all addresses.
			* \return A network address.
			*/
			static base_ip_network_address any()
			{
				return base_ip_network_address(address_type(), 0);
			}

			/**
			 * \brief The single address prefix length.
			 */
			static const size_t single_address_prefix_length = std::tuple_size<typename address_type::bytes_type>::value * 8;

			/**
			 * \brief Create an IP network address.
			 */
			base_ip_network_address() : m_address(), m_prefix_length(single_address_prefix_length) {};

			/**
			 * \brief Create an IP network address.
			 * \param _address The address.
			 */
			base_ip_network_address(const address_type& _address) : m_address(_address), m_prefix_length(single_address_prefix_length) {};

			/**
			 * \brief Create an IP network address.
			 * \param _address The address.
			 * \param _prefix_length The prefix length.
			 */
			base_ip_network_address(const address_type& _address, unsigned int _prefix_length) : m_address(_address), m_prefix_length(_prefix_length) {};

			/**
			 * \brief Check if the instance is null.
			 * \return true if the instance is null.
			 */
			bool is_null() const
			{
				return (*this == null());
			}

			/**
			 * \brief Get the address.
			 * \return The address.
			 */
			const address_type& address() const
			{
				return m_address;
			}

			/**
			 * \brief Get the prefix length.
			 * \return The prefix length.
			 */
			unsigned int prefix_length() const
			{
				return m_prefix_length;
			}

			/**
			 * \brief Check if the specified address belongs to the network address.
			 * \param addr The address to check.
			 * \return true if addr belongs to the network address, false otherwise.
			 */
			bool has_address(const address_type& addr) const;

			/**
			 * \brief Check if the specified network address is a subnet of the current network address.
			 * \param addr The network address to check.
			 * \return true if addr belongs to the network address, false otherwise.
			 */
			bool has_network(const base_ip_network_address& addr) const;

			/**
			 * \brief Check if the specified network address represents an unicast address.
			 * \return true if the specified network address represents an unicast address.
			 */
			bool is_unicast() const
			{
				return (m_prefix_length == single_address_prefix_length);
			}

			/**
			 * \brief Get the network address.
			 * \return The network address.
			 */
			address_type get_network_address() const;

		private:

			address_type m_address;
			unsigned int m_prefix_length;

		public:

			friend bool operator==(const base_ip_network_address& lhs, const base_ip_network_address& rhs)
			{
				return (lhs.address() == rhs.address()) && (lhs.prefix_length() == rhs.prefix_length());
			}

			friend bool operator!=(const base_ip_network_address& lhs, const base_ip_network_address& rhs)
			{
				return !(lhs == rhs);
			}

			friend bool operator<(const base_ip_network_address& lhs, const base_ip_network_address& rhs)
			{
				if (lhs.prefix_length() == rhs.prefix_length())
				{
					return (lhs.address() < rhs.address());
				}
				else
				{
					// More specific means a higher prefix length.
					return (lhs.prefix_length() > rhs.prefix_length());
				}
			}
	};

	/**
	 * \brief Write an network address to an output stream.
	 * \tparam AddressType The address type.
	 * \param os The output stream.
	 * \param value The value.
	 * \return os.
	 */
	template <typename AddressType>
	std::ostream& operator<<(std::ostream& os, const base_ip_network_address<AddressType>& value);

	/**
	 * \brief Read a network address from an input stream.
	 * \tparam AddressType The address type.
	 * \param is The input stream.
	 * \param value The value.
	 * \return is.
	 */
	template <typename AddressType>
	std::istream& operator>>(std::istream& is, base_ip_network_address<AddressType>& value);

	/**
	 * \brief The IPv4 instantiation.
	 */
	typedef base_ip_network_address<boost::asio::ip::address_v4> ipv4_network_address;

	/**
	 * \brief The IPv6 instantiation.
	 */
	typedef base_ip_network_address<boost::asio::ip::address_v6> ipv6_network_address;

	/**
	 * \brief The generic IP type.
	 */
	typedef boost::variant<ipv4_network_address, ipv6_network_address> ip_network_address;

	/**
	 * \brief An IPv4 network list type.
	 */
	typedef std::vector<ipv4_network_address> ipv4_network_address_list;

	/**
	 * \brief An IPv6 network list type.
	 */
	typedef std::vector<ipv6_network_address> ipv6_network_address_list;

	/**
	 * \brief A generic IP network list type.
	 */
	typedef std::vector<ip_network_address> ip_network_address_list;

	/**
	 * \brief Convert an IP address into a network address.
	 * \param addr The address.
	 * \return The network address.
	 */
	inline ip_network_address to_network_address(const boost::asio::ip::address& addr)
	{
		if (addr.is_v4())
		{
			return addr.to_v4();
		}
		else
		{
			return addr.to_v6();
		}
	}

	/**
	 * \brief Convert an IP address into a network address.
	 * \param addr The address.
	 * \param prefix_len The prefix length.
	 * \return The network address.
	 */
	inline ip_network_address to_network_address(const boost::asio::ip::address& addr, unsigned int prefix_len)
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
	 * \brief A visitor that writes ip_network_address to output streams.
	 */
	class ip_network_address_output_visitor : public boost::static_visitor<std::ostream&>
	{
		public:

			/**
			 * \brief Create a new ip_network_address_output_visitor.
			 * \param os The output stream.
			 */
			ip_network_address_output_visitor(result_type os) : m_os(os) {}

			/**
			 * \brief Write the specified ip_network_address.
			 * \tparam T The type of the ip_network_address.
			 * \param ina The ip_network_address.
			 * \return os.
			 */
			template <typename T>
			result_type operator()(const T& ina) const
			{
				return m_os << ina;
			}

		private:

			result_type m_os;
	};

	/**
	 * \brief Write an ip_network_address to an output stream.
	 * \param os The output stream.
	 * \param value The value.
	 * \return os.
	 */
	inline std::ostream& operator<<(std::ostream& os, const ip_network_address& value)
	{
		return boost::apply_visitor(ip_network_address_output_visitor(os), value);
	}

	/**
	 * \brief Read an ip_network_address from an input stream.
	 * \param is The input stream.
	 * \param value The value.
	 * \return is.
	 */
	std::istream& operator>>(std::istream& is, ip_network_address& value);

	/**
	 * \brief A visitor that checks if the ip_network_address contains an address.
	 */
	class ip_network_address_has_address_visitor : public boost::static_visitor<bool>
	{
		public:

			/**
			 * \brief Create a new ip_network_address_has_address_visitor.
			 * \param addr The address.
			 */
			ip_network_address_has_address_visitor(const boost::asio::ip::address& addr) : m_addr(addr) {}

			/**
			 * \brief Check if the ip_network_address contains an address.
			 * \param ina The ipv4_network_address.
			 * \return os.
			 */
			result_type operator()(const ipv4_network_address& ina) const
			{
				return (m_addr.is_v4() && ina.has_address(m_addr.to_v4()));
			}

			/**
			 * \brief Check if the ip_network_address contains an address.
			 * \param ina The ipv6_network_address.
			 * \return os.
			 */
			result_type operator()(const ipv6_network_address& ina) const
			{
				return (m_addr.is_v6() && ina.has_address(m_addr.to_v6()));
			}

		private:

			boost::asio::ip::address m_addr;
	};

	/**
	 * \brief Check if an ip_network_address contains an address.
	 * \param ina The base_ip_network_address.
	 * \param addr The address.
	 * \return true if addr is contained in ina.
	 */
	template <typename AddressType>
	inline bool has_address(const base_ip_network_address<AddressType>& ina, const AddressType& addr)
	{
		return ina.has_address(addr);
	}

	/**
	 * \brief Check if an ip_network_address contains an address.
	 * \param ina The ip_network_address.
	 * \param addr The address.
	 * \return true if addr is contained in ina.
	 */
	template <typename AddressType>
	inline bool has_address(const ip_network_address& ina, const AddressType& addr)
	{
		return boost::apply_visitor(ip_network_address_has_address_visitor(addr), ina);
	}

	/**
	 * \brief A visitor that checks if the ip_network_address contains a network.
	 */
	class ip_network_address_has_network_visitor : public boost::static_visitor<bool>
	{
		private:
			template <typename AddressType>
			class ip_network_address_belongs_to_network_visitor : public boost::static_visitor<bool>
			{
				public:

					ip_network_address_belongs_to_network_visitor(const base_ip_network_address<AddressType>& ina) : m_ina(ina) {}

					template <typename Any>
					result_type operator()(const Any&) const
					{
						return false;
					}

					result_type operator()(const base_ip_network_address<AddressType>& addr) const
					{
						return m_ina.has_network(addr);
					}

				private:

					base_ip_network_address<AddressType> m_ina;
			};

		public:

			/**
			 * \brief Create a new ip_network_address_has_network_visitor.
			 * \param addr The address.
			 */
			ip_network_address_has_network_visitor(const ip_network_address& addr) : m_addr(addr) {}

			/**
			 * \brief Default implementation.
			 * \return true if m_addr belongs to ina.
			 */
			template <typename AddressType>
			result_type operator()(const base_ip_network_address<AddressType>& ina) const
			{
				return boost::apply_visitor(ip_network_address_belongs_to_network_visitor<AddressType>(ina), m_addr);
			}

		private:

			ip_network_address m_addr;
	};

	/**
	 * \brief Check if an ip_network_address contains a network.
	 * \param ina The ip_network_address.
	 * \param addr The network address.
	 * \return true if addr is contained in ina.
	 */
	template <typename AddressType>
	inline bool has_network(const base_ip_network_address<AddressType>& ina, const base_ip_network_address<AddressType>& addr)
	{
		return ina.has_network(addr);
	}

	/**
	 * \brief Check if an ip_network_address contains a network.
	 * \param ina The ip_network_address.
	 * \param addr The network address.
	 * \return true if addr is contained in ina.
	 */
	inline bool has_network(const ip_network_address& ina, const ip_network_address& addr)
	{
		return boost::apply_visitor(ip_network_address_has_network_visitor(addr), ina);
	}

	/**
	 * \brief A visitor that checks if the ip_network_address is an unicast address.
	 */
	class ip_network_address_is_unicast_visitor : public boost::static_visitor<bool>
	{
		public:

			/**
			 * \brief Default implementation.
			 * \return false.
			 */
			template <typename AddressType>
			result_type operator()(const base_ip_network_address<AddressType>& ina) const
			{
				return ina.is_unicast();
			}
	};

	/**
	 * \brief Check if an ip_network_address is an unicast address.
	 * \param ina The ip_network_address.
	 * \return true if ina is an unicast address.
	 */
	template <typename AddressType>
	inline bool is_unicast(const base_ip_network_address<AddressType>& ina)
	{
		return ina.is_unicast();
	}

	/**
	 * \brief Check if an ip_network_address is an unicast address.
	 * \param ina The ip_network_address.
	 * \return true if ina is an unicast address.
	 */
	inline bool is_unicast(const ip_network_address& ina)
	{
		return boost::apply_visitor(ip_network_address_is_unicast_visitor(), ina);
	}

	/**
	 * \brief A visitor that gets the network address.
	 */
	class ip_network_address_get_network_address_visitor : public boost::static_visitor<boost::asio::ip::address>
	{
		public:

			/**
			 * \brief Get the network address of the specified address.
			 * \param ina The base_ip_network_address instance.
			 * \return The network address.
			 */
			template <typename AddressType>
			result_type operator()(const base_ip_network_address<AddressType>& ina) const
			{
				return ina.get_network_address();
			}
	};

	/**
	 * \brief Get the network address associated to an address.
	 * \param ina The ip_network_address.
	 * \return The network address.
	 */
	inline boost::asio::ip::address get_network_address(const ip_network_address& ina)
	{
		return boost::apply_visitor(ip_network_address_get_network_address_visitor(), ina);
	}

	/**
	 * \brief Look for an address in a list.
	 * \param begin An iterator to the first element of the list.
	 * \param end An iterator past the last element of the list.
	 * \param addr The address to look for.
	 * \return An iterator to the entry that has the address, or end if no such element exists.
	 */
	template <typename NetworkAddressIterator, typename AddressType>
	NetworkAddressIterator find_address(NetworkAddressIterator begin, NetworkAddressIterator end, const AddressType& addr)
	{
		for (; begin != end; ++begin)
		{
			if (has_address(*begin, addr))
			{
				break;
			}
		}

		return begin;
	}

	/**
	 * \brief Look for an address in a list.
	 * \param begin An iterator to the first element of the list.
	 * \param end An iterator past the last element of the list.
	 * \param addr The address to look for.
	 * \return true if the address was found in the list.
	 */
	template <typename NetworkAddressIterator, typename AddressType>
	inline bool has_address(NetworkAddressIterator begin, NetworkAddressIterator end, const AddressType& addr)
	{
		return (find_address(begin, end, addr) != end);
	}

	/**
	* \brief A visitor that gets the IP address.
	*/
	class ip_network_address_ip_address_visitor : public boost::static_visitor<boost::asio::ip::address>
	{
	public:

		/**
		* \brief Get the IP address of the specified ip_network_address.
		* \tparam T The type of the ip_network_address.
		* \param ina The ip_network_address.
		* \return The prefix length.
		*/
		template <typename T>
		result_type operator()(const T& ina) const
		{
			return ina.address();
		}
	};

	/**
	* \brief Get the IP address of a given network address.
	* \param ina the ip_network_address.
	* \return The network address IP address.
	*/
	inline boost::asio::ip::address to_ip_address(const ip_network_address& ina)
	{
		return boost::apply_visitor(ip_network_address_ip_address_visitor(), ina);
	}

	/**
	 * \brief A visitor that gets the prefix length.
	 */
	class ip_network_address_prefix_len_visitor : public boost::static_visitor<unsigned int>
	{
		public:

			/**
			 * \brief Get the prefix length of the specified ip_network_address.
			 * \tparam T The type of the ip_network_address.
			 * \param ina The ip_network_address.
			 * \return The prefix length.
			 */
			template <typename T>
			result_type operator()(const T& ina) const
			{
				return ina.prefix_length();
			}
	};

	/**
	 * \brief Get the prefix length of a given network address.
	 * \param ina the ip_network_address.
	 * \return The network address prefix length.
	 */
	inline unsigned int to_prefix_length(const ip_network_address& ina)
	{
		return boost::apply_visitor(ip_network_address_prefix_len_visitor(), ina);
	}
}

#endif /* ASIOTAP_IP_NETWORK_ADDRESS_HPP */
