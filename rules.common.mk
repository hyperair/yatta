clean all run gdb valgrind cscope check:
	cd $(top_srcdir) && $(MAKE) $@

.PHONY: clean all run gdb valgrind cscope check
