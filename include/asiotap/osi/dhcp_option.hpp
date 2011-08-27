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
 * \file dhcp_option.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A DHCP option class.
 */

#ifndef ASIOTAP_OSI_DHCP_OPTION_HPP
#define ASIOTAP_OSI_DHCP_OPTION_HPP

#include <boost/asio.hpp>

namespace asiotap
{
	namespace osi
	{
		/**
		 * \brief A DHCP option class.
		 */
		class dhcp_option
		{
			public:

				/**
				 * \brief The DHCP option.
				 */
				enum dhcp_option_tag
				{
					pad = 																						0x00,
					subnet_mask =																			0x01,
					time_offset =																			0x02,
					router =																					0x03,
					time_server =																			0x04,
					name_server = 																		0x05,
					domain_name_server =															0x06,
					log_server = 																			0x07,
					quote_server =																		0x08,
					lpr_server =																			0x09,
					impress_serve =																		0x0a,
					resource_location_server =												0x0b,
					host_name =																				0x0c,
					boot_file_size =																	0x0d,
					merit_dump_file =																	0x0e,
					domain_name =																			0x0f,
					swap_server =																			0x10,
					root_path =																				0x11,
					extensions_path =																	0x12,
					ip_forwarding =																		0x13,
					non_local_source_routing =												0x14,
					policy_filter =																		0x15,
					maximum_datagram_reassembly_size =								0x16,
					default_ip_ttl =																	0x17,
					path_mtu_aging_timeout =													0x18,
					path_mtu_plateau_table =													0x19,
					interface_mtu =																		0x1a,
					all_subnets_are_local =														0x1b,
					broadcast_address =																0x1c,
					perform_mask_discovery =													0x1d,
					mask_supplier =																		0x1e,
					perform_router_discovery =												0x1f,
					router_solicitation_address =											0x20,
					static_routing_table =														0x21,
					trailer_encapsulation =														0x22,
					arp_cache_timeout =																0x23,
					ethernet_encapsulation =													0x24,
					default_tcp_ttl =																	0x25,
					tcp_keepalive_internal =													0x26,
					tcp_keepalive_garbage =														0x27,
					network_information_service_domain =							0x28,
					network_information_service_servers =							0x29,
					ntp_servers =																			0x2a,
					vendor_specific =																	0x2b,
					netbios_over_tcpip_name_server =									0x2c,
					netbios_over_tcpip_datagram_distribution_server =	0x2d,
					netbios_over_tcpip_node_type =										0x2e,
					netbios_over_tcpip_scope =												0x2f,
					x_window_system_font_server =											0x30,
					x_window_system_display_manager =									0x31,
					requested_ip_address =														0x32,
					ip_address_lease_time =														0x33,
					option_overload =																	0x34,
					dhcp_message_type =																0x35,
					server_identifier =																0x36,
					parameter_request_list =													0x37,
					message =																					0x38,
					maximum_dhcp_message_size =												0x39,
					renew_time_value =																0x3a,
					rebinding_time_value =														0x3b,
					class_identifier =																0x3c,
					client_identifier =																0x3d,
					netware_ip_domain_name =													0x3e,
					netware_ip_information =													0x3f,
					network_information_service_plus_domain =					0x40,
					network_information_service_plus_servers =				0x41,
					tftp_server_name =																0x42,
					bootfile_name =																		0x43,
					mobile_ip_home_agent =														0x44,
					smtp_server =																			0x45,
					pop_server =																			0x46,
					nntp_server =																			0x47,
					default_www_server =															0x48,
					default_finger_server	=														0x49,
					default_irc_server =															0x4a,
					streettalk_server =																0x4b,
					streettalk_directory_assistance_server =					0x4c,
					user_clas_information =														0x4d,
					slp_directory_agent =															0x4e,
					slp_service_scope =																0x4f,
					rapid_commit =																		0x50,
					fully_qualified_domain_name =											0x51,
					relay_agent_information =													0x52,
					internet_storage_name_service =										0x53,
					nds_servers =																			0x55,
					nds_tree_name =																		0x56,
					nds_context =																			0x57,
					bcmcs_controller_domain_name_list =								0x58,
					bcmcs_controller_ipv4_address_list =							0x59,
					authentication =																	0x5a,
					client_last_transaction_time =										0x5b,
					associated_ip =																		0x5c,
					client_system_architecture_type =									0x5d,
					client_network_interface_identifier =							0x5e,
					ldap =																						0x5f,
					client_machine_identifier =												0x61,
					open_group_user_authentication =									0x62,
					geoconf_civic =																		0x63,
					ieee_1003_1_tz_string =														0x64,
					reference_to_the_tz_database =										0x65,
					netinfo_parent_server_address =										0x70,
					netinfo_parent_server_tag =												0x71,
					url =																							0x72,
					auto_configure =																	0x74,
					name_service_search =															0x75,
					subnet_selection =																0x76,
					dns_domain_search_list =													0x77,
					sip_servers =																			0x78,
					classless_static_route =													0x79,
					cablelabs_client_configuration =									0x7a,
					geoconf =																					0x7b,
					vendor_identifying_vendor_class =									0x7c,
					vendor_identifying_vendor_specific =							0x7d,
					tftp_server_ip_address =													0x80,
					call_server_ip_address =													0x81,
					discrimination_string =														0x82,
					remote_statistics_server_ip_address =							0x83,
					_802_1p_vlan_id =																	0x84,
					_802_1q_l2_priority =															0x85,
					diffserv_code_point =															0x86,
					http_proxy_for_phone_specific_applications =			0x87,
					pana_authentication_agent =												0x88,
					lost_server =																			0x89,
					capwap_access_controller_address =								0x8a,
					ipv4_address_mos =																0x8b,
					ipv4_fqdn_mos =																		0x8c,
					sip_ua_configuration_service_domains =						0x8d,
					ipv4_address_andsf =															0x8e,
					ipv4_fqdn_andsf =																	0x8f,
					tftp_server_address =															0x96,
					etherboot =																				0xaf,
					ip_telephone =																		0xb0,
					packet_cable =																		0xb1,
					pxelinux_magic =																	0xd0,
					pxelinux_configfile =															0xd1,
					pxelinux_pathprefix =															0xd2,
					pxelinux_reboottime =															0xd3,
					_6rd =																						0xd4,
					v4_access_domain =																0xd5,
					subnet_allocation =																0xdc,
					virtual_subnet_selection =												0xdd,
					end =																							0xff
				};

				/**
				 * \brief Check if the specified DHCP option code has a length.
				 * \param option_tag The DHCP option tag.
				 * \return true if it has a length, false otherwise.
				 */
				static bool has_length(dhcp_option_tag option_tag);

			private:

				static const uint8_t has_length_map[256];
				dhcp_option();
		};

		inline bool dhcp_option::has_length(dhcp_option::dhcp_option_tag option_tag)
		{
			return (has_length_map[static_cast<uint8_t>(option_tag)] != 0);
		}

		inline dhcp_option::dhcp_option()
		{
		}
	}
}

#endif /* ASIOTAP_OSI_DHCP_OPTION_HPP */

