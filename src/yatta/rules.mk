libyatta_la_SOURCES += \
	src/yatta/options.cc \
	src/yatta/options.hh \
	src/yatta/ioqueue.cc \
	src/yatta/ioqueue.hh \
	src/yatta/download.cc \
	src/yatta/download.hh

AM_CXXFLAGS += \
	-DDATADIR=\""$(pkgdatadir)"\"

include src/yatta/ui/rules.mk
include src/yatta/curl/rules.mk
