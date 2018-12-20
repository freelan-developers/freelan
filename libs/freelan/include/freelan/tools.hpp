/*
 * libfreelan - A C++ library to establish peer-to-peer virtual private
 * networks.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libfreelan.
 *
 * libfreelan is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libfreelan is distributed in the hope that it will be useful, but
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
 * If you intend to use libfreelan in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file tools.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Generic purposes functions and classes.
 */

#pragma once

#include "os.hpp"

#include <cryptoplus/x509/certificate.hpp>
#include <cryptoplus/x509/certificate_request.hpp>
#include <cryptoplus/pkey/pkey.hpp>

namespace freelan
{
	/**
	 * \brief Get the hostname of the current host.
	 * \return The current hostname.
	 */
	std::string get_hostname();

	/**
	 * \brief Generate a private key.
	 * \param size The size of the key. Defaults to 3072.
	 * \param prime A prime number to generate the key. Defaults to 17.
	 * \return The private key.
	 */
	cryptoplus::pkey::pkey generate_private_key(unsigned int size = 3072, unsigned int prime = 17);

	/**
	 * \brief Generate a certificate request.
	 * \param private_key The private key to use.
	 * \param common_name The common name to use. Defaults to get_hostname().
	 * \param The duration of the certificate. Defaults to a year.
	 * \return The self-signed certificate.
	 */
	cryptoplus::x509::certificate_request generate_certificate_request(const cryptoplus::pkey::pkey& private_key, const std::string& common_name = get_hostname());

	/**
	 * \brief Generate a self-signed certificate.
	 * \param private_key The private key to use.
	 * \param common_name The common name to use. Defaults to get_hostname().
	 * \param The duration of the certificate. Defaults to a year.
	 * \return The self-signed certificate.
	 */
	cryptoplus::x509::certificate generate_self_signed_certificate(const cryptoplus::pkey::pkey& private_key, const std::string& common_name = get_hostname(), unsigned int duration = 365);

	/**
	 * \brief Sign a certificate request.
	 * \param req The certificate request to sign.
	 * \param ca_certificate The CA certificate to use.
	 * \param private_key The private key that matches the CA certificate.
	 * \param common_name The common name to use. Will override the common name in the certificate request.
	 * \param The duration of the certificate. Defaults to a day.
	 * \return The signed certificate.
	 */
	cryptoplus::x509::certificate sign_certificate_request(const cryptoplus::x509::certificate_request& req, const cryptoplus::x509::certificate& ca_certificate, const cryptoplus::pkey::pkey& private_key, const std::string& common_name, unsigned int duration = 1);
}
