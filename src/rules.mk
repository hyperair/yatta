bin_PROGRAMS += yatta

yatta_SOURCES = \
	src/main.cc

yatta_CXXFLAGS = \
	-D PROGRAMNAME_LOCALEDIR=\""$(PROGRAMNAME_LOCALEDIR)"\" \
	$(AM_CFLAGS) \
	$(GTKMM_CFLAGS)

yatta_LDADD = \
	$(GTKMM_LIBS)

include src/yatta/rules.mk
