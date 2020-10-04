dnl
dnl AVCodec
dnl

AC_DEFUN([GMERLIN_CHECK_AVCODEC],[

AH_TEMPLATE([HAVE_LIBAVCODEC],
            [Do we have libavcodec installed?])

have_avcodec=false

AVCODEC_BUILD="3345152"

AC_ARG_ENABLE(libavcodec,
[AC_HELP_STRING([--disable-libavcodec],[Disable libavcodec (default: autodetect)])],
[case "${enableval}" in
   yes) test_avcodec=true ;;
   no)  test_avcodec=false ;;
esac],[test_avcodec=true])


if test x$test_avcodec = xtrue; then
ACL_PATH_AVCODEC($AVCODEC_BUILD , have_avcodec="true", have_avcodec="false")
AVCODEC_REQUIRED=$AVCODEC_VERSION

fi

AM_CONDITIONAL(HAVE_LIBAVCODEC, test x$have_avcodec = xtrue)

AC_SUBST(AVCODEC_REQUIRED)
AC_SUBST(AVCODEC_LIBS)
AC_SUBST(AVCODEC_CFLAGS)

if test "x$have_avcodec" = "xtrue"; then
AC_DEFINE([HAVE_LIBAVCODEC])
fi

])

dnl
dnl Avformat
dnl

AC_DEFUN([GMERLIN_CHECK_AVFORMAT],[

AH_TEMPLATE([HAVE_LIBAVFORMAT], [Do we have libavformat installed?])
AH_TEMPLATE([AVFORMAT_HEADER], [libavformat header])

have_avformat=false

AVFORMAT_BUILD="3278080"

AC_ARG_ENABLE(libavformat,
[AC_HELP_STRING([--disable-libavformat],[Disable libavformat (default: autodetect)])],
[case "${enableval}" in
   yes) test_avformat=true ;;
   no)  test_avformat=false ;;
esac],[test_avformat=true])

if test x$test_avformat = xtrue; then

ACL_PATH_AVFORMAT($AVFORMAT_BUILD , have_avformat="true", have_avformat="false")
AVFORMAT_REQUIRED=$AVFORMAT_VERSION

fi

AM_CONDITIONAL(HAVE_LIBAVFORMAT, test x$have_avformat = xtrue)

AC_SUBST(AVFORMAT_REQUIRED)
AC_SUBST(AVFORMAT_LIBS)
AC_SUBST(AVFORMAT_CFLAGS)

if test "x$have_avformat" = "xtrue"; then
AC_DEFINE([HAVE_LIBAVFORMAT])
AC_DEFINE_UNQUOTED(AVFORMAT_HEADER, $AVFORMAT_HEADER)
fi

])


dnl
dnl libpostproc
dnl

AC_DEFUN([GMERLIN_CHECK_LIBPOSTPROC],[

AH_TEMPLATE([HAVE_LIBPOSTPROC],
            [Do we have libpostproc installed?])
AH_TEMPLATE([POSTPROC_HEADER], [libpostproc header])

have_libpostproc=false

LIBPOSTPROC_REQUIRED="51.0.0"

AC_ARG_ENABLE(libpostproc,
[AC_HELP_STRING([--disable-libpostproc],[Disable libpostproc (default: autodetect)])],
[case "${enableval}" in
   yes) test_libpostproc=true ;;
   no)  test_libpostproc=false ;;
esac],[test_libpostproc=true])

if test x$test_libpostproc = xtrue; then

PKG_CHECK_MODULES(LIBPOSTPROC, libpostproc >= $LIBPOSTPROC_REQUIRED,
                  have_libpostproc="true", have_libpostproc="false")
fi

if test x$have_libpostproc = xtrue; then

CFLAGS_save=$CFLAGS
CFLAGS="$CFLAGS $GMERLIN_DEP_CFLAGS $LIBPOSTPROC_CFLAGS"
found_header="false"

AC_TRY_COMPILE([
#include <inttypes.h>
#include <libpostproc/postprocess.h>],[], [found_header="true";POSTPROC_HEADER="<libpostproc/postprocess.h>" ],)


if test $found_header = "false"; then
AC_TRY_COMPILE([
#include <inttypes.h> 
#include <postproc/postprocess.h>],[], [found_header="true";POSTPROC_HEADER="<postproc/postprocess.h>" ],)
fi

if test $found_header = "false"; then
AC_TRY_COMPILE([
#include <inttypes.h> 
#include <postprocess.h>],[],[found_header="true";POSTPROC_HEADER="<postprocess.h>"])
fi

if test $found_header = "false"; then
have_libpostproc=false
fi

CFLAGS="$CFLAGS_save"
       
fi

AC_SUBST(LIBPOSTPROC_REQUIRED)
AC_SUBST(LIBPOSTPROC_LIBS)
AC_SUBST(LIBPOSTPROC_CFLAGS)

AM_CONDITIONAL(HAVE_LIBPOSTPROC, test x$have_libpostproc = xtrue)

if test "x$have_libpostproc" = "xtrue"; then
AC_DEFINE([HAVE_LIBPOSTPROC])
AC_DEFINE_UNQUOTED(POSTPROC_HEADER, $POSTPROC_HEADER)
fi

])

dnl
dnl libswscale
dnl

AC_DEFUN([GMERLIN_CHECK_LIBSWSCALE],[

AH_TEMPLATE([HAVE_LIBSWSCALE],
            [Do we have libswscale installed?])
AH_TEMPLATE([SWSCALE_HEADER], [libswscale header])

have_libswscale=false

LIBSWSCALE_REQUIRED="0.5.0"

AC_ARG_ENABLE(libswscale,
[AC_HELP_STRING([--disable-libswscale],[Disable libswscale (default: autodetect)])],
[case "${enableval}" in
   yes) test_libswscale=true ;;
   no)  test_libswscale=false ;;
esac],[test_libswscale=true])

if test x$test_libswscale = xtrue; then

PKG_CHECK_MODULES(LIBSWSCALE, libswscale >= $LIBSWSCALE_REQUIRED,
                  have_libswscale="true", have_libswscale="false")
fi

if test x$have_libswscale = xtrue; then

CFLAGS_save=$CFLAGS
CFLAGS="$CFLAGS $GMERLIN_DEP_CFLAGS $LIBSWSCALE_CFLAGS"
found_header="false"

AC_TRY_COMPILE([
#include <inttypes.h>
#include <libswscale/swscale.h>],[], [found_header="true";SWSCALE_HEADER="<libswscale/swscale.h>"],)

if test $found_header = "false"; then
AC_TRY_COMPILE([
#include <inttypes.h>
#include <swscale/swscale.h>],[], [found_header="true";SWSCALE_HEADER="<swscale/swscale.h>"],)
fi

if test $found_header = "false"; then
AC_TRY_COMPILE([
#include <inttypes.h>
#include <swscale.h>],[],[found_header="true";SWSCALE_HEADER="<swscale.h>"])
fi

CFLAGS="$CFLAGS_save"
       
fi

AC_SUBST(LIBSWSCALE_REQUIRED)
AC_SUBST(LIBSWSCALE_LIBS)
AC_SUBST(LIBSWSCALE_CFLAGS)

AM_CONDITIONAL(HAVE_LIBSWSCALE, test x$have_libswscale = xtrue)

if test "x$have_libswscale" = "xtrue"; then
AC_DEFINE([HAVE_LIBSWSCALE])
AC_DEFINE_UNQUOTED(SWSCALE_HEADER, $SWSCALE_HEADER)
fi

])

dnl
dnl Check for theora decoder
dnl

AC_DEFUN([GMERLIN_CHECK_THEORADEC],[

AH_TEMPLATE([HAVE_THEORADEC],
            [Do we have theora decoder installed?])

have_theora="false"

THEORADEC_REQUIRED="1.0.0"

AC_ARG_ENABLE(theoradec,
[AC_HELP_STRING([--disable-theoradec],[Disable theoradec (default: autodetect)])],
[case "${enableval}" in
   yes) test_theoradec=true ;;
   no)  test_theoradec=false ;;
esac],[test_theoradec=true])

if test x$test_theoradec = xtrue; then

PKG_CHECK_MODULES(THEORADEC, theoradec, have_theoradec="true", have_theoradec="false")
fi

AC_SUBST(THEORADEC_REQUIRED)
AC_SUBST(THEORADEC_LIBS)
AC_SUBST(THEORADEC_CFLAGS)

AM_CONDITIONAL(HAVE_THEORADEC, test x$have_theoradec = xtrue)

if test "x$have_theoradec" = "xtrue"; then
AC_DEFINE([HAVE_THEORADEC])
fi

])

dnl
dnl Check for theora encoder
dnl

AC_DEFUN([GMERLIN_CHECK_THEORAENC],[

AH_TEMPLATE([HAVE_THEORAENC],
            [Do we have theoraenc installed?])

have_theoraenc="false"

THEORAENC_REQUIRED="1.0.0"

AC_ARG_ENABLE(theoraenc,
[AC_HELP_STRING([--disable-theoraenc],[Disable theoraenc (default: autodetect)])],
[case "${enableval}" in
   yes) test_theoraenc=true ;;
   no)  test_theoraenc=false ;;
esac],[test_theoraenc=true])

if test x$test_theoraenc = xtrue; then

PKG_CHECK_MODULES(THEORAENC, theoraenc, have_theoraenc="true", have_theoraenc="false")
fi

AC_SUBST(THEORAENC_REQUIRED)
AC_SUBST(THEORAENC_LIBS)
AC_SUBST(THEORAENC_CFLAGS)

AM_CONDITIONAL(HAVE_THEORAENC, test x$have_theoraenc = xtrue)

if test "x$have_theoraenc" = "xtrue"; then
AC_DEFINE([HAVE_THEORAENC])
fi

])


dnl
dnl Check for schroedinger
dnl

AC_DEFUN([GMERLIN_CHECK_SCHROEDINGER],[

AH_TEMPLATE([HAVE_SCHROEDINGER],
            [Do we have schroedinger installed?])

have_schroedinger="false"

SCHROEDINGER_REQUIRED="1.0.5"

AC_ARG_ENABLE(schroedinger,
[AC_HELP_STRING([--disable-schroedinger],[Disable schroedinger (default: autodetect)])],
[case "${enableval}" in
   yes) test_schroedinger=true ;;
   no)  test_schroedinger=false ;;
esac],[test_schroedinger=true])

if test x$test_schroedinger = xtrue; then

PKG_CHECK_MODULES(SCHROEDINGER, schroedinger-1.0 >= $SCHROEDINGER_REQUIRED, have_schroedinger="true", have_schroedinger="false")
fi

AC_SUBST(SCHROEDINGER_REQUIRED)
AC_SUBST(SCHROEDINGER_LIBS)
AC_SUBST(SCHROEDINGER_CFLAGS)

AM_CONDITIONAL(HAVE_SCHROEDINGER, test x$have_schroedinger = xtrue)

if test "x$have_schroedinger" = "xtrue"; then
AC_DEFINE([HAVE_SCHROEDINGER])
fi

])

dnl
dnl Check for speex
dnl

AC_DEFUN([GMERLIN_CHECK_SPEEX],[

AH_TEMPLATE([HAVE_SPEEX],
            [Do we have speex installed?])

have_speex="false"

SPEEX_REQUIRED="1.0.4"

AC_ARG_ENABLE(speex,
[AC_HELP_STRING([--disable-speex],[Disable speex (default: autodetect)])],
[case "${enableval}" in
   yes) test_speex=true ;;
   no)  test_speex=false ;;
esac],[test_speex=true])

if test x$test_speex = xtrue; then

PKG_CHECK_MODULES(SPEEX, speex >= $SPEEX_REQUIRED, have_speex="true", have_speex="false")

fi

AC_SUBST(SPEEX_REQUIRED)
AC_SUBST(SPEEX_LIBS)
AC_SUBST(SPEEX_CFLAGS)

AM_CONDITIONAL(HAVE_SPEEX, test x$have_speex = xtrue)

if test "x$have_speex" = "xtrue"; then
AC_DEFINE([HAVE_SPEEX])
fi

])

dnl
dnl Check for mjpegtools
dnl

AC_DEFUN([GMERLIN_CHECK_MJPEGTOOLS],[

AH_TEMPLATE([HAVE_MJPEGTOOLS],
            [Do we have mjpegtools installed?])

have_mjpegtools="false"

MJPEGTOOLS_REQUIRED="1.9.0"

AC_ARG_ENABLE(mjpegtools,
[AC_HELP_STRING([--disable-mjpegtools],[Disable mjpegtools (default: autodetect)])],
[case "${enableval}" in
   yes) test_mjpegtools=true ;;
   no)  test_mjpegtools=false ;;
esac],[test_mjpegtools=true])

if test x$test_mjpegtools = xtrue; then

PKG_CHECK_MODULES(MJPEGTOOLS, mjpegtools >= $MJPEGTOOLS_REQUIRED, have_mjpegtools="true", have_mjpegtools="false")

fi

AC_SUBST(MJPEGTOOLS_REQUIRED)
AC_SUBST(MJPEGTOOLS_LIBS)
AC_SUBST(MJPEGTOOLS_CFLAGS)

AM_CONDITIONAL(HAVE_MJPEGTOOLS, test x$have_mjpegtools = xtrue)

if test "x$have_mjpegtools" = "xtrue"; then
AC_DEFINE([HAVE_MJPEGTOOLS])
fi

])

dnl
dnl Check for pulseaudio
dnl

AC_DEFUN([GMERLIN_CHECK_PULSEAUDIO],[

AH_TEMPLATE([HAVE_PULSEAUDIO],
            [Do we have pulseaudio installed?])

have_pulseaudio="false"

PULSEAUDIO_REQUIRED="0.9.6"

AC_ARG_ENABLE(pulseaudio,
[AC_HELP_STRING([--disable-pulseaudio],[Disable pulseaudio (default: autodetect)])],
[case "${enableval}" in
   yes) test_pulseaudio=true ;;
   no)  test_pulseaudio=false ;;
esac],[test_pulseaudio=true])

if test x$test_pulseaudio = xtrue; then

PKG_CHECK_MODULES(PULSEAUDIO, libpulse-simple >= $PULSEAUDIO_REQUIRED, have_pulseaudio="true", have_pulseaudio="false")

fi

AC_SUBST(PULSEAUDIO_REQUIRED)
AC_SUBST(PULSEAUDIO_LIBS)
AC_SUBST(PULSEAUDIO_CFLAGS)

AM_CONDITIONAL(HAVE_PULSEAUDIO, test x$have_pulseaudio = xtrue)

if test "x$have_pulseaudio" = "xtrue"; then
AC_DEFINE([HAVE_PULSEAUDIO])
fi

])

dnl
dnl Ogg
dnl 

AC_DEFUN([GMERLIN_CHECK_OGG],[

OGG_REQUIRED="1.0"

have_ogg=false
AH_TEMPLATE([HAVE_OGG], [Ogg libraries are there])

AC_ARG_ENABLE(ogg,
[AC_HELP_STRING([--disable-ogg],[Disable ogg (default: autodetect)])],
[case "${enableval}" in
   yes) test_ogg=true ;;
   no)  test_ogg=false ;;
esac],[test_ogg=true])

if test x$test_ogg = xtrue; then
    
XIPH_PATH_OGG(have_ogg=true)
fi

AM_CONDITIONAL(HAVE_OGG, test x$have_ogg = xtrue)
 
if test x$have_ogg = xtrue; then
AC_DEFINE(HAVE_OGG)
fi

AC_SUBST(OGG_REQUIRED)

])



dnl
dnl Vorbis
dnl 

AC_DEFUN([GMERLIN_CHECK_VORBIS],[

VORBIS_REQUIRED="1.0"

have_vorbis=false
AH_TEMPLATE([HAVE_VORBIS], [Vorbis libraries are there])

AC_ARG_ENABLE(vorbis,
[AC_HELP_STRING([--disable-vorbis],[Disable vorbis (default: autodetect)])],
[case "${enableval}" in
   yes) test_vorbis=true ;;
   no)  test_vorbis=false ;;
esac],[test_vorbis=true])

if test x$test_vorbis = xtrue; then
    
XIPH_PATH_VORBIS(have_vorbis=true)
fi

AM_CONDITIONAL(HAVE_VORBIS, test x$have_vorbis = xtrue)
 
if test x$have_vorbis = xtrue; then
AC_DEFINE(HAVE_VORBIS)

OLD_CFLAGS=$CFLAGS
OLD_LIBS=$LIBS

CFLAGS="$VORBIS_CFLAGS"
LIBS="$VORBIS_LIBS"

AC_CHECK_FUNCS(vorbis_synthesis_restart)

CFLAGS="$OLD_CFLAGS"
LIBS="$OLD_LIBS"


fi

AC_SUBST(VORBIS_REQUIRED)

])

dnl
dnl libmpeg2
dnl 

AC_DEFUN([GMERLIN_CHECK_LIBMPEG2],[

LIBMPEG2_REQUIRED="0.4.0"

have_libmpeg2=false
AH_TEMPLATE([HAVE_LIBMPEG2], [libmpeg2 found])

AC_ARG_ENABLE(libmpeg2,
[AC_HELP_STRING([--disable-libmpeg2],[Disable libmpeg2 (default: autodetect)])],
[case "${enableval}" in
   yes) test_libmpeg2=true ;;
   no)  test_libmpeg2=false ;;
esac],[test_libmpeg2=true])

if test x$test_libmpeg2 = xtrue; then


PKG_CHECK_MODULES(LIBMPEG2, libmpeg2 >= $LIBMPEG2_REQUIRED, have_libmpeg2=true, have_libmpeg2=false)

fi

AM_CONDITIONAL(HAVE_LIBMPEG2, test x$have_libmpeg2 = xtrue)

if test x$have_libmpeg2 = xtrue; then
AC_DEFINE(HAVE_LIBMPEG2)
fi

AC_SUBST(LIBMPEG2_REQUIRED)

])

dnl
dnl libtiff
dnl

AC_DEFUN([GMERLIN_CHECK_LIBTIFF],[

AH_TEMPLATE([HAVE_LIBTIFF], [Enable tiff codec])
 
have_libtiff=false
TIFF_REQUIRED="3.5.0"

AC_ARG_ENABLE(libtiff,
[AC_HELP_STRING([--disable-libtiff],[Disable libtiff (default: autodetect)])],
[case "${enableval}" in
   yes) test_libtiff=true ;;
   no)  test_libtiff=false ;;
esac],[test_libtiff=true])

if test x$test_libtiff = xtrue; then
   
OLD_CFLAGS=$CFLAGS
OLD_LIBS=$LIBS

LIBS="$LIBS -ltiff"
CFLAGS="$CFLAGS"
   
AC_MSG_CHECKING(for libtiff)
AC_TRY_LINK([#include <tiffio.h>],
            [TIFF * tiff = (TIFF*)0;
	     int i = 0;
	     /* We ensure the function is here but never call it */
             if(i)
	       TIFFReadRGBAImage(tiff, 0, 0, (uint32*)0, 0);
	     return 0;],
            [have_libtiff=true])
 
case $have_libtiff in
  true) AC_DEFINE(HAVE_LIBTIFF)
        AC_MSG_RESULT(yes)
        TIFF_LIBS=$LIBS;
        TIFF_CFLAGS=$CFLAGS ;;
  false) AC_MSG_RESULT(no); TIFF_LIBS=""; TIFF_CFLAGS="";;
esac
CFLAGS=$OLD_CFLAGS
LIBS=$OLD_LIBS

fi

AC_SUBST(TIFF_CFLAGS)
AC_SUBST(TIFF_LIBS)
AC_SUBST(TIFF_REQUIRED)

AM_CONDITIONAL(HAVE_LIBTIFF, test x$have_libtiff = xtrue)

if test x$have_libtiff = xtrue; then
AC_DEFINE(HAVE_LIBTIFF)
fi

])

dnl
dnl libopenjpeg
dnl

AC_DEFUN([GMERLIN_CHECK_OPENJPEG],[

AH_TEMPLATE([HAVE_OPENJPEG], [Enable openjpeg codec])
 
have_openjpeg=false
OPENJPEG_REQUIRED="1.3"

AC_ARG_ENABLE(openjpeg,
[AC_HELP_STRING([--disable-openjpeg],[Disable openjpeg (default: autodetect)])],
[case "${enableval}" in
   yes) test_openjpeg=true ;;
   no)  test_openjpeg=false ;;
esac],[test_openjpeg=true])

if test x$test_openjpeg = xtrue; then
   
OLD_CFLAGS=$CFLAGS
OLD_LIBS=$LIBS

LIBS="$GMERLIN_DEP_LIBS $LIBS -lopenjpeg -lm"
CFLAGS="$CFLAGS $GMERLIN_DEP_CFLAGS"
   
AC_MSG_CHECKING(for openjpeg)
AC_TRY_LINK([#include <openjpeg.h>],
            [opj_version();
	     return 0;],
            [have_openjpeg=true])
 
case $have_openjpeg in
  true) AC_DEFINE(HAVE_OPENJPEG)
        AC_MSG_RESULT(yes)
        OPENJPEG_LIBS=$LIBS;
        OPENJPEG_CFLAGS=$CFLAGS ;;
  false) AC_MSG_RESULT(no); OPENJPEG_LIBS=""; OPENJPEG_CFLAGS="";;
esac
CFLAGS=$OLD_CFLAGS
LIBS=$OLD_LIBS

fi

AC_SUBST(OPENJPEG_CFLAGS)
AC_SUBST(OPENJPEG_LIBS)
AC_SUBST(OPENJPEG_REQUIRED)

AM_CONDITIONAL(HAVE_OPENJPEG, test x$have_openjpeg = xtrue)

if test x$have_openjpeg = xtrue; then
AC_DEFINE(HAVE_OPENJPEG)
fi

])


dnl
dnl libsmbclient
dnl

AC_DEFUN([GMERLIN_CHECK_SAMBA],[

AH_TEMPLATE([HAVE_SAMBA], [Samba support])
 
have_libsmbclient=false

SAMBA_REQUIRED="3.0.0"

AC_ARG_ENABLE(samba,
[AC_HELP_STRING([--disable-samba],[Disable samba (default autodetect)])],
[case "${enableval}" in
   yes) test_libsmbclient=true ;;
   no)  test_libsmbclient=false ;;
esac],[test_libsmbclient=true])

if test x$test_libsmbclient = xtrue; then
 
OLD_CFLAGS=$CFLAGS
OLD_LIBS=$LIBS

LIBS="-lsmbclient"
CFLAGS=""
 
AC_MSG_CHECKING(for libsmbclient)
AC_TRY_LINK([#include <libsmbclient.h>
             #include <stdio.h>],
            [int i = 0;
	     /* We ensure the function is here but never call it */
             if(i)
		smbc_lseek(0,0,SEEK_SET);
	     return 0;],
            [have_libsmbclient=true])
 
case $have_libsmbclient in
  true) AC_DEFINE(HAVE_SAMBA)
        AC_MSG_RESULT(yes)
        SAMBA_LIBS=$LIBS;
        SAMBA_CFLAGS=$CFLAGS ;;
  false) AC_MSG_RESULT(no); SAMBA_LIBS=""; SAMBA_CFLAGS="";;
esac
CFLAGS=$OLD_CFLAGS
LIBS=$OLD_LIBS

fi

AC_SUBST(SAMBA_CFLAGS)
AC_SUBST(SAMBA_LIBS)
AC_SUBST(SAMBA_REQUIRED)

AM_CONDITIONAL(HAVE_SAMBA, test x$have_libsmbclient = xtrue)

if test x$have_libsmbclient = xtrue; then
AC_DEFINE(HAVE_SAMBA)
fi

])

dnl
dnl PNG
dnl 

AC_DEFUN([GMERLIN_CHECK_LIBPNG],[

AH_TEMPLATE([HAVE_LIBPNG], [Enable png codec])
 
have_libpng=false
PNG_REQUIRED="1.2.2"

AC_ARG_ENABLE(libpng,
[AC_HELP_STRING([--disable-libpng],[Disable libpng (default: autodetect)])],
[case "${enableval}" in
   yes) test_libpng=true ;;
   no)  test_libpng=false ;;
esac],[test_libpng=true])

if test x$test_libpng = xtrue; then
   
OLD_CFLAGS=$CFLAGS
OLD_LIBS=$LIBS

LIBS="$LIBS -lpng -lm -lz"
CFLAGS="$CFLAGS"
 
AC_MSG_CHECKING(for libpng)
AC_TRY_LINK([#include <png.h>],
            [png_structp png_ptr;
             png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                               (png_voidp)0,
                                                NULL, NULL);],
            [have_libpng=true])
 
case $have_libpng in
  true) AC_DEFINE(HAVE_LIBPNG)
        AC_MSG_RESULT(yes)
        PNG_LIBS=$LIBS;
        PNG_CFLAGS=$CFLAGS ;;
  false) AC_MSG_RESULT(no); PNG_LIBS=""; PNG_CFLAGS="";;
esac
CFLAGS=$OLD_CFLAGS
LIBS=$OLD_LIBS

fi

AC_SUBST(PNG_CFLAGS)
AC_SUBST(PNG_LIBS)
AC_SUBST(PNG_REQUIRED)

AM_CONDITIONAL(HAVE_LIBPNG, test x$have_libpng = xtrue)

if test x$have_libpng = xtrue; then
AC_DEFINE(HAVE_LIBPNG)
fi

])

dnl
dnl FAAD2
dnl

AC_DEFUN([GMERLIN_CHECK_FAAD2],[

FAAD2_PREFIX=""
FAAD2_REQUIRED="2.0"
have_faad2="false"
AH_TEMPLATE([HAVE_FAAD2], [Enable FAAD2])
AH_TEMPLATE([HAVE_NEAACDEC_H], [Use new header file for faad2])
AC_ARG_WITH(faad2-prefix, [ --with-faad2-prefix=PFX   Prefix to search for faad2],FAAD2_PREFIX=${withval})


AC_ARG_ENABLE(faad2,
[AC_HELP_STRING([--disable-faad2],[Disable faad2 (default: autodetect)])],
[case "${enableval}" in
   yes) test_faad2=true ;;
   no)  test_faad2=false ;;
esac],[test_faad2=true])

if test x$test_faad2 = xtrue; then

OLD_CFLAGS=$CFLAGS
OLD_LIBS=$LIBS

if test "x$FAAD2_PREFIX" = "x"; then 
CFLAGS="$GMERLIN_DEP_CFLAGS"
LIBS="$GMERLIN_DEP_LIBS -lfaad -lm"
else
CFLAGS="-I$FAAD2_PREFIX/include"
LIBS="-L$FAAD2_PREFIX/lib -lfaad -lm"
fi

dnl
dnl Check for neaacdec.h
dnl

AC_MSG_CHECKING(for neaacdec.h usability for faad2)

  AC_TRY_RUN([
    #include <neaacdec.h>
    #include <stdio.h>
    main()
    {
    int faad_major;
    int faad_minor;
    NeAACDecHandle dec;

    if(sscanf(FAAD2_VERSION, "%d.%d", &faad_major, &faad_minor) < 2)
      return -1;
    dec = NeAACDecOpen();
    if(!dec)
      return -1;
    return 0;
    }
  ],
  [
    # program could be run
    have_faad2="true"
    AC_MSG_RESULT(yes)
    FAAD2_CFLAGS=$CFLAGS
    FAAD2_LIBS=$LIBS
    AC_DEFINE(HAVE_NEAACDEC_H)
  ],
    # program could not be run
    AC_MSG_RESULT(no)
)

if test "x$have_faad2" != "xtrue"; then

AC_MSG_CHECKING(for faad.h usability for faad2)

  AC_TRY_RUN([
    #include <faad.h>
    #include <stdio.h>
    main()
    {
    int faad_major;
    int faad_minor;
    faacDecHandle dec;

    if(sscanf(FAAD2_VERSION, "%d.%d", &faad_major, &faad_minor) < 2)
      return -1;
    dec = faacDecOpen();
    if(!dec)
      return -1;
    return 0;
    }
  ],
  [
    # program could be run
    have_faad2="true"
    AC_MSG_RESULT(yes)
    FAAD2_CFLAGS=$CFLAGS
    FAAD2_LIBS=$LIBS

  ],
    # program could not be run
    AC_MSG_RESULT(no)
)
fi


CFLAGS=$OLD_CFLAGS
LIBS=$OLD_LIBS

fi

AC_SUBST(FAAD2_CFLAGS)
AC_SUBST(FAAD2_LIBS)
AC_SUBST(FAAD2_REQUIRED)
AM_CONDITIONAL(HAVE_FAAD2, test x$have_faad2 = xtrue)

if test x$have_faad2 = xtrue; then
AC_DEFINE(HAVE_FAAD2)
fi

])

dnl
dnl DVDREAD
dnl

AC_DEFUN([GMERLIN_CHECK_DVDREAD],[

DVDREAD_REQUIRED="0.9.5"
AH_TEMPLATE([HAVE_DVDREAD], [Enable libdvdread])
have_dvdread="false"

AC_ARG_ENABLE(dvdread,
[AC_HELP_STRING([--disable-dvdread],[Disable libdvdread (default: autodetect)])],
[case "${enableval}" in
   yes) test_dvdread=true ;;
   no)  test_dvdread=false ;;
esac],[test_dvdread=true])

if test x$test_dvdread = xtrue; then

OLD_CFLAGS=$CFLAGS
OLD_LIBS=$LIBS

CFLAGS="$GMERLIN_DEP_CFLAGS"
LIBS="$GMERLIN_DEP_LIBS -ldvdread"

AC_MSG_CHECKING(for libdvdread >= 0.9.5)

  AC_TRY_RUN([
    #include <dvdread/dvd_reader.h>
    #include <stdio.h>
    main()
    {
#if DVDREAD_VERSION < 905
    return -1;
#else
    return 0;
#endif
    }
  ],
  [
    # program could be run
    have_dvdread="true"
    AC_MSG_RESULT(yes)
    DVDREAD_CFLAGS=""
    DVDREAD_LIBS="-ldvdread"

  ],
    # program could not be run
    AC_MSG_RESULT(no)
)

CFLAGS=$OLD_CFLAGS
LIBS=$OLD_LIBS

fi

AC_SUBST(DVDREAD_CFLAGS)
AC_SUBST(DVDREAD_LIBS)
AC_SUBST(DVDREAD_REQUIRED)
AM_CONDITIONAL(HAVE_DVDREAD, test x$have_dvdread = xtrue)

if test x$have_dvdread = xtrue; then
AC_DEFINE(HAVE_DVDREAD)
fi

])

dnl
dnl FLAC
dnl

AC_DEFUN([GMERLIN_CHECK_FLAC],[

FLAC_REQUIRED="1.1.0"
have_flac="false"

AC_ARG_ENABLE(flac,
[AC_HELP_STRING([--disable-flac],[Disable flac (default: autodetect)])],
[case "${enableval}" in
   yes) test_flac=true ;;
   no)  test_flac=false ;;
esac],[test_flac=true])

if test x$test_flac = xtrue; then

AH_TEMPLATE([HAVE_FLAC], [Enable FLAC])

AC_MSG_CHECKING(for flac)

OLD_CFLAGS=$CFLAGS
OLD_LIBS=$LIBS

CFLAGS="$GMERLIN_DEP_CFLAGS"
LIBS="$GMERLIN_DEP_LIBS -lFLAC -lm"

  AC_TRY_RUN([
    #include <FLAC/stream_decoder.h>
    #include <stdio.h>
    main()
    {
    FILE * version_file;
    int version_major;
    int version_minor;
    int version_patchlevel;
    fprintf(stderr, "FLAC__VERSION_STRING: %s\n", FLAC__VERSION_STRING);
    if(sscanf(FLAC__VERSION_STRING, "%d.%d.%d", &version_major,
              &version_minor, &version_patchlevel) < 3)
      return -1;
    if((version_major != 1) || (version_minor < 1))
      return 1;
    version_file = fopen("flac_version", "w");
    fprintf(version_file, "%d.%d.%d\n", version_major,
            version_minor, version_patchlevel);
    fclose(version_file);
    return 0;
    }
  ],
  [
    # program could be run
    have_flac="true"
    AC_MSG_RESULT(yes)
    FLAC_CFLAGS=$CFLAGS
    FLAC_LIBS=$LIBS
    BGAV_FLAC_MAJOR=`cat flac_version | cut -d . -f 1`
    BGAV_FLAC_MINOR=`cat flac_version | cut -d . -f 2`
    BGAV_FLAC_PATCHLEVEL=`cat flac_version | cut -d . -f 3`
    rm -f flac_version
  ],
    # program could not be run
    AC_MSG_RESULT(no)
)

CFLAGS=$OLD_CFLAGS
LIBS=$OLD_LIBS

fi

AC_SUBST(FLAC_CFLAGS)
AC_SUBST(FLAC_LIBS)
AC_SUBST(FLAC_REQUIRED)

AM_CONDITIONAL(HAVE_FLAC, test x$have_flac = xtrue)

if test x$have_flac = xtrue; then
AC_DEFINE(HAVE_FLAC)
AC_DEFINE_UNQUOTED(BGAV_FLAC_MAJOR, $BGAV_FLAC_MAJOR, Flac major version)
AC_DEFINE_UNQUOTED(BGAV_FLAC_MINOR, $BGAV_FLAC_MINOR, Flac minor version)
AC_DEFINE_UNQUOTED(BGAV_FLAC_PATCHLEVEL, $BGAV_FLAC_PATCHLEVEL, Flac patchlevel)
fi

])

dnl
dnl Musepack
dnl

AC_DEFUN([GMERLIN_CHECK_MUSEPACK],[

have_musepack="false"
MUSEPACK_REQUIRED="1.1"

AC_ARG_ENABLE(musepack,
[AC_HELP_STRING([--disable-musepack],[Disable musepack (default: autodetect)])],
[case "${enableval}" in
   yes) test_musepack=true ;;
   no)  test_musepack=false ;;
esac],[test_musepack=true])

if test x$test_musepack = xtrue; then

OLD_CFLAGS=$CFLAGS
OLD_LIBS=$LIBS

LIBS="$GMERLIN_DEP_LIBS -lmpcdec"
CFLAGS="$GMERLIN_DEP_CFLAGS"

AH_TEMPLATE([HAVE_MUSEPACK], [Enable Musepack])
AC_MSG_CHECKING(for libmpcdec)

  AC_TRY_RUN([
    #include <mpcdec/mpcdec.h>
    #include <stdio.h>
    main()
    {
    mpc_streaminfo si;
    mpc_streaminfo_init(&si);
    return 0;
    }
  ],
  [
    # program could be run
    have_musepack="true"
    AC_MSG_RESULT(yes)
    MUSEPACK_CFLAGS=$CFLAGS
    MUSEPACK_LIBS=$LIBS

  ],
    # program could not be run
    AC_MSG_RESULT(no)
)

CFLAGS=$OLD_CFLAGS
LIBS=$OLD_LIBS

fi

AC_SUBST(MUSEPACK_CFLAGS)
AC_SUBST(MUSEPACK_LIBS)
AC_SUBST(MUSEPACK_REQUIRED)

AM_CONDITIONAL(HAVE_MUSEPACK, test x$have_musepack = xtrue)

if test x$have_musepack = xtrue; then
AC_DEFINE(HAVE_MUSEPACK)
fi

])

dnl
dnl MAD
dnl

AC_DEFUN([GMERLIN_CHECK_MAD],[

MAD_REQUIRED="0.15.0"
AH_TEMPLATE([HAVE_MAD], [Enable MAD])
have_mad="false"

AC_ARG_ENABLE(mad,
[AC_HELP_STRING([--disable-mad],[Disable libmad (default: autodetect)])],
[case "${enableval}" in
   yes) test_mad=true ;;
   no)  test_mad=false ;;
esac],[test_mad=true])

if test x$test_mad = xtrue; then

OLD_CFLAGS=$CFLAGS
OLD_LIBS=$LIBS
   
LIBS="$GMERLIN_DEP_LIBS -lmad"
CFLAGS="$GMERLIN_DEP_CFLAGS"

AC_MSG_CHECKING(for libmad 0.15.x)

  AC_TRY_RUN([
    #include "mad.h"
    #include <stdio.h>
    main()
    {
    struct mad_stream stream;
    int version_major = MAD_VERSION_MAJOR;
    int version_minor = MAD_VERSION_MINOR;
    if((version_major != 0) || (version_minor != 15))
      return -1;
    mad_stream_init(&stream);
    return 0;
    }
  ],
  [
    # program could be run
    have_mad="true"
    AC_MSG_RESULT(yes)
    MAD_CFLAGS=$CFLAGS
    MAD_LIBS=$LIBS

  ],
    # program could not be run
    AC_MSG_RESULT(no)
)

CFLAGS=$OLD_CFLAGS
LIBS=$OLD_LIBS

fi

AC_SUBST(MAD_CFLAGS)
AC_SUBST(MAD_LIBS)
AC_SUBST(MAD_REQUIRED)

AM_CONDITIONAL(HAVE_MAD, test x$have_mad = xtrue)

if test x$have_mad = xtrue; then
AC_DEFINE(HAVE_MAD)
fi

])

dnl
dnl liba52
dnl

AC_DEFUN([GMERLIN_CHECK_LIBA52],[

AH_TEMPLATE([HAVE_LIBA52], [Enable liba52])
have_liba52="false"

AC_ARG_ENABLE(liba52,
[AC_HELP_STRING([--disable-liba52],[Disable liba52 (default: autodetect)])],
[case "${enableval}" in
   yes) test_liba52=true ;;
   no)  test_liba52=false ;;
esac],[test_liba52=true])

if test x$test_liba52 = xtrue; then

OLD_CFLAGS=$CFLAGS
OLD_LIBS=$LIBS
   
LIBS="$GMERLIN_DEP_LIBS -la52 -lm"
CFLAGS="$GMERLIN_DEP_CFLAGS"
LIBA52_REQUIRED="0.7.4"
AC_MSG_CHECKING(for liba52)

  AC_TRY_RUN([
    #include <inttypes.h>
    #include <a52dec/a52.h>
    main()
    {
    a52_state_t * state = a52_init(0);
    return 0;
    }
  ],
  [
    # program could be run
    have_liba52="true"
    AC_MSG_RESULT(yes)
    LIBA52_CFLAGS=$CFLAGS
    LIBA52_LIBS=$LIBS

  ],
    # program could not be run
    AC_MSG_RESULT(no)
)

CFLAGS=$OLD_CFLAGS
LIBS=$OLD_LIBS

fi

AC_SUBST(LIBA52_CFLAGS)
AC_SUBST(LIBA52_LIBS)
AC_SUBST(LIBA52_REQUIRED)

AM_CONDITIONAL(HAVE_LIBA52, test x$have_liba52 = xtrue)

if test x$have_liba52 = xtrue; then
AC_DEFINE(HAVE_LIBA52)
fi

])

dnl
dnl CDrom support
dnl

AC_DEFUN([GMERLIN_CHECK_CDIO],[

AH_TEMPLATE([HAVE_CDIO], [ libcdio found ])

have_cdio="false"
CDIO_REQUIRED="0.76"

AC_ARG_ENABLE(libcdio,
[AC_HELP_STRING([--disable-libcdio],[Disable libcdio (default: autodetect)])],
[case "${enableval}" in
   yes) test_cdio=true ;;
   no)  test_cdio=false ;;
esac],[test_cdio=true])

if test x$test_cdio = xtrue; then
PKG_CHECK_MODULES(CDIO, libcdio >= $CDIO_REQUIRED, have_cdio="true", have_cdio="false")
fi

AM_CONDITIONAL(HAVE_CDIO, test x$have_cdio = xtrue)
AC_SUBST(CDIO_REQUIRED)

if test "x$have_cdio" = "xtrue"; then
AC_DEFINE([HAVE_CDIO])
fi

])

dnl
dnl libdca
dnl

AC_DEFUN([GMERLIN_CHECK_DCA],[

AH_TEMPLATE([HAVE_DCA], [ libdca found ])

have_dca="false"
have_dts="false"

DCA_REQUIRED="0.0.2"

AC_ARG_ENABLE(libcda,
[AC_HELP_STRING([--disable-libdca],[Disable libdca (default: autodetect)])],
[case "${enableval}" in
   yes) test_libdca=true ;;
   no)  test_libdca=false ;;
esac],[test_libdca=true])

if test x$test_libdca = xtrue; then
PKG_CHECK_MODULES(DCA, libdca >= $DCA_REQUIRED, have_dca="true", have_dca="false")

if test "x$have_dca" != "xtrue"; then
PKG_CHECK_MODULES(DCA, libdts >= $DCA_REQUIRED, have_dts="true", have_dts="false")

dnl
dnl Check for old dts.h header
dnl

OLD_CPPFLAGS=$CPPFLAGS
CPPFLAGS="$CFLAGS $GMERLIN_DEP_CFLAGS $DCA_CFLAGS"
AC_CHECK_HEADERS([dts.h])
CPPFLAGS=$OLD_CPPFLAGS


dnl
dnl Some systems need -ldts_pic
dnl

if test x$have_dts = xtrue; then
have_libdts_pic=false
OLD_CFLAGS=$CFLAGS
OLD_LIBS=$LIBS
CFLAGS=$DCA_CFLAGS
LIBS=`pkg-config --libs-only-L libdts`
LIBS="$LIBS -lm"
AC_CHECK_LIB(dts_pic, dts_init, have_libdts_pic=true, have_libdts_pic=false)

if test x$have_libdts_pic = xtrue; then
DCA_LIBS="$LIBS -ldts_pic"
fi

CFLAGS=$OLD_CFLAGS
LIBS=$OLD_LIBS

have_dca="true"

fi
fi
fi

AM_CONDITIONAL(HAVE_DCA, test x$have_dca = xtrue)
AC_SUBST(DCA_REQUIRED)

if test "x$have_dca" = "xtrue"; then
AC_DEFINE([HAVE_DCA])
fi

])



dnl
dnl Ogg
dnl

AC_DEFUN([GMERLIN_CHECK_OGG],[

OGG_REQUIRED="1.1"
have_ogg=false
AH_TEMPLATE([HAVE_OGG], [Ogg libraries are there])

AC_ARG_ENABLE(ogg,
[AC_HELP_STRING([--disable-ogg],[Disable libogg (default: autodetect)])],
[case "${enableval}" in
   yes) test_ogg=true ;;
   no)  test_ogg=false ;;
esac],[test_ogg=true])

if test x$test_ogg = xtrue; then
XIPH_PATH_OGG(have_ogg=true)
fi

AM_CONDITIONAL(HAVE_OGG, test x$have_ogg = xtrue)

if test x$have_ogg = xtrue; then
AC_DEFINE(HAVE_OGG)
fi

AC_SUBST(OGG_REQUIRED)

])

dnl
dnl lame
dnl

AC_DEFUN([GMERLIN_CHECK_LAME],[
LAME_REQUIRED="3.93"
have_lame="false"

AC_ARG_ENABLE(lame,
[AC_HELP_STRING([--disable-lame],[Disable lame (default: autodetect)])],
[case "${enableval}" in
   yes) test_lame=true ;;
   no)  test_lame=false ;;
esac],[test_lame=true])

if test x$test_lame = xtrue; then

OLD_CFLAGS=$CFLAGS
OLD_LIBS=$LIBS

if test x$have_vorbis = xtrue; then
LIBS="$GMERLIN_DEP_LIBS -lmp3lame -lvorbis -lm"
else
LIBS="$GMERLIN_DEP_LIBS -lmp3lame -lm"
fi

CFLAGS="$GMERLIN_DEP_CFLAGS"


AH_TEMPLATE([HAVE_LAME], [Enable lame])
AC_MSG_CHECKING(for lame)
  AC_TRY_RUN([
    #include <lame/lame.h>
    #include <stdio.h>
    main()
    {
    int version_major;
    int version_minor;
    const char * version;
    version = get_lame_version();
    fprintf(stderr, "lame version: %s\n", version);
    if(sscanf(version, "%d.%d", &version_major,
              &version_minor) < 2)
      return -1;
    if((version_major != 3) || (version_minor < 93))
      return 1;
    return 0;
    }
  ],
  [
    # program could be run
    have_lame="true"
    AC_MSG_RESULT(yes)
    LAME_CFLAGS=$CFLAGS
    LAME_LIBS=$LIBS

  ],
    # program could not be run
    AC_MSG_RESULT(no)
)

CFLAGS=$OLD_CFLAGS
LIBS=$OLD_LIBS

fi

AC_SUBST(LAME_CFLAGS)
AC_SUBST(LAME_LIBS)
AC_SUBST(LAME_REQUIRED)

AM_CONDITIONAL(HAVE_LAME, test x$have_lame = xtrue)
if test x$have_lame = xtrue; then
AC_DEFINE(HAVE_LAME)
fi

])

dnl
dnl faac
dnl

AC_DEFUN([GMERLIN_CHECK_FAAC],[

have_faac="false"
FAAC_REQUIRED="1.24"


AC_ARG_ENABLE(faac,
[AC_HELP_STRING([--disable-faac],[Disable faac (default: autodetect)])],
[case "${enableval}" in
   yes) test_faac=true ;;
   no)  test_faac=false ;;
esac],[test_faac=true])

if test x$test_faac = xtrue; then


OLD_CFLAGS=$CFLAGS
OLD_LIBS=$LIBS

AH_TEMPLATE([HAVE_FAAC], [Enable faac])

LIBS="$GMERLIN_DEP_LIBS -lfaac -lm"
CFLAGS="$GMERLIN_DEP_CFLAGS"

AC_MSG_CHECKING(for faac)
AC_TRY_RUN([
    #include <inttypes.h>
    #include <faac.h>
    main()
    {
    int samplerate = 44100, num_channels = 2;
    unsigned long input_samples, output_bytes;
    faacEncHandle enc;
    enc = faacEncOpen(samplerate,
                      num_channels,
                      &input_samples,
                      &output_bytes);

    return 0;
    }
  ],
  [
    # program could be run
    have_faac="true"
    AC_MSG_RESULT(yes)
    FAAC_CFLAGS=$CFLAGS
    FAAC_LIBS=$LIBS

  ],
    # program could not be run
    AC_MSG_RESULT(no)
)

CFLAGS=$OLD_CFLAGS
LIBS=$OLD_LIBS

fi

AC_SUBST(FAAC_CFLAGS)
AC_SUBST(FAAC_LIBS)
AC_SUBST(FAAC_REQUIRED)

AM_CONDITIONAL(HAVE_FAAC, test x$have_faac = xtrue)

if test x$have_faac = xtrue; then
AC_DEFINE(HAVE_FAAC)
fi

])

dnl
dnl libjpeg
dnl

AC_DEFUN([GMERLIN_CHECK_LIBJPEG],[

AH_TEMPLATE([HAVE_LIBJPEG],
            [Do we have libjpeg installed?])

have_libjpeg=false
JPEG_REQUIRED="6b"

AC_ARG_ENABLE(libjpeg,
[AC_HELP_STRING([--disable-libjpeg],[Disable libjpeg (default: autodetect)])],
[case "${enableval}" in
   yes) test_libjpeg=true ;;
   no)  test_libjpeg=false ;;
esac],[test_libjpeg=true])

if test x$test_libjpeg = xtrue; then

OLD_CFLAGS=$CFLAGS
OLD_LIBS=$LIBS
LIBS="$LIBS -ljpeg"
CFLAGS="$CFLAGS"

AC_MSG_CHECKING(for libjpeg)
AC_TRY_LINK([#include <stdio.h>
             #include <jpeglib.h>],
            [struct jpeg_decompress_struct cinfo;
             jpeg_create_decompress(&cinfo);],
            [have_libjpeg=true])
case $have_libjpeg in
  true) AC_DEFINE(HAVE_LIBJPEG)
        AC_MSG_RESULT(yes)
        JPEG_LIBS=$LIBS;
        JPEG_CFLAGS=$CFLAGS;;
  false) AC_MSG_RESULT(no); JPEG_LIBS=""; JPEG_CFLAGS="";;
  * ) AC_MSG_RESULT("Somethings wrong: $have_libjpeg") ;;
esac

CFLAGS=$OLD_CFLAGS
LIBS=$OLD_LIBS

fi

AC_SUBST(JPEG_LIBS)
AC_SUBST(JPEG_CFLAGS)
AC_SUBST(JPEG_REQUIRED)
AM_CONDITIONAL(HAVE_LIBJPEG, test x$have_libjpeg = xtrue)

])

dnl
dnl Linux DVB
dnl

AC_DEFUN([GMERLIN_CHECK_LINUXDVB],[
AH_TEMPLATE([HAVE_LINUXDVB],
            [Linux DVB Support available])

have_linuxdvb="true"
AC_CHECK_HEADERS([linux/dvb/frontend.h linux/dvb/dmx.h], [],
[have_linuxdvb="false"; break] )

if test "x$have_linuxdvb" = "xtrue"; then
AC_DEFINE(HAVE_LINUXDVB)
fi

AM_CONDITIONAL(HAVE_LINUXDVB, test x$have_linuxdvb = xtrue)

])

dnl
dnl OpenGL
dnl
AC_DEFUN([GMERLIN_CHECK_OPENGL],[
AH_TEMPLATE([HAVE_GL],[OpenGL available])
AH_TEMPLATE([HAVE_GLX],[GLX available])

dnl
dnl Search for OpenGL libraries
dnl

OLD_CFLAGS=$CFLAGS
OLD_LIBS=$LIBS

have_GL="true"
AC_SEARCH_LIBS([glBegin], [GL], [], [have_GL="false"], [])

if test "x$have_GL" = "xtrue"; then
AC_TRY_RUN([
#include <GL/gl.h>
int main() { if(0) glBegin(GL_QUADS); return 0;}
],[],[have_GL="false"])
fi

GL_CFLAGS=$CFLAGS
GL_LIBS=$LIBS

CFLAGS="$OLD_CFLAGS"
LIBS="$OLD_LIBS"

dnl
dnl Check for GLX
dnl

OLD_CFLAGS=$CFLAGS
OLD_LIBS=$LIBS

have_GLX="true"
AC_SEARCH_LIBS([glXCreateContext], [GL glx], [], [have_GLX="false"], [])

if test "x$have_GL" = "xtrue"; then
AC_TRY_RUN([
#include <GL/glx.h>
int main() { if(0) glXChooseFBConfig(NULL, 0,
	NULL, NULL); return 0;}],[],[have_GLX="false"])
fi

GLX_CFLAGS=$CFLAGS
GLX_LIBS=$LIBS

CFLAGS="$OLD_CFLAGS"
LIBS="$OLD_LIBS"

if test "x$have_GL" = "xtrue"; then
AC_DEFINE(HAVE_GL)

if test "x$have_GLX" = "xtrue"; then
AC_DEFINE(HAVE_GLX)
fi

fi

AM_CONDITIONAL(HAVE_GL, test x$have_GL = xtrue)
AM_CONDITIONAL(HAVE_GLX, test x$have_GLX = xtrue)

AC_SUBST(GL_CFLAGS)
AC_SUBST(GL_LIBS)
AC_SUBST(GLX_CFLAGS)
AC_SUBST(GLX_LIBS)

])


dnl
dnl GLU
dnl

AC_DEFUN([GMERLIN_CHECK_GLU],[
AH_TEMPLATE([HAVE_GLU],[GLU available])

OLD_CFLAGS=$CFLAGS
OLD_LIBS=$LIBS

have_GLU="true"
AC_SEARCH_LIBS([gluLookAt], [GLU], [], [have_GLU="false"], [])

if test "x$have_GLU" = "xtrue"; then
AC_TRY_RUN([
#include <GL/glu.h>
int main() { if(0) gluLookAt(0, 0, 0, 0, 0, 0, 0, 0, 0); return 0;}],[],[have_GLU="false"])
fi

GLU_CFLAGS=$CFLAGS
GLU_LIBS=$LIBS

CFLAGS="$OLD_CFLAGS"
LIBS="$OLD_LIBS"

if test "x$have_GLU" = "xtrue"; then
AC_DEFINE(HAVE_GLU)
fi

AM_CONDITIONAL(HAVE_GLU, test x$have_GLU = xtrue)

AC_SUBST(GLU_CFLAGS)
AC_SUBST(GLU_LIBS)

])

dnl
dnl inotify
dnl

AC_DEFUN([GMERLIN_CHECK_INOTIFY],[
have_inotify="false"
AH_TEMPLATE([HAVE_INOTIFY], [System supports inotify])
AC_CHECK_FUNC(inotify_init,have_inotify="true";AC_DEFINE(HAVE_INOTIFY))

])

dnl
dnl Semaphores
dnl

AC_DEFUN([GMERLIN_CHECK_SEMAPHORES],[
AH_TEMPLATE([HAVE_POSIX_SEMAPHORES], [System supports POSIX semaphores])

have_posix_semaphores="false"

OLD_LIBS=$LIBS
LIBS="$LIBS -lpthread"

AC_MSG_CHECKING([for POSIX unnamed semaphores]);

  AC_TRY_RUN([
    #include <semaphore.h>
	  
    #include <stdio.h>
    main()
    {
    int result;
    sem_t s;
    result = sem_init(&s, 0, 0);
    if(result)
      return -1;
    return 0;
    }
  ],
  [
    # program could be run
    have_posix_semaphores="true"
    AC_MSG_RESULT(yes)
    AC_DEFINE(HAVE_POSIX_SEMAPHORES)
  ],
    # program could not be run
    AC_MSG_RESULT(no)
)

LIBS=$OLD_LIBS


AM_CONDITIONAL(HAVE_POSIX_SEMAPHORES, test x$have_posix_semaphores = xtrue)

])

dnl
dnl vdpau
dnl

AC_DEFUN([GMERLIN_CHECK_VDPAU],[

AH_TEMPLATE([HAVE_VDPAU], [Enable tiff codec])
 
have_vdpau=false

AC_ARG_ENABLE(vdpau,
[AC_HELP_STRING([--disable-vdpau],[Disable vdpau (default: autodetect)])],
[case "${enableval}" in
   yes) test_vdpau=true ;;
   no)  test_vdpau=false ;;
esac],[test_vdpau=true])

if test x$test_vdpau = xtrue; then
   
OLD_CFLAGS=$CFLAGS
OLD_LIBS=$LIBS

LIBS="$LIBS -lvdpau"
CFLAGS="$CFLAGS"
   
AC_MSG_CHECKING(for vdpau)
AC_TRY_LINK([#include <vdpau/vdpau_x11.h>],
            [int i = 0;
	     /* We ensure the function is here but never call it */
             if(i)
	       vdp_device_create_x11((Display*)0, 0,
		                     (VdpDevice*)0,
				     (VdpGetProcAddress**)0);
	       return 0;],
             [have_vdpau=true])
 
case $have_vdpau in
  true) AC_MSG_RESULT(yes)
        VDPAU_LIBS=$LIBS;
        VDPAU_CFLAGS=$CFLAGS ;;
  false) AC_MSG_RESULT(no); VDPAU_LIBS=""; VDPAU_CFLAGS="";;
esac
CFLAGS=$OLD_CFLAGS
LIBS=$OLD_LIBS

fi

AC_SUBST(VDPAU_CFLAGS)
AC_SUBST(VDPAU_LIBS)
AC_SUBST(VDPAU_REQUIRED)

AM_CONDITIONAL(HAVE_VDPAU, test x$have_vdpau = xtrue)

if test x$have_vdpau = xtrue; then
AC_DEFINE(HAVE_VDPAU)
fi

])

dnl
dnl libshout
dnl

AC_DEFUN([GMERLIN_CHECK_SHOUT],[

AH_TEMPLATE([HAVE_SHOUT],
            [Do we have libshout installed?])

have_shout="false"

SHOUT_REQUIRED="2.2.2"

AC_ARG_ENABLE(libshout,
[AC_HELP_STRING([--disable-libshout],[Disable libshout (default: autodetect)])],
[case "${enableval}" in
   yes) test_shout=true ;;
   no)  test_shout=false ;;
esac],[test_shout=true])

if test x$test_shout = xtrue; then

PKG_CHECK_MODULES(SHOUT, shout >= $SHOUT_REQUIRED, have_shout="true", have_shout="false")
fi

AC_SUBST(SHOUT_REQUIRED)
AC_SUBST(SHOUT_LIBS)
AC_SUBST(SHOUT_CFLAGS)

AM_CONDITIONAL(HAVE_SHOUT, test x$have_shout = xtrue)

if test "x$have_shout" = "xtrue"; then
AC_DEFINE([HAVE_SHOUT])
fi

])
