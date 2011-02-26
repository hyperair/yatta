libyatta_la_SOURCES += \
	src/yatta/curl/download.cc \
	src/yatta/curl/manager.cc \
	src/yatta/curl/chunk.cc \
	src/yatta/curl/download.hh \
	src/yatta/curl/manager.hh \
	src/yatta/curl/chunk.hh

libyatta_la_CXXFLAGS += \
	$(CURL_CFLAGS)

libyatta_la_LIBADD += \
	$(CURL_LIBS)

include src/yatta/curl/tests/rules.mk
