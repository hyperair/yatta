clean all run gdb valgrind cscope:
	cd $(top_srcdir) && $(MAKE) $@

.PHONY: clean all run gdb valgrind cscope
