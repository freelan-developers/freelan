/*
 * libiconvplus - A C++ lightweight wrapper around the iconv library.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libiconvplus.
 *
 * libiconvplus is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libiconvplus is distributed in the hope that it will be useful, but
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
 * If you intend to use libiconvplus in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file converter.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The iconv converter class.
 */

#ifndef ICONVPLUS_CONVERTER_HPP
#define ICONVPLUS_CONVERTER_HPP

#include "iconv_instance.hpp"

#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

#include <iostream>
#include <vector>
#include <cassert>

namespace iconvplus
{
	/**
	 * \brief An input stream type.
	 */
	template <typename CharType>
	struct input_stream;

	/**
	 * \brief A char-specialized input stream type.
	 */
	template <>
	struct input_stream<char>
	{
		typedef std::istream type; /**< Input stream type. */
	};

	/**
	 * \brief A wchar_t-specialized input stream type.
	 */
	template <>
	struct input_stream<wchar_t>
	{
		typedef std::wistream type; /**< Input stream type. */
	};

	/**
	 * \brief An output stream type.
	 */
	template <typename CharType>
	struct output_stream;

	/**
	 * \brief A char-specialized output stream type.
	 */
	template <>
	struct output_stream<char>
	{
		typedef std::ostream type; /**< Output stream type. */
	};

	/**
	 * \brief A wchar_t-specialized output stream type.
	 */
	template <>
	struct output_stream<wchar_t>
	{
		typedef std::wostream type; /**< Output stream type. */
	};

	/**
	 * \brief A converter class.
	 */
	template <typename InputCharType, typename OutputCharType>
	class converter
	{
		public:

			/**
			 * \brief The default size of chunks, in bytes.
			 */
			static const size_t DEFAULT_CHUNK_SIZE = 8192;

			/**
			 * \brief The input char type.
			 */
			typedef InputCharType input_char_type;

			/**
			 * \brief The output char type.
			 */
			typedef OutputCharType output_char_type;

			/**
			 * \brief The input stream type.
			 */
			typedef typename input_stream<input_char_type>::type input_stream_type;

			/**
			 * \brief The onput stream type.
			 */
			typedef typename output_stream<output_char_type>::type output_stream_type;

			/**
			 * \brief Create a new converter.
			 * \param ic The iconv instance to use.
			 * \param chunk_size The chunk size, in bytes. Must be at least max(sizeof(InputCharType), sizeof(OutputCharType)).
			 * \see set_iconv_instance
			 *
			 * A reference is kept on the specified iconv_instance, so it must remain valid during all the lifetime of the converter instance or until set_iconv_instance() is called.
			 */
			converter(const iconv_instance& ic, size_t chunk_size = DEFAULT_CHUNK_SIZE);

			/**
			 * \brief Set another iconv_instance to use.
			 * \param ic The new iconv instance to use.
			 *
			 * A reference is kept on the specified iconv_instance, so it must remain valid during all the lifetime of the converter instance or until set_iconv_instance() is called again.
			 */
			void set_iconv_instance(const iconv_instance& ic);

			/**
			 * \brief Proceed to the conversion.
			 * \param is The input stream.
			 * \param os The output stream.
			 * \param ec The error code, if an error occurs.
			 * \param non_reversible_conversions If not NULL, *non_reversible_conversions will be updated to indicate the count of non-reversible conversions performed during the call.
			 * \return true on success. On error, ec is updated to indicate the error.
			 */
			bool convert(input_stream_type& is, output_stream_type& os, boost::system::error_code& ec, size_t* non_reversible_conversions = NULL);

			/**
			 * \brief Proceed to the conversion.
			 * \param is The input stream.
			 * \param os The output stream.
			 * \param non_reversible_conversions If not NULL, *non_reversible_conversions will be updated to indicate the count of non-reversible conversions performed during the call.
			 *
			 * On error, a boost::system::system_error is thrown.
			 */
			void convert(input_stream_type& is, output_stream_type& os, size_t* non_reversible_conversions = NULL);

		private:

			const iconv_instance* m_iconv_instance;
			std::vector<input_char_type> m_input_buffer;
			std::vector<output_char_type> m_output_buffer;
	};

	template <typename InputCharType, typename OutputCharType>
	inline converter<InputCharType, OutputCharType>::converter(const iconv_instance& ic, size_t chunk_size) :
		m_iconv_instance(&ic),
		m_input_buffer(chunk_size / sizeof(input_char_type)),
		m_output_buffer(chunk_size / sizeof(output_char_type))
	{
		assert(chunk_size > 0);
		assert(m_input_buffer.size() > 0);
		assert(m_output_buffer.size() > 0);
	}

	template <typename InputCharType, typename OutputCharType>
	inline void converter<InputCharType, OutputCharType>::set_iconv_instance(const iconv_instance& ic)
	{
		m_iconv_instance = &ic;
	}

	template <typename InputCharType, typename OutputCharType>
	inline void converter<InputCharType, OutputCharType>::convert(input_stream_type& is, output_stream_type& os, size_t* non_reversible_conversions)
	{
		boost::system::error_code ec;

		if (!convert(is, os, ec, non_reversible_conversions))
		{
			throw boost::system::system_error(ec);
		}
	}
}

#endif /* ICONVPLUS_CONVERTER_HPP */
