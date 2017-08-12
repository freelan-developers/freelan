/*
 * libminiupnpcplus - C++ portable miniupnpc wrapper library.
 * Copyright (C) 2010-2017 Julien Kauffmann <julien.kauffmann@freelan.org>
 *
 * This file is part of libminiupnpcplus.
 *
 * libfscp is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libfscp is distributed in the hope that it will be useful, but
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
 * If you intend to use libfscp in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file upnp_device.hpp
 * \author Sebastien Vincent <sebastien.vincent@freelan.org>
 * \brief A UPnP device class.
 */

#ifndef MINIUPNPCPLUS_UPNP_DEVICE_HPP
#define MINIUPNPCPLUS_UPNP_DEVICE_HPP

#include <memory>
#include <string>
#include <list>

#include <boost/noncopyable.hpp>

#include <miniupnpc/miniupnpc.h>

namespace miniupnpcplus
{
  /**
   * \enum transport_protocol
   * \brief Transport protocol.
   */
  enum transport_protocol
  {
    UDP,
    TCP
  };

  /**
   * \brief UPnP mapping information.
   */
  struct upnp_mapping
  {
    enum transport_protocol protocol;
    std::string external_port;
    std::string lan_port;
    std::string description;
  };

  /**
   * \class upnp_device
   * \brief UPnP device.
   */
  class upnp_device : public boost::noncopyable
  {
    public:
      /**
       * \brief Constructor.
       * \param timeout UPnP discovery timeout in milliseconds.
       */
      upnp_device(int timeout);

      /**
       * \brief Destructor.
       */
      ~upnp_device();

      /**
       * \brief Registers a port mapping to UPnP IGD.
       * \param protocol protocol (TCP or UDP).
       * \param external_port the external port wanted.
       * \param lan_port the local port.
       * \param description a description.
       */
      void register_port_mapping(enum transport_protocol protocol,
          const std::string& external_port, const std::string& lan_port,
          const std::string& description);

      /**
       * \brief Unregisters a port mapping from UPnP IGD.
       * \param protocol protocol (TCP or UDP).
       * \param external_port the external port wanted.
       */
      void unregister_port_mapping(enum transport_protocol protocol, 
          const std::string& external_port);

      /**
       * \brief Returns external (WAN) IP address.
       * \return external (WAN) IP address.
       */
      std::string get_external_ip() const;

      /**
       * \brief Returns LAN IP address.
       * \return LAN IP address.
       */
      std::string get_lan_ip() const;

    private:
      /**
       * \brief Underlying miniupnpc library pointer.
       */
      std::unique_ptr<UPNPDev, decltype(&freeUPNPDevlist)> m_upnp;

      /**
       * \brief UPnP data.
       */
      struct IGDdatas m_upnp_data;

      /**
       * \brief UPnP URLs.
       */
      struct UPNPUrls m_upnp_urls;

      /**
       * \brief LAN address.
       */
      std::string m_lan_address;
      
      /**
       * \brief External WAN address.
       */
      std::string m_wan_address;

      /**
       * \brief List of UPnP mappings.
       */
      std::list<struct upnp_mapping> m_mappings;
   };
} /* namespace miniupnpcplus */

#endif /* MINIUPNPCPLUS_UPNP_DEVICE_HPP */

