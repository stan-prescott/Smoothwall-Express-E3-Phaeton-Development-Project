# SmoothWall Build system.
#
# (c) SmoothWall Ltd 2005
#
# This code is distributed under the terms of the GPL v2.

include ../Makefile.conf

PACKAGE = less
VERSION = 436

BASE_URL = http://www.greenwoodsoftware.com/less/

INSTALL = yes

install: compile
	@mkdir -p $(PKG_ROOT)
	@(cd $(DIR); \
	install -D less $(PKG_ROOT)$(PKG_DIR)/bin/less);

include ../Makefile.rules
