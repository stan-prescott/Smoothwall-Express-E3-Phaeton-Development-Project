# SmoothWall Build system.
#
# (c) SmoothWall Ltd 2005
#
# This code is distributed under the terms of the GPL v2.

include ../Makefile.conf

PACKAGE = perl
VERSION = 5.10.1

BASE_URL = ftp://ftp.cpan.org/pub/CPAN/src/5.0/

#PATCH_URL1 = http://www.linuxfromscratch.org/patches/lfs/6.4/perl-5.10.0-consolidated-1.patch


# Toolchain, Stage 3 build
#
ifeq ($(BUILD_PHASE), Stage_3)

# Fixme: improper "../"!!!!!
PATCH_FILE1 = ../perl-5.10.1-libc-1.patch

CONFIG_OPTS = -des -Dprefix=/tools -Dstatic_ext="Data/Dumper Fcntl IO POSIX" \
	      -Donlyextensions="Data/Dumper Fcntl IO POSIX" \
	      -Dglincpth="/tools/include" -Dglibpth="/tools/lib" \
	      -Dinc_version_list=none

CONFIGURE = yes
configure: patch
	@cd $(COMPILE_DIR); sh Configure $(CONFIG_OPTS)

COMPILE = yes
compile: configure
	make $(JOBS) -C $(COMPILE_DIR) perl utilities ext/Errno/pm_to_blib

INSTALL = yes
install: compile
	cd $(COMPILE_DIR); \
	  cp -v perl pod/pod2man $(TOOLS_DIR)/bin; \
	  mkdir -pv $(TOOLS_DIR)/lib/perl5/5.10.1; \
	  cp -Rv lib/* $(TOOLS_DIR)/lib/perl5/5.10.1
endif


# Final build
#
ifeq ($(BUILD_PHASE),Final)
CONFIGURE = yes
configure: patch
	@(cd $(COMPILE_DIR); \
	./configure.gnu --prefix=/$(PKG_DIR) -Dpager="/bin/less -isR");
endif


include ../Makefile.rules
