##### http:///                                  -*- Autoconf -*-
#
# WARNING
#
#   This file is a copy of
#   'common_build_files/m4/metno_have_freetype2.m4'.  The next time
#   'common_build_files/distribute.sh' is run with the appropriate
#   arguments, all changes to this file will disappear.  Please edit
#   the original.
#
# SYNOPSIS
#
#   METNO_HAVE_FREETYPE2([ACTION-IF-TRUE], [ACTION-IF-FALSE])
#
# DESCRIPTION
#
#   This macro will check for the existence of the FreeType 2 library
#   (http://www.freetype.org/).  The check is done by calling
#   pkg-config.  A --with-freetype2 option is supported as well.  The
#   following output variables are set with AC_SUBST:
#
#     AC_SUBST(FT2_CFLAGS)
#     AC_SUBST(FT2_LIBS)
#
#   You can use them like this in Makefile.am:
#
#     AM_CFLAGS = $(FT2_CFLAGS)
#     program_LDADD = $(FT2_LIBS)
#
#   Additionally, the C preprocessor symbol HAVE_FREETYPE2 will be
#   defined with AC_DEFINE([HAVE_FREETYPE2]) if the library is
#   available.  Forthermore, the variable have_freetype2 will be set
#   to "yes" if the library is available.
#
# AUTHOR
#
#   Martin Thorsen Ranang <mtr@linpro.no>
#
# LAST MODIFICATION
#
#   $Date: 2007-12-07 23:53:49 +0100 (fre, 07 des 2007) $
#
# ID
#
#   $Id: metno_have_freetype2.m4 778 2007-12-07 22:53:49Z martinr $
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

# METNO_HAVE_FREETYPE2([ACTION-IF-TRUE], [ACTION-IF-FALSE])
# ------------------------------------------------------
AC_DEFUN([METNO_HAVE_FREETYPE2], [
  AH_TEMPLATE([HAVE_FREETYPE2], [Define if FreeType 2 is available])
  AC_LANG_PUSH(C)
  AC_CHECK_FT2([9.5.3], [have_freetype2=yes
                         AC_DEFINE([HAVE_FREETYPE2], [1])])
  AC_LANG_POP(C)
])
