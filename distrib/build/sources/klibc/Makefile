# SmoothWall Build system.
#
# (c) SmoothWall Ltd 2005
#
# This code is distributed under the terms of the GPL v2.

include ../Makefile
include ../Makefile.conf
include ../Makefile.versions

PACKAGE = klibc
VER_MM = 1.5
VERSION = $(VER_MM).24

EXTENSION = .tar.bz2

BASE_URL = http://mirror.linux.org.au/linux/libs/klibc/$(VER_MM)/

DOWNLOAD = yes
download:
	@$(DL_CMD) $(BASE_URL)/$(TARBALL)$(EXTENSION) $(MD5)
	@$(DL_CMD) http://www.kernel.org/pub/linux/kernel/v2.6/linux-$(KLIBKERNVER).tar.bz2

CONFIGURE = YES
configure: patch
	true

COMPILE = yes

compile: configure
	tar -xvf /build/downloads/linux-$(KLIBKERNVER).tar.bz2
	sed -e 's/^CONFIG_LOCALVERSION.*$$/CONFIG_LOCALVERSION="$(SWETYPE)"/' ../kernel.config$(SWETYPE)-i686 > linux-$(KLIBKERNVER)/.config
	cd linux-$(KLIBKERNVER); make prepare headers_install
	cd linux-$(KLIBKERNVER); ln -s ../arch/x86/include/asm include/asm
	cd $(DIR); \
	  make mrproper; \
	  ln -sf ../linux-$(KLIBKERNVER) linux; \
	  make $(JOBS)

INSTALL = yes
install: compile
	@mkdir -p $(PKG_ROOT)
	cd $(DIR); \
	  make prefix=$(PKG_ROOT)$(PKG_DIR) install

CLEAN = yes
clean:
	@echo -n " $(PACKAGE)"
	@rm -rf $(DIR)
	@rm -rf $(COMPILE_DIR)
	@rm -rf linux-$(KLIBKERNVER)
ifeq ($(BUILD_PHASE), Final)
	@rm -rf $(PKG_ROOT)
	@rm -f $(TARGET_DIR)/smoothwall-$(PACKAGE).tar.gz
endif

include ../Makefile.rules
