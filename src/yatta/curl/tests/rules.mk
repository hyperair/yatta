check_PROGRAMS += curl-check
TESTS += curl-check

curl_check_SOURCES = \
	src/yatta/curl/tests/curl-check.cc \
	src/yatta/curl/download.cc \
	src/yatta/curl/manager.cc \
	src/yatta/curl/chunk.cc \
	src/yatta/curl/ioqueue.cc
curl_check_LDADD = \
	$(CURL_LIBS) \
	$(GTKMM_LIBS)
curl_check_CXXFLAGS = \
	$(CURL_CFLAGS) \
	$(GTKMM_CFLAGS)
