bin_PROGRAMS = 
EXTRA_DIST =
SUBDIRS = po

run: yatta
	./yatta --datadir="$(top_srcdir)/data"

gdb: yatta
	gdb ./yatta --datadir="$(top_srcdir)/data"

valgrind: yatta
	valgrind ./yatta --datadir="$(top_srcdir)/data"

cscope: cscope.out
cscope.out: $(yatta_SOURCES)
	cscope -R -b

include src/Makefile.am
include data/Makefile.am

.PHONY: run gdb valgrind cscope
