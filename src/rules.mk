bin_PROGRAMS += yatta
noinst_LTLIBRARIES += libyatta.la

yatta_SOURCES = \
	src/main.cc
libyatta_la_SOURCES =

AM_CXXFLAGS = \
	-D PROGRAMNAME_LOCALEDIR=\""$(PROGRAMNAME_LOCALEDIR)"\" \
	-Wall -Wextra -pedantic \
	$(GTKMM_CFLAGS)

yatta_CXXFLAGS = $(AM_CXXFLAGS)
yatta_LDADD = libyatta.la

libyatta_la_CXXFLAGS = \
	$(AM_CXXFLAGS)


libyatta_la_LIBADD = \
	$(GTKMM_LIBS)

include src/yatta/rules.mk
