#
# Copyright (C) 2007-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=freelan
PKG_VERSION:=2013-02-20
PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=git://github.com/freelan-developers/freelan-all.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=97b07df88267ba12e42111be261de5cf188f2a70
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

PKG_LICENSE:=GPLv3+
PKG_LICENSE_FILES:=

PKG_MAINTAINER:=Etienne CHAMPETIER <etienne.champetier <at> free.fr>

include $(INCLUDE_DIR)/package.mk

define Package/freelan
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=VPN
  DEPENDS:=+libfreelan +boost-filesystem +boost-program_options 
  TITLE:=highly-configurable peer-to-peer VPN software
  URL:=http://www.freelan.org
endef

define Package/freelan/description
    A free, open-source, multi-platform, highly-configurable and peer-to-peer VPN software, designed to easily connect remote hosts and mainly focused on security and performance.
endef

define Package/freelanlibs/Default
  SECTION:=libs
  CATEGORY:=Libraries
  URL:=http://www.freelan.org
endef

define Package/libasiotap
  $(call Package/freelanlibs/Default)
  TITLE:=TAP adapters C++ wrapper (FreeLAN)
  DEPENDS:= <at> !USE_UCLIBC +kmod-tun +boost-system +librt +libpthread
endef

define Package/libcryptoplus
  $(call Package/freelanlibs/Default)
  TITLE:=OpenSSL C++ wrapper (FreeLAN)
  DEPENDS:=+libopenssl +libstdcpp
endef

define Package/libfreelan
  $(call Package/freelanlibs/Default)
  TITLE:=The FreeLAN C++ library
  DEPENDS:=+libasiotap +libfscp +boost-thread +libcurl +libkfather
endef

define Package/libfscp
  $(call Package/freelanlibs/Default)
  TITLE:=FreeLAN Secure Channel protocol library
  DEPENDS:=+libcryptoplus +boost-system +boost-date_time +libpthread
endef

define Package/libiconvplus
  $(call Package/freelanlibs/Default)
  TITLE:=C++ wrapper for iconv (FreeLAN)
  DEPENDS:=+boost-system
endef

define Package/libkfather
  $(call Package/freelanlibs/Default)
  TITLE:=A C++ JSON lib (FreeLAN)
  DEPENDS:=+libiconvplus
endef

export PLATFORM=posix

SCONS_VARS = \
    PYTHONPATH="$(PKG_BUILD_DIR)/freelan-buildtools" \
    FREELAN_ARCH=$(ARCH) \
    CXX=$(TARGET_CXX_NOCACHE) \
    FREELAN_INSTALL_PREFIX="$(PKG_INSTALL_DIR)/usr" \
    CPLUS_INCLUDE_PATH="$(PKG_INSTALL_DIR)/usr/include:$(PKG_BUILD_DIR)/lib*/include" \
    CXXFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS)"


ifneq ($(CONFIG_PACKAGE_freelan),)
 define Build/Configure/freelan
    cd $(PKG_BUILD_DIR)/freelan
    $(SCONS_VARS) scons -C $(PKG_BUILD_DIR)/freelan install
 endef
endif

ifneq ($(CONFIG_PACKAGE_libasiotap),)
 define Build/Configure/libasiotap
    cd $(PKG_BUILD_DIR)/libasiotap
    $(SCONS_VARS) scons -C $(PKG_BUILD_DIR)/libasiotap install
 endef
endif

ifneq ($(CONFIG_PACKAGE_libcryptoplus),)
 define Build/Configure/libcryptoplus
    cd $(PKG_BUILD_DIR)/libcryptoplus
    $(SCONS_VARS) scons -C $(PKG_BUILD_DIR)/libcryptoplus install
 endef
endif

ifneq ($(CONFIG_PACKAGE_libfreelan),)
 define Build/Configure/libfreelan
    cd $(PKG_BUILD_DIR)/libfreelan
    $(SCONS_VARS) scons -C $(PKG_BUILD_DIR)/libfreelan install
 endef
endif

ifneq ($(CONFIG_PACKAGE_libfscp),)
 define Build/Configure/libfscp
    cd $(PKG_BUILD_DIR)/libfscp
    $(SCONS_VARS) scons -C $(PKG_BUILD_DIR)/libfscp install
 endef
endif

ifneq ($(CONFIG_PACKAGE_libiconvplus),)
 define Build/Configure/libiconvplus
    cd $(PKG_BUILD_DIR)/libiconvplus
    $(SCONS_VARS) scons -C $(PKG_BUILD_DIR)/libiconvplus install
 endef
endif

ifneq ($(CONFIG_PACKAGE_libkfather),)
 define Build/Configure/libkfather
    cd $(PKG_BUILD_DIR)/libkfather
    $(SCONS_VARS) scons -C $(PKG_BUILD_DIR)/libkfather install
 endef
endif

define Build/Configure
    $(call Build/Configure/libiconvplus)
    $(call Build/Configure/libkfather)
    $(call Build/Configure/libcryptoplus)
    $(call Build/Configure/libasiotap)
    $(call Build/Configure/libfscp)
    $(call Build/Configure/libfreelan)
    $(call Build/Configure/freelan)
endef

define Build/Compile
endef

define Package/freelan/install
    $(INSTALL_DIR) \
        $(1)/usr/sbin \
        $(1)/etc/freelan \
        $(1)/etc/init.d   

    # copying binary
    $(INSTALL_BIN) \
        $(PKG_INSTALL_DIR)/usr/bin/freelan \
        $(1)/usr/sbin/freelan

    # copying configuration
    $(INSTALL_CONF) \
        $(PKG_BUILD_DIR)/freelan/config/* \
        $(1)/etc/freelan

    # creatign service
    $(INSTALL_BIN) \
        files/freelan.init \
        $(1)/etc/init.d/freelan
endef

define Package/freelanlibs/Default/install
    $(INSTALL_DIR) \
        $(1)/usr/lib
    $(CP) \
        $(PKG_INSTALL_DIR)/usr/lib/$(2).so.*.* \
        $(1)/usr/lib/
    cd $(1)/usr/lib/;LIB=`ls $(2).so.*.*`;ln -s $$$${LIB} $$$${LIB%.*};ln -s $$$${LIB} $$$${LIB%.*.*};
endef

define Package/libasiotap/install
  $(call Package/freelanlibs/Default/install,$(1),libasiotap)
endef

define Package/libcryptoplus/install
  $(call Package/freelanlibs/Default/install,$(1),libcryptoplus)
endef

define Package/libfreelan/install
  $(call Package/freelanlibs/Default/install,$(1),libfreelan)
endef

define Package/libfscp/install
  $(call Package/freelanlibs/Default/install,$(1),libfscp)
endef

define Package/libiconvplus/install
  $(call Package/freelanlibs/Default/install,$(1),libiconvplus)
endef

define Package/libkfather/install
  $(call Package/freelanlibs/Default/install,$(1),libkfather)
endef

define Package/freelan/conffiles
  /etc/freelan
endef

$(eval $(call BuildPackage,freelan))
$(eval $(call BuildPackage,libasiotap))
$(eval $(call BuildPackage,libcryptoplus))
$(eval $(call BuildPackage,libfreelan))
$(eval $(call BuildPackage,libfscp))
$(eval $(call BuildPackage,libiconvplus))
$(eval $(call BuildPackage,libkfather))
