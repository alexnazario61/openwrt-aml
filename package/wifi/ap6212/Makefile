# All rights reserved.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ap6212
PKG_SOURCE_VERSION:=amlogic-pd-00001
DRIVER_DIR:=bcmdhd_1_141_xx
PKG_SOURCE:=$(PKG_NAME)-$(PKG_SOURCE_VERSION).tar.bz2
PKG_SOURCE_URL:="git://git.myamlogic.com/platform/hardware/wifi/broadcom/drivers/ap6xxx.git"
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)
DRIVER_BUILD_DIR := $(KERNEL_BUILD_DIR)/$(PKG_NAME)/$(PKG_SOURCE_SUBDIR)
PKG_SOURCE_PROTO:=git

PKG_KCONFIG:=\
BCMDHD_FW_PATH BCMDHD_NVRAM_PATH BCMDHD_CONFIG_PATH BCMDHD_WEXT BCMDHD_141_OOB CFG80211
PKG_CONFIG_DEPENDS:=$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)))

include $(INCLUDE_DIR)/package.mk

TAR_CMD=$(HOST_TAR) -C $(1)/ $(TAR_OPTIONS)

define KernelPackage/ap6212
  CATEGORY:=ampak Properties
  TITLE:=ampak ap6212 wifi AP driver
  FILES:=$(DRIVER_BUILD_DIR)/$(DRIVER_DIR)/dhd.ko
  AUTOLOAD:=$(call AutoLoad,90,ap6212)
  SUBMENU:=Drivers
  MENU:=1
endef

define KernelPackage/ap6212/config
	source "$(SOURCE)/Config-ap6212.in"
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		SUBDIRS="$(DRIVER_BUILD_DIR)/$(DRIVER_DIR)/" \
		$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)=y))\
		modules
endef

define KernelPackage/ap6212/install
	$(INSTALL_DIR) $(1)/etc/wifi/
	$(INSTALL_BIN) ./files/bcm43438a0.hcd $(1)/etc/wifi/
	$(INSTALL_BIN) ./files/bcm43438a0_37M.hcd $(1)/etc/wifi/
	$(INSTALL_BIN) ./files/nvram.txt $(1)/etc/wifi/
	$(INSTALL_BIN) ./files/wpa_supplicant.conf $(1)/etc/wifi/
	$(INSTALL_BIN) ./files/fw_bcm43438a0_apsta.bin $(1)/etc/wifi/
	$(INSTALL_BIN) ./files/fw_bcm43438a0_p2p.bin $(1)/etc/wifi/
	-$(INSTALL_BIN) ./files/fw_bcm43438a0.bin $(1)/etc/wifi/
endef

$(eval $(call KernelPackage,ap6212))
