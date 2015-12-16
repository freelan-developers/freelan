/*
 * freelan - An open, multi-platform software to establish peer-to-peer virtual
 * private networks.
 *
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of freelan.
 *
 * freelan is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * freelan is distributed in the hope that it will be useful, but
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
 * If you intend to use freelan in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file x509_certificate.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A X509 certificate class.
 */

#pragma once

#include <cassert>
#include <vector>

#include <openssl/x509.h>
#include <openssl/pem.h>

namespace freelan
{
    class X509Certificate {
        public:
            static X509Certificate read_as_der(const void* buf, size_t buf_len) {
                const unsigned char* pbuf = static_cast<const unsigned char*>(buf);

                return X509Certificate(d2i_X509(NULL, &pbuf, static_cast<long>(buf_len)));
            }

            X509Certificate() : X509Certificate(::X509_new()) {}
            X509Certificate(const X509Certificate&) = delete;
            X509Certificate(X509Certificate&& other) : m_ptr(other.m_ptr) {
                other.m_ptr = nullptr;
            }
            ~X509Certificate() {
                if (m_ptr) {
                    ::X509_free(m_ptr);
                }
            }

            X509Certificate& operator=(const X509Certificate&) = delete;
            X509Certificate& operator=(X509Certificate&& other) {
                std::swap(m_ptr, other.m_ptr);

                return *this;
            }

            size_t write_as_der(void* buf) const {
                unsigned char* out = static_cast<unsigned char*>(buf);
                unsigned char** pout = out != NULL ? &out : NULL;

                int result = ::i2d_X509(m_ptr, pout);

                //TODO: Proper error handling.
                assert(result >= 0);

                return result;
            }

            std::vector<uint8_t> write_as_der() const {
                std::vector<uint8_t> buf(write_as_der(nullptr));
                write_as_der(&buf[0]);
                return buf;
            }

        private:
            X509Certificate(X509* ptr) :
                m_ptr(ptr)
            {
                //TODO: Proper error handling.
                assert(ptr);
            }

            X509* m_ptr;
    };
}
