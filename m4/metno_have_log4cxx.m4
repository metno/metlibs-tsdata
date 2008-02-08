##### http:///                                  -*- Autoconf -*-
#
# WARNING
#
#   This file is a copy of
#   'common_build_files/m4/metno_have_log4cxx.m4'.  The next time
#   'common_build_files/distribute.sh' is run with the appropriate
#   arguments, all changes to this file will disappear.  Please edit
#   the original.
#
# SYNOPSIS
#
#   METNO_HAVE_LOG4CXX([ACTION-IF-TRUE], [ACTION-IF-FALSE])
#
# DESCRIPTION
#
#   This macro will check for the existence of the log4cxx library
#   (http://logging.apache.org/log4cxx/).  The check is done by
#   checking for the header file log4cxx/logger.h and the log4cxx
#   library object file.  A --with-log4cxx option is supported as
#   well.  The following output variables are set with AC_SUBST:
#
#     AC_SUBST(LOG4CXX_CPPFLAGS)
#     AC_SUBST(LOG4CXX_LDFLAGS)
#     AC_SUBST(LOG4CXX_LIBS)
#
#   You can use them like this in Makefile.am:
#
#     AM_CPPFLAGS = $(LOG4CXX_CPPFLAGS)
#     AM_LDFLAGS = $(LOG4CXX_LDFLAGS)
#     program_LDADD = $(LOG4CXX_LIBS)
#
#   Additionally, the C preprocessor symbol HAVE_LOG4CXX will be
#   defined with AC_DEFINE([HAVE_LOG4CXX]) if the library is
#   available.
#
# AUTHOR
#
#   Martin Thorsen Ranang <mtr@linpro.no>
#
# THANKS
#
#   This macro was based on the AC_LIB_ID3 macro by Oskar Liljeblad
#   <oskar@osk.mine.nu>.
#
# LAST MODIFICATION
#
#   $Date: 2007-12-07 23:53:49 +0100 (fre, 07 des 2007) $
#
# ID
#
#   $Id: metno_have_log4cxx.m4 778 2007-12-07 22:53:49Z martinr $
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

# METNO_HAVE_LOG4CXX([ACTION-IF-TRUE], [ACTION-IF-FALSE])
# ------------------------------------------------------
AC_DEFUN([METNO_HAVE_LOG4CXX], [
  AH_TEMPLATE([HAVE_LOG4CXX], [Define if log4cxx is available])
  AC_ARG_WITH(log4cxx, 
  	      [  --with-log4cxx=DIR      prefix for log4cxx library files and headers], 
              [if test "$withval" = "no"; then
                 ac_log4cxx_path=
                 $2
               elif test "$withval" = "yes"; then
                 ac_log4cxx_path=/usr
               else
                 ac_log4cxx_path="$withval"
               fi], 
	      [ac_log4cxx_path=/usr])
  if test "$ac_log4cxx_path" != ""; then
    AC_LANG_PUSH(C++)
    saved_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS -I$ac_log4cxx_path/include"
    AC_CHECK_HEADER([log4cxx/logger.h], [
      AC_CHECK_HEADER([log4cxx/basicconfigurator.h])
      saved_LDFLAGS="$LDFLAGS"
      LDFLAGS="$LDFLAGS -L$ac_log4cxx_path/lib"
      saved_LIBS="$LIBS"
      LIBS="$LIBS -llog4cxx"
      AC_MSG_CHECKING([for log4cxx::BasicConfigurator::configure in -llog4cxx])
      AC_LINK_IFELSE(
        [AC_LANG_PROGRAM([\
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
                         ],
			 [log4cxx::BasicConfigurator::configure()])],
        [AC_MSG_RESULT([yes])
	 AC_SUBST(LOG4CXX_CPPFLAGS, [-I$ac_log4cxx_path/include])
         AC_SUBST(LOG4CXX_LDFLAGS, [-L$ac_log4cxx_path/lib])
         AC_SUBST(LOG4CXX_LIBS, [-llog4cxx])
	 AC_DEFINE([HAVE_LOG4CXX])
	 $1
        ], [
        :
	AC_MSG_RESULT([no])
        $2
        ])
      LIBS="$saved_LIBS"
      LDFLAGS="$saved_LDFLAGS"
    ], [
      :
      $2
    ])
    CPPFLAGS="$saved_CPPFLAGS"
    AC_LANG_POP(C++)
  fi
])
