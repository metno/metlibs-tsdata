#
# configure.ac for tsData
#

AC_PREREQ(2.59)
AC_INIT([tsData], [5.4.1], [diana@met.no])
AM_INIT_AUTOMAKE([foreign])

AC_CONFIG_SRCDIR([src/FimexStream.h])
AC_CONFIG_MACRO_DIR([m4])

METNO_SET_VERSION_INFO(6.0.0)
METNO_PROG_PKG_CONFIG

AC_SUBST([LIBTOOL_DEPS]) 

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LIBTOOL


# Language stack
AC_LANG_PUSH(C++)

# Checks for libraries.
AC_CXX_HAVE_STD
AC_CXX_HAVE_STL
AC_LANG_POP([C++])

METNO_ENABLE_CONDITIONAL([SHARED_LIBRARY], [shared])
METNO_REQUIRE_BOOST_MT
METNO_REQUIRE_BOOST_FEATURE([date_time])

METNO_REQUIRE_PKG([MILOGGER], [miLogger])
METNO_REQUIRE_PKG([PUCTOOLS], [puCtools])
METNO_REQUIRE_PKG([PUTOOLS],[puTools])
METNO_REQUIRE_PKG([PUDATATYPES],[puDatatypes])
METNO_REQUIRE_PKG([PUMET],[puMet], [5.1.0])
METNO_REQUIRE_PKG([FIMEX], [fimex])

# for wdb
METNO_REQUIRE_PKG([POSTGRESQL], [libpqxx])

# klimastream, SMHI mora
METNO_REQUIRE_C_LIBRARY([CURL], [curl], [curl/curl.h])

# for SMHI mora
METNO_REQUIRE_QT4([
	Core
	Xml
])

METNO_WITH_HDF4([yes])
METNO_WITH_CONDITIONAL([HDF4], [hdf4])

AC_CONFIG_FILES([
	tsData.pc
	Makefile
	src/Makefile
])

AC_OUTPUT
