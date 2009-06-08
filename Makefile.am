AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = 
EXTRA_DIST =
SUBDIRS = po

run: yatta
	./yatta --datadir="./data"

include src/Makefile.am
include data/Makefile.am

.PHONY: run
