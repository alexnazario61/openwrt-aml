#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/m201
    NAME:= meson8b m201
    PACKAGES:= 
endef

m201_KERNELDTS:="meson8b_m201_1G"

define Profile/m201/Description
	Package set optimized for m201
endef

$(eval $(call Profile,m201))
