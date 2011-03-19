libyatta_la_SOURCES += \
	src/yatta/ui/main.cc \
	src/yatta/ui/main.hh \
	src/yatta/ui/mainwindow.cc \
	src/yatta/ui/mainwindow.hh \
	src/yatta/ui/mainwindow/main_menu.cc \
	src/yatta/ui/mainwindow/main_tb.cc \
	src/yatta/ui/aboutdialog.cc \
	src/yatta/ui/aboutdialog.hh

EXTRA_DIST += \
	src/yatta/ui/mainwindow/main_menu.ui \
	src/yatta/ui/mainwindow/main_tb.ui

CLEANFILES += \
	src/yatta/ui/mainwindow/main_menu.cc \
	src/yatta/ui/mainwindow/main_tb.cc

noinst_PROGRAMS = convert-ui
convert_ui_SOURCES = src/yatta/ui/convert_ui.cc
convert_ui_CXXFLAGS = $(LIBXML_CFLAGS)
convert_ui_LDFLAGS = $(LIBXML_LIBS)

# embed .ui files into executable
src/yatta/ui/mainwindow/%.cc: src/yatta/ui/mainwindow/%.ui convert-ui
	$(AM_V_GEN)"$(top_builddir)/convert-ui" $< $@ \
		Yatta::UI::MainWindow::$(lastword $(subst /, , $(<:.ui=_uidata)))
