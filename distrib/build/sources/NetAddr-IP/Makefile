# SmoothWall Build system.
#
# (c) SmoothWall Ltd 2005
#
# This code is distributed under the terms of the GPL v2.

include ../Makefile.conf

PACKAGE = NetAddr-IP
VERSION = 4.059

BASE_URL = http://search.cpan.org/CPAN/authors/id/M/MI/MIKER/

CONFIGURE = yes

configure: $(DIR)/
	@(cd $(DIR); echo 'y' | perl Makefile.PL ) 

include ../Makefile.rules
