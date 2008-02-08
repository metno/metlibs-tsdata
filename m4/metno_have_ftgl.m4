##### http:///                                  -*- Autoconf -*-
#
# WARNING
#
#   This file is a copy of 'common_build_files/m4/metno_have_ftgl.m4'.
#   The next time 'common_build_files/distribute.sh' is run with the
#   appropriate arguments, all changes to this file will disappear.
#   Please edit the original.
#
# SYNOPSIS
#
#   METNO_HAVE_FTGL([ACTION-IF-TRUE], [ACTION-IF-FALSE])
#
# DESCRIPTION
#
#   This macro will check for the existence of the FTGL library
#   (http://ftgl.wiki.sourceforge.net/).  The check is done by
#   checking for the header file ftgl.h and the ftgl library object
#   file.  A --with-ftgl option is supported as well.  The following
#   output variables are set with AC_SUBST:
#
#     AC_SUBST(FTGL_CPPFLAGS)
#     AC_SUBST(FTGL_LDFLAGS)
#     AC_SUBST(FTGL_LIBS)
#
#   You can use them like this in Makefile.am:
#
#     AM_CPPFLAGS = $(FTGL_CPPFLAGS)
#     AM_LDFLAGS = $(FTGL_LDFLAGS)
#     program_LDADD = $(FTGL_LIBS)
#
#   Additionally, the C preprocessor symbol HAVE_FTGL will be defined
#   with AC_DEFINE([HAVE_FTGL]) if the library is available.
#   Forthermore, the variable have_ftgl will be set to "yes" if the
#   library is available.
#
# AUTHOR
#
#   Martin Thorsen Ranang <mtr@linpro.no>
#
# LAST MODIFICATION
#
#   $Date: 2007-12-07 16:19:40 +0100 (fre, 07 des 2007) $
#
# ID
#
#   $Id: metno_have_ftgl.m4 775 2007-12-07 15:19:40Z martinr $
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

# METNO_HAVE_FTGL([ACTION-IF-TRUE], [ACTION-IF-FALSE])
# ------------------------------------------------------
AC_DEFUN([METNO_HAVE_FTGL], [
  AC_REQUIRE([METNO_HAVE_FREETYPE2])
  AH_TEMPLATE([HAVE_FTGL], [Define if ftgl is available])
  AC_ARG_WITH(ftgl, 
              [  --with-FTGL=DIR         prefix for FTGL library files and headers], 
              [if test "$withval" = "no"; then
                 ac_ftgl_path=
                 $2
               elif test "$withval" = "yes"; then
                 ac_ftgl_path=/usr
               else
                 ac_ftgl_path="$withval"
               fi], 
              [ac_ftgl_path=/usr])
  if test "$ac_ftgl_path" != ""; then
    AC_LANG_PUSH(C++)
    saved_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS $FT2_CFLAGS -I$ac_ftgl_path/include"
    AC_CHECK_HEADER([FTGL/FTFace.h], [
      #saved_LDFLAGS="$LDFLAGS"
      #LDFLAGS="$LDFLAGS "
      saved_LIBS="$LIBS"
      LIBS="$LIBS $FT2_LIBS -L$ac_ftgl_path/lib -lftgl"
      AC_MSG_CHECKING([for FTFace(const char *) in -lftgl])
      AC_LINK_IFELSE(
        [AC_LANG_PROGRAM([\
#include <FTGL/FTFace.h>
                         ],
			 [FTFace("MyFont")])],
        [AC_MSG_RESULT([yes])
	 AC_SUBST(FTGL_CPPFLAGS, ["$FT2_CFLAGS -I$ac_ftgl_path/include"])
         #AC_SUBST(FTGL_LDFLAGS, [])
         AC_SUBST(FTGL_LIBS, ["$FT2_LIBS -L$ac_ftgl_path/lib -lftgl"])
	 AC_DEFINE([HAVE_FTGL])
	 $1
        ], [
        :
	AC_MSG_RESULT([no])
        $2
        ])
      LIBS="$saved_LIBS"
      #LDFLAGS="$saved_LDFLAGS"
    ], [
      :
      $2
    ])
    CPPFLAGS="$saved_CPPFLAGS"
    AC_LANG_POP(C++)
  fi
])
