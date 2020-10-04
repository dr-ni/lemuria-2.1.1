dnl @synopsis MDL_CHECK_LIBM
dnl
dnl Search for math library (typically -lm).
dnl
dnl The variable LIBM (which is not an output variable by default) is
dnl set to a value which is suitable for use in a Makefile (for example,
dnl in make's LOADLIBES macro) provided you AC_SUBST it first.
dnl
dnl @version 0.01 $Id: acinclude.m4,v 1.2 2007/11/19 12:38:51 gmerlin Exp $
dnl @author Matthew D. Langston <langston@SLAC.Stanford.EDU>
                                                                                
# AC_CHECK_LIBM - check for math library
AC_DEFUN([MDL_CHECK_LIBM],
[AC_REQUIRE([AC_CANONICAL_HOST])dnl
LIBM=
case "$host" in
*-*-beos* | *-*-cygwin*)
  # These system don't have libm
  ;;
*-ncr-sysv4.3*)
  AC_CHECK_LIB(mw, _mwvalidcheckl, LIBM="-lmw")
  AC_CHECK_LIB(m, main, LIBM="$LIBM -lm")
  ;;
*)
  AC_CHECK_LIB(m, main, LIBM="-lm")
  ;;
esac
])

dnl ########################### -*- Mode: M4 -*- #######################
dnl Copyright (C) 98, 1999 Matthew D. Langston <langston@SLAC.Stanford.EDU>
dnl
dnl This file is free software; you can redistribute it and/or modify it
dnl under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl
dnl This file is distributed in the hope that it will be useful, but
dnl WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this file; if not, write to:
dnl
dnl   Free Software Foundation, Inc.
dnl   Suite 330
dnl   59 Temple Place
dnl   Boston, MA 02111-1307, USA.
dnl ####################################################################


dnl @synopsis MDL_HAVE_OPENGL
dnl 
dnl Search for OpenGL.  We search first for Mesa (a GPL'ed version of
dnl OpenGL) before a vendor's version of OpenGL, unless we were
dnl specifically asked not to with `--with-Mesa=no' or `--without-Mesa'.
dnl
dnl The four "standard" OpenGL libraries are searched for: "-lGL",
dnl "-lGLU", "-lGLX" (or "-lMesaGL", "-lMesaGLU" as the case may be) and
dnl "-lglut".
dnl
dnl All of the libraries that are found (since "-lglut" or "-lGLX" might
dnl be missing) are added to the shell output variable "GL_LIBS", along
dnl with any other libraries that are necessary to successfully link an
dnl OpenGL application (e.g. the X11 libraries).  Care has been taken to
dnl make sure that all of the libraries in "GL_LIBS" are listed in the
dnl proper order.
dnl
dnl Additionally, the shell output variable "GL_CFLAGS" is set to any
dnl flags (e.g. "-I" flags) that are necessary to successfully compile
dnl an OpenGL application.
dnl
dnl The following shell variable (which are not output variables) are
dnl also set to either "yes" or "no" (depending on which libraries were
dnl found) to help you determine exactly what was found.
dnl
dnl   have_GL
dnl   have_GLU
dnl   have_GLX
dnl   have_glut
dnl
dnl A complete little toy "Automake `make distcheck'" package of how to
dnl use this macro is available at:
dnl
dnl   ftp://ftp.slac.stanford.edu/users/langston/autoconf/ac_opengl-0.01.tar.gz
dnl
dnl Please note that as the ac_opengl macro and the toy example evolves,
dnl the version number increases, so you may have to adjust the above
dnl URL accordingly.
dnl
dnl @version 0.01 $Id: acinclude.m4,v 1.2 2007/11/19 12:38:51 gmerlin Exp $
dnl @author Matthew D. Langston <langston@SLAC.Stanford.EDU>

AC_DEFUN([MDL_HAVE_OPENGL],
[
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([AC_PATH_X])
  AC_REQUIRE([AC_PATH_XTRA])
  AC_REQUIRE([MDL_CHECK_LIBM])

  AC_CACHE_CHECK([for OpenGL], mdl_cv_have_OpenGL,
  [
dnl Check for Mesa first, unless we were asked not to.
    AC_ARG_ENABLE(Mesa,
                  AC_HELP_STRING([--with-Mesa],	
                  [Prefer the Mesa library over a vendors native OpenGL library (default=no)]),
                  use_Mesa=$enableval, use_Mesa=no)

    if test x"$use_Mesa" = xyes; then
       GL_search_list="MesaGL   GL"
      GLU_search_list="MesaGLU GLU"
      GLX_search_list="MesaGLX GLX"
    else
       GL_search_list="GL  MesaGL"
      GLU_search_list="GLU MesaGLU"
      GLX_search_list="GLX MesaGLX"
    fi      

    AC_LANG_SAVE
    AC_LANG_C

dnl If we are running under X11 then add in the appropriate libraries.
    if ! test x"$no_x" = xyes; then
dnl Add everything we need to compile and link X programs to GL_CFLAGS
dnl and GL_X_LIBS.
      GL_CFLAGS="$X_CFLAGS"
      GL_X_LIBS="$X_PRE_LIBS $X_LIBS -lX11 -lXext $X_EXTRA_LIBS $LIBM"
    fi
    GL_save_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$GL_CFLAGS"

    GL_save_LIBS="$LIBS"
    LIBS="$GL_X_LIBS"

    # Save the "AC_MSG_RESULT file descriptor" to FD 8.
    exec 8>&AC_FD_MSG

    # Temporarily turn off AC_MSG_RESULT so that the user gets pretty
    # messages.
    exec AC_FD_MSG>/dev/null

    AC_SEARCH_LIBS(glAccum,          $GL_search_list, have_GL=yes,   have_GL=no)
    AC_SEARCH_LIBS(gluBeginCurve,   $GLU_search_list, have_GLU=yes,  have_GLU=no)
    AC_SEARCH_LIBS(glXChooseVisual, $GLX_search_list, have_GLX=yes,  have_GLX=no)

    # Restore pretty messages.
    exec AC_FD_MSG>&8

    if test -n "$LIBS"; then
      mdl_cv_have_OpenGL=yes
      GL_LIBS="$LIBS"
      AC_SUBST(GL_CFLAGS)
      AC_SUBST(GL_LIBS)
    else
      mdl_cv_have_OpenGL=no
      GL_CFLAGS=
    fi

dnl Reset GL_X_LIBS regardless, since it was just a temporary variable
dnl and we don't want to be global namespace polluters.
    GL_X_LIBS=

    LIBS="$GL_save_LIBS"
    CPPFLAGS="$GL_save_CPPFLAGS"

    AC_LANG_RESTORE
  ])
])
#
# Automatic OPT_CFLAGS generation by Burkhard Plaum (2005-05-24)
#

dnl LQT_TRY_CFLAGS (CFLAGS, [ACTION-IF-WORKS], [ACTION-IF-FAILS])
dnl check if $CC supports a given set of cflags
AC_DEFUN([LQT_TRY_CFLAGS],
    [AC_MSG_CHECKING([if $CC supports $1 flags])
    SAVE_CFLAGS="$CFLAGS"
    CFLAGS="$1"
    AC_TRY_COMPILE([],[],[lqt_try_cflags_ok=yes],[lqt_try_cflags_ok=no])
    CFLAGS="$SAVE_CFLAGS"
    AC_MSG_RESULT([$lqt_try_cflags_ok])
    if test x"$lqt_try_cflags_ok" = x"yes"; then
        ifelse([$2],[],[:],[$2])
    else
        ifelse([$3],[],[:],[$3])
    fi])

dnl LQT_OPT_CFLAGS(CPU_TYPE [, ADDITIONAL_OPT_FLAGS])
dnl Get proper optimization flags. 
dnl
dnl CPU_TYPE: host_cpu variable as obtained from AC_CANONICAL_HOST
dnl ADDITIONAL_OPT_FLAGS:  Additional optimization flags (e.g. -O3 --fast-math)
dnl On output, the Variable LQT_OPT_CFLAGS will be set to the compiler flags
dnl Furthermore, is debuggind was requested, the variable LQT_DEBUG will be
dnl set to "true"

AC_DEFUN([LQT_OPT_CFLAGS],[

dnl
dnl Debugging Support
dnl


LQT_DEBUG=false

AC_ARG_ENABLE(debug,
[AC_HELP_STRING([--enable-debug],[Enable debugging, disable optimization])],
[case "${enableval}" in
   yes) LQT_DEBUG=true ;;
   no)  LQT_DEBUG=false ;;
esac],[LQT_DEBUG=false])


dnl
dnl Extra cflags from the commandline. Can have the special values "none" or "auto"
dnl

AC_ARG_WITH(cpuflags, 
AC_HELP_STRING([--with-cpuflags],[Set CPU specific compiler flags. Default is auto, which
               does autodetection. Specify none for compiling the most portable binaries]),
               lqt_cpuflags="$withval", lqt_cpuflags="auto")

if test x$lqt_cpuflags = xnone; then
lqt_cpuflags=""
fi

dnl
dnl Autodetect CPU specific flags
dnl

if test x$lqt_cpuflags = xauto; then

lqt_cpu_family=""
case [$1] in
  i[[3-7]]86)
    lqt_cpu_family=x86;;
  x86_64*)
    lqt_cpu_family=x86;;
  powerpc | powerpc64)
    lqt_cpu_family=ppc;;
  *)
    lqt_cpu_family="";;
esac

if test x$lqt_cpu_family = x; then
  lqt_cpuflags=""
else
  lqt_cpuflags=`$srcdir/cpuinfo.sh $lqt_cpu_family`
fi

fi

dnl
dnl Build the final flags
dnl

lqt_additional_opt_flags=ifelse([$2],[],[],[$2])

lqt_test_flags=$lqt_cpuflags

if test x$LQT_DEBUG = xtrue; then
  lqt_test_cflags="$lqt_test_flags -g"
else
  lqt_test_cflags="$lqt_test_flags $lqt_additional_opt_flags"
fi

OPT_CFLAGS=""
for i in $lqt_test_cflags; do
  LQT_TRY_CFLAGS($i, OPT_CFLAGS="$OPT_CFLAGS $i") ;
done

])
