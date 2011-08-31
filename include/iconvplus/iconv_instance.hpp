/*
 * libiconvplus - A C++ lightweight wrapper around the iconv library.
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
 * \file iconv_instance.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The iconv class.
 */

#ifndef ICONVPLUS_ICONV_INSTANCE_HPP
#define ICONVPLUS_ICONV_INSTANCE_HPP

#include <iconv.h>

#include <boost/noncopyable.hpp>
#include <boost/system/error_code.hpp>

#include <stdexcept>

namespace iconvplus
{
	/**
	 * \brief A class that wraps a iconv_t.
	 */
	class iconv_instance : public boost::noncopyable
	{
		public:

			/**
			 * \brief The native type.
			 */
			typedef iconv_t native_type;

			/**
			 * \brief The error value.
			 */
			static const size_t ERROR_VALUE = static_cast<size_t>(-1);

			/**
			 * \brief The default chunk size.
			 */
			static const size_t DEFAULT_CHUNK_SIZE = 1024;

			/**
			 * \brief Create a new iconv instance.
			 * \param to The destination encoding.
			 * \param from The source encoding.
			 */
			iconv_instance(const char* to, const char* from);

			/**
			 * \brief Create a new iconv instance.
			 * \param to The destination encoding.
			 * \param from The source encoding.
			 */
			iconv_instance(const std::string& to, const std::string& from);

			/**
			 * \brief Destroy the iconv instance.
			 */
			~iconv_instance();

			/**
			 * \brief Get the native pointer.
			 * \return The native pointer.
			 */
			native_type raw() const;

			/**
			 * \brief Proceed to a native iconv() call.
			 * \param inbuf The input buffer.
			 * \param inbytesleft The count of bytes left to be converted.
			 * \param outbuf The output buffer.
			 * \param outbytesleft The count of bytes left to output.
			 * \return The number of characters converted in a non-reversible way during the call. In case of error, ERROR_VALUE is returned and errno is set accordingly. See iconv() man page for details.
			 *
			 * This call cannot throw.
			 */
			size_t raw_convert(const char** inbuf, size_t* inbytesleft, char** outbuf, size_t* outbytesleft) const;

			/**
			 * \brief Reset the iconv context.
			 *
			 * Reset the iconv context, as specified in the iconv man page.
			 */
			void reset() const;

			/**
			 * \brief Proceed to a conversion.
			 * \param inbuf The input buffer.
			 * \param inbytesleft The count of bytes left to be converted.
			 * \param outbuf The output buffer.
			 * \param outbytesleft The count of bytes left to output.
			 * \param ec The error code, if an error occurs.
			 * \return The number of characters converted in a non-reversible way during the call. In case of error, ERROR_VALUE is returned and ec is updated to indicate the error.
			 *
			 * This call cannot throw.
			 */
			size_t convert(const char** inbuf, size_t* inbytesleft, char** outbuf, size_t* outbytesleft, boost::system::error_code& ec) const;

			/**
			 * \brief Proceed to a conversion.
			 * \param inbuf The input buffer.
			 * \param inbytesleft The count of bytes left to be converted.
			 * \param outbuf The output buffer.
			 * \param outbytesleft The count of bytes left to output.
			 * \return The number of characters converted in a non-reversible way during the call. In case of error, a boost::system::system_error is thrown.
			 */
			size_t convert(const char** inbuf, size_t* inbytesleft, char** outbuf, size_t* outbytesleft) const;

			/**
			 * \brief Convert some data using the specified iconv instance.
			 * \param in The input buffer. Cannot be NULL.
			 * \param in_len The length of in, in bytes.
			 * \param out The output buffer. Cannot be NULL.
			 * \param out_len The length of out, in bytes.
			 * \param non_reversible_conversions If not NULL, *non_reversible_conversions will be updated to indicate the count of non-reversible conversions performed during the call.
			 * \return The count of bytes written to out. In case of error, a boost::system::system_error is thrown.
			 *
			 * A reset() is performed inside the call, before the conversion takes place.
			 */
			size_t convert_all(const void* in, size_t in_len, void* out, size_t out_len, size_t* non_reversible_conversions = NULL) const;

			/**
			 * \brief Convert a string.
			 * \param ostr The string that receives the result.
			 * \param istr The string to convert.
			 * \param ec The error code, if an error occurs.
			 * \param non_reversible_conversions If not NULL, *non_reversible_conversions will be updated to indicate the count of non-reversible conversions performed during the call.
			 * \param chunk_size The size of the internal buffers to use, for the conversion. A good value is something near the expected result size.
			 * \return true on success. In case of error, false is returned and ec is updated to indicate the error.
			 */
			bool convert_string(std::string& ostr, const std::string& istr, boost::system::error_code& ec, size_t* non_reversible_conversions = NULL, size_t chunk_size = DEFAULT_CHUNK_SIZE) const;

			/**
			 * \brief Convert a string.
			 * \param istr The string to convert.
			 * \param non_reversible_conversions If not NULL, *non_reversible_conversions will be updated to indicate the count of non-reversible conversions performed during the call.
			 * \param chunk_size The size of the internal buffers to use, for the conversion. A good value is something near the expected result size.
			 * \return The converted string. In case of error, a boost::system::system_error is thrown.
			 */
			std::string convert_string(const std::string& istr, size_t* non_reversible_conversions = NULL, size_t chunk_size = DEFAULT_CHUNK_SIZE) const;

		private:

			void check_iconv() const;

			native_type m_iconv;
	};
	
	inline iconv_instance::iconv_instance(const char* to, const char* from) :
		m_iconv(::iconv_open(to, from))
	{
		check_iconv();
	}

	inline iconv_instance::iconv_instance(const std::string& to, const std::string& from) :
		m_iconv(::iconv_open(to.c_str(), from.c_str()))
	{
		check_iconv();
	}
	
	inline iconv_instance::~iconv_instance()
	{
		::iconv_close(m_iconv);
	}
	
	inline iconv_instance::native_type iconv_instance::raw() const
	{
		return m_iconv;
	}

	inline size_t iconv_instance::raw_convert(const char** inbuf, size_t* inbytesleft, char** outbuf, size_t* outbytesleft) const
	{
		return ::iconv(m_iconv, const_cast<char**>(inbuf), inbytesleft, outbuf, outbytesleft);
	}
	
	inline void iconv_instance::reset() const
	{
		::iconv(m_iconv, NULL, NULL, NULL, NULL);
	}
	
	inline void iconv_instance::check_iconv() const
	{
		if (m_iconv == reinterpret_cast<native_type>(-1))
		{
			throw std::runtime_error("Unknown encoding");
		}
	}
}

#endif /* ICONVPLUS_ICONV_INSTANCE_HPP */
