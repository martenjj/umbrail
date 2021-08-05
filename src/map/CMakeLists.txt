#########################################################################
#  Project:		Track Editor					#
#  Edit:		29-Jul-21					#
#########################################################################

project(navtracks)

#########################################################################
#									#
#  Map widgets								#
#									#
#########################################################################

# see https://cgit.kde.org/marble.git/tree/examples/cpp/hello-marble/CMakeLists.txt.external
set(MARBLE_LIBRARIES "Marble")

set(navtracksmap_SRCS
  mapcontroller.cpp
  mapview.cpp
  layerbase.cpp
  routeslayer.cpp
  stopslayer.cpp
  trackslayer.cpp
  waypointslayer.cpp
  positioninfodialogue.cpp
  mapthemedialogue.cpp
)

add_library(navtracksmap STATIC ${navtracksmap_SRCS})
target_link_libraries(navtracksmap
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  KF5::I18n
  KF5::XmlGui
  navtrackscore
  navtrackssettings
  navtracksgui
  ${MARBLE_LIBRARIES}
)
