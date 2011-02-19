yatta_SOURCES += \
	src/yatta/curl/download.cc \
	src/yatta/curl/manager.cc \
	src/yatta/curl/chunk.cc \
	src/yatta/curl/download.hh \
	src/yatta/curl/manager.hh \
	src/yatta/curl/chunk.hh

yatta_CXXFLAGS += \
	$(CURL_CFLAGS)

yatta_LDADD += \
	$(CURL_LIBS)

include src/yatta/curl/tests/rules.mk
