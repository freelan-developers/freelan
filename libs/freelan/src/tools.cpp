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
 * \file tools.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Generic purposes functions and classes.
 */

#include "tools.hpp"

#include <string>

#include <boost/system/system_error.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#ifdef WINDOWS
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <cryptoplus/pkey/rsa_key.hpp>

namespace freelan
{
	std::string get_hostname()
	{
#ifdef WINDOWS
		std::wstring whostname;
		whostname.resize(1024);
		DWORD size = static_cast<DWORD>(whostname.size()) - 1;

		// GetComputerNameA doesn't seem to work sadly.
		if (::GetComputerNameW(&whostname[0], &size))
		{
			whostname.resize(size);

			return std::string(whostname.begin(), whostname.end());
		}

		throw boost::system::system_error(::GetLastError(), boost::system::system_category());
#else
		char buf[256] = {};

		if (gethostname(buf, sizeof(buf)) == 0)
		{
			return std::string(buf);
		}

		throw boost::system::system_error(errno, boost::system::system_category());
#endif
	}

	cryptoplus::pkey::pkey generate_private_key(unsigned int size, unsigned int prime)
	{
		return cryptoplus::pkey::pkey::from_rsa_key(cryptoplus::pkey::rsa_key::generate_private_key(size, prime));
	}

	cryptoplus::x509::certificate_request generate_certificate_request(const cryptoplus::pkey::pkey& private_key, const std::string& common_name)
	{
		cryptoplus::x509::certificate_request certificate_request = cryptoplus::x509::certificate_request::create();

		certificate_request.set_version(2);
		certificate_request.subject().push_back("CN", MBSTRING_ASC, common_name);

		certificate_request.set_public_key(private_key);
		certificate_request.sign(private_key, cryptoplus::hash::message_digest_algorithm(NID_sha1));

		return certificate_request;
	}

	cryptoplus::x509::certificate generate_self_signed_certificate(const cryptoplus::pkey::pkey& private_key, const std::string& common_name, unsigned int duration)
	{
		cryptoplus::x509::certificate certificate = cryptoplus::x509::certificate::create();

		certificate.set_version(2);
		certificate.subject().push_back("CN", MBSTRING_ASC, common_name);
		certificate.set_issuer(certificate.subject());
		certificate.set_serial_number(cryptoplus::asn1::integer::from_long(1));
		certificate.push_back(cryptoplus::x509::extension::from_nconf_nid(NID_basic_constraints, "critical,CA:TRUE"));

		const cryptoplus::asn1::utctime not_before = cryptoplus::asn1::utctime::from_ptime(boost::posix_time::second_clock::universal_time() - boost::gregorian::days(1));
		const cryptoplus::asn1::utctime not_after = cryptoplus::asn1::utctime::from_ptime(boost::posix_time::second_clock::universal_time() + boost::gregorian::days(duration));

		certificate.set_not_before(not_before);
		certificate.set_not_after(not_after);

		certificate.set_public_key(private_key);
		certificate.sign(private_key, cryptoplus::hash::message_digest_algorithm(NID_sha1));

		return certificate;
	}

	cryptoplus::x509::certificate sign_certificate_request(const cryptoplus::x509::certificate_request& req, const cryptoplus::x509::certificate& ca_certificate, const cryptoplus::pkey::pkey& private_key, const std::string& common_name, unsigned int duration)
	{
		cryptoplus::x509::certificate certificate = cryptoplus::x509::certificate::create();

		certificate.set_version(2);
		certificate.subject().push_back("CN", MBSTRING_ASC, common_name);
		certificate.set_issuer(ca_certificate.subject());
		certificate.set_serial_number(cryptoplus::asn1::integer::from_long(1));
		certificate.push_back(cryptoplus::x509::extension::from_nconf_nid(NID_basic_constraints, "critical,CA:FALSE"));

		const cryptoplus::asn1::utctime not_before = cryptoplus::asn1::utctime::from_ptime(boost::posix_time::second_clock::universal_time() - boost::gregorian::days(1));
		const cryptoplus::asn1::utctime not_after = cryptoplus::asn1::utctime::from_ptime(boost::posix_time::second_clock::universal_time() + boost::gregorian::days(duration));

		certificate.set_not_before(not_before);
		certificate.set_not_after(not_after);

		certificate.set_public_key(req.public_key());
		certificate.sign(private_key, cryptoplus::hash::message_digest_algorithm(NID_sha1));

		return certificate;
	}
}
