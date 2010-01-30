yatta_SOURCES += \
	src/yatta/options.cc \
	src/yatta/options.hh

yatta_CXXFLAGS += \
	-DDATADIR=\""$(pkgdatadir)"\"

include src/yatta/ui/rules.mk
include src/yatta/curl/rules.mk
