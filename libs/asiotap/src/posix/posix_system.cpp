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
 * \file posix_system.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Posix system primitives.
 */

#include "posix/posix_system.hpp"

#include "os.hpp"
#include "error.hpp"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <sstream>

#include <boost/lexical_cast.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/algorithm/string.hpp>

namespace asiotap
{
	int execute(const std::vector<std::string>& args, boost::system::error_code& ec, std::ostream* output)
	{
#if FREELAN_DEBUG
		std::cout << "Executing:";

		for (auto&& arg : args)
		{
			std::cout << " " << arg;
		}

		std::cout << std::endl;
#endif

		int fd[2] = {0, 0};

		if (::pipe(fd) < 0)
		{
			ec = boost::system::error_code(errno, boost::system::system_category());

			return -1;
		}

		int output_fd[2] = {0, 0};

		if (output)
		{
			if (::pipe(output_fd) < 0)
			{
				ec = boost::system::error_code(errno, boost::system::system_category());

				return -1;
			}
		}

		const pid_t pid = fork();

		switch (pid)
		{
			case -1:
				{
					// fork() failed.
					if (output)
					{
						::close(output_fd[0]);
						::close(output_fd[1]);
					}

					::close(fd[0]);
					::close(fd[1]);

					ec = boost::system::error_code(errno, boost::system::system_category());

					return -1;
				}
			case 0:
				{
					// Child process
					const int fdlimit = ::sysconf(_SC_OPEN_MAX);

					if (output)
					{
						::dup2(output_fd[1], STDOUT_FILENO);
						::close(output_fd[1]);
					}

					for (int n = 0; n < fdlimit; ++n)
					{
						if ((n != fd[1]) && (!output || (n != STDOUT_FILENO)))
						{
							::close(n);
						}
					}

					fcntl(fd[1], F_SETFD, FD_CLOEXEC);

					// Estimate the required size for the argv buffer.
					// One null-terminated byte per arg.
					size_t buffer_size = args.size();

					for (auto&& arg : args)
					{
						buffer_size += arg.size();
					}

					std::vector<char> buffer(buffer_size, 0x00);
					std::vector<char*> argv(args.size() + 1, nullptr);
					auto offset = buffer.begin();

					for (size_t i = 0; i != args.size(); ++i)
					{
						const auto& arg = args[i];

						argv[i] = &*offset;
						offset = std::copy(arg.begin(), arg.end(), offset);
						*(offset++) = '\0';
					}

					// Execute the file specified
					::execv(argv[0], &argv[0]);

					// Something went wrong. Sending back errno to parent process then exiting.
					if (::write(fd[1], &errno, sizeof(errno))) {}

					_exit(127);

					break;
				}
			default:
				{
					// Parent process
					::close(fd[1]);

					if (output)
					{
						::close(output_fd[1]);
					}

					int child_errno = 0;

					if (output)
					{
						// This will take ownership of the file descriptor.
						boost::iostreams::file_descriptor_source output_src(output_fd[0], boost::iostreams::close_handle);
						boost::iostreams::stream<boost::iostreams::file_descriptor_source> output_is(output_src);

						(*output) << output_is.rdbuf();
					}

					const ssize_t readcnt = ::read(fd[0], &child_errno, sizeof(child_errno));

					::close(fd[0]);

					if (readcnt < 0)
					{
						ec = boost::system::error_code(errno, boost::system::system_category());

						return -1;
					}
					else if (readcnt == sizeof(child_errno))
					{
						ec = boost::system::error_code(child_errno, boost::system::system_category());

						return -1;
					}

					int status = 0;

					if (::waitpid(pid, &status, 0) != pid)
					{
						ec = boost::system::error_code(child_errno, boost::system::system_category());

						return -1;
					}
					else
					{
						if (WIFEXITED(status))
						{
							const int result = WEXITSTATUS(status);

#if FREELAN_DEBUG
							std::cout << "Exit status: " << result << std::endl;
#endif

							return result;
						}
					}

					break;
				}
		}

		return EXIT_FAILURE;
	}

	int execute(const std::vector<std::string>& args, std::ostream* output)
	{
		boost::system::error_code ec;

		const auto result = execute(args, ec, output);

		if (result < 0)
		{
			throw boost::system::system_error(ec);
		}

		return result;
	}

	void checked_execute(const std::vector<std::string>& args, std::ostream* output)
	{
		if (execute(args, output) != 0)
		{
			throw boost::system::system_error(make_error_code(asiotap_error::external_process_failed));
		}
	}

	posix_route_manager::route_type get_route_for(const boost::asio::ip::address& host)
	{
#ifdef MACINTOSH
		const std::vector<std::string> real_args { "/sbin/route", "-n", "get", boost::lexical_cast<std::string>(host) };

		std::stringstream ss;
		checked_execute(real_args, &ss);

		//The output is like:
		/*
			   route to: 8.8.8.8
			destination: default
				   mask: default
				gateway: 10.7.0.254
			  interface: en0
				  flags: <UP,GATEWAY,DONE,STATIC,PRCLONING>
			 recvpipe  sendpipe  ssthresh  rtt,msec    rttvar  hopcount      mtu     expire
				   0         0         0         0         0         0      1500         0
		*/
		std::map<std::string, std::string> values;
		std::string line;

		while (std::getline(ss, line))
		{
			const auto colon_pos = line.find_first_of(':');

			if (colon_pos == std::string::npos)
			{
				continue;
			}

			std::string key = line.substr(0, colon_pos);
			std::string value = line.substr(colon_pos + 1, std::string::npos);

			boost::algorithm::trim(key);
			boost::algorithm::trim(value);

			values[key] = value;
		}

		if (values.find("interface") == values.end())
		{
			throw boost::system::system_error(make_error_code(asiotap_error::external_process_output_parsing_error));
		}

		const std::string interface = values["interface"];
		boost::optional<boost::asio::ip::address> gw;

		if (values.find("gateway") != values.end())
		{
			gw = boost::asio::ip::address::from_string(values["gateway"]);
		}
#else
		const std::vector<std::string> real_args { "/bin/ip", "route", "get", boost::lexical_cast<std::string>(host) };

		std::stringstream ss;
		checked_execute(real_args, &ss);

		// The output is like:
		/*
			8.8.8.8 via 37.59.15.254 dev eth0  src 46.105.57.112
				cache
		*/

		std::string host_addr;

		if (!(ss >> host_addr))
		{
			throw boost::system::system_error(make_error_code(asiotap_error::external_process_output_parsing_error));
		}

		std::map<std::string, std::string> values;
		std::string key, value;

		while (ss >> key >> value)
		{
			boost::algorithm::trim(key);
			boost::algorithm::trim(value);

			values[key] = value;
		}

		if (values.find("dev") == values.end())
		{
			throw boost::system::system_error(make_error_code(asiotap_error::external_process_output_parsing_error));
		}

		const std::string interface = values["dev"];
		boost::optional<boost::asio::ip::address> gw;

		if (values.find("via") != values.end())
		{
			gw = boost::asio::ip::address::from_string(values["via"]);
		}
#endif

		const auto route = to_ip_route(to_network_address(host), gw);
		const posix_route_manager::route_type route_entry = { interface, route, 0 };

		return route_entry;
	}

	void ifconfig(const std::string& interface, const ip_network_address& address)
	{
		const std::vector<std::string> real_args { "/sbin/ifconfig", interface, boost::lexical_cast<std::string>(address) };

		checked_execute(real_args);
	}

	void ifconfig(const std::string& interface, const ip_network_address& address, const boost::asio::ip::address& remote_address)
	{
		const std::vector<std::string> real_args { "/sbin/ifconfig", interface, boost::lexical_cast<std::string>(address), boost::lexical_cast<std::string>(remote_address) };

		checked_execute(real_args);
	}

	void route(const std::string& command, const std::string& interface, const ip_network_address& dest)
	{
		const std::string net_host = is_unicast(dest) ? "-host" : "-net";

#ifdef MACINTOSH
		const std::vector<std::string> real_args { "/sbin/route", "-n", command, net_host, boost::lexical_cast<std::string>(dest), "-interface", interface };
#else
		const std::vector<std::string> real_args { "/sbin/route", "-n", command, net_host, boost::lexical_cast<std::string>(dest), "dev", interface };
#endif

		checked_execute(real_args);
	}

	void route(const std::string& command, const std::string& interface, const ip_network_address& dest, const boost::asio::ip::address& gateway)
	{
		const std::string net_host = is_unicast(dest) ? "-host" : "-net";

#ifdef MACINTOSH
		static_cast<void>(interface);
		const std::vector<std::string> real_args { "/sbin/route", "-n", command, net_host, boost::lexical_cast<std::string>(dest), boost::lexical_cast<std::string>(gateway) };
#else
		const std::vector<std::string> real_args { "/sbin/route", "-n", command, net_host, boost::lexical_cast<std::string>(dest), "gw", boost::lexical_cast<std::string>(gateway), "dev", interface };
#endif

		checked_execute(real_args);
	}
}
