pkgname = tsData

lib_LTLIBRARIES = \
	libmetlibs-tsdata-core.la \
	libmetlibs-tsdata-fimex.la \
	libmetlibs-tsdata-mora.la \
	libmetlibs-tsdata-wdb.la

libmetlibs_tsdata_core_la_SOURCES = \
	ptAsciiStream.cc \
	ptDataStream.cc \
	ptDiagramData.cc \
	ptError.cc \
	ptMIROS22File.cc \
	ptParameterDefinition.cc \
	ptPrimitiveType.cc \
	ptSHCinfo.cc \
	ptWeatherParameter.cc \
	DynamicFunction.cc \
	KlimaStream.cc

libmetlibs_tsdata_fimex_la_SOURCES = \
	FimexStream.cc \
	FimexTools.cc

libmetlibs_tsdata_mora_la_SOURCES = \
	SMHIMoraStream.cc

libmetlibs_tsdata_wdb_la_SOURCES = \
	WdbStream.cc \
	WdbQueries.cc \
	WdbCacheQuery.cc

pkg_includedir = ${includedir}/tsData

pkg_include_HEADERS = \
	$(libmetlibs_tsdata_core_la_SOURCES:.cc=.h) \
	$(libmetlibs_tsdata_fimex_la_SOURCES:.cc=.h) \
	$(libmetlibs_tsdata_mora_la_SOURCES:.cc=.h) \
	$(libmetlibs_tsdata_wdb_la_SOURCES:.cc=.h) \
	tsDataVersion.h  \
	PetsExceptions.h \
	ptTimeLine.h     \
	ptParameter.h

libmetlibs_tsdata_core_la_CPPFLAGS = \
	$(MILOGGER_CPPFLAGS) \
	$(PUCTOOLS_CPPFLAGS) \
	$(PUDATATYPES_CPPFLAGS) \
	$(PUMET_CPPFLAGS) \
	$(PUTOOLS_CPPFLAGS) \
	$(WEATHERSYMBOL_CFLAGS) \
	$(CURL_CPPFLAGS) \
	$(BOOST_CPPFLAGS)

libmetlibs_tsdata_fimex_la_CPPFLAGS = \
	$(libmetlibs_tsdata_core_la_CPPFLAGS) \
	$(FIMEX_CPPFLAGS)

libmetlibs_tsdata_mora_la_CPPFLAGS = \
	$(libmetlibs_tsdata_core_la_CPPFLAGS) \
	$(QT4_CPPFLAGS)

libmetlibs_tsdata_wdb_la_CPPFLAGS = \
	$(libmetlibs_tsdata_core_la_CPPFLAGS) \
	$(POSTGRESQL_CPPFLAGS)


libmetlibs_tsdata_core_la_LDFLAGS = $(VERSION_INFO)
libmetlibs_tsdata_fimex_la_LDFLAGS = $(VERSION_INFO)
libmetlibs_tsdata_mora_la_LDFLAGS = $(VERSION_INFO)
libmetlibs_tsdata_wdb_la_LDFLAGS = $(VERSION_INFO)

libmetlibs_tsdata_core_la_LIBADD =
libmetlibs_tsdata_fimex_la_LIBADD =
libmetlibs_tsdata_mora_la_LIBADD =
libmetlibs_tsdata_wdb_la_LIBADD =

if ENABLE_SHARED_LIBRARY
libmetlibs_tsdata_core_la_LDFLAGS += \
	$(MILOGGER_LDFLAGS) \
	$(PUCTOOLS_LDFLAGS) \
	$(PUDATATYPES_LDFLAGS) \
	$(PUMET_LDFLAGS) \
	$(PUTOOLS_LDFLAGS) \
	$(BOOST_LDFLAGS) \
	$(CURL_LDFLAGS)

libmetlibs_tsdata_fimex_la_LDFLAGS += \
	$(FIMEX_LDFLAGS)

libmetlibs_tsdata_mora_la_LDFLAGS += \
	$(QT4_LDFLAGS)

libmetlibs_tsdata_wdb_la_LDFLAGS += \
	$(POSTGRESQL_LDFLAGS) \
	$(BOOST_LDFLAGS)

libmetlibs_tsdata_core_la_LIBADD += \
	$(MILOGGER_LIBS) \
	$(PUCTOOLS_LIBS) \
	$(PUDATATYPES_LIBS) \
	$(PUMET_LIBS) \
	$(PUTOOLS_LIBS) \
	$(BOOST_LIBS) \
	$(CURL_LIBS)

libmetlibs_tsdata_fimex_la_LIBADD += \
	libmetlibs-tsdata-core.la \
	$(FIMEX_LIBS) \
	$(BOOST_LIBS) $(BOOST_DATE_TIME_LIBS)

libmetlibs_tsdata_mora_la_LIBADD += \
	libmetlibs-tsdata-core.la \
	$(CURL_LIBS) \
	$(QT4_LIBS)

libmetlibs_tsdata_wdb_la_LIBADD += \
	libmetlibs-tsdata-core.la \
	$(POSTGRESQL_LIBS) \
	$(BOOST_LIBS) $(BOOST_DATE_TIME_LIBS)
endif

if WITH_HDF4
lib_LTLIBRARIES += \
	libmetlibs-tsdata-hdf4.la

libmetlibs_tsdata_hdf4_la_SOURCES = \
	ptHDFFile.cc \
	ptHDFUtils.cc

pkg_include_HEADERS += \
	$(libmetlibs_tsdata_hdf4_la_SOURCES:.cc=.h) \
	HDFdefs.h

libmetlibs_tsdata_hdf4_la_CPPFLAGS = \
	$(libmetlibs_tsdata_core_la_CPPFLAGS) \
	$(HDF4_CPPFLAGS)

libmetlibs_tsdata_hdf4_la_LDFLAGS = \
	$(VERSION_INFO) \
	$(HDF4_LDFLAGS)

if ENABLE_SHARED_LIBRARY
libmetlibs_tsdata_hdf4_la_LIBADD = \
	libmetlibs-tsdata-core.la \
	$(HDF4_LIBS)
endif # ENABLE_SHARED_LIBRARY
endif # WITH_HDF4

INSTALL = install -p
