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
 * \file registry.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Windows registry access.
 */

#ifndef ASIOTAP_WINDOWS_REGISTRY_HPP
#define ASIOTAP_WINDOWS_REGISTRY_HPP

#include <windows.h>

#include <boost/system/system_error.hpp>
#include <boost/optional.hpp>
#include <boost/array.hpp>
#include <boost/filesystem.hpp>

#include <string>
#include <iterator>

#include "../error.hpp"

namespace asiotap
{
	class registry_key
	{
		public:

			registry_key() :
				m_key(),
				m_name()
			{
			}

			registry_key(HKEY hKey, const std::string& name, REGSAM samDesired = KEY_READ) :
				m_key(),
				m_name(name)
			{
				HKEY key;

				const LONG status = ::RegOpenKeyExA(hKey, m_name.c_str(), 0, samDesired, &key);

				if (status != ERROR_SUCCESS)
				{
					throw boost::system::system_error(status, boost::system::system_category());
				}

				m_key = key;
			}

			registry_key(const registry_key& parent, const std::string& name, REGSAM samDesired = KEY_READ) :
				registry_key(parent.native_handle(), name, samDesired)
			{
			}

			~registry_key()
			{
				if (m_key)
				{
					::RegCloseKey(*m_key);
				}
			}

			registry_key(const registry_key&) = delete;
			registry_key& operator=(const registry_key&) = delete;

			registry_key(registry_key&& other) throw() :
				m_key(other.m_key),
				m_name(other.m_name)
			{
				other.m_key.reset();
				other.m_name.clear();
			}

			registry_key& operator=(registry_key&& other) throw()
			{
				using std::swap;

				swap(m_key, other.m_key);

				return *this;
			}

			bool is_open() const
			{
				return static_cast<bool>(m_key);
			}

			HKEY native_handle() const
			{
				return *m_key;
			}

			const std::string& name() const
			{
				return m_name;
			}

			void query_value(const std::string& value_name, DWORD& type, void* buf, size_t& buflen, boost::system::error_code& ec) const
			{
				DWORD data_len = static_cast<DWORD>(buflen);

				const LONG status = ::RegQueryValueExA(native_handle(), value_name.c_str(), NULL, &type, static_cast<LPBYTE>(buf), &data_len);

				buflen = static_cast<size_t>(data_len);

				if (status != ERROR_SUCCESS)
				{
					ec = boost::system::error_code(status, boost::system::system_category());

					return;
				}
			}

			std::string query_string(const std::string& value_name, boost::system::error_code& ec) const
			{
				boost::array<char, 256> value;

				DWORD type = REG_NONE;

				size_t value_size = value.size();

				query_value(value_name, type, value.data(), value_size, ec);

				if (ec)
				{
					return std::string();
				}

				if (type != REG_SZ)
				{
					ec = make_error_code(asiotap_error::invalid_type);
				}

				return std::string(value.begin(), value.begin() + value_size - 1);
			}

			std::string query_string(const std::string& value_name) const
			{
				boost::system::error_code ec;

				const std::string result = query_string(value_name, ec);

				if (ec)
				{
					throw boost::system::system_error(ec);
				}

				return result;
			}

			boost::filesystem::path query_path(const std::string& value_name, boost::system::error_code& ec) const
			{
				boost::array<char, 4096> value;

				DWORD type = REG_NONE;

				size_t value_size = value.size();

				query_value(value_name, type, value.data(), value_size, ec);

				if (ec)
				{
					return std::string();
				}

				if (type != REG_SZ)
				{
					ec = make_error_code(asiotap_error::invalid_type);
				}

				return boost::filesystem::path(value.begin(), value.begin() + value_size - 1);
			}

			boost::filesystem::path query_path(const std::string& value_name) const
			{
				boost::system::error_code ec;

				const boost::filesystem::path result = query_path(value_name, ec);

				if (ec)
				{
					throw boost::system::system_error(ec);
				}

				return result;
			}

			registry_key operator[](size_t index) const
			{
				boost::array<char, 256> name;
				DWORD name_size = static_cast<DWORD>(name.size());

				const LONG status = ::RegEnumKeyExA(native_handle(), static_cast<DWORD>(index), name.data(), &name_size, NULL, NULL, NULL, NULL);

				switch (status)
				{
					case ERROR_SUCCESS:
					{
						try
						{
							return registry_key(*this, std::string(name.begin(), name.begin() + name_size));
						}
						catch (const boost::system::system_error&)
						{
							return registry_key();
						}
					}
					case ERROR_NO_MORE_ITEMS:
					{
						return registry_key();
					}
					default:
					{
						throw boost::system::system_error(status, boost::system::system_category());
					}
				}
			}

			size_t size() const
			{
				DWORD count = 0;

				const LONG status = ::RegQueryInfoKey(native_handle(), NULL, NULL, NULL, &count, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

				if (status != ERROR_SUCCESS)
				{
					throw boost::system::system_error(status, boost::system::system_category());
				}

				return static_cast<size_t>(count);
			}

			class const_iterator : public std::iterator<std::forward_iterator_tag, registry_key>
			{
				public:

					const_iterator operator++(int)
					{
						const const_iterator result = *this;

						++m_index;

						return result;
					}

					const_iterator& operator++()
					{
						m_index++;

						return *this;
					}

					value_type operator*() const
					{
						return m_key[m_index];
					}

				private:

					const_iterator(const registry_key& key, size_t index) :
						m_key(key),
						m_index(index)
					{
					}

					friend bool operator<(const const_iterator& lhs, const const_iterator& rhs)
					{
						assert(&lhs.m_key == &rhs.m_key);

						return (lhs.m_index < rhs.m_index);
					}

					friend bool operator==(const const_iterator& lhs, const const_iterator& rhs)
					{
						assert(&lhs.m_key == &rhs.m_key);

						return (lhs.m_index == rhs.m_index);
					}

					friend bool operator!=(const const_iterator& lhs, const const_iterator& rhs)
					{
						assert(&lhs.m_key == &rhs.m_key);

						return (lhs.m_index != rhs.m_index);
					}

					const registry_key& m_key;
					size_t m_index;

					friend class registry_key;
			};

			class available_keys_range
			{
				public:

					const_iterator begin() const
					{
						return const_iterator(m_key, 0);
					}

					const_iterator end() const
					{
						return const_iterator(m_key, m_key.size());
					}

				private:

					available_keys_range(const registry_key& key) :
						m_key(key)
					{}

					const registry_key& m_key;

					friend class registry_key;
			};

			available_keys_range available_keys() const
			{
				return available_keys_range(*this);
			}

		private:

			boost::optional<HKEY> m_key;
			std::string m_name;
	};
}

#endif /* ASIOTAP_WINDOWS_REGISTRY_HPP */
