#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([tsData], [4.0.0], [diana@met.no])
AM_INIT_AUTOMAKE
METNO_SET_VERSION_INFO

AC_CONFIG_SRCDIR([src/ptDiagramData.cc])
AC_CONFIG_HEADER([include/config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LIBTOOL
#AC_PROG_SED

AC_LANG_PUSH(C++)

# Checks for libraries.
AC_CXX_HAVE_STD
AC_CXX_HAVE_STL

AC_CHECK_HDF4

export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$prefix/lib/pkgconfig

PKG_CHECK_MODULES([puTools], [putools])
METNO_CHECK_LIB([putools], [main], , , , [$puTools_LIBS], 
                [the puTools package])
METNO_CHECK_HEADERS([puTools/miString.h], , , ,
                    [$puTools_CFLAGS], [the puTools package])

# Checks for header files.
AC_PATH_X
#AC_HEADER_STDC
#AC_CHECK_HEADERS([float.h limits.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
#AC_STRUCT_TM

# Checks for library functions.
AC_LANG_POP(C++)

#making our own .pc file, checking that it is ok and forfilled
AC_CONFIG_FILES(tsdata.pc)
AC_OUTPUT

export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:.
PKG_CHECK_MODULES([own_dependencies], [${PACKAGE_TARNAME}])

AC_CONFIG_FILES([Makefile
                 include/Makefile
                 src/Makefile])
AC_OUTPUT

#AC_CHECK_FUNCS([rint sqrt strdup])
