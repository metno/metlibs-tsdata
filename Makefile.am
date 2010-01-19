# $Id$

pkgname = tsData

include $(top_srcdir)/metlibs.mk

lib_LTLIBRARIES = libtsData.la

libtsData_la_SOURCES = \
	ptAsciiStream.cc \
	ptDiagramData.cc \
	ptHDFFile.cc \
	ptHDFUtils.cc \
	ptMIROS22File.cc \
	ptParameterDefinition.cc \
	ptPrimitiveType.cc \
	ptSHCinfo.cc \
	ptWeatherParameter.cc

libtsData_la_CPPFLAGS = \
	${HDF4_CPPFLAGS} \
	${LOG4CXX_CPPFLAGS} \
	${MILOGGER_CPPFLAGS} \
	${PARAMETER_CPPFLAGS} \
	${PUCTOOLS_CPPFLAGS} \
	${PUDATATYPES_CPPFLAGS} \
	${PUMET_CPPFLAGS} \
	${PUTOOLS_CPPFLAGS}

libtsData_la_LDFLAGS = \
	${VERSION_INFO} \
	${HDF4_LDFLAGS} \
	${LOG4CXX_LDFLAGS} \
	${MILOGGER_LDFLAGS} \
	${PARAMETER_LDFLAGS} \
	${PUCTOOLS_LDFLAGS} \
	${PUDATATYPES_LDFLAGS} \
	${PUMET_LDFLAGS} \
	${PUTOOLS_LDFLAGS}

libtsData_la_LIBADD = \
	${HDF4_LIBS} \
	${LOG4CXX_LIBS} \
	${MILOGGER_LIBS} \
	${PARAMETER_LIBS} \
	${PUCTOOLS_LIBS} \
	${PUDATATYPES_LIBS} \
	${PUMET_LIBS} \
	${PUTOOLS_LIBS}

pkg_includedir = ${prefix}/include/tsData

pkg_include_HEADERS = \
	HDFdefs.h \
	ptAsciiStream.h \
	ptDataStream.h \
	ptDiagramData.h \
	ptError.h \
	ptHDFFile.h \
	ptHDFUtils.h \
	ptMIROS22File.h \
	ptParameterDefinition.h \
	ptParameter.h \
	ptPrimitiveType.h \
	ptSHCinfo.h \
	ptTimeLine.h \
	ptWeatherParameter.h

if WITH_GRIBFILE
libtsData_la_SOURCES += ptGribStream.cc
pkg_include_HEADERS += ptGribStream.h
endif