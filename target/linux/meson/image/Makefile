#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk
include $(INCLUDE_DIR)/host.mk

IMAGE_PROFILE:=$(if $(PROFILE),$(PROFILE),Default)

rootfs_type=$(patsubst ext4-%,ext4,$(patsubst squashfs-%,squashfs,$(1)))

# $(1): rootfs type.
# $(2): board name.
define imgname
$(BIN_DIR)/$(IMG_PREFIX)-$(2)-$(call rootfs_type,$(1))
endef


define Image/Build/ext4
	cp $(KDIR)/root.ext4 $(BIN_DIR)/root.ext4
endef
define Image/Build/squashfs
	cp $(KDIR)/root.squashfs $(BIN_DIR)/root.squashfs
endef
define Image/Build
	$(call Image/Build/$(call rootfs_type,$(1)),$(1))
	$(call Image/Build/Profile/$(IMAGE_PROFILE),$(1))
endef

$(eval $(call BuildImage))
