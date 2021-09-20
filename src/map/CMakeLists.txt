#########################################################################
#  Project:		Track Editor					#
#  Edit:		20-Sep-21					#
#########################################################################


#########################################################################
#									#
#  Map widgets								#
#									#
#########################################################################

# see https://cgit.kde.org/marble.git/tree/examples/cpp/hello-marble/CMakeLists.txt.external
set(MARBLE_LIBRARIES "Marble")

set(map_SRCS
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

add_library(${PN}map STATIC ${map_SRCS})
target_link_libraries(${PN}map
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  KF5::I18n
  KF5::XmlGui
  ${PN}core
  ${PN}settings
  ${PN}gui
  ${MARBLE_LIBRARIES}
)
