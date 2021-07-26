#########################################################################
#  Project:		Track Editor					#
#  Edit:		26-Jul-21					#
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
  createpointdialogue.cpp
  filescontroller.cpp
  filesview.cpp
  folderselectdialogue.cpp
  folderselectwidget.cpp
  itemselectdialogue.cpp
  layerbase.cpp
  mainwindow.cpp
  mapcontroller.cpp
  mapview.cpp
  moveitemdialogue.cpp
  ploteditwidget.cpp
  positioninfodialogue.cpp
  profilewidget.cpp
  routeslayer.cpp
  settingsdialogue.cpp
  statisticswidget.cpp
  stopdetectdialogue.cpp
  stopslayer.cpp
  trackpropertiesdialogue.cpp
  trackpropertiespage.cpp
  trackpropertiesdetailpages.cpp
  trackpropertiesgeneralpages.cpp
  trackpropertiesmetadatapages.cpp
  trackpropertiesplotpages.cpp
  trackpropertiesstylepages.cpp
  trackslayer.cpp
  waypointselectdialogue.cpp
  waypointslayer.cpp
)

if (KF5KExiv2_FOUND)
  add_definitions("-DHAVE_KEXIV2=1")
endif (KF5KExiv2_FOUND)

# see https://cgit.kde.org/marble.git/tree/examples/cpp/hello-marble/CMakeLists.txt.external
set(MARBLE_LIBRARIES "Marble")

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
  navtracksmedia
  navtracksio
  qcustomplot
  ${LIBKFDIALOG_LIBRARIES}
  ${MARBLE_LIBRARIES})
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
install(FILES settings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR} RENAME navtracks.kcfg)
