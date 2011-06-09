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

			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param frame The frame to refer to.
				 */
				_base_const_helper(const OSIFrameType& frame);

			private:

				const OSIFrameType& m_frame;
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

			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param frame The frame to refer to.
				 */
				_base_mutable_helper(OSIFrameType& frame);

			private:

				OSIFrameType& m_frame;
		};

		/**
		 * \brief The implementation type for const helpers.
		 */
		template <typename OSIFrameType>
		class _const_helper_impl : public _base_const_helper<OSIFrameType>
		{
			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param frame The frame to refer to.
				 */
				_const_helper_impl(const OSIFrameType& frame);
		};

		/**
		 * \brief The implementation type for mutable helpers.
		 */
		template <typename OSIFrameType>
		class _mutable_helper_impl : public _base_mutable_helper<OSIFrameType>
		{
			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param frame The frame to refer to.
				 */
				_mutable_helper_impl(OSIFrameType& frame);
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
				 * \param frame The frame to refer to.
				 */
				const_helper(const OSIFrameType& frame);

				/**
				 * \brief The constructor.
				 * \param frame The frame to refer to.
				 */
				const_helper(const OSIFrameType* frame);

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
		class mutable_helper : public _base_mutable_helper<OSIFrameType>
		{
			public:

				/**
				 * \brief The constructor.
				 * \param frame The frame to refer to.
				 */
				mutable_helper(OSIFrameType& frame);

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
		 * \brief Create a helper from a frame.
		 * \param frame The frame.
		 * \return The helper.
		 */
		template <typename OSIFrameType>
		const_helper<OSIFrameType> helper(const OSIFrameType& frame);

		/**
		 * \brief Create a helper from a frame.
		 * \param frame The frame.
		 * \return The helper.
		 */
		template <typename OSIFrameType>
		mutable_helper<OSIFrameType> helper(OSIFrameType& frame);

		template <typename OSIFrameType>
		inline const OSIFrameType& _base_const_helper<OSIFrameType>::frame() const
		{
			return m_frame;
		}

		template <typename OSIFrameType>
		inline _base_const_helper<OSIFrameType>::_base_const_helper(const OSIFrameType& _frame) :
			m_frame(_frame)
		{
		}

		template <typename OSIFrameType>
		inline OSIFrameType& _base_mutable_helper<OSIFrameType>::frame() const
		{
			return m_frame;
		}
		
		template <typename OSIFrameType>
		inline _base_mutable_helper<OSIFrameType>::_base_mutable_helper(OSIFrameType& _frame) :
			m_frame(_frame)
		{
		}
		
		template <typename OSIFrameType>
		inline _const_helper_impl<OSIFrameType>::_const_helper_impl(const OSIFrameType& _frame) :
			_base_const_helper<OSIFrameType>(_frame)
		{
		}

		template <typename OSIFrameType>
		inline _mutable_helper_impl<OSIFrameType>::_mutable_helper_impl(OSIFrameType& _frame) :
			_base_mutable_helper<OSIFrameType>(_frame)
		{
		}

		template <typename OSIFrameType>
		inline const_helper<OSIFrameType>::const_helper(const OSIFrameType& _frame) :
			_const_helper_impl<OSIFrameType>(_frame)
		{
		}

		template <typename OSIFrameType>
		inline const_helper<OSIFrameType>::operator const OSIFrameType&() const
		{
			return const_helper<OSIFrameType>::frame();
		}

		template <typename OSIFrameType>
		inline mutable_helper<OSIFrameType>::mutable_helper(OSIFrameType& _frame) :
			_mutable_helper_impl<OSIFrameType>(_frame)
		{
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
		inline const_helper<OSIFrameType> helper(const OSIFrameType& frame)
		{
			return const_helper<OSIFrameType>(frame);
		}

		template <typename OSIFrameType>
		inline mutable_helper<OSIFrameType> helper(OSIFrameType& frame)
		{
			return mutable_helper<OSIFrameType>(frame);
		}
	}
}

#endif /* ASIOTAP_OSI_HELPER_HPP */

