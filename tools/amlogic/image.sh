cp ./build_dir/target-arm_cortex-a9+neon_eglibc-2.19_eabi/linux-meson_m3/linux-3.10.33/arch/arm/boot/uImage ./bin/meson-eglibc/
cp ./build_dir/target-arm_cortex-a9+neon_eglibc-2.19_eabi/linux-meson_m3/meson3_f40.dtb ./bin/meson-eglibc/
cp ./tools/amlogic/320x240.bmp ./bin/meson-eglibc/
cp ./tools/amlogic/platform.conf ./bin/meson-eglibc/
cp ./tools/amlogic/usb_spl.bin ./bin/meson-eglibc/
cp ./tools/amlogic/u-boot-comp.bin ./bin/meson-eglibc/
cp ./tools/amlogic/u-boot.bin ./bin/meson-eglibc/
./tools/amlogic/aml_image_v2_packer -r ./tools/amlogic/aml_upgrade_package.conf ./bin/meson-eglibc/ ./bin/meson-eglibc/aml_upgrade_package.img
