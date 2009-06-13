bin_PROGRAMS = 
EXTRA_DIST =
SUBDIRS = po

run: yatta
	./yatta --datadir="$(VPATH)/data"

include src/Makefile.am
include data/Makefile.am

.PHONY: run
