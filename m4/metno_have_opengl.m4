##### http:///                                  -*- Autoconf -*-
#
# WARNING
#
#   This file is a copy of
#   'common_build_files/m4/metno_have_opengl.m4'.  The next time
#   'common_build_files/distribute.sh' is run with the appropriate
#   arguments, all changes to this file will disappear.  Please edit
#   the original.
#
# SYNOPSIS
#
#   METNO_HAVE_OPENGL([ACTION-IF-TRUE], [ACTION-IF-FALSE])
#
# DESCRIPTION
#
#   This macro will check for the existence of the OpenGL library.
#   The check is done by checking for the header files GL/gl.h and
#   the libshp library object file.  A --with-opengl option is
#   supported as well.  The following output variables are set with
#   AC_SUBST:
#
#     AC_SUBST(OPENGL_CPPFLAGS)
#     AC_SUBST(OPENGL_LDFLAGS)
#     AC_SUBST(OPENGL_LIBS)
#
#   You can use them like this in Makefile.am:
#
#     AM_CPPFLAGS = $(OPENGL_CPPFLAGS)
#     AM_LDFLAGS = $(OPENGL_LDFLAGS)
#     program_LDADD = $(OPENGL_LIBS)
#
#   Additionally, the C preprocessor symbol HAVE_OPENGL will be
#   defined with AC_DEFINE([HAVE_OPENGL]) if the library is
#   available.
#
# AUTHOR
#
#   Martin Thorsen Ranang <mtr@linpro.no>
#
# LAST MODIFICATION
#
#   $Date: 2007-12-07 16:13:40 +0100 (Fri, 07 Dec 2007) $
#
# ID
#
#   $Id: metno_have_opengl.m4 774 2007-12-07 15:13:40Z martinr $
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

# METNO_HAVE_GL([ACTION-IF-TRUE], [ACTION-IF-FALSE])
# ------------------------------------------------------
AC_DEFUN([METNO_HAVE_GL], [
  AC_LANG_PUSH(C)

  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([AC_PATH_X])
  AC_REQUIRE([AC_PATH_XTRA])

  AH_TEMPLATE([HAVE_GL], [Define if GL is available])
  AC_ARG_WITH([gl],
              [  --with-GL=DIR           prefix for GL/MesaGL library files and headers],
              [if test "$withval" = "no"; then
                 metno_gl_path=
                 $2
               elif test "$withval" = "yes"; then
                 metno_gl_path=/usr
               else
                 metno_gl_path="$withval"
               fi],
              [metno_gl_path=/usr])
  if test "$metno_gl_path" != ""; then
    # Check for GL.
    saved_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS -I$metno_gl_path/include"
    AC_CHECK_HEADER([GL/gl.h], [
      saved_LDFLAGS="$LDFLAGS"
      LDFLAGS="$LDFLAGS -L$metno_gl_path/lib"
      saved_LIBS="$LIBS"
      #LIBS="$LIBS -lGL"
      AC_CHECK_LIB([GL], [glAccum],
        [AC_SUBST(GL_CPPFLAGS, [-I$metno_gl_path/include])
         AC_SUBST(GL_LDFLAGS, [-L$metno_gl_path/lib])
         AC_SUBST(GL_LIBS, [-lGL])
         AC_DEFINE([HAVE_GL])
         metno_have_gl=yes
         $1
        ], [
        :
        $2
        ])
      LIBS="$saved_LIBS"
      LDFLAGS="$saved_LDFLAGS"
    ], [
      :
      $2
    ])
    CPPFLAGS="$saved_CPPFLAGS"

  fi
  AC_LANG_POP(C)
])

# METNO_HAVE_GLU([ACTION-IF-TRUE], [ACTION-IF-FALSE])
# ------------------------------------------------------
AC_DEFUN([METNO_HAVE_GLU], [
  AC_LANG_PUSH(C)

  AC_REQUIRE([METNO_HAVE_GL])

  AH_TEMPLATE([HAVE_GLU], [Define if GLU is available])
  AC_ARG_WITH([glu],
              [  --with-GLU=DIR          prefix for GLU library files and headers],
              [if test "$withval" = "no"; then
                 metno_glu_path=
                 $2
               elif test "$withval" = "yes"; then
                 metno_glu_path=/usr
               else
                 metno_glu_path="$withval"
               fi],
              [metno_glu_path=/usr])
  if test "$metno_glu_path" != ""; then
    # Check for GLU.
    saved_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS -I$metno_glu_path/include"
    AC_CHECK_HEADER([GL/glu.h], [
      saved_LDFLAGS="$LDFLAGS"
      LDFLAGS="$LDFLAGS -L$metno_glu_path/lib"
      saved_LIBS="$LIBS"
      #LIBS="$LIBS -lGLU"
      AC_CHECK_LIB([GLU], [gluBeginCurve],
        [AC_SUBST(GLU_CPPFLAGS, [-I$metno_glu_path/include])
         AC_SUBST(GLU_LDFLAGS, [-L$metno_glu_path/lib])
         AC_SUBST(GLU_LIBS, [-lGLU])
         AC_DEFINE([HAVE_GLU])
         metno_have_glu=yes
         $1
        ], [
        :
        $2
        ])
      LIBS="$saved_LIBS"
      LDFLAGS="$saved_LDFLAGS"
    ], [
      :
      $2
    ])
    CPPFLAGS="$saved_CPPFLAGS"

  fi
  AC_LANG_POP(C)
])

# METNO_HAVE_GLUT([ACTION-IF-TRUE], [ACTION-IF-FALSE])
# ------------------------------------------------------
AC_DEFUN([METNO_HAVE_GLUT], [
  AC_LANG_PUSH(C)

  AC_REQUIRE([METNO_HAVE_GLU])

  AH_TEMPLATE([HAVE_GLUT], [Define if glut is available])
  AC_ARG_WITH([glut],
              [  --with-glut=DIR         prefix for glut library files and headers],
              [if test "$withval" = "no"; then
                 metno_glut_path=
                 $2
               elif test "$withval" = "yes"; then
                 metno_glut_path=/usr
               else
                 metno_glut_path="$withval"
               fi],
              [metno_glut_path=/usr])
  if test "$metno_glut_path" != ""; then
    # Check for GLUT.
    saved_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS -I$metno_glut_path/include"
    AC_CHECK_HEADER([GL/glut.h], [
      saved_LDFLAGS="$LDFLAGS"
      LDFLAGS="$LDFLAGS -L$metno_glut_path/lib"
      saved_LIBS="$LIBS"
      #LIBS="$LIBS -lglut"
      AC_CHECK_LIB([glut], [glutInit],
        [AC_SUBST(GLUT_CPPFLAGS, [-I$metno_glut_path/include])
         AC_SUBST(GLUT_LDFLAGS, [-L$metno_glut_path/lib])
         AC_SUBST(GLUT_LIBS, [-lglut])
         AC_DEFINE([HAVE_GLUT])
         metno_have_glut=yes
         $1
        ], [
        :
        $2
        ])
      LIBS="$saved_LIBS"
      LDFLAGS="$saved_LDFLAGS"
    ], [
      :
      $2
    ])
    CPPFLAGS="$saved_CPPFLAGS"

  fi
  AC_LANG_POP(C)
])

# METNO_HAVE_GLX([ACTION-IF-TRUE], [ACTION-IF-FALSE])
# ------------------------------------------------------
AC_DEFUN([METNO_HAVE_GLX], [
  AC_LANG_PUSH(C)

  AC_REQUIRE([METNO_HAVE_GL])

  AH_TEMPLATE([HAVE_GLX], [Define if GLX is available])
  AC_ARG_WITH([glx],
              [  --with-GLX=DIR          prefix for GLX library files and headers],
              [if test "$withval" = "no"; then
                 metno_glx_path=
                 $2
               elif test "$withval" = "yes"; then
                 metno_glx_path=/usr
               else
                 metno_glx_path="$withval"
               fi],
              [metno_glx_path=/usr])
  if test "$metno_glx_path" != ""; then
    # Check for GLX.
    saved_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS -I$metno_glx_path/include"
    AC_CHECK_HEADER([GL/glx.h], [
      # Accept cases where only the GLX header can be found.
      metno_have_glx_header=yes
      AC_SUBST(GLX_CPPFLAGS, [-I$metno_glx_path/include])
      saved_LDFLAGS="$LDFLAGS"
      LDFLAGS="$LDFLAGS -L$metno_glx_path/lib"
      saved_LIBS="$LIBS"
      #LIBS="$LIBS -lGLX"
      AC_CHECK_LIB([GLX], [glxBeginCurve],
        [# Already defined GLX_CPPFLAGS
         AC_SUBST(GLX_LDFLAGS, [-L$metno_glx_path/lib])
         AC_SUBST(GLX_LIBS, [-lGLX])
         AC_DEFINE([HAVE_GLX])
         metno_have_glx=yes
         $1
        ], [
        :
        $2
        ])
      LIBS="$saved_LIBS"
      LDFLAGS="$saved_LDFLAGS"
    ], [
      :
      $2
    ])
    CPPFLAGS="$saved_CPPFLAGS"

  fi
  AC_LANG_POP(C)
])

# METNO_HAVE_OSMESA([ACTION-IF-TRUE], [ACTION-IF-FALSE])
# ------------------------------------------------------
AC_DEFUN([METNO_HAVE_OSMESA], [
  AC_LANG_PUSH(C)

  AH_TEMPLATE([HAVE_OSMESA], [Define if OSMesa is available])
  AC_ARG_WITH([osmesa],
              [  --with-OSMesa=DIR          prefix for OSMesa library files and headers],
              [if test "$withval" = "no"; then
                 metno_osmesa_path=
                 $2
               elif test "$withval" = "yes"; then
                 metno_osmesa_path=/usr
               else
                 metno_osmesa_path="$withval"
               fi],
              [metno_osmesa_path=/usr])
  if test "$metno_osmesa_path" != ""; then
    # Check for OSMesa.
    saved_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS -I$metno_osmesa_path/include"
    AC_CHECK_HEADER([GL/osmesa.h], [
      saved_LDFLAGS="$LDFLAGS"
      LDFLAGS="$LDFLAGS -L$metno_osmesa_path/lib"
      saved_LIBS="$LIBS"
      #LIBS="$LIBS -lOSMesa"
      AC_CHECK_LIB([OSMesa], [OSMesaCreateContext],
        [AC_SUBST(OSMESA_CPPFLAGS, [-I$metno_osmesa_path/include])
         AC_SUBST(OSMESA_LDFLAGS, [-L$metno_osmesa_path/lib])
         AC_SUBST(OSMESA_LIBS, [-lOSMesa])
         AC_DEFINE([HAVE_OSMESA])
         metno_have_osmesa=yes
         $1
        ], [
        :
        $2
        ])
      LIBS="$saved_LIBS"
      LDFLAGS="$saved_LDFLAGS"
    ], [
      :
      $2
    ])
    CPPFLAGS="$saved_CPPFLAGS"

  fi
  AC_LANG_POP(C)
])

# METNO_HAVE_OPENGL([ACTION-IF-TRUE], [ACTION-IF-FALSE])
# ------------------------------------------------------
AC_DEFUN([METNO_HAVE_OPENGL], [
  AC_REQUIRE([METNO_HAVE_GL])
  AC_REQUIRE([METNO_HAVE_GLU])
  AC_REQUIRE([METNO_HAVE_GLUT])
  AC_REQUIRE([METNO_HAVE_GLX])
  #AC_REQUIRE([METNO_HAVE_OSMESA])

  OPENGL_CPPFLAGS="$GL_CPPFLAGS $GLU_CPPFLAGS $GLUT_CPPFLAGS $GLX_CPPFLAGS"
  OPENGL_CPPFLAGS=`echo "$OPENGL_CPPFLAGS" |tr ' ' '\n' |uniq |tr '\n' ' '`
  AC_SUBST([OPENGL_CPPFLAGS])

  OPENGL_LDFLAGS=""
  if test x$GL_LDFLAGS != x ; then
     OPENGL_LDFLAGS="$OPENGL_LDFLAGS $GL_LDFLAGS $GL_LIBS"
  fi

  if test x$GLU_LDFLAGS != x ; then
     OPENGL_LDFLAGS="$OPENGL_LDFLAGS $GLU_LDFLAGS $GLU_LIBS"
  fi
  
  if test x$GLUT_LDFLAGS != x ; then
     OPENGL_LDFLAGS="$OPENGL_LDFLAGS $GLUT_LDFLAGS $GLUT_LIBS"
  fi

  if test x$GLX_LDFLAGS != x ; then
     OPENGL_LDFLAGS="$OPENGL_LDFLAGS $GLX_LDFLAGS $GLX_LIBS"
  fi
  AC_SUBST([OPENGL_LDFLAGS])
  
  OPENGL_LIBS="$GL_LIBS $GLU_LIBS $GLUT_LIBS $GLX_LIBS"
  OPENGL_LIBS=`echo "$OPENGL_LIBS" |tr ' ' '\n' |uniq |tr '\n' ' '`
  AC_SUBST([OPENGL_LIBS])
  
  if test "$metno_have_gl" == "yes" ; then
    AC_DEFINE([HAVE_OPENGL], [], [Define if OpenGL is available])
  fi
  
  # For debugging.
  #echo "metno_have_gl = $metno_have_gl"
  #echo "metno_have_glu = $metno_have_glu"
  #echo "metno_have_glut = $metno_have_glut"
  #echo "metno_have_glx = $metno_have_glx"
  #echo "metno_have_glx_header = $metno_have_glx_header"
  #echo "metno_have_osmesa = $metno_have_osmesa"
  #echo "OPENGL_CPPFLAGS = $OPENGL_CPPFLAGS"
  #echo "OPENGL_LDFLAGS = $OPENGL_LDFLAGS"
  #echo "OPENGL_LIBS = $OPENGL_LIBS"
])
