/**
 * \file upnp_device.cpp
 * \author Sebastien Vincent <sebastien.vincent@freelan.org>
 * \brief A UPnP device class.
 */

#include <boost/system/system_error.hpp>

#include <miniupnpc/upnpcommands.h>

#include "upnp_device.hpp"
#include "error.hpp"

namespace miniupnpcplus
{
  upnp_device::upnp_device(int timeout)
    : m_upnp(nullptr, freeUPNPDevlist)
  {
    int error = 0;
    char lan[256];
    char wan[256];

#if MINIUPNPC_API_VERSION < 14
    m_upnp.reset(upnpDiscover(timeout, nullptr, nullptr, 0, 0, &error));
#else
    m_upnp.reset(upnpDiscover(timeout, nullptr, nullptr, 0, 0, 2, &error));
#endif
    if(!m_upnp)
    {
      boost::system::error_code ec = boost::system::error_code(error,
          miniupnpcplus::miniupnpcplus_category());
      throw boost::system::system_error(ec);
    }

    int status = UPNP_GetValidIGD(m_upnp.get(), &m_upnp_urls, &m_upnp_data,
        lan, sizeof(lan));

    if(status != 1)
    {
      // no IGD gateway on LAN
      FreeUPNPUrls(&m_upnp_urls);
      boost::system::error_code ec = boost::system::error_code(-1,
          miniupnpcplus::miniupnpcplus_category());
      throw boost::system::system_error(ec);
    }

    m_lan_address = std::string(lan);

    error = UPNP_GetExternalIPAddress(m_upnp_urls.controlURL,
        m_upnp_data.first.servicetype, wan);

    if(error)
    {
      // problem retrieving WAN IP address
      FreeUPNPUrls(&m_upnp_urls);

      boost::system::error_code ec = boost::system::error_code(error,
          miniupnpcplus::miniupnpcplus_category());
      throw boost::system::system_error(ec);
    }

    m_wan_address = std::string(wan);
  }

  upnp_device::~upnp_device()
  {
    // unregister all mappings, do not use unregister_port_mapping here
    // because it will modify the list
    for(auto& mapping : m_mappings)
    {
      UPNP_DeletePortMapping(m_upnp_urls.controlURL,
        m_upnp_data.first.servicetype, mapping.external_port.c_str(),
        (mapping.protocol == TCP ? "TCP" : "UDP"), nullptr);
    }

    m_mappings.clear();
    FreeUPNPUrls(&m_upnp_urls);
  }

  void upnp_device::register_port_mapping(enum transport_protocol protocol,
      const std::string& external_port, const std::string& lan_port,
      const std::string& description)
  {
    // last parameters ("0" means the lease will be as long as possible
    int error = UPNP_AddPortMapping(m_upnp_urls.controlURL,
        m_upnp_data.first.servicetype, external_port.c_str(), lan_port.c_str(),
        m_lan_address.c_str(), description.c_str(),
        (protocol == TCP ? "TCP" : "UDP"), nullptr, "0");

    if(error)
    {
      boost::system::error_code ec = boost::system::error_code(error,
          miniupnpcplus::miniupnpcplus_category());
      throw boost::system::system_error(ec);
      struct upnp_mapping mapping;
    }

    struct upnp_mapping mapping;
    mapping.description = description;
    mapping.protocol = protocol;
    mapping.external_port = external_port;
    mapping.lan_port = lan_port;
    m_mappings.push_back(mapping);
  }

  void upnp_device::unregister_port_mapping(enum transport_protocol protocol,
      const std::string& external_port)
  {
    int error = UPNP_DeletePortMapping(m_upnp_urls.controlURL,
        m_upnp_data.first.servicetype, external_port.c_str(),
        (protocol == TCP ? "TCP" : "UDP"), nullptr);

    if(error)
    {
      boost::system::error_code ec = boost::system::error_code(error,
          miniupnpcplus::miniupnpcplus_category());
      throw boost::system::system_error(ec);
    }

    for(std::list<upnp_mapping>::iterator it = m_mappings.begin() ;
        it != m_mappings.end() ; ++it)
    {
      if(it->external_port == external_port &&
          it->protocol == protocol)
      {
        m_mappings.erase(it);
        break;
      }
    }
  }

  std::string upnp_device::get_external_ip() const
  {
    return m_wan_address;
  }

  std::string upnp_device::get_lan_ip() const
  {
    return m_lan_address;
  }
}

