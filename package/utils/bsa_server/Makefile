include $(TOPDIR)/rules.mk

PKG_NAME:=bsa_server
PKG_VERSION:=2014-12-19
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
BSA_EXAMPLES_DIR := 3rdparty/embedded/bsa_examples/linux

include $(INCLUDE_DIR)/package.mk

define Package/bsa_server
  SECTION:=utils
  CATEGORY:=Base system
  TITLE:=Broadcom bluetooth software
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./3rdparty $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/$(BSA_EXAMPLES_DIR)/app_manager/build/ CPU=arm \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS) -Wall" \
	LDFLAGS="$(TARGET_LDFLAGS)"
	$(MAKE) -C $(PKG_BUILD_DIR)/$(BSA_EXAMPLES_DIR)/app_avk/build/ CPU=arm \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS) -Wall" \
	LDFLAGS="$(TARGET_LDFLAGS)"
	$(MAKE) -C $(PKG_BUILD_DIR)/$(BSA_EXAMPLES_DIR)/app_hs/build/ CPU=arm \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS) -Wall" \
	LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Package/bsa_server/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/bsa_server $(1)/usr/bin/bsa_server
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(BSA_EXAMPLES_DIR)/app_manager/build/arm/app_manager $(1)/usr/bin/app_manager
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(BSA_EXAMPLES_DIR)/app_avk/build/arm/app_avk $(1)/usr/bin/app_avk
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(BSA_EXAMPLES_DIR)/app_hs/build/arm/app_hs $(1)/usr/bin/app_hs
endef

$(eval $(call BuildPackage,bsa_server))