#########################################################################
#  Project:		Track Editor					#
#  Edit:		28-Jul-21					#
#########################################################################

project(navtracks)

#########################################################################
#									#
#  GUI widgets								#
#									#
#########################################################################

set(navtracksgui_SRCS
  autotooltipdelegate.cpp
  itemtypecombo.cpp
  latlongdialogue.cpp
  latlongwidget.cpp
  mapthemedialogue.cpp
  ploteditwidget.cpp
  timezonedialogue.cpp
  timezoneselector.cpp
  timezonewidget.cpp
  trackdatalabel.cpp
  valueslider.cpp
  variableunitdisplay.cpp
  variableunitcombo.cpp
  filesview.cpp
  createpointdialogue.cpp
  itemselectdialogue.cpp
  moveitemdialogue.cpp
  waypointselectdialogue.cpp
)

add_library(navtracksgui STATIC ${navtracksgui_SRCS})
target_link_libraries(navtracksgui
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  KF5::I18n
  KF5::Completion
  KF5::ConfigCore
  KF5::ConfigWidgets
  KF5::ItemViews
  KF5::XmlGui
  navtrackscore
  navtrackssettings
  navtracksservices
  abstractcoordinate
)
