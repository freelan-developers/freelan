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
 * \file windows_tap_adapter.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The Windows tap adapter class.
 */

#include "windows/windows_tap_adapter.hpp"

#include <boost/optional.hpp>

#include <vector>
#include <iterator>
#include <limits>

#include <winioctl.h>
#include <iphlpapi.h>
#include <winbase.h>
#include <shellapi.h>

#include "../windows/common.h"

namespace asiotap
{
	namespace
	{
		boost::system::error_code get_error_code(LONG error)
		{
			return boost::system::error_code(error, boost::system::system_category());
		}

		void throw_system_error(LONG error)
		{
			throw boost::system::system_error(get_error_code(error));
		}

		void throw_system_error(const boost::system::error_code& ec)
		{
			throw boost::system::system_error(ec);
		}

		void throw_system_error_if_not(LONG error)
		{
			if (error != ERROR_SUCCESS)
			{
				throw_system_error(error);
			}
		}

		typedef std::vector<std::string> guid_array_type;
		typedef std::map<std::string, std::string> guid_map_type;
		typedef std::pair<std::string, std::string> guid_pair_type;

		class reg_key
		{
			public:

				reg_key() :
					m_key(INVALID_HANDLE)
				{
				}

				reg_key(HKEY hKey, const std::string& name, REGSAM samDesired = KEY_READ) :
					m_key(INVALID_HANDLE),
					m_name(name)
				{
					const LONG status = RegOpenKeyExA(hKey, m_name.c_str(), 0, samDesired, &m_key);

					throw_system_error_if_not(status);
				}

				reg_key(const reg_key parent, const std::string& name, REGSAM samDesired = KEY_READ) :
					reg_key(parent.native_handle(), name, samDesired)
				{
				}

				~reg_key()
				{
					if (m_key != INVALID_HANDLE)
					{
						RegCloseKey(m_key);
					}
				}

				reg_key(reg_key&& other) throw() :
					m_key(other.m_key)
				{
					other.m_key = INVALID_HANDLE;
				}

				reg_key& operator=(reg_key&& other) throw()
				{
					swap(m_key, other.m_key);

					return *this;
				}

				HKEY native_handle() const
				{
					return m_key;
				}

				const std::string& name() const
				{
					return m_name;
				}

				void query_value(LPCSTR lpValueName, DWORD& type, void* buf, size_t& buflen, boost::system::error_code& ec) const
				{
					DWORD data_len = static_cast<DWORD>(buflen);

					const LONG status = RegQueryValueExA(m_key, lpValueName, NULL, &type, static_cast<LPBYTE>(buf), &data_len);

					buflen = static_cast<size_t>(data_len);

					if (status != ERROR_SUCCESS)
					{
						ec = get_error_code(status);

						return;
					}
				}

				void query_value(const std::string& value_name, DWORD& type, void* buf, size_t& buflen, boost::system::error_code& ec) const
				{
					query_value(value_name.c_str(), type, buf, buflen, ec);
				}

				std::string query_string(const std::string& value_name, boost::system::error_code& ec) const
				{
					boost::array<char, 256> value;

					DWORD type;

					size_t value_size = value.size();

					query_value(value_name, type, value.data(), value_size, ec);

					if (ec)
					{
						return std::string();
					}

					if (type != REG_SZ)
					{
						return std::string();
					}

					return std::string(value.begin(), value.begin() + value_size - 1);
				}

				std::string query_string(const std::string& value_name) const
				{
					boost::system::error_code ec;

					const std::string result = query_string(value_name, ec);

					if (!ec)
					{
						throw_system_error(ec);
					}

					return result;
				}

				reg_key operator[](size_t index) const
				{
					boost::array<char, 256> name;
					DWORD name_size = static_cast<DWORD>(name.size());

					const LONG status = RegEnumKeyExA(m_key, static_cast<DWORD>(index), name.data(), &name_size, NULL, NULL, NULL, NULL);

					switch (status)
					{
						case ERROR_SUCCESS:
						{
							return reg_key(*this, std::string(name.begin(), name.begin() + name_size - 1));
						}
						case ERROR_NO_MORE_ITEMS:
						{
							return reg_key();
						}
						default:
						{
							throw_system_error(status);
						}
					}
				}

				size_t size() const
				{
					DWORD count = 0;

					const LONG status = RegQueryInfoKey(m_key, NULL, NULL, NULL, &count, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

					throw_system_error_if_not(status);

					return static_cast<size_t>(count);
				}

				class iterator : public std::iterator<std::forward_iterator_tag, reg_key>
				{
					public:
					private:

						iterator(const reg_key& key, size_t index) :
							m_key(key),
							m_index(index)
						{
						}

						iterator operator++(int)
						{
							const iterator result = *this;

							++m_index;

							return result;
						}

						iterator& operator++()
						{
							m_index++;

							return *this;
						}

						value_type operator*() const
						{
							return m_key[m_index];
						}

						friend bool operator<(const iterator& lhs, const iterator& rhs)
						{
							assert(&lhs.m_key == &rhs.m_key);

							return (lhs.m_index < rhs.m_index);
						}

						friend bool operator==(const iterator& lhs, const iterator& rhs)
						{
							assert(&lhs.m_key == &rhs.m_key);

							return (lhs.m_index == rhs.m_index);
						}

						const reg_key& m_key;
						size_t m_index;
				};

				iterator begin() const
				{
					return iterator(*this, 0);
				}

				iterator end() const
				{
					return iterator(*this, size());
				}

			private:

				HKEY m_key;
				std::string m_name;
		};

		guid_array_type enumerate_tap_adapters_guid()
		{
			guid_array_type tap_adapters_list;

			HKEY adapter_key;
			LONG status;

			const std::string tap_component_id(TAP_COMPONENT_ID);
			const reg_key adapter_key(HKEY_LOCAL_MACHINE, ADAPTER_KEY);

			for (reg_key network_adapter_key : adapter_key)
			{
				const std::string component_id_str = network_adapter_key.query_string("ComponentId");

				if (tap_component_id == component_id_str)
				{
					const std::string net_cfg_instance_id_str = network_adapter_key.query_string("NetCfgInstanceId");

					tap_adapters_list.push_back(net_cfg_instance_id_str);
				}
			}

			return tap_adapters_list;
		}

		guid_map_type enumerate_network_connections()
		{
			guid_map_type network_connections_map;

			const reg_key network_connections_key(HKEY_LOCAL_MACHINE, NETWORK_CONNECTIONS_KEY);

			for (reg_key network_connection_key : network_connections_key)
			{
				const reg_key connection_key(network_connection_key, "Connection");
				const std::string name = connection_key.query_string("Name");

				network_connections_map[network_connection_key.name()] = name;
			}

			return network_connections_map;
		}

		guid_map_type enumerate_tap_adapters()
		{
			guid_map_type network_connections_map = enumerate_network_connections();

			const guid_array_type tap_adapters_list = enumerate_tap_adapters_guid();

			guid_map_type tap_adapters_map;

			for (const std::string& guid : tap_adapters_list)
			{
				if (network_connections_map.find(guid) != network_connections_map.end())
				{
					tap_adapters_map[guid] = network_connections_map[guid];
				}
			}

			return tap_adapters_map;
		}

		guid_pair_type find_tap_adapter_by_guid(const std::string& guid)
		{
			const guid_map_type tap_adapters_map = enumerate_tap_adapters();

			const guid_map_type::const_iterator it = tap_adapters_map.find(guid);

			if (it == tap_adapters_map.end())
			{
				throw std::runtime_error("No such tap adapter: " + guid);
			}

			return *it;
		}
	}

	std::map<std::string, std::string> windows_tap_adapter::enumerate()
	{
		return enumerate_tap_adapters();
	}
}
