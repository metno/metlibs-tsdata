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
	FimexTools.cc   \
	ptHDFFile.cc    \
	ptHDFUtils.cc

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
	$(PUDATATYPES_CPPFLAGS) \
	$(PUMET_CPPFLAGS) \
	$(PUTOOLS_CPPFLAGS) \
	$(POSTGRESQL_CPPFLAGS) \
	$(CURL_CPPFLAGS) \
	$(HDF4_CPPFLAGS) \
	$(FIMEX_CPPFLAGS) \
	$(BOOST_CPPFLAGS)


libtsData_la_LDFLAGS = $(VERSION_INFO)
libtsData_la_LIBADD =

if ENABLE_SHARED_LIBRARY
libtsData_la_LDFLAGS += 
	$(PUDATATYPES_LDFLAGS) \
	$(PUMET_LDFLAGS) \
	$(PUTOOLS_LDFLAGS) \
	$(POSTGRESQL_LDFLAGS) \
	${BOOST_LDFLAGS} \
	${PROJ_LDFLAGS} \
	$(CURL_LDFLAGS) \
	$(HDF4_LDFLAGS)

libtsData_la_LIBADD += \
	$(PUDATATYPES_LIBS) \
	$(PUMET_LIBS) \
	$(PUTOOLS_LIBS) \
	$(POSTGRESQL_LIBS) \
	${BOOST_LIBS}  \
	${PROJ_LIBS}   \
	$(CURL_LIBS)   \
	$(HDF4_LIBS)
endif

