SUBDIRS = include src

ACLOCAL_AMFLAGS = -I m4 --install

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = @PACKAGE_TARNAME@.pc

$(pkgconfig_DATA): config.status
