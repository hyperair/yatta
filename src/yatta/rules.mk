yatta_SOURCES += \
	src/yatta/options.cc

EXTRA_DIST += \
	src/yatta/options.h

yatta_CXXFLAGS += \
	-DDATADIR=\""$(pkgdatadir)"\"

include src/yatta/ui/rules.mk
include src/yatta/curl/rules.mk
