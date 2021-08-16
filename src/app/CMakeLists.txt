#########################################################################
#  Project:		Track Editor					#
#  Edit:		16-Aug-21					#
#########################################################################

project(navtracks)

#########################################################################
#									#
#  Main application							#
#									#
#########################################################################

set(navtracks_SRCS
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

add_executable(navtracks ${navtracks_SRCS})
target_link_libraries(navtracks
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  Qt5::PrintSupport
  KF5::I18n
  KF5::XmlGui
  KF5::Crash
  navtrackscore
  navtrackssettings
  navtracksgui
  navtracksmap
  navtracksmedia
  navtracksio
  qcustomplot
  ${LIBKFDIALOG_LIBRARIES}
)
if (KF5KExiv2_FOUND)
  target_link_libraries(navtracks KF5::KExiv2)
endif (KF5KExiv2_FOUND)

add_dependencies(navtracks vcsversion)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../..)

#########################################################################
#									#
#  Installation								#
#									#
#########################################################################

if (INSTALL_BINARIES)
  install(TARGETS navtracks ${INSTALL_TARGETS_DEFAULT_ARGS})
endif (INSTALL_BINARIES)
