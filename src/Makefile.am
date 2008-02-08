pkglib_LTLIBRARIES = lib@PACKAGE_NAME@.la

lib@PACKAGE_NAME@_la_SOURCES = \
ptAsciiStream.cc \
ptDiagramData.cc \
ptHDFFile.cc \
ptHDFUtils.cc \
ptMIROS22File.cc \
ptParameterDefinition.cc \
ptPrimitiveType.cc \
ptSHCinfo.cc \
ptWeatherParameter.cc

lib@PACKAGE_NAME@_la_CPPFLAGS = \
	${puTools_CFLAGS} ${miLogger_CFLAGS} \
	${LOG4CXX_CPPFLAGS} ${HDF4_CFLAGS}

lib@PACKAGE_NAME@_la_LDFLAGS = \
	${puTools_LIBS} ${miLogger_LIBS} \
	${LOG4CXX_LDFLAGS} ${LOG4CXX_LIBS}  \
	${HDF4_LDFLAGS} \
	${VERSION_INFO}

INSTALL=install -p # tells install to preserve timestamp
DEFAULT_INCLUDES=-I${srcdir}/../include
