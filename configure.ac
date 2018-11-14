dnl This file will be processed by autoconf (which is called by autogen.sh) to
dnl generate configure

dnl comments starting with # are copied to configure (if after AC_INIT),
dnl comments starting with dnl ("do not list") are dropped.

dnl see http://www.gnu.org/software/autoconf/manual/ for further information

dnl Ensure that a recent enough version of Autoconf is being used.
dnl Only from version 2.65 PACKAGE_URL will be set in config.h.
dnl see http://www.mail-archive.com/autoconf@gnu.org/msg19725.html 
dnl see http://www.gnu.org/software/autoconf/manual/autoconf.html#Versioning 
AC_PREREQ([2.65])

dnl initialize autoconf, this should be the first thing in configure.ac
dnl arguments: full package name, version, email address for bug reports,
dnl tarball name, homepage
AC_INIT([SSR (SoundScape Renderer)],
        [m4_esyscmd_s([git describe 2> /dev/null || date +$USER%Y%m%d%H%M])],
        [ssr@spatialaudio.net],
        [ssr],
        [http://spatialaudio.net/ssr/])

SSR_COPYRIGHT="Copyright © 2016 Division of Applied Acoustics, Chalmers University of Technology\nCopyright © 2014 Institut für Nachrichtentechnik, Universität Rostock\nCopyright © 2012 Quality & Usability Lab, Telekom Innovation Laboratories, TU Berlin\n\nLicense GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\nThis is free software: you are free to change and redistribute it.\nThere is NO WARRANTY, to the extent permitted by law."

AC_DEFINE_UNQUOTED([SSR_COPYRIGHT], ["$SSR_COPYRIGHT"], [SSR copyright notice])

dnl read AUTHORS file and save (reformatted by sed) to SSR_AUTHORS(_QT)
SSR_AUTHORS=esyscmd([cat AUTHORS | sed -e :a -e '$!N;s/\n\n/\\n/;ta' -e '$!N;s/\n/\\n/;ta' -e 's/.*/"&"/'])
SSR_AUTHORS_QT=esyscmd([cat AUTHORS | sed -e 's|^.*:|<EM>&</EM>|' -e 's/ /\&nbsp;/g' -e 's/é/\&eacute;/g' -e 's/ö/\&ouml;/g' | sed -e :a -e '$!N;s/\n/<BR>/;ta' -e 's/.*/"&"/'])

AC_DEFINE_UNQUOTED([SSR_AUTHORS], ["$SSR_AUTHORS"], [List of SSR authors])
AC_DEFINE_UNQUOTED([SSR_AUTHORS_QT], ["$SSR_AUTHORS_QT"],
                   [List of SSR authors (with Qt markup)])

dnl include macro definitions
m4_include([macros.m4])

dnl Check if the correct source directory is used by specifying a file in it
AC_CONFIG_SRCDIR([src/controller.h])

dnl Automake looks here for various helper scripts
AC_CONFIG_AUX_DIR(autotools/config)

dnl this gets rid of a warning message from libtoolize
AC_CONFIG_MACRO_DIR([autotools/m4])

dnl Compute the canonical target-system type variable, 'target', and its three
dnl individual parts 'target_cpu', 'target_vendor', and 'target_os'
AC_CANONICAL_TARGET

dnl Compute the canonical host-system type variable, 'host', and its three
dnl individual parts 'host_cpu', 'host_vendor', and 'host_os'.
dnl AC_CANONICAL_HOST

dnl Runs many macros required for proper operation of the generated Makefiles. 
dnl see http://sources.redhat.com/automake/automake.html#Options for options
dnl
dnl We use the option "foreign" because we do not provide a ChangeLog-file. 
dnl Major changes are documented in the NEWS file.
AM_INIT_AUTOMAKE([foreign std-options subdir-objects -Wall])

dnl Check for pkg-config manually first. If it's not installed the
dnl PKG_PROG_PKG_CONFIG macro won't be defined.
AC_CHECK_PROG([have_pkg_config], [pkg-config], [yes], [no])
AS_IF([test x$have_pkg_config != xyes],
  AC_MSG_ERROR(['pkg-config' is required to install this program]))
dnl check for pkg-config version
PKG_PROG_PKG_CONFIG

dnl Create header file(s) with C preprocessor #define statements (e.g. ENABLE_*)
AC_CONFIG_HEADERS(src/config.h)

dnl AM_MAINTAINER_MODE enables automatic rebuild rules only using 
dnl ./configure --enable-maintainer-mode. This can lead to insecurity.
dnl We avoid using this Macro. 
dnl see: http://www.gnu.org/software/hello/manual/automake/maintainer_002dmode.html#maintainer_002dmode
dnl see: https://cims.nyu.edu/cgi-systems/info2html?(automake)Rebuilding

dnl set the relevant programming language (e.g. for AC_CHECK_HEADER)
dnl this is e.g. necessary for the C++ header eca-control-interface.h
AC_LANG([C++])

dnl check if the user specified CXXFLAGS (as environment variable or
dnl in $prefix/share/config.site
AS_IF([test -n "${CXXFLAGS+x}"], [usercxxflags=yes], [usercxxflags=no])

dnl AC_PROG_CC sets default CFLAGS, this can be disabled with
dnl : ${CFLAGS=""}

dnl AC_PROG_CXX sets default CXXFLAGS, this can be disabled with
dnl : ${CXXFLAGS=""}

dnl Determine a C compiler to use (this is done anyway, we do it explicitly)
AC_PROG_CC

dnl Determine a C++ compiler to use
AC_PROG_CXX

AC_COMPILE_IFELSE([AC_LANG_PROGRAM()], ,
  AC_MSG_ERROR([C++ compiler ($CXX) not found!]))

dnl Add support for the `--enable-shared' and `--disable-shared' configure flags
dnl we don't need that (yet)
LT_INIT
dnl AM_PROG_LIBTOOL

dnl set the output variable LN_S (used in data/Makefile.am)
AC_PROG_LN_S

dnl If the C compiler does not accept the -c and -o options simultaneously,
dnl define NO_MINUS_C_MINUS_O.
dnl TODO: check if we need AM_PROG_CC_C_O!
dnl AM_PROG_CC_C_O

dnl Macro AC_PROG_LD is obsolete.
dnl Please use LT_PATH_LD when necessity is discovered. 
dnl see: http://lists.gnu.org/archive/html/autoconf/2010-05/msg00016.html
dnl AC_PROG_LD

dnl Check how the architecture deals with big-endian.
dnl The default for action-if-true is to define ‘WORDS_BIGENDIAN’. The default
dnl for action-if-false is to do nothing. 
dnl AC_C_BIGENDIAN

dnl AC_DEFINE writes values in config.h
dnl AC_DEFINE_UNQUOTED performs additional shell expansions, e.g. substitution
dnl of variable with value.

AC_CHECK_PROG([have_help2man], [help2man], [yes], [no])
AM_CONDITIONAL([HAVE_HELP2MAN], [test x$have_help2man = xyes])
AS_IF([test x$have_help2man != xyes],
 AC_MSG_WARN([Disabling manpage generation due to missing help2man])
)

ENABLE_AUTO([all],
            [all renderers (use --enable-xyz to re-enable certain renderers)],
[
  AS_IF([test x$enable_all = xno], [have_all=no])
])

ENABLE_AUTO([binaural], [binaural renderer (using HRIRs)],
[
  AS_IF([test x$enable_binaural = xyes -o x$have_all = xyes],
    [SSR_executables="$SSR_executables ssr-binaural"])
])

ENABLE_AUTO([brs], [Binaural Room Synthesis renderer (using BRIRs)],
[
  AS_IF([test x$enable_brs = xyes -o x$have_all = xyes],
    [SSR_executables="$SSR_executables ssr-brs"])
])

ENABLE_AUTO([wfs], [Wave Field Synthesis renderer],
[
  AS_IF([test x$enable_wfs = xyes -o x$have_all = xyes],
    [SSR_executables="$SSR_executables ssr-wfs"])
])

ENABLE_AUTO([vbap], [Vector Base Amplitude Panning renderer],
[
  AS_IF([test x$enable_vbap = xyes -o x$have_all = xyes],
    [SSR_executables="$SSR_executables ssr-vbap"])
])

ENABLE_AUTO([aap], [Ambisonics Amplitude Panning renderer],
[
  AS_IF([test x$enable_aap = xyes -o x$have_all = xyes],
    [SSR_executables="$SSR_executables ssr-aap"])
])

ENABLE_AUTO([generic], [generic renderer],
[
  AS_IF([test x$enable_generic = xyes -o x$have_all = xyes],
    [SSR_executables="$SSR_executables ssr-generic"])
])

ENABLE_AUTO([dca],[Distance-coded Ambisonics renderer],
[
  AS_IF([test x$enable_dca = xyes -o x$have_all = xyes],
    [SSR_executables="$SSR_executables ssr-dca"])
])

dnl Note: For what happens with SSR_executables see src/Makefile.am
AC_SUBST(SSR_executables)

dnl Checking for sndfile
PKG_CHECK_MODULES([SNDFILE], [sndfile >= 1.0],
[
  PKG_FLAGS="$PKG_FLAGS $SNDFILE_CFLAGS"
  LIBS="$LIBS $SNDFILE_LIBS"
])

dnl Checking for FFTW3
PKG_CHECK_MODULES([FFTW], [fftw3f >= 3.0.0],
[
  PKG_FLAGS="$PKG_FLAGS $FFTW_CFLAGS"
  LIBS="$LIBS $FFTW_LIBS"
])

dnl Checking for JACK
PKG_CHECK_MODULES([JACK], [jack >= 0.118.0],
[
  PKG_FLAGS="$PKG_FLAGS $JACK_CFLAGS"
  LIBS="$LIBS $JACK_LIBS"
])

dnl Checking for libxml2
PKG_CHECK_MODULES([LIBXML], [libxml-2.0 >= 2.0],
[
  PKG_FLAGS="$PKG_FLAGS $LIBXML_CFLAGS"
  LIBS="$LIBS $LIBXML_LIBS"
])

dnl by the way, AC_HELP_STRING is deprecated, use AS_HELP_STRING instead!

ENABLE_EXPLICIT([debugging],[debugging symbols, asserts, ...])

AS_IF([test x$have_debugging = xyes],
[
  dnl DEBUGGING_FLAGS="$DEBUGGING_FLAGS -D_GLIBCXX_DEBUG"
],
[
  DEBUGGING_FLAGS="$DEBUGGING_FLAGS -DNDEBUG"
])

dnl overwrite default CXXFLAGS set by AC_PROG_CXX
AS_IF([test x$usercxxflags = xno], [CXXFLAGS="-g"])

dnl select C++11 (unconditionally)
CXXFLAGS="$CXXFLAGS -std=c++11"
AC_MSG_CHECKING([if $CXX supports "-std=c++11"])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM()], AC_MSG_RESULT([yes]),
[
  AC_MSG_RESULT([no])
  AC_MSG_ERROR([C++11 not supported! Please upgrade.])
])

dnl see http://www.open-std.org/JTC1/SC22/WG21/docs/cwg_defects.html#1123
dnl and http://stackoverflow.com/q/11497252/500098
AC_MSG_CHECKING([if $CXX implements core/1123])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[
  struct X { virtual ~X() = default; };
  struct Y : X { virtual ~Y() = default; };
], [])], AC_MSG_RESULT([yes]),
[
  AC_MSG_RESULT([no])
  AC_MSG_ERROR([core/1123 not implemented! Please upgrade.])
])

dnl NOTE: -pthread works on GCC, other compilers may need differnt flags!
AS_IF([test x$GCC = xyes],
      [CXXFLAGS="$CXXFLAGS -pthread"])

ENABLE_AUTO([optimization], [code optimization],
[
  dnl for now, we have only optimization if GCC is used
  AS_IF([test x$GCC = xyes],
  [
    dnl add -O3 to CXXFLAGS (only if the user didn't specify CXXFLAGS):
    AS_IF([test x$usercxxflags = xno], [CXXFLAGS="$CXXFLAGS -O3"])

    dnl common x86 optimization flags
    OPT_FLAGS="$OPT_FLAGS -fomit-frame-pointer -ffast-math -funroll-loops"

    dnl OPT_FLAGS="$OPT_FLAGS -fexpensive-optimizations"

    dnl its a little sad that OS X doesn't make it possible to identify 
    dnl the target_cpu a little more precisely. on os x we always get "i386"
    dnl as the CPU type. we miss out on some possible optimizations as 
    dnl a result. oh well.
    dnl anyway, the user can specify -march with CXXFLAGS, if desired
    AS_IF([test $target_cpu = i586],   [OPT_FLAGS="$OPT_FLAGS -march=i586"],
          [test $target_cpu = i686],   [OPT_FLAGS="$OPT_FLAGS -march=i686"],
          [test $target_cpu = x86_64], [OPT_FLAGS="$OPT_FLAGS -march=k8"])
  ], dnl this is the end of "if GCC", else:
  [
    AC_MSG_WARN([no optimization.........................])
    have_optimization=no
  ])
])

ENABLE_AUTO([mmx], [MMX support],
[
  AS_IF([test x$have_optimization != xyes],
  [
    have_mmx=no
    AS_IF([test x$enable_mmx != x],
      AC_MSG_ERROR([You have to --enable-optimization in order to use MMX!]))
  ],
  [
    AS_IF([echo $target_cpu | egrep '(i.86|x86_64)' >/dev/null],
    [
      AS_IF([test -r /proc/cpuinfo],
      [
        procflags=`grep '^flags' /proc/cpuinfo`
        AS_IF([echo $procflags | grep -s mmx >/dev/null], , [have_mmx=no])
      ],
      [
        # this is not linux, but assume that if the processor
        # is x86 then it doesn't need MMX
        AS_IF([test x$enable_mmx = x],
        [
          have_mmx=no
          AC_MSG_WARN([Assuming your x86/x86_64 system does not need to use MMX. Use --enable-mmx if this is not the case])
        ])
        dnl if --enable-mmx was called explicitly, have_mmx=yes automatically!
      ])

      dnl now actually check for MMX support (if requested)
      AS_IF([test x$have_mmx = xyes],
      [
        AC_MSG_CHECKING([whether we can compile MMX code])
        AC_TRY_LINK( ,
          asm ("movq 0, %mm0");
        ,
        [
          OPT_FLAGS="$OPT_FLAGS -mmmx"
          AC_MSG_RESULT([yes])
        ],
        [
          have_mmx=no
          AC_MSG_RESULT([no])
          AC_MSG_WARN([The assembler does not support the MMX command set.])
        ])
      ])
    ])
  ])
])

ENABLE_AUTO([sse], [SSE support],
[
  AS_IF([test x$have_optimization != xyes],
  [
    have_sse=no
    AS_IF([test x$enable_sse != x],
      AC_MSG_ERROR([You have to --enable-optimization in order to use SSE!]))
  ],
  [
    AS_IF([echo $target_cpu | egrep '(i.86|x86_64)' >/dev/null],
    [
      AS_IF([test -r /proc/cpuinfo],
      [
        procflags=`grep '^flags' /proc/cpuinfo`
        AS_IF([echo $procflags | grep -s sse >/dev/null], , [have_sse=no])
      ],
      [
        # this is not linux, but assume that if the processor
        # is x86 then it supports SSE
        AS_IF([test x$enable_mmx = xno], [have_sse=no],
        AC_MSG_WARN([Assuming your x86/x86_64 system can support SSE. Use --disable-sse if this is not the case]))
      ])

      dnl now actually check for SSE support (if requested)
      AS_IF([test x$have_sse = xyes],
      [
        AC_MSG_CHECKING([whether we can compile SSE code])
        AC_TRY_LINK( ,
          asm ("movntps %xmm0, 0");
        ,
        [
          OPT_FLAGS="$OPT_FLAGS -msse -mfpmath=sse"
          AC_MSG_RESULT([yes])
          dnl the convolver checks for __SSE__
          dnl we check if __SSE__ is defined, just to be sure:
          AC_MSG_CHECKING([for __SSE__])
          AS_IF([test x$usercxxflags = xno],
          [
            CXXFLAGS_BACKUP="$CXXFLAGS"
            dnl add OPT_FLAGS just for the AC_TRY_LINK test:
            CXXFLAGS="$CXXFLAGS $OPT_FLAGS"
          ])
          AC_TRY_LINK( ,
            #ifndef __SSE__
            #error
            #endif
          ,
          [
            AC_MSG_RESULT([yes])
          ],
          [
            AC_MSG_RESULT([no])
            AC_MSG_ERROR([__SSE__ is undefined! Contact $PACKAGE_BUGREPORT!])
          ])
          AS_IF([test x$usercxxflags = xno], [CXXFLAGS="$CXXFLAGS_BACKUP"])
        ],
        [
          have_sse=no
          AC_MSG_RESULT([no])
          AC_MSG_WARN([The assembler does not support the SSE command set.])
        ])
      ])
    ])
  ])
])

ENABLE_AUTO([sse2], [SSE2 support],
[
  AS_IF([test x$have_optimization != xyes],
  [
    have_sse2=no
    AS_IF([test x$enable_sse2 != x],
      AC_MSG_ERROR([You have to --enable-optimization in order to use SSE2!]))
  ],
  [
    AS_IF([echo $target_cpu | egrep '(i.86|x86_64)' >/dev/null],
    [
      AS_IF([test -r /proc/cpuinfo],
      [
        procflags=`grep '^flags' /proc/cpuinfo`
        AS_IF([echo $procflags | grep -s sse2 >/dev/null], , [have_sse2=no])
      ],
      [
        AS_IF([test x$enable_sse = xno], [have_sse2=no],
        AC_MSG_WARN([Assuming your x86/x86_64 system can support SSE2. Use --disable-sse2 if this is not the case]))
      ])

      dnl now actually check for SSE2 support (if requested)
      AS_IF([test x$have_sse2 = xyes],
      [
        AC_MSG_CHECKING([whether we can compile SSE2 code])
        AC_TRY_LINK(
          #include <emmintrin.h>
        ,
          __m128i one;
        ,
        [
          OPT_FLAGS="$OPT_FLAGS -msse2 -mfpmath=sse"
          AC_MSG_RESULT([yes])
          dnl check if __SSE2__ is defined, just to be sure:
          AC_MSG_CHECKING([for __SSE2__])
          AS_IF([test x$usercxxflags = xno],
          [
            CXXFLAGS_BACKUP="$CXXFLAGS"
            dnl add OPT_FLAGS just for the AC_TRY_LINK test:
            CXXFLAGS="$CXXFLAGS $OPT_FLAGS"
          ])
          AC_TRY_LINK( ,
            #ifndef __SSE2__
            #error
            #endif
          ,
          [
            AC_MSG_RESULT([yes])
          ],
          [
            AC_MSG_RESULT([no])
            AC_MSG_ERROR([__SSE2__ is undefined! Contact $PACKAGE_BUGREPORT!])
          ])
          AS_IF([test x$usercxxflags = xno], [CXXFLAGS="$CXXFLAGS_BACKUP"])
        ],
        [
          have_sse=no
          AC_MSG_RESULT([no])
          AC_MSG_WARN([The assembler does not support the SSE2 command set.])
        ])
      ])
    ])
  ])
])

dnl enable/disable GUI, default: enabled
ENABLE_FORCED([gui], [graphical user interface (using Qt)],
[
  # Checking for Qt
  PKG_CHECK_MODULES([QT], [Qt5Core >= 5.0.0 Qt5Gui >= 5.0.0 Qt5OpenGL >= 5.0.0]
    ,
    , [have_gui=no])

  dnl BTW, the use of AC_CHECK_LIB is discouraged in favor of AC_SEARCH_LIBS!

  # Checking for glu (depending on the platform, it's in GL/ or OpenGL/)
  AC_CHECK_HEADER([GL/glu.h], ,
  [
    # this is only needed on MacOSX:
    CPPFLAGS="$CPPFLAGS -iframeworkOpenGL"
    AC_CHECK_HEADER([OpenGL/glu.h], , [have_gui=no])
    LIBS="$LIBS -framework OpenGL"
  ])
  AC_SEARCH_LIBS([gluNewQuadric], [GLU], , [have_gui=no])
  # on some systems this is needed, on the others it doesn't hurt:
  AC_SEARCH_LIBS([glSelectBuffer], [GL], , [have_gui=no])
])

AS_IF([test x$have_gui = xyes],
[
  # settings for Qt
  LIBS="$LIBS $QT_LIBS"
  PKG_FLAGS="$PKG_FLAGS $QT_CFLAGS"
  AC_MSG_CHECKING([if we have to add -fPIC to make Qt happy])
  CPPFLAGS_backup="$CPPFLAGS"
  CPPFLAGS="$CPPFLAGS $QT_CFLAGS"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
  [
    #include <QtCore/QObject>
  ], [])], AC_MSG_RESULT([no]),
  [
    AC_MSG_RESULT([yes])
    PKG_FLAGS="$PKG_FLAGS -fPIC"
  ])
  CPPFLAGS="$CPPFLAGS_backup"
  MOCFLAGS="$QT_CFLAGS"
  AS_IF([which moc 2>&1 >/dev/null],
  [
    QTMOC="moc"
  ],
  [
    QTMOC=`$PKG_CONFIG --variable=moc_location Qt5Core`
  ])
  QTLIBDIR=`$PKG_CONFIG --variable=libdir Qt5Gui`

  AC_SUBST(QTMOC)
  AC_SUBST(QTLIBDIR)
  AC_SUBST(MOCFLAGS)
])

ENABLE_FORCED([ip-interface], [network (TCP/IP) interface (needs Asio, https://think-async.com/)],
[
  CPPFLAGS="$CPPFLAGS -DASIO_STANDALONE"
  AC_CHECK_HEADER([asio.hpp], , [have_ip_interface=no])
])

ENABLE_FORCED([ecasound], [Ecasound soundfile playback/recording],
[
  dnl Checking for libecasoundc. It does not provide pkg-config and installs
  dnl on some systems to non-standard path /usr/include/libecasoundc. 
  AC_MSG_CHECKING([for non-default libecasoundc include directory])
  for incdir in /{usr,opt}/{,local/}include/libecasoundc; do
    AS_IF([test -d $incdir], [ECASOUND_INCLUDE="$incdir"])
  done
  AS_IF([test x$ECASOUND_INCLUDE = x], [AC_MSG_RESULT([none found.])],
  [
    AC_MSG_RESULT([$ECASOUND_INCLUDE])
    CPPFLAGS="$CPPFLAGS -I$ECASOUND_INCLUDE"
  ])

  # now check with possibly redefined CPPFLAGS
  AC_CHECK_HEADER([ecasoundc.h],             , [have_ecasound=no])
  AC_CHECK_HEADER([eca-control-interface.h], , [have_ecasound=no])
  AC_SEARCH_LIBS([eci_init], [ecasoundc],    , [have_ecasound=no])
  AC_MSG_CHECKING([Ecasound library support (see above)])
  AC_MSG_RESULT([$have_ecasound])
])

AS_IF([test x$have_ecasound != xno],
[
  dnl check for Ecasound program (required for soundfile playback)
  AC_CHECK_PROG([have_ecasound_program], [ecasound], [yes], [no])
])

dnl Checking for Polhemus Fastrak/Patriot support
ENABLE_AUTO([polhemus], [Polhemus Fastrak/Patriot tracker support],
[
  AC_MSG_CHECKING([various header files for Polhemus Fastrak/Patriot tracker support])
  AC_MSG_RESULT([see below])
  AC_CHECK_HEADER([termios.h], , [have_polhemus=no])
  AC_CHECK_HEADER([unistd.h],  , [have_polhemus=no])
  AC_CHECK_HEADER([fcntl.h],   , [have_polhemus=no])
  AC_CHECK_HEADER([poll.h],    , [have_polhemus=no])
  AC_MSG_CHECKING([Polhemus Fastrak/Patriot support])
  AC_MSG_RESULT([$have_polhemus])
])

dnl Checking for Razor AHRS support
ENABLE_AUTO([razor], [Razor AHRS tracker support],
[
  AC_MSG_CHECKING([various header files for Razor AHRS tracker support])
  AC_MSG_RESULT([see below])
  AC_CHECK_HEADER([termios.h], , [have_razor=no])
  AC_CHECK_HEADER([unistd.h],  , [have_razor=no])
  AC_CHECK_HEADER([fcntl.h],   , [have_razor=no])
  AC_CHECK_HEADER([poll.h],    , [have_razor=no])
  AC_MSG_CHECKING([Razor AHRS support])
  AC_MSG_RESULT([$have_razor])
])

dnl Checking for VRPN tracker support
ENABLE_AUTO([vrpn], [VRPN tracker support],
[
  AC_MSG_CHECKING([various header files for VRPN tracker support])
  AC_MSG_RESULT([see below])
  AC_CHECK_HEADER([vrpn_Tracker.h],   , [have_vrpn=no])
  AC_SEARCH_LIBS([q_from_axis_angle], [quat], , [have_vrpn=no])
  AC_SEARCH_LIBS([vrpn_MAGIC], [vrpn], , [have_vrpn=no])
  AC_MSG_CHECKING([VRPN tracker support])
  AC_MSG_RESULT([$have_vrpn])
])

dnl Checking for InterSense support
ENABLE_AUTO([intersense], [InterSense tracker support],
[
  AC_MSG_CHECKING([various header files for InterSense tracker support])
  AC_MSG_RESULT([see below])

  dnl the InterSense people changed the API with version 4.04!

  dnl Checking for InterSense header 
  AC_CHECK_HEADER([isense.h], , [have_intersense=no])
  AC_CHECK_HEADER([types.h],  , [have_intersense=no])

  AS_IF([test x$have_intersense = xyes],
  [
    dnl first we check for the newer function
    AC_SEARCH_LIBS([ISD_GetTrackingData], [isense], [have_intersense_404=yes],
      dnl then we check for the older function
      [AC_SEARCH_LIBS([ISD_GetData], [isense],
         [have_intersense_404=no], [have_intersense=no])])
  ])
])

AS_IF([test x$have_intersense = xyes -a x$have_intersense_404 = xyes],
[
  AC_DEFINE([HAVE_INTERSENSE_404], 1,[Using InterSense library version >= 4.04])
])

dnl enable/disable compiler warnings
ENABLE_AUTO([warnings], [compiler warnings])
AS_IF([test x$have_warnings = xyes],
[
  AS_IF([test x$GCC = xyes],
  [
    dnl all warnings ... and a few more!
    WARNING_FLAGS="$WARNING_FLAGS -Wall -Wextra"
    WARNING_FLAGS="$WARNING_FLAGS -pedantic"
    dnl this makes -pedantic less pedantic (Qt often uses "long long"):
    WARNING_FLAGS="$WARNING_FLAGS -Wno-long-long"

    WARNING_FLAGS="$WARNING_FLAGS -Winit-self -Wcast-align"
    WARNING_FLAGS="$WARNING_FLAGS -Wmissing-declarations -Wredundant-decls"
    WARNING_FLAGS="$WARNING_FLAGS -Woverloaded-virtual -Wnon-virtual-dtor"
    WARNING_FLAGS="$WARNING_FLAGS -Wwrite-strings"

    dnl -Wall sets -Wstrict-overflow=1, we set it to 0:
    WARNING_FLAGS="$WARNING_FLAGS -Wstrict-overflow=0"

    dnl those should be enabled at some point:
    dnl WARNING_FLAGS="$WARNING_FLAGS -Wold-style-cast"
    dnl WARNING_FLAGS="$WARNING_FLAGS -Wshadow"

    dnl this may be too annoying:
    dnl WARNING_FLAGS="$WARNING_FLAGS -Wconversion"
    dnl maybe only warn for sign conversions:
    dnl WARNING_FLAGS="$WARNING_FLAGS -Wsign-conversion"

    dnl interesting, but maybe too annoying:
    dnl WARNING_FLAGS="$WARNING_FLAGS -Winline"

    dnl suggestions from Meyers "Effective C++":
    dnl WARNING_FLAGS="$WARNING_FLAGS -Weffc++"

    dnl turn warnings into errors:
    dnl WARNING_FLAGS="$WARNING_FLAGS -Werror"
    dnl WARNING_FLAGS="$WARNING_FLAGS -pedantic-errors"

    dnl part of -Wall and -Wextra:
    dnl WARNING_FLAGS="$WARNING_FLAGS -Wuninitialized"
    dnl part of -pedantic:
    dnl WARNING_FLAGS="$WARNING_FLAGS -Wpointer-arith"
  ])
])

ENABLE_AUTO([isatty], [support for isatty()],
[
  AC_CHECK_HEADER([unistd.h], , [have_isatty=no])
  AC_SEARCH_LIBS([isatty], , , [have_isatty=no])
])

dnl trying to mimic the default setting
pkgdatadir="\$(datadir)/$PACKAGE"

ENABLE_EXPLICIT([app-bundle], [creation of a MacOSX application bundle],
[
  AS_IF([test x$enable_app_bundle = xyes -o x$enable_app_bundle = x],
    [DMG_NAME=SoundScapeRenderer-$PACKAGE_VERSION.dmg],
    [DMG_NAME=$enable_app_bundle])
  dmgrootdir=$(pwd)/MacOSX-App-Bundle
  bundledir=\${dmgrootdir}/SoundScapeRenderer-$PACKAGE_VERSION

  dnl WARNING: any user-specified --prefix, --bindir, ... is overwritten!
  dnl however, it can still be specified at make time (although kinda useless)!
  prefix=\${bundledir}/SoundScapeRenderer.app/Contents
  bindir=\${exec_prefix}/MacOS
  datarootdir=\${prefix}/Resources
  pkgdatadir="\$(datadir)"
  docdir=${bundledir}/Documentation

  dnl linker flag to add space to the output file header padding,
  dnl so when dylibbundler is changing paths to shared libs later on,
  dnl longer paths will fit in too.
  LDFLAGS="$LDFLAGS -headerpad_max_install_names"
])

AC_SUBST(DMG_NAME)
AC_SUBST(dmgrootdir)
AC_SUBST(bundledir)
AC_SUBST(pkgdatadir)

AC_SUBST(OPT_FLAGS)
AC_SUBST(PKG_FLAGS)
AC_SUBST(WARNING_FLAGS)
AC_SUBST(DEBUGGING_FLAGS)

dnl List of output files generated by AC_OUTPUT from their respective *.in files
AC_CONFIG_FILES([Makefile src/Makefile man/Makefile])
AC_CONFIG_FILES([data/Makefile data/MacOSX/Makefile])
AC_CONFIG_FILES([data/MacOSX/Info.plist data/MacOSX/DMG-Layout.applescript])

dnl AC_OUTPUT should be the last command (except maybe some status messages)
dnl It generates and runs config.status, which in turn creates the Makefiles and
dnl any other files resulting from configuration
dnl usage with arguments, e.g. AC_OUTPUT(Makefile src/Makefile), is deprecated!
AC_OUTPUT

echo
echo SSR_executables: $SSR_executables
echo
echo CXX: $CXX
echo
echo CFLAGS: $CFLAGS
echo
echo CXXFLAGS: $CXXFLAGS
echo
echo CPPFLAGS: $CPPFLAGS
echo
echo PKG_FLAGS: $PKG_FLAGS
echo
echo OPT_FLAGS: $OPT_FLAGS
echo
echo WARNING_FLAGS: $WARNING_FLAGS
echo
echo DEBUGGING_FLAGS: $DEBUGGING_FLAGS
echo
echo LIBS: $LIBS
echo
echo LDFLAGS: $LDFLAGS
echo
echo
echo
echo '         ___     '
echo '        /  ___   '
echo '    ___/  /  ___ '
echo '      ___/  /    ' $PACKAGE_STRING:
echo '           /     '
echo '                 '

AS_IF([test x$have_intersense = xyes],
  AS_IF([test x$have_intersense_404 = xyes],
    [intersense_version="version >= 4.04"],
    [intersense_version="version < 4.04"]),
  [intersense_version=no])

gui_string=$have_gui
AS_IF([test x$have_gui = xyes],
  [gui_string="yes"])

echo "|"
echo "| Build with tracker support:"
echo "|    InterSense .......................... : $intersense_version"
echo "|    Polhemus Fastrak/Patriot ............ : $have_polhemus"
echo "|    Razor AHRS .......................... : $have_razor"
echo "|    VRPN ................................ : $have_vrpn"
echo "|"
echo "| Build with Ecasound support ............ : $have_ecasound"
echo "| Build with IP interface ................ : $have_ip_interface"
echo "| Build with GUI ......................... : $gui_string"
echo "|"
echo "| Enable debugging/optimization .......... : $have_debugging/$have_optimization"
AS_IF([test x$have_app_bundle = xyes],
[
echo "| Mac OS X application bundle (disk image) : $DMG_NAME"
],
[
echo "| Install prefix ......................... : $prefix"
])

AS_IF([test x$have_ecasound = xyes -a x$have_ecasound_program != xyes],
[
  echo "|"
  echo "|>  WARNING: Ecasound (the program, not the library) was not found!"
  echo "|>  It is needed for playing (and recording) sound files with the SSR."
])
AS_IF([test x$have_help2man != xyes],
[
  echo "|"
  echo "|>  WARNING: help2man was not found!"
  echo "|>  Disabling building of manpages"
])

echo "|"
echo
AS_IF([test x$have_app_bundle = xyes],
[
echo 'If everything looks OK, continue with "make" and "make dmg".'
],
[
echo 'If everything looks OK, continue with "make" and "make install".'
])
echo

dnl Settings for Vim (http://www.vim.org/), please do not remove:
dnl vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:comments+=bO\:dnl
