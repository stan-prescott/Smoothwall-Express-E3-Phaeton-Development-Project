# SmoothWall Build system.
#
# (c) SmoothWall Ltd 2005
#
# This code is distributed under the terms of the GPL v2.

include ../Makefile.conf
# # include ../Makefile.versions

PACKAGE = jam
VERSION = 3.1.18

# This is a hack to make the package name "jam" and not "boost-jam"
TARBALL = boost-jam-$(VERSION)

EXTENSION = .tgz

BASE_URL = http://hivelocity.dl.sourceforge.net/project/boost/boost-jam/$(VERSION)/

CONFIGURE = yes
COMPILE = yes
INSTALL = yes

configure: patch
	@true;

compile: configure
	@(cd $(COMPILE_DIR); ./build.sh)

install: compile
	@mkdir -p $(PKG_ROOT)
	@install -D $(COMPILE_DIR)/bin.linuxx86/bjam $(PKG_ROOT)/usr/bin/bjam

include ../Makefile.rules
