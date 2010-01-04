yatta_SOURCES += \
	src/yatta/curl/download.cc \
	src/yatta/curl/manager.cc \
	src/yatta/curl/chunk.cc \
	src/yatta/curl/ioqueue.cc \
	src/yatta/curl/download.h \
	src/yatta/curl/manager.h \
	src/yatta/curl/chunk.h \
	src/yatta/curl/ioqueue.h

yatta_CXXFLAGS += \
	$(CURL_CFLAGS)

yatta_LDADD += \
	$(CURL_LIBS)

include src/yatta/curl/tests/rules.mk
