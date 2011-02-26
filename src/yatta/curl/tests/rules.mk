check_PROGRAMS += curl-check
TESTS += curl-check

curl_check_SOURCES = \
	src/yatta/curl/tests/curl-check.cc

curl_check_LDADD = \
	libyatta.la

curl_check_CXXFLAGS = \
	$(CURL_CFLAGS) \
	$(GTKMM_CFLAGS)
