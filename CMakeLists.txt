cmake_minimum_required (VERSION 2.6)
project (Yatta)

include (CPack)
find_package (PkgConfig REQUIRED)
find_package (CURL REQUIRED)

pkg_check_modules (GTKMM gtkmm-2.4)
pkg_check_modules (LIBXMLPP libxml++-2.6)

# configure paths
set (bindir ${CMAKE_INSTALL_PREFIX}/bin)

# embedding ui files into cc files
add_executable (convert_ui src/yatta/ui/convert_ui.cc)
file (MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/src/yatta/ui/mainwindow/)
target_link_libraries (convert_ui ${LIBXMLPP_LIBRARIES})

set (uimgrdir src/yatta/ui/mainwindow)
file (GLOB ui_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/yatta/ui/mainwindow/*.ui)

foreach (ui_file ${ui_files})
  get_filename_component (filename ${ui_file} NAME_WE)
  set (ui_file ${CMAKE_CURRENT_SOURCE_DIR}/${uimgrdir}/${filename}.ui)
  set (cc_file ${CMAKE_CURRENT_BINARY_DIR}/${uimgrdir}/${filename}.cc)

  add_custom_command (
    OUTPUT ${cc_file}
    MAIN_DEPENDENCY ${ui_file}
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/convert_ui
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/convert_ui
            ${ui_file} ${cc_file} Yatta::UI::MainWindow::${filename}_uidata
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/src/yatta/ui/mainwindow.hh
            ${CMAKE_CURRENT_BINARY_DIR}/src/yatta/ui/mainwindow.hh
  )
endforeach (ui_file)

# begin actual yatta stuff
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
include_directories (${GTKMM_INCLUDE_DIRS} ${LIBXMLPP_INCLUDE_DIRS})

set (GETTEXT_PACKAGE yatta)
set (PROGRAMNAME_LOCALEDIR ${datadir}/locale)

set_target_properties (yatta PROPERTIES
  COMPILE_FLAGS
  "-DGETTEXT_PACKAGE=\\\"${GETTEXT_PACKAGE}\\\" \\
  -DPROGRAMNAME_LOCALEDIR=\\\"${PROGRAMNAME_LOCALEDIR}\\\" \\
  -DVERSION=\\\"\\\""
)

install (PROGRAMS yatta DESTINATION ${bindir})
add_custom_target (run
  DEPENDS yatta
  COMMAND ./yatta
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
