bin_PROGRAMS =
EXTRA_DIST =
SUBDIRS = po

run: yatta
	./yatta --datadir="$(top_srcdir)/data"

gdb: yatta
	gdb --args ./yatta --datadir="$(top_srcdir)/data"

valgrind: yatta
	valgrind ./yatta --datadir="$(top_srcdir)/data"

cscope: cscope.out
cscope.out: $(yatta_SOURCES)
	cscope -R -b

BROWSE:
	ebrowse $(yatta_SOURCES)

# dist-hook: ChangeLog AUTHORS
ChangeLog:
	git log --stat > ChangeLog
AUTHORS:
	git log --format="%an <%ae>" | sort -u > AUTHORS

include src/Makefile.am
include data/Makefile.am

.PHONY: gen-ChangeLog run gdb valgrind cscope
