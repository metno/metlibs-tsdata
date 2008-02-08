pkglib_LTLIBRARIES = libtsdata.la

libtsdata_la_SOURCES = \
ptAsciiStream.cc \
ptDiagramData.cc \
ptHDFFile.cc \
ptHDFUtils.cc \
ptMIROS22File.cc \
ptParameterDefinition.cc \
ptPrimitiveType.cc \
ptSHCinfo.cc \
ptWeatherParameter.cc

libtsdata_la_CPPFLAGS = \
	${puMet_CFLAGS} ${puDatatypes_CFLAGS} ${parameter_CFLAGS} \
	${puCtools_CFLAGS} ${puTools_CFLAGS} ${miLogger_CFLAGS} \
	${LOG4CXX_CPPFLAGS} ${HDF4_CFLAGS}

libtsdata_la_LDFLAGS = \
	${puMet_LIBS} ${puDatatypes_LIBS} ${parameter_LIBS} \
	${puCtools_LIBS} ${puTools_LIBS} ${miLogger_LIBS} \
	${LOG4CXX_LDFLAGS} ${LOG4CXX_LIBS}  \
	${HDF4_LDFLAGS} ${HDF4_LIBS} \
	${VERSION_INFO}

INSTALL=install -p # tells install to preserve timestamp
DEFAULT_INCLUDES=-I${srcdir}/../include
