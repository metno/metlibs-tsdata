# includefile contains platform-definitions
include ../conf/$(OSTYPE).mk

LIBNAME=tsData
LOCALIDIR = $(LOCALDIR)/include/$(LIBNAME)

INCLUDE=-I../include   -I$(LOCALDIR)/include -I$(HDF4DIR) $(GLINCLUDE) $(PNGINCLUDE) $(XINCLUDE)

DEFINES=

LOCALOPTIONS=
include ../conf/targets.mk
