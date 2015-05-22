# $Id$

pkgname = tsData

lib_LTLIBRARIES = libtsData.la

libtsData_la_SOURCES = \
	ptAsciiStream.cc \
	ptDiagramData.cc \
	ptMIROS22File.cc \
	ptParameterDefinition.cc \
	ptPrimitiveType.cc \
	ptSHCinfo.cc \
	ptWeatherParameter.cc \
	WdbStream.cc          \
	WdbQueries.cc         \
	WdbCacheQuery.cc      \
	DynamicFunction.cc    \
	KlimaStream.cc     \
	FimexStream.cc  \
	FimexTools.cc

pkg_includedir = ${includedir}/tsData

pkg_include_HEADERS = \
	$(libtsData_la_SOURCES:.cc=.h) \
	HDFdefs.h        \
	ptDataStream.h   \
	ptError.h	 \
	PetsExceptions.h \
	ptTimeLine.h     \
	ptParameter.h

libtsData_la_CPPFLAGS = \
    $(MILOGGER_CPPFLAGS) \
    $(PUCTOOLS_CPPFLAGS) \
	$(PUDATATYPES_CPPFLAGS) \
	$(PUMET_CPPFLAGS) \
	$(PUTOOLS_CPPFLAGS) \
	$(POSTGRESQL_CPPFLAGS) \
	$(CURL_CPPFLAGS) \
	$(FIMEX_CPPFLAGS) \
	$(WEATHERSYMBOL_CFLAGS) \
	$(BOOST_CPPFLAGS)


libtsData_la_LDFLAGS = $(VERSION_INFO)
libtsData_la_LIBADD =

if ENABLE_SHARED_LIBRARY
libtsData_la_LDFLAGS += \
    $(MILOGGER_LDFLAGS) \
    $(PUCTOOLS_LDFLAGS) \
	$(PUDATATYPES_LDFLAGS) \
	$(PUMET_LDFLAGS) \
	$(PUTOOLS_LDFLAGS) \
	$(POSTGRESQL_LDFLAGS) \
	$(BOOST_LDFLAGS) \
	$(PROJ_LDFLAGS) \
	$(CURL_LDFLAGS) 

libtsData_la_LIBADD += \
    $(MILOGGER_LIBS) \
    $(PUCTOOLS_LIBS) \
	$(PUDATATYPES_LIBS) \
	$(PUMET_LIBS) \
	$(PUTOOLS_LIBS) \
	$(POSTGRESQL_LIBS) \
	${BOOST_LIBS}  \
	${PROJ_LIBS}   \
	$(CURL_LIBS)
endif

if WITH_HDF4
libtsData_la_SOURCES += \
	ptHDFFile.cc \
	ptHDFUtils.cc

libtsData_la_CPPFLAGS += \
	$(HDF4_CPPFLAGS)

libtsData_la_LDFLAGS += \
	$(HDF4_LDFLAGS)
if ENABLE_SHARED_LIBRARY
libtsData_la_LIBADD += \
	$(HDF4_LIBS)
endif
endif

if ENABLE_GRIBFILE
libtsData_la_SOURCES += ptGribStream.cc
libtsData_la_LDFLAGS += \
	${DIFIELD_CPPFLAGS}
libtsData_la_LDFLAGS += \
	$(DIFIELD_LDFLAGS)
if ENABLE_SHARED_LIBRARY
libtsData_la_LIBADD += \
	$(DIFIELD_LIBS)
endif

endif

INSTALL = install -p