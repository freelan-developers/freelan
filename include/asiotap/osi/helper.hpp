/*
 * libasiotap - A portable TAP adapter extension for Boost::ASIO.
 * Copyright (C) 2010-2011 Julien Kauffmann <julien.kauffmann@freelan.org>
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
		 * \brief A base const helper class.
		 */
		template <typename OSIFrameType>
		class _base_const_helper
		{
			public:

				/**
				 * \brief The frame type.
				 */
				typedef OSIFrameType frame_type;

				/**
				 * \brief Get the associated frame.
				 * \return The associated frame.
				 */
				const OSIFrameType& frame() const;

				/**
				 * \brief Get the underlying buffer.
				 * \return The underlying buffer.
				 */
				boost::asio::const_buffer buffer() const;

			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param buf The buffer to refer to.
				 */
				_base_const_helper(boost::asio::const_buffer buf);

			private:

				const boost::asio::const_buffer m_buf;
		};

		/**
		 * \brief A base mutable helper class.
		 */
		template <typename OSIFrameType>
		class _base_mutable_helper
		{
			public:

				/**
				 * \brief The frame type.
				 */
				typedef OSIFrameType frame_type;

				/**
				 * \brief Get the associated frame.
				 * \return The associated frame.
				 */
				OSIFrameType& frame() const;

				/**
				 * \brief Get the underlying buffer.
				 * \return The underlying buffer.
				 */
				boost::asio::mutable_buffer buffer() const;

			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param buf The buffer to refer to.
				 */
				_base_mutable_helper(boost::asio::mutable_buffer buf);

			private:

				const boost::asio::mutable_buffer m_buf;
		};

		/**
		 * \brief The implementation type for const helpers.
		 */
		template <typename OSIFrameType>
		class _const_helper_impl : public _base_const_helper<OSIFrameType>
		{
		};

		/**
		 * \brief The implementation type for mutable helpers.
		 */
		template <typename OSIFrameType>
		class _mutable_helper_impl : public _base_mutable_helper<OSIFrameType>
		{
		};

		/**
		 * \brief A const helper class.
		 */
		template <typename OSIFrameType>
		class const_helper : public _const_helper_impl<OSIFrameType>
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
		class mutable_helper : public _mutable_helper_impl<OSIFrameType>
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
		 * \brief Check if a frame is valid.
		 * \param frame The frame.
		 * \return true on success.
		 */
		template <typename OSIFrameType>
		bool check_frame(mutable_helper<OSIFrameType> frame);

		/**
		 * \brief Check if a frame is valid.
		 * \param frame The frame.
		 * \return true on success.
		 */
		template <typename OSIFrameType>
		bool check_frame(const_helper<OSIFrameType> frame);

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

		template <typename OSIFrameType>
		inline const OSIFrameType& _base_const_helper<OSIFrameType>::frame() const
		{
			return *boost::asio::buffer_cast<const OSIFrameType*>(m_buf);
		}

		template <typename OSIFrameType>
		inline boost::asio::const_buffer _base_const_helper<OSIFrameType>::buffer() const
		{
			return m_buf;
		}

		template <typename OSIFrameType>
		inline _base_const_helper<OSIFrameType>::_base_const_helper(boost::asio::const_buffer buf) :
			m_buf(buf)
		{
			if (boost::asio::buffer_size(buf) < sizeof(OSIFrameType))
			{
				throw std::length_error("buf");
			}
		}

		template <typename OSIFrameType>
		inline OSIFrameType& _base_mutable_helper<OSIFrameType>::frame() const
		{
			return *boost::asio::buffer_cast<OSIFrameType*>(m_buf);
		}

		template <typename OSIFrameType>
		inline boost::asio::mutable_buffer _base_mutable_helper<OSIFrameType>::buffer() const
		{
			return m_buf;
		}

		template <typename OSIFrameType>
		inline _base_mutable_helper<OSIFrameType>::_base_mutable_helper(boost::asio::mutable_buffer buf) :
			m_buf(buf)
		{
			if (boost::asio::buffer_size(buf) < sizeof(OSIFrameType))
			{
				throw std::length_error("buf");
			}
		}

		template <typename OSIFrameType>
		inline const_helper<OSIFrameType>::const_helper(boost::asio::const_buffer buf) :
			_const_helper_impl<OSIFrameType>(buf)
		{
			if (!check_frame(*this))
			{
				throw std::domain_error("buf");
			}
		}

		template <typename OSIFrameType>
		inline const_helper<OSIFrameType>::operator const OSIFrameType&() const
		{
			return const_helper<OSIFrameType>::frame();
		}

		template <typename OSIFrameType>
		inline mutable_helper<OSIFrameType>::mutable_helper(boost::asio::mutable_buffer buf) :
			_mutable_helper_impl<OSIFrameType>(buf)
		{
			if (!check_frame(*this))
			{
				throw std::domain_error("buf");
			}
		}

		template <typename OSIFrameType>
		inline mutable_helper<OSIFrameType>::operator const_helper<OSIFrameType>() const
		{
			return const_helper<OSIFrameType>(mutable_helper<OSIFrameType>::frame());
		}

		template <typename OSIFrameType>
		inline mutable_helper<OSIFrameType>::operator OSIFrameType&() const
		{
			return mutable_helper<OSIFrameType>::frame();
		}

		template <typename OSIFrameType>
		inline bool check_frame(mutable_helper<OSIFrameType> frame)
		{
			return check_frame(const_helper<OSIFrameType>(frame));
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

