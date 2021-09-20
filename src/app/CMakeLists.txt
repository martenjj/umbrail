#########################################################################
#  Project:		Track Editor					#
#  Edit:		20-Sep-21					#
#########################################################################


#########################################################################
#									#
#  Main application							#
#									#
#########################################################################

set(app_SRCS
  main.cpp
  commands.cpp
  filescontroller.cpp
  folderselectdialogue.cpp
  folderselectwidget.cpp
  mainwindow.cpp
  profilewidget.cpp
  settingsdialogue.cpp
  statisticswidget.cpp
  stopdetectdialogue.cpp
  trackpropertiesdialogue.cpp
  trackpropertiespage.cpp
  trackpropertiesdetailpages.cpp
  trackpropertiesgeneralpages.cpp
  trackpropertiesmetadatapages.cpp
  trackpropertiesplotpages.cpp
  trackpropertiesstylepages.cpp
)

if (KF5KExiv2_FOUND)
  add_definitions("-DHAVE_KEXIV2=1")
endif (KF5KExiv2_FOUND)

add_executable(${PN} ${app_SRCS})
target_link_libraries(${PN}
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  Qt5::PrintSupport
  KF5::I18n
  KF5::XmlGui
  KF5::Crash
  ${PN}core
  ${PN}settings
  ${PN}gui
  ${PN}map
  ${PN}media
  ${PN}io
  qcustomplot
  ${LIBKFDIALOG_LIBRARIES}
)
if (KF5KExiv2_FOUND)
  target_link_libraries(${PN} KF5::KExiv2)
endif (KF5KExiv2_FOUND)

add_dependencies(${PN} vcsversion)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../..)

#########################################################################
#									#
#  Installation								#
#									#
#########################################################################

if (INSTALL_BINARIES)
  install(TARGETS ${PN} ${INSTALL_TARGETS_DEFAULT_ARGS})
endif (INSTALL_BINARIES)
