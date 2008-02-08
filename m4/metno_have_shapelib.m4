##### http:///                                  -*- Autoconf -*-
#
# WARNING
#
#   This file is a copy of
#   'common_build_files/m4/metno_have_shapelib.m4'.  The next time
#   'common_build_files/distribute.sh' is run with the appropriate
#   arguments, all changes to this file will disappear.  Please edit
#   the original.
#
# SYNOPSIS
#
#   METNO_HAVE_SHAPELIB([ACTION-IF-TRUE], [ACTION-IF-FALSE])
#
# DESCRIPTION
#
#   This macro will check for the existence of the shapelib library
#   (http://shapelib.maptools.org/).  The check is done by checking
#   for the header file shapefil.h and the libshp library object file.
#   A --with-shapelib option is supported as well.  The following
#   output variables are set with AC_SUBST:
#
#     AC_SUBST(SHAPELIB_CPPFLAGS)
#     AC_SUBST(SHAPELIB_LDFLAGS)
#     AC_SUBST(SHAPELIB_LIBS)
#
#   You can use them like this in Makefile.am:
#
#     AM_CPPFLAGS = $(SHAPELIB_CPPFLAGS)
#     AM_LDFLAGS = $(SHAPELIB_LDFLAGS)
#     program_LDADD = $(SHAPELIB_LIBS)
#
#   Additionally, the C preprocessor symbol HAVE_SHAPELIB will be
#   defined with AC_DEFINE([HAVE_SHAPELIB]) if the library is
#   available.
#
# AUTHOR
#
#   Martin Thorsen Ranang <mtr@linpro.no>
#
# LAST MODIFICATION
#
#   $Date: 2007-12-07 16:13:40 +0100 (fre, 07 des 2007) $
#
# ID
#
#   $Id: metno_have_shapelib.m4 774 2007-12-07 15:13:40Z martinr $
#
# COPYLEFT
#
#   Copyright (c) 2007 Meteorologisk institutt <diana@met.no>
#
#   This program is free software: you can redistribute it and/or
#   modify it under the terms of the GNU General Public License as
#   published by the Free Software Foundation, either version 3 of the
#   License, or (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#   General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program. If not, see
#   <http://www.gnu.org/licenses/>.
#

# METNO_HAVE_SHAPELIB([ACTION-IF-TRUE], [ACTION-IF-FALSE])
# ------------------------------------------------------
AC_DEFUN([METNO_HAVE_SHAPELIB], [
  AH_TEMPLATE([HAVE_SHAPELIB], [Define if shapelib is available])
  AC_ARG_WITH(shapelib,
    dnl don't quote AS_HELP_STRING!
    AS_HELP_STRING([--with-shapelib=DIR],
                   [prefix for shapelib library files and headers]),
    [if test "$withval" = "no"; then
       ac_shapelib_path=
       $2
     elif test "$withval" = "yes"; then
       ac_shapelib_path=/usr
     else
       ac_shapelib_path="$withval"
     fi],
    [ac_shapelib_path=/usr])
  if test "$ac_shapelib_path" != ""; then
    AC_LANG_PUSH(C)
    saved_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS -I$ac_shapelib_path/include"
    AC_CHECK_HEADER([shapefil.h], [
      AC_CHECK_HEADER([shapelib/basicconfigurator.h])
      saved_LDFLAGS="$LDFLAGS"
      LDFLAGS="$LDFLAGS -L$ac_shapelib_path/lib"
      saved_LIBS="$LIBS"
      LIBS="$LIBS -lshapelib"
      AC_CHECK_LIB([shp], [SHPGetInfo],
        [AC_MSG_RESULT([yes])
	 AC_SUBST(SHAPELIB_CPPFLAGS, [-I$ac_shapelib_path/include])
         AC_SUBST(SHAPELIB_LDFLAGS, [-L$ac_shapelib_path/lib])
         AC_SUBST(SHAPELIB_LIBS, [-lshapelib])
	 AC_DEFINE([HAVE_SHAPELIB])
	 $1
        ], [
        :
	AC_MSG_RESULT([no])
	AC_MSG_WARN([

Please make sure you have installed shapelib
(http://shapelib.maptools.org/).  Under Debian, the package is called
libshp-dev.
        ])
        $2
        ])
      LIBS="$saved_LIBS"
      LDFLAGS="$saved_LDFLAGS"
    ], [
      AC_MSG_RESULT([no])
      $2
    ])
    CPPFLAGS="$saved_CPPFLAGS"
    AC_LANG_POP(C)
  fi
])
