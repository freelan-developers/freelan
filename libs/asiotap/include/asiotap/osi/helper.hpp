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
 * \file helper.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An OSI helper class.
 */

#ifndef ASIOTAP_OSI_HELPER_HPP
#define ASIOTAP_OSI_HELPER_HPP

#include <boost/asio.hpp>

namespace asiotap
{
	namespace osi
	{
		/**
		 * \brief The const helper tag.
		 */
		class const_helper_tag;

		/**
		 * \brief The mutable helper tag.
		 */
		class mutable_helper_tag;

		/**
		 * \brief The helper buffer type.
		 */
		template <class HelperTag>
		struct helper_buffer;

		/**
		 * \brief The const helper buffer type.
		 */
		template <>
		struct helper_buffer<const_helper_tag>
		{
			/**
			 * \brief The buffer type.
			 */
			typedef boost::asio::const_buffer type;
		};

		/**
		 * \brief The mutable helper buffer type.
		 */
		template <>
		struct helper_buffer<mutable_helper_tag>
		{
			/**
			 * \brief The buffer type.
			 */
			typedef boost::asio::mutable_buffer type;
		};

		/**
		 * \brief A base helper class.
		 */
		template <class HelperTag, typename OSIFrameType>
		class _generic_base_helper
		{
			public:

				/**
				 * \brief The helper tag.
				 */
				typedef HelperTag helper_tag;

				/**
				 * \brief The buffer type.
				 */
				typedef typename helper_buffer<helper_tag>::type buffer_type;

				/**
				 * \brief The frame type.
				 */
				typedef OSIFrameType frame_type;

				/**
				 * \brief Get the underlying buffer.
				 * \return The underlying buffer.
				 */
				buffer_type buffer() const;

			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param buf The buffer to refer to.
				 */
				_generic_base_helper(buffer_type buf);

			private:

				buffer_type m_buf;
		};

		/**
		 * \brief A base helper class.
		 */
		template <class HelperTag, typename OSIFrameType>
		class _base_helper;

		/**
		 * \brief A base const helper class.
		 */
		template <typename OSIFrameType>
		class _base_helper<const_helper_tag, OSIFrameType> : public _generic_base_helper<const_helper_tag, OSIFrameType>
		{
			public:

				/**
				 * \brief Get the associated frame.
				 * \return The associated frame.
				 */
				const OSIFrameType& frame() const;

			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param buf The buffer to refer to.
				 */
				_base_helper(typename _base_helper::buffer_type buf);
		};

		/**
		 * \brief A base mutable helper class.
		 */
		template <typename OSIFrameType>
		class _base_helper<mutable_helper_tag, OSIFrameType> : public _generic_base_helper<mutable_helper_tag, OSIFrameType>
		{
			public:

				/**
				 * \brief Get the associated frame.
				 * \return The associated frame.
				 */
				OSIFrameType& frame() const;

			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param buf The buffer to refer to.
				 */
				_base_helper(typename _base_helper::buffer_type buf);
		};

		/**
		 * \brief The base implementation type for helpers.
		 */
		template <class HelperTag, typename OSIFrameType>
		class _base_helper_impl;

		/**
		 * \brief The implementation type for helpers.
		 */
		template <class HelperTag, typename OSIFrameType>
		class _helper_impl : public _base_helper_impl<HelperTag, OSIFrameType>
		{
			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param buf The buffer to refer to.
				 */
				_helper_impl(typename _helper_impl::buffer_type buf);
		};

		/**
		 * \brief A const helper class.
		 */
		template <typename OSIFrameType>
		class const_helper : public _helper_impl<const_helper_tag, OSIFrameType>
		{
			public:

				/**
				 * \brief The constructor.
				 * \param buf The buffer to refer to.
				 */
				const_helper(boost::asio::const_buffer buf);

				/**
				 * \brief Convert to the referenced type.
				 * \return The referenced type.
				 */
				operator const OSIFrameType&() const;
		};

		/**
		 * \brief A mutable helper class.
		 */
		template <typename OSIFrameType>
		class mutable_helper : public _helper_impl<mutable_helper_tag, OSIFrameType>
		{
			public:

				/**
				 * \brief The constructor.
				 * \param buf The buffer to refer to.
				 */
				mutable_helper(boost::asio::mutable_buffer buf);

				/**
				 * \brief Convert to a const_helper.
				 * \return A const_helper.
				 */
				operator const_helper<OSIFrameType>() const;

				/**
				 * \brief Convert to the referenced type.
				 * \return The referenced type.
				 */
				operator OSIFrameType&() const;
		};

		/**
		 * \brief Create a helper from a buffer.
		 * \param buf The buffer.
		 * \return The helper.
		 */
		template <typename OSIFrameType>
		const_helper<OSIFrameType> helper(boost::asio::const_buffer buf);

		/**
		 * \brief Create a helper from a buffer.
		 * \param buf The buffer.
		 * \return The helper.
		 */
		template <typename OSIFrameType>
		mutable_helper<OSIFrameType> helper(boost::asio::mutable_buffer buf);

		template <class HelperTag, typename OSIFrameType>
		inline typename _generic_base_helper<HelperTag, OSIFrameType>::buffer_type _generic_base_helper<HelperTag, OSIFrameType>::buffer() const
		{
			return m_buf;
		}

		template <class HelperTag, typename OSIFrameType>
		inline _generic_base_helper<HelperTag, OSIFrameType>::_generic_base_helper(buffer_type buf) :
			m_buf(buf)
		{
			if (boost::asio::buffer_size(buf) < sizeof(OSIFrameType))
			{
				throw std::length_error("buf");
			}
		}

		template <typename OSIFrameType>
		inline const OSIFrameType& _base_helper<const_helper_tag, OSIFrameType>::frame() const
		{
			return *boost::asio::buffer_cast<const OSIFrameType*>(_base_helper<const_helper_tag, OSIFrameType>::buffer());
		}

		template <typename OSIFrameType>
		inline _base_helper<const_helper_tag, OSIFrameType>::_base_helper(typename _base_helper<const_helper_tag, OSIFrameType>::buffer_type buf) : _generic_base_helper<const_helper_tag, OSIFrameType>(buf)
		{
		}

		template <typename OSIFrameType>
		inline OSIFrameType& _base_helper<mutable_helper_tag, OSIFrameType>::frame() const
		{
			return *boost::asio::buffer_cast<OSIFrameType*>(_base_helper<mutable_helper_tag, OSIFrameType>::buffer());
		}

		template <typename OSIFrameType>
		inline _base_helper<mutable_helper_tag, OSIFrameType>::_base_helper(typename _base_helper<mutable_helper_tag, OSIFrameType>::buffer_type buf) : _generic_base_helper<mutable_helper_tag, OSIFrameType>(buf)
		{
		}

		template <typename HelperTag, typename OSIFrameType>
		inline _helper_impl<HelperTag, OSIFrameType>::_helper_impl(typename _helper_impl::buffer_type buf) :
			_base_helper_impl<HelperTag, OSIFrameType>(buf)
		{
		}

		template <typename OSIFrameType>
		inline const_helper<OSIFrameType>::const_helper(boost::asio::const_buffer buf) :
			_helper_impl<const_helper_tag, OSIFrameType>(buf)
		{
		}

		template <typename OSIFrameType>
		inline const_helper<OSIFrameType>::operator const OSIFrameType&() const
		{
			return const_helper<OSIFrameType>::frame();
		}

		template <typename OSIFrameType>
		inline mutable_helper<OSIFrameType>::mutable_helper(boost::asio::mutable_buffer buf) :
			_helper_impl<mutable_helper_tag, OSIFrameType>(buf)
		{
		}

		template <typename OSIFrameType>
		inline mutable_helper<OSIFrameType>::operator const_helper<OSIFrameType>() const
		{
			return const_helper<OSIFrameType>(mutable_helper<OSIFrameType>::buffer());
		}

		template <typename OSIFrameType>
		inline mutable_helper<OSIFrameType>::operator OSIFrameType&() const
		{
			return mutable_helper<OSIFrameType>::frame();
		}

		template <typename OSIFrameType>
		inline const_helper<OSIFrameType> helper(boost::asio::const_buffer buf)
		{
			return const_helper<OSIFrameType>(buf);
		}

		template <typename OSIFrameType>
		inline mutable_helper<OSIFrameType> helper(boost::asio::mutable_buffer buf)
		{
			return mutable_helper<OSIFrameType>(buf);
		}
	}
}

#endif /* ASIOTAP_OSI_HELPER_HPP */

