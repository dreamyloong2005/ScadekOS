# SPDX-License-Identifier: MPL-2.0
# Copyright (c) 2026 The Scadek OS Project contributors

KERNEL_DIR := kernel/scdk
BUILD_DIR := build
VERSION_FILE := VERSION
SCADEKOS_VERSION := $(shell sed -n '1p' $(VERSION_FILE))
KERNEL_VERSION_FILE := KERNEL_VERSION
SCDK_VERSION := $(shell sed -n 's/^SCDK_VERSION=//p' $(KERNEL_VERSION_FILE))
SCDK_COMMIT := $(shell sed -n 's/^SCDK_COMMIT=//p' $(KERNEL_VERSION_FILE))
PAYLOAD_DIR := $(BUILD_DIR)/payload
PAYLOAD_USER_DIR := $(PAYLOAD_DIR)/user
PAYLOAD_INITRD_DIR := $(PAYLOAD_DIR)/initrd
SCDK_BUILD_DIR := $(KERNEL_DIR)/build
SCDK_ISO_DIR := $(SCDK_BUILD_DIR)/iso_root
SCDK_INITRD_ROOT := $(SCDK_BUILD_DIR)/initrd_root
SCDK_KERNEL := $(SCDK_BUILD_DIR)/scdk.elf
SCDK_INITRD := $(SCDK_BUILD_DIR)/scdk.initrd
SCDK_ISO := $(KERNEL_DIR)/build/scdk.iso
SCADEKOS_ISO := $(BUILD_DIR)/scadekos.iso
INITRD_TAR_FILES := init hello bin/hello grant-test ring-test runner prompt etc/scdk.conf etc/scadekos.conf etc/scadek.rc etc/scadekos.version etc/scdk.version hello.txt

.PHONY: all submodules check-kernel-env prepare-payload iso run run-framebuffer smoke smoke-no-serial clean

all: iso

submodules:
	git submodule update --init --recursive

check-kernel-env: submodules
	cd $(KERNEL_DIR) && . ../../tools/kernel-env.sh && .devtools/check-env.sh

prepare-payload:
	rm -rf $(PAYLOAD_DIR) $(SCDK_INITRD_ROOT)
	mkdir -p $(PAYLOAD_USER_DIR) $(PAYLOAD_INITRD_DIR)
	cp userspace/init/init.c $(PAYLOAD_USER_DIR)/init.c
	cp userspace/hello/hello.c $(PAYLOAD_USER_DIR)/hello.c
	cp userspace/grant/grant.S $(PAYLOAD_USER_DIR)/grant.S
	cp userspace/ring/ring.S $(PAYLOAD_USER_DIR)/ring.S
	cp userspace/runner/runner.c $(PAYLOAD_USER_DIR)/runner.c
	cp userspace/prompt/prompt.c $(PAYLOAD_USER_DIR)/prompt.c
	cp -R initrd/. $(PAYLOAD_INITRD_DIR)/
	mkdir -p $(PAYLOAD_INITRD_DIR)/etc
	printf '%s\n' "$(SCADEKOS_VERSION)" > $(PAYLOAD_INITRD_DIR)/etc/scadekos.version
	cp $(KERNEL_VERSION_FILE) $(PAYLOAD_INITRD_DIR)/etc/scdk.version

iso: submodules $(VERSION_FILE) $(KERNEL_VERSION_FILE) prepare-payload
	cd $(KERNEL_DIR) && . ../../tools/kernel-env.sh && \
		$(MAKE) $(SCDK_KERNEL:$(KERNEL_DIR)/%=%)
	tools/build-users.sh "$(CURDIR)/$(PAYLOAD_USER_DIR)" "$(CURDIR)/$(SCDK_INITRD_ROOT)"
	cp -R $(PAYLOAD_INITRD_DIR)/. $(SCDK_INITRD_ROOT)/
	cd $(SCDK_INITRD_ROOT) && tar --format=ustar -cf ../scdk.initrd $(INITRD_TAR_FILES)
	rm -rf $(SCDK_ISO_DIR)
	mkdir -p $(SCDK_ISO_DIR)/boot/limine $(SCDK_ISO_DIR)/EFI/BOOT
	cp $(SCDK_KERNEL) $(SCDK_ISO_DIR)/boot/scdk.elf
	cp $(SCDK_INITRD) $(SCDK_ISO_DIR)/boot/scdk.initrd
	cp $(KERNEL_DIR)/limine.conf $(SCDK_ISO_DIR)/boot/limine/limine.conf
	cd $(KERNEL_DIR) && . ../../tools/kernel-env.sh && cd ../.. && \
		cp "$$SCDK_DEVTOOLS/limine/limine-bios.sys" $(SCDK_ISO_DIR)/boot/limine/ && \
		cp "$$SCDK_DEVTOOLS/limine/limine-bios-cd.bin" $(SCDK_ISO_DIR)/boot/limine/ && \
		cp "$$SCDK_DEVTOOLS/limine/limine-uefi-cd.bin" $(SCDK_ISO_DIR)/boot/limine/ && \
		cp "$$SCDK_DEVTOOLS/limine/BOOTX64.EFI" $(SCDK_ISO_DIR)/EFI/BOOT/BOOTX64.EFI && \
		cp "$$SCDK_DEVTOOLS/limine/BOOTIA32.EFI" $(SCDK_ISO_DIR)/EFI/BOOT/BOOTIA32.EFI && \
		xorriso -as mkisofs \
			-b boot/limine/limine-bios-cd.bin \
			-no-emul-boot \
			-boot-load-size 4 \
			-boot-info-table \
			--efi-boot boot/limine/limine-uefi-cd.bin \
			-efi-boot-part \
			--efi-boot-image \
			--protective-msdos-label \
			$(SCDK_ISO_DIR) \
			-o $(SCDK_ISO) && \
		"$$SCDK_DEVTOOLS/limine/limine" bios-install $(SCDK_ISO)
	mkdir -p $(BUILD_DIR)
	cp $(SCDK_ISO) $(SCADEKOS_ISO)
	@printf '[scadekos] version: %s\n' "$(SCADEKOS_VERSION)"
	@printf '[scadekos] kernel: SCDK %s %s\n' "$(SCDK_VERSION)" "$(SCDK_COMMIT)"
	@printf '[scadekos] image: %s\n' "$(SCADEKOS_ISO)"

run: iso
	qemu-system-x86_64 \
		-M q35 \
		-m 256M \
		-cdrom $(SCADEKOS_ISO) \
		-boot d \
		-serial stdio \
		-display none \
		-no-reboot

run-framebuffer: iso
	qemu-system-x86_64 \
		-M q35 \
		-m 256M \
		-cdrom $(SCADEKOS_ISO) \
		-boot d \
		-serial none \
		-no-reboot

smoke:
	tools/smoke-qemu.sh

smoke-no-serial:
	tools/smoke-qemu-no-serial.sh

clean:
	$(MAKE) -C $(KERNEL_DIR) clean
	rm -rf $(BUILD_DIR)
