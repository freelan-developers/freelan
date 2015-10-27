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
 * \file traits.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Type traits.
 */

#pragma once

#include <iostream>
#include <type_traits>

#include <boost/system/system_error.hpp>
#include <boost/system/error_code.hpp>
#include <boost/variant.hpp>
#include <boost/operators.hpp>

namespace freelan {

template <typename Type>
class HasFromString {
    public:
        static Type from_string(const std::string& str) {
            boost::system::error_code ec;

            const auto result = from_string(str, ec);

            if (ec) {
                throw boost::system::system_error(ec);
            }

            return result;
        }

        static Type from_string(const std::string& str, boost::system::error_code& ec) {
            std::istringstream iss(str);
            Type result;

            if (!Type::read_from(iss, result) || !iss.eof()) {
                ec = make_error_code(boost::system::errc::invalid_argument);

                return {};
            }

            return result;
        }
};

template <typename Type>
class HasToString {
    public:
        std::string to_string() const {
            std::ostringstream oss;

            static_cast<const Type*>(this)->write_to(oss);

            return oss.str();
        }
};

class WriteToVisitor : public boost::static_visitor<std::ostream&> {
    public:
        WriteToVisitor(std::ostream& os) :
            m_os(os)
        {}

        template <typename Any>
        result_type operator()(const Any& value) const {
            return value.write_to(m_os);
        }

    private:
        std::ostream& m_os;
};

template <typename Type>
class HasWriteTo {
    public:
        std::ostream& write_to(std::ostream& os) const {
            return boost::apply_visitor(WriteToVisitor(os), static_cast<const Type&>(*this));
        }
};

template <typename Type, typename... VariantTypes>
class HasReadFrom {
    public:
        static std::istream& read_from(std::istream& is, Type& value, std::string* buf = nullptr) {
            const std::ios::iostate state = is.rdstate();

            read_from_impl<VariantTypes...>(is, value, buf, state);

            return is;
        }
    private:
        template <typename VariantType, typename NextVariantType, typename... ExtraTypes>
        static bool read_from_impl(std::istream& is, Type& value, std::string* buf, const std::ios::iostate state) {
            if (read_from_subtype<VariantType>(is, value, buf)) {
                return true;
            } else {
                is.clear();
                is.setstate(state);

                return read_from_impl<NextVariantType, ExtraTypes...>(is, value, buf, state);
            }
        }

        template <typename VariantType>
        static bool read_from_impl(std::istream& is, Type& value, std::string* buf, const std::ios::iostate) {
            return read_from_subtype<VariantType>(is, value, buf);
        }

        template <typename VariantType>
        static bool read_from_subtype(std::istream& is, Type& value, std::string* buf) {
            VariantType sub_value;

            if (VariantType::read_from(is, sub_value, buf)) {
                value = sub_value;
                return true;
            }

            return false;
        }
};

template <typename Type>
class HasAccessors {
    public:
        template <typename VariantType>
        bool is() const { return this->as<VariantType>() != nullptr; }

        template <typename VariantType>
        const VariantType* as() const { return boost::get<VariantType>(static_cast<const Type*>(this)); }

        template <typename VariantType>
        VariantType* as() { return boost::get<VariantType>(static_cast<Type*>(this)); }
};

template <typename Type, typename... VariantTypes>
class GenericVariant : public HasFromString<Type>, public HasToString<Type>, public HasWriteTo<Type>, public HasReadFrom<Type, VariantTypes...>, public HasAccessors<Type>, public boost::operators<Type> {
};

/**
 * \brief Make sure the specified type has a to_string() const method.
 */
template <typename T>
class has_to_string {
    private:
        template <typename U, U>
        class check {};

        template <typename C>
        static char f(check<std::string (C::*)() const, &T::to_string>*);

        template <typename C>
        static char f(check<std::string (HasToString<C>::*)() const, &T::to_string>*);

        template <typename C>
        static long f(...);

    public:
        static const bool value = (sizeof(f<T>(0)) == sizeof(char));
};

/**
 * \brief Variant for std::enable_if that has a default value.
 */
template <bool B, typename TrueType, typename FalseType>
struct enable_if_else { typedef FalseType type; };

template <typename TrueType, typename FalseType>
struct enable_if_else<true, TrueType, FalseType> { typedef TrueType type; };

}
