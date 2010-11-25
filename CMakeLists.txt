cmake_minimum_required (VERSION 2.6)
project (Yatta)

find_package (PkgConfig REQUIRED)
find_package (CURL REQUIRED)

pkg_check_modules(GTKMM gtkmm-2.4)

add_executable (yatta
  src/main.cc
  src/yatta/curl/chunk.cc
  src/yatta/curl/chunk.hh
  src/yatta/curl/download.cc
  src/yatta/curl/download.hh
  src/yatta/curl/ioqueue.cc
  src/yatta/curl/ioqueue.hh
  src/yatta/curl/manager.cc
  src/yatta/curl/manager.hh
  src/yatta/options.cc
  src/yatta/options.hh
  src/yatta/ui/aboutdialog.cc
  src/yatta/ui/aboutdialog.hh
  src/yatta/ui/main.cc
  src/yatta/ui/main.hh
  src/yatta/ui/mainwindow.cc
  src/yatta/ui/mainwindow.hh
  src/yatta/ui/mainwindow/main_menu.cc
  src/yatta/ui/mainwindow/main_tb.cc
)

target_link_libraries (yatta ${CURL_LIBRARIES} ${GTKMM_LIBRARIES})
include_directories (${GTKMM_INCLUDE_DIRS} ${GTKMM_CFLAGS})

set (GETTEXT_PACKAGE yatta)
set (PROGRAMNAME_LOCALEDIR ${datadir}/locale)

set_target_properties (yatta PROPERTIES
  COMPILE_FLAGS
  "-DGETTEXT_PACKAGE=\\\"${GETTEXT_PACKAGE}\\\" -DPROGRAMNAME_LOCALEDIR=\\\"${PROGRAMNAME_LOCALEDIR}\\\" -DVERSION=\\\"\\\""
)
