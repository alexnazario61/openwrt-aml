#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/m200
    NAME:= meson8b m200
    PACKAGES:= 
endef

m200_KERNELDTS:="meson8b_m200_1G"

define Profile/m200/Description
	Package set optimized for m200
endef

$(eval $(call Profile,m200))
