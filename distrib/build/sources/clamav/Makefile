include ../Makefile.conf

PACKAGE = clamav
VERSION = 0.97.4

BASE_URL = http://softlayer.dl.sourceforge.net/project/$(PACKAGE)/$(PACKAGE)/$(VERSION)

CONFIG_OPTS = --sysconfdir=/usr/lib/smoothwall --with-dbdir=/var/clamav --disable-clamav --enable-ltdl-install

PATCH_FILE1 = clamav-pidperms.patch

INSTALL = yes

install: compile
	@mkdir -p $(PKG_ROOT)
	@make -C $(COMPILE_DIR) DESTDIR=$(PKG_ROOT) install
	@chown -R 105:105 $(PKG_ROOT)/var/clamav
	@rm -f $(PKG_ROOT)/usr/lib/smoothwall/clamd.conf
	@rm -f $(PKG_ROOT)/usr/lib/smoothwall/freshclam.conf

include ../Makefile.rules
