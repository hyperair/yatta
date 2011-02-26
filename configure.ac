#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([yatta], [0.1], [hyperair@gmail.com])
AM_INIT_AUTOMAKE([-Wall foreign dist-bzip2 tar-ustar subdir-objects])

dnl silent rules
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AM_MAINTAINER_MODE([enable])

AC_CONFIG_SRCDIR([src/main.cc])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CXX
AC_PROG_LIBTOOL

dnl i18n bits
IT_PROG_INTLTOOL([0.35.0])
GETTEXT_PACKAGE=yatta
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(
    [GETTEXT_PACKAGE],
    ["$GETTEXT_PACKAGE"],
    [The domain to use with gettext])
AM_GLIB_GNU_GETTEXT
PROGRAMNAME_LOCALEDIR=[${datadir}/locale]
AC_SUBST(PROGRAMNAME_LOCALEDIR)
dnl end i18n bits

dnl program dependencies
PKG_CHECK_MODULES([GTKMM], [gtkmm-2.4])
PKG_CHECK_MODULES([CURL], [libcurl])
PKG_CHECK_MODULES([LIBXML], [libxml++-2.6])

AC_CONFIG_FILES([
    Makefile
    src/Makefile
    src/yatta/Makefile
    src/yatta/curl/Makefile
    src/yatta/curl/tests/Makefile
    src/yatta/ui/Makefile
    po/Makefile.in
])
AC_OUTPUT
