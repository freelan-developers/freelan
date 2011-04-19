/**
 * \file client.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A simple client.
 */

#include <fscp/fscp.hpp>

#include <boost/asio.hpp>

#include <cstdlib>

int main()
{
	boost::asio::io_service io_service;
	fscp::server server(io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 12000));

	io_service.run();

	return EXIT_SUCCESS;
}
