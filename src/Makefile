#----------------------------------------
# ABI-independent makefile for libtsData
# called from ../Makefile
#----------------------------------------

COBJ=

CCOBJ=ptPrimitiveType.o ptParameterDefinition.o ptWeatherParameter.o  ptDiagramData.o ptAsciiStream.o ptHDFFile.o ptHDFUtils.o ptMIROS22File.o ptSHCinfo.o

CSRC=$(COBJ:.o=.c)

CCSRC=$(CCOBJ:.o=.cc)

LIBOBJ=$(COBJ) $(CCOBJ)

LIBSRC=$(CSRC) $(CCSRC)

VPATH=$(LIBDIR)

.SUFFIXES: .cc .o

.cc.o:
	$(CXX) $(CCFLAGS) -c $< $(INCLUDE) 

.c.o:
	$(CC) $(CFLAGS) -c $< $(INCLUDE) 

all: $(LIBDIR)/libtsData.a

depends:
	rm -f $(DEPENDSFILE)
	echo '# Automatically-generated dependencies list:' > $(DEPENDSFILE)
	$(CXX) $(CCFLAGS) $(INCLUDE) -M $(LIBSRC) >> $(DEPENDSFILE)

$(LIBDIR)/libtsData.a: $(LIBOBJ)
	$(AR) $(ARFLAGS) $@ $(LIBOBJ)


include $(DEPENDSFILE)
