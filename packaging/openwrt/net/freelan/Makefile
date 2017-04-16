#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=freelan2
PKG_VERSION:=2014-06-22
PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=git://github.com/freelan-developers/freelan-all.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=22873e1c85
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

PKG_LICENSE:=GPLv3+
PKG_LICENSE_FILES:=

PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

PKG_MAINTAINER:=

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/freelan2
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=VPN
  DEPENDS:=+boost-date_time +boost-filesystem +boost-iostreams +boost-program_options +boost-system +boost-thread +kmod-tun +libopenssl +libstdcpp +libiconv +libcurl
  TITLE:=highly-configurable peer-to-peer VPN software
  URL:=http://www.freelan.org
endef

define Package/freelan2/description
	A free, open-source, multi-platform, highly-configurable and peer-to-peer VPN software, designed to easily connect remote hosts and mainly focused on security and performance.
endef

export PLATFORM=posix

SCONS_VARS = \
	ARCH=$(ARCH) \
	CXX=$(TARGET_CXX_NOCACHE) \
	CXXFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS) -I$(PKG_INSTALL_DIR)/usr/include" \
	FREELAN_NO_GIT=1 \
	FREELAN_NO_GIT_VERSION="$(PKG_VERSION)-$(PKG_SOURCE_VERSION)"

#        DESTDIR="$(PKG_INSTALL_DIR)"

define Build/Configure
endef

define Build/Compile
	cd $(PKG_BUILD_DIR); $(SCONS_VARS) scons --mode=release --prefix=$(PKG_INSTALL_DIR)/usr install
endef

define Package/freelan2/install
	$(INSTALL_DIR) \
		$(1)/usr/sbin \
		$(1)/etc/freelan2 \
		$(1)/etc/init.d

	# copying binary
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/freelan2 \
		$(1)/usr/sbin/freelan2

	# copying configuration
	#$(INSTALL_CONF) \
	#	$(PKG_BUILD_DIR)/freelan/config/* \
	#	$(1)/etc/freelan2

	# creating service
	$(INSTALL_BIN) \
		files/freelan2.init \
		$(1)/etc/init.d/freelan2
endef

define Package/freelan2/conffiles
  /etc/freelan2
endef

$(eval $(call BuildPackage,freelan2))