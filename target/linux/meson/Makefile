#
# Copyright (C) 2013-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=meson
BOARDNAME:=Meson
FEATURES:=fpu usb ext4 display rtc
SUBTARGETS=m3 m6 m8 m8b m8m2
LINUX_PROTO:=$(subst ",,$(CONFIG_LINUX_MESON_PKG_SOURCE_PROTO))
INITRAMFS:=$(PLATFORM_SUBDIR)/initramfs.lst

MAINTAINER:=Matthew Shyu <matthew.shyu@amlogic.com>

LINUX_VERSION:=3.10.33
KERNELNAME:="uImage"

define Kernel/Patch/Amlogic
	$(call PatchDir,$(PKG_BUILD_DIR),$(PATCH_DIR),platform/)
endef

define Kernel/Patch/Generic
	rm -rf $(PKG_BUILD_DIR)/patches; mkdir -p $(PKG_BUILD_DIR)/patches
	$(if $(PLATFORM_DIR)/files,$(CP) $(PLATFORM_DIR)/files/. $(LINUX_DIR)/)
	find $(LINUX_DIR)/ -name \*.rej -or -name \*.orig | $(XARGS) rm -f
	$(call PatchDir,$(PKG_BUILD_DIR),patches-generic-3.10,platform/)
endef

include $(INCLUDE_DIR)/target.mk
$(eval $(call BuildTarget))
