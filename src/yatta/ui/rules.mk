yatta_SOURCES += \
	src/yatta/ui/main.cc \
	src/yatta/ui/mainwindow.cc \
	src/yatta/ui/mainwindow/main_menu.cc \
	src/yatta/ui/mainwindow/main_tb.cc \
	src/yatta/ui/aboutdialog.cc

EXTRA_DIST += \
	src/yatta/ui/main.hh \
	src/yatta/ui/mainwindow.hh \
	src/yatta/ui/aboutdialog.hh \
	src/yatta/ui/mainwindow/main_menu.ui \
	src/yatta/ui/mainwindow/main_tb.ui

CLEANFILES += \
	src/yatta/ui/mainwindow/main_menu.cc \
	src/yatta/ui/mainwindow/main_tb.cc

# embed .ui files into executable
src/yatta/ui/mainwindow/%.cc: src/yatta/ui/mainwindow/%.ui
	sed -e '/<\!\[CDATA\[/,/\]\]>/d' "$<" | \
	sed -e 's/"/\\"/g; s/^/"/; s/$$/"/; $$s/$$/;/;' \
		-e '1i\
\#include "../mainwindow.hh" \
namespace Yatta { namespace UI { \
const char *MainWindow::$(lastword $(subst /, , $(<:.ui=_uidata))) =' \
		-e '$$a}\;}\;' > $@
