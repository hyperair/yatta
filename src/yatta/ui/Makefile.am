yatta_SOURCES += \
	src/yatta/ui/main.cc \
	src/yatta/ui/mainwindow.cc \
	src/yatta/ui/mainwindow-main_menu.cc \
	src/yatta/ui/mainwindow-main_tb.cc \
	src/yatta/ui/aboutdialog.cc

EXTRA_DIST += \
	src/yatta/ui/main.h \
	src/yatta/ui/mainwindow.h \
	src/yatta/ui/aboutdialog.h \
	src/yatta/ui/main_menu.ui \
	src/yatta/ui/main_tb.ui

CLEANFILES += \
	src/yatta/ui/mainwindow-main_menu.cc \
	src/yatta/ui/mainwindow-main_tb.cc

# embed .ui files into executable
src/yatta/ui/mainwindow-%.cc: src/yatta/ui/%.ui
	( echo "#include \"mainwindow.h\""; \
	echo "namespace Yatta { namespace UI {"; \
	echo "const char *MainWindow::$(shell echo $< | \
		sed -re 's/^.*\///; s/\.ui$$//;')_uidata = "; \
	cat $< | \
	( comment=''; \
	while read line; do \
		if [ -z $$comment ]; then \
			if echo "$$line" | grep -q '<!\[CDATA\['; then \
				echo "$$line" | sed -e 's,<\!\[CDATA\[.*$$,,'; \
				comment=true; \
			else \
				echo "$$line"; \
			fi; \
		else \
			if echo "$$line" | grep -q '\]\]>'; then \
				echo "$$line" | sed -e 's,^.*\]\]>,,'; \
				comment=''; \
			fi; \
		fi; \
	done ) | sed -e 's|"|\\"|g; s|^|"|; s|$$|"|; $$s|$$|\;|;'; \
	echo "} }"; ) > $@
