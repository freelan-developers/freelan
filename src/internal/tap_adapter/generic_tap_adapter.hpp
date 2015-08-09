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
 * \file generic_tap_adapter.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A generic TAP adapter class.
 */

#pragma once

#include <boost/asio.hpp>
#include <boost/system/system_error.hpp>

#include <iostream>

#include "../types/ethernet_address.hpp"
#include "../types/tap_adapter_layer.hpp"

namespace freelan {

template <typename DescriptorType>
class GenericTapAdapter {
    public:

        template <typename MutableBufferSequence, typename ReadHandler>
        void async_read(const MutableBufferSequence& buffers, ReadHandler handler) {
            m_descriptor.async_read_some(buffers, handler);
        }

        template <typename ConstBufferSequence, typename WriteHandler>
        void async_write(const ConstBufferSequence& buffers, WriteHandler handler) {
            m_descriptor.async_write_some(buffers, handler);
        }

        template <typename MutableBufferSequence>
        size_t read(const MutableBufferSequence& buffers) {
            return m_descriptor.read_some(buffers);
        }

        template <typename MutableBufferSequence>
        size_t read(const MutableBufferSequence& buffers, boost::system::error_code& ec) {
            return m_descriptor.read_some(buffers, ec);
        }

        template <typename ConstBufferSequence>
        size_t write(const ConstBufferSequence& buffers) {
            return m_descriptor.write_some(buffers);
        }

        template <typename ConstBufferSequence>
        size_t write(const ConstBufferSequence& buffers, boost::system::error_code& ec) {
            return m_descriptor.write_some(buffers, ec);
        }

        void cancel() {
            m_descriptor.cancel();
        }

        void cancel(boost::system::error_code& ec) {
            m_descriptor.cancel(ec);
        }

        boost::asio::io_service& get_io_service() {
            return m_descriptor.get_io_service();
        }

        TapAdapterLayer layer() const { return m_layer; }
        const std::string& name() const { return m_name; }
        size_t mtu() const { return m_mtu; }
        const EthernetAddress& ethernet_address() const { return m_ethernet_address; }
        bool is_open() const { return m_descriptor.is_open(); }
        void close() { m_descriptor.close(); }
        boost::system::error_code close(boost::system::error_code& ec) { return m_descriptor.close(ec); }

    protected:

        GenericTapAdapter(boost::asio::io_service& _io_service, TapAdapterLayer _layer) :
            m_descriptor(_io_service),
            m_layer(_layer),
            m_name(),
            m_mtu(),
            m_ethernet_address()
        {}

        DescriptorType& descriptor() { return m_descriptor; }
        void set_name(const std::string& _name) { m_name = _name; }
        void set_mtu(size_t _mtu) { m_mtu = _mtu; }
        void set_ethernet_address(const EthernetAddress& _ethernet_address) { m_ethernet_address = _ethernet_address; }

    private:

        DescriptorType m_descriptor;
        TapAdapterLayer m_layer;
        std::string m_name;
        size_t m_mtu;
        EthernetAddress m_ethernet_address;

        friend std::ostream& operator<<(std::ostream& os, const GenericTapAdapter& value)
        {
            return os << value.name();
        }
};

}
