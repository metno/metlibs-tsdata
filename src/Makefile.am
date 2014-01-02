# $Id$

pkgname = tsData

include $(top_srcdir)/metlibs.mk

lib_LTLIBRARIES = libtsData.la

libtsData_la_SOURCES = \
	ptAsciiStream.cc \
	ptDiagramData.cc \
	ptMIROS22File.cc \
	ptParameter.cc \
	ptParameterDefinition.cc \
	ptPrimitiveType.cc \
	ptSHCinfo.cc \
	ptWeatherParameter.cc \
	WdbStream.cc          \
	WdbQueries.cc         \
	WdbCacheQuery.cc      \
	DynamicFunction.cc    \
	KlimaStream.cc       
	

libtsData_la_CPPFLAGS = \
	$(LOG4CXX_CPPFLAGS) \
	$(MILOGGER_CPPFLAGS) \
	$(PARAMETER_CPPFLAGS) \
	$(PUCTOOLS_CPPFLAGS) \
	$(PUDATATYPES_CPPFLAGS) \
	$(PUMET_CPPFLAGS) \
	$(PUTOOLS_CPPFLAGS) \
	$(DIFIELD_CPPFLAGS) \
	$(POSTGRESQL_CPPFLAGS) \
	$(CURL_CPPFLAGS)


libtsData_la_LDFLAGS = $(tsData_VERSION_INFO)
libtsData_la_LIBADD =
if ENABLE_SHARED_LIBRARY
libtsData_la_LDFLAGS += \
	$(LOG4CXX_LDFLAGS) \
	$(MILOGGER_LDFLAGS) \
	$(PARAMETER_LDFLAGS) \
	$(PUCTOOLS_LDFLAGS) \
	$(PUDATATYPES_LDFLAGS) \
	$(PUMET_LDFLAGS) \
	$(PUTOOLS_LDFLAGS) \
	$(DIFIELD_LDFLAGS) \
	$(POSTGRESQL_LDFLAGS) \
	${LIBMI_LDFLAGS} \
	${MILIB_LDFLAGS} \
	${BOOST_LDFLAGS} \
	${PROJ_LDFLAGS} \
	$(CURL_LDFLAGS)

libtsData_la_LIBADD += \
	$(LOG4CXX_LIBS) \
	$(MILOGGER_LIBS) \
	$(PARAMETER_LIBS) \
	$(PUCTOOLS_LIBS) \
	$(PUDATATYPES_LIBS) \
	$(PUMET_LIBS) \
	$(PUTOOLS_LIBS) \
	$(DIFIELD_LIBS) \
	$(POSTGRESQL_LIBS) \
	${LIBMI_LIBS} \
	${MILIB_LIBS} \
	${BOOST_LIBS} \
	${PROJ_LIBS} \
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
libtsData_la_LDFLAGS += \
	${DIFIELD_CPPFLAGS}

libtsData_la_SOURCES += ptGribStream.cc
endif

if WITH_FIMEX
libtsData_la_SOURCES += \
	FimexStream.cc \
	FimexTools.cc
libtsData_la_CPPFLAGS += \
	-DFIMEX \
	${FIMEX_CPPFLAGS}
libtsData_la_LDFLAGS += \
	${FIMEX_LDFLAGS}
libtsData_la_LIBADD += \
	${FIMEX_LIBS}
endif

pkg_includedir = ${includedir}/tsData

pkg_include_HEADERS = \
	$(libtsData_la_SOURCES:.cc=.h) \
	HDFdefs.h      \
	ptDataStream.h \
	ptError.h	   \
	PetsExceptions.h \
	ptTimeLine.h


