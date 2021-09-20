#########################################################################
#  Project:		Track Editor					#
#  Edit:		20-Sep-21					#
#########################################################################


#########################################################################
#									#
#  GUI widgets								#
#									#
#########################################################################

set(gui_SRCS
  autotooltipdelegate.cpp
  itemtypecombo.cpp
  latlongdialogue.cpp
  latlongwidget.cpp
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

add_library(${PN}gui STATIC ${gui_SRCS})
target_link_libraries(${PN}gui
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  KF5::I18n
  KF5::Completion
  KF5::ConfigCore
  KF5::ConfigWidgets
  KF5::ItemViews
  KF5::XmlGui
  ${PN}core
  ${PN}settings
  ${PN}services
  ${PN}coordinate
)
