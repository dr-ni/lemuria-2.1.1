AC_INIT
AC_CONFIG_SRCDIR(include/lemuria.h)
AM_CONFIG_HEADER(include/config.h)

AM_INIT_AUTOMAKE(lemuria, 2.1.1-pre)

AC_DISABLE_STATIC
AC_PROG_LIBTOOL

dnl
dnl gmerlin
dnl 

have_gmerlin="false"

GMERLIN_REQUIRED="0.4.2"

AC_ARG_ENABLE(gmerlin,
[AC_HELP_STRING([--disable-gmerlin],[Disable gmerlin plugins (default: autodetect)])],
[case "${enableval}" in
   yes) test_gmerlin=true ;;
   no)  test_gmerlin=false ;;
esac],[test_gmerlin=true])

if test x$test_gmerlin = xtrue; then

PKG_CHECK_MODULES(GMERLIN, gmerlin >= $GMERLIN_REQUIRED, have_gmerlin="true",
have_gmerlin="false")

GMERLIN_PLUGIN_DIR=""
GMERLIN_PLUGIN_LDFLAGS=""

if test "x$have_gmerlin" = "xtrue"; then
GMERLIN_PLUGIN_DIR=`pkg-config --variable=prefix gmerlin`"/lib/gmerlin/plugins"
GMERLIN_PLUGIN_LDFLAGS="-export-symbols "`pkg-config --variable=prefix gmerlin`"/share/gmerlin/plugin.sym"
fi

dnl LDFLAGS for plugins

fi

AC_SUBST(GMERLIN_PLUGIN_DIR)
AC_SUBST(GMERLIN_REQUIRED)
AC_SUBST(GMERLIN_PLUGIN_LDFLAGS)

AM_CONDITIONAL(HAVE_GMERLIN, test x$have_gmerlin = xtrue)

dnl
dnl Libvisual
dnl

LV_REQUIRED="0.5.0"
have_LV=false
AC_ARG_ENABLE(libvisual,
              AC_HELP_STRING(--disable-libvisual, [Disable Libvisual support (default: autodetect)]),
              [case "${enableval}" in
                 yes) test_lv=true ;;
                 no) test_lv=false ;;
               esac],
               test_lv=true)

if test x$test_lv = "xtrue"; then
PKG_CHECK_MODULES(LV, libvisual-0.5 >= $LV_REQUIRED, have_lv=true, have_lv=false)
fi

LV_PLUGIN_DIR=""

if test x$have_lv = "xtrue"; then
LV_PLUGIN_DIR=`pkg-config --variable=pluginsbasedir libvisual-0.5`"/actor"

fi
AM_CONDITIONAL(HAVE_LV, test x$have_lv = xtrue)
AC_SUBST(LV_REQUIRED)
AC_SUBST(LV_PLUGIN_DIR)

dnl
dnl Installation prefix to increase the chances to find goom
dnl

if test "x$prefix" = xNONE; then
  prefix="${ac_default_prefix}"
fi

AC_DEFINE_UNQUOTED(PREFIX, "$prefix", Installation prefix)

dnl
dnl OpenGL
dnl
GMERLIN_CHECK_OPENGL

if test x$have_GL = "xfalse"; then
  AC_MSG_ERROR(Lemuria needs OpenGL headers, 1)
fi

GMERLIN_CHECK_GLU

if test x$have_GLU = "xfalse"; then
  AC_MSG_ERROR(Lemuria needs GLU headers, 1)
fi

dnl
dnl Goom and xaos sizes
dnl

XAOS_SIZE=256
GOOM_SIZE=256

AC_ARG_WITH(xaos-size,
[AC_HELP_STRING([--with-xaos-size=size],
                [Set xaos size, must be power of 2, default: 256])],
                [XAOS_SIZE=${withval}])

AC_ARG_WITH(goom-size,
[AC_HELP_STRING([--with-goom-size=size],
                [Set goom size, must be power of 2, default: 256])],
                [GOOM_SIZE=${withval}])

AC_DEFINE_UNQUOTED(XAOS_SIZE, [$XAOS_SIZE], [Render size for xaos])
AC_DEFINE_UNQUOTED(GOOM_SIZE, [$GOOM_SIZE], [Render size for goom])
dnl
dnl CFLAGS
dnl

LQT_OPT_CFLAGS($host_cpu, ["--fast-math -O2"])

CFLAGS="$OPT_CFLAGS $GL_CFLAGS"

LQT_TRY_CFLAGS(-Wall, CFLAGS="$CFLAGS -Wall", )
LQT_TRY_CFLAGS(-Wmissing-declarations, CFLAGS="$CFLAGS -Wmissing-declarations", )
LQT_TRY_CFLAGS(-Wdeclaration-after-statement, CFLAGS="$CFLAGS -Wdeclaration-after-statement")


if test "x$LQT_DEBUG" = "xtrue"; then
  CFLAGS="$CFLAGS -DDEBUG"
fi

AC_CONFIG_SUBDIRS(lib/xaos)

AC_SUBST(GL_LIBS,   $GL_LIBS)

AC_OUTPUT(Makefile m4/Makefile lib/Makefile lib/meshes/Makefile lib/images/Makefile lib/gradients/Makefile plugins/Makefile plugins/gmerlin/Makefile plugins/libvisual/Makefile include/Makefile utils/Makefile lemuria.spec)

echo
echo
echo "Lemuria configuration summary"
echo
echo

echo -n "Libvisual: "
if test "x$have_lv" = "xtrue"; then
echo "Yes (CFLAGS=$LV_CFLAGS LIBS=$LV_LIBS)"
else
if test "x$test_lv" = "xtrue"; then
echo "Missing (Go to https://github.com/Libvisual/libvisual/)"
else
echo "Disabled"
fi
fi

echo -n "Gmerlin:   "
if test "x$have_gmerlin" = "xtrue"; then
echo "Yes (CFLAGS=$GMERLIN_CFLAGS LIBS=$GMERLIN_LIBS)"
else
if test "x$test_gmerlin" = "xtrue"; then
echo "Missing (Go to http://gmerlin.sourceforge.net/)"
else
echo "Disabled"
fi
fi

echo
echo "If you installed a library but it was not detected, check the file INSTALL"
echo "for troubleshooting tips. Also note that if the configure script reaches"
echo "this point, all missing packages are optional so compilation should succeed"
echo "anyway."
