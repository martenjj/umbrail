#########################################################################
#  Project:		Track Editor					#
#  Edit:		20-Sep-21					#
#########################################################################


#########################################################################
#									#
#  Common definitions for plugins					#
#									#
#########################################################################

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

# see https://gcc.gnu.org/wiki/Visibility
add_definitions("-DPLUGIN_EXPORT='__attribute__ ((visibility (\"default\")))'")

#########################################################################
#									#
#  Coordinate system library						#
#									#
#########################################################################

add_library(${PN}coordinate SHARED abstractcoordinatehandler.cpp)
set_target_properties(${PN}coordinate PROPERTIES SOVERSION "1.0.0")
target_link_libraries(${PN}coordinate Qt5::Core Qt5::Gui Qt5::Widgets)

if (INSTALL_BINARIES)
  install(TARGETS ${PN}coordinate ${INSTALL_TARGETS_DEFAULT_ARGS})
endif (INSTALL_BINARIES)

#########################################################################
#									#
#  Coordinate system plugins						#
#									#
#########################################################################

# Decimal
add_library(decimalcoordinate MODULE decimalcoordinate.cpp)
target_link_libraries(decimalcoordinate
  ${PN}coordinate
  Qt5::Gui
  Qt5::Widgets
  KF5::I18n
  ${PN}core
  ${LIBKFDIALOG_LIBRARIES}
)
if (INSTALL_BINARIES)
  install(TARGETS decimalcoordinate DESTINATION ${KDE_INSTALL_PLUGINDIR}/${PN})
endif (INSTALL_BINARIES)

# DMS
add_library(dmscoordinate MODULE dmscoordinate.cpp)
target_link_libraries(dmscoordinate
  ${PN}coordinate
  Qt5::Gui
  Qt5::Widgets
  KF5::I18n
  ${PN}core
  ${LIBKFDIALOG_LIBRARIES}
)
if (INSTALL_BINARIES)
  install(TARGETS dmscoordinate DESTINATION ${KDE_INSTALL_PLUGINDIR}/${PN})
endif (INSTALL_BINARIES)

# Swiss
add_library(swisscoordinate MODULE swisscoordinate.cpp)
target_link_libraries(swisscoordinate
  ${PN}coordinate
  Qt5::Gui
  Qt5::Widgets
  KF5::I18n
  ${PN}core
  ${LIBKFDIALOG_LIBRARIES}
)
if (INSTALL_BINARIES)
  install(TARGETS swisscoordinate DESTINATION ${KDE_INSTALL_PLUGINDIR}/${PN})
endif (INSTALL_BINARIES)

# OSGB
add_library(osgbcoordinate MODULE osgbcoordinate.cpp)
target_link_libraries(osgbcoordinate
  ${PN}coordinate
  Qt5::Gui
  Qt5::Widgets
  KF5::I18n
  KF5::ConfigWidgets
  ${PN}core
  ${LIBKFDIALOG_LIBRARIES}
)
if (INSTALL_BINARIES)
  install(TARGETS osgbcoordinate DESTINATION ${KDE_INSTALL_PLUGINDIR}/${PN})
endif (INSTALL_BINARIES)

# The OSGB map image is as downloaded from
# https://upload.wikimedia.org/wikipedia/commons/thumb/1/18/British_National_Grid.svg/324px-British_National_Grid.svg.png
install(FILES 324px-British_National_Grid.svg.png RENAME osgbgrid.png DESTINATION ${KDE_INSTALL_DATADIR}/${PN}/pics)
