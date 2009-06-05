#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([yatta], [0.1], [hyperair@gmail.com])
AM_INIT_AUTOMAKE([-Wall gnu dist-bzip2 tar-ustar])

AC_CONFIG_SRCDIR([src/main.cc])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CXX
AC_LANG([C++])

PKG_CHECK_MODULES([GTKMM], [gtkmm-2.4])

AC_CONFIG_FILES([
    Makefile
])
AC_OUTPUT
