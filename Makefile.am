bin_PROGRAMS =
EXTRA_DIST =
SUBDIRS = po
CLEANFILES =

run: yatta
	./yatta

gdb: yatta
	gdb --args ./yatta

valgrind: yatta
	valgrind ./yatta

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

include src/rules.mk

.PHONY: run gdb valgrind cscope
