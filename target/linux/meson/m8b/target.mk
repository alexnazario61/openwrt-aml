#
# Copyright (C) 2009 OpenWrt.org
#

SUBTARGET:=m8b
BOARDNAME:=Meson8b based boards
ARCH_PACKAGES:=
FEATURES+=
CPU_TYPE:=cortex-a5
CPU_SUBTYPE:=neon
Kernel/Patch:=$(Kernel/Patch/Amlogic)
#DEFAULT_PACKAGES += 

define Target/Description
	Build firmware images for Meson8b based boards.
endef

