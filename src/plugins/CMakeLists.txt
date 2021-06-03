#########################################################################
#  Project:		Track Editor					#
#  Edit:		03-Jun-21					#
#########################################################################

project(navtracks)

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

add_library(abstractcoordinate SHARED abstractcoordinatehandler.cpp)
set_target_properties(abstractcoordinate PROPERTIES SOVERSION "1.0.0")
target_link_libraries(abstractcoordinate Qt5::Core Qt5::Gui Qt5::Widgets)

if (INSTALL_BINARIES)
  install(TARGETS abstractcoordinate ${INSTALL_TARGETS_DEFAULT_ARGS})
endif (INSTALL_BINARIES)

#########################################################################
#									#
#  Coordinate system plugins						#
#									#
#########################################################################

add_library(decimalcoordinate MODULE decimalcoordinate.cpp)
target_link_libraries(decimalcoordinate
  abstractcoordinate
  Qt5::Gui
  Qt5::Widgets
  KF5::I18n
)
if (INSTALL_BINARIES)
  install(TARGETS decimalcoordinate DESTINATION ${KDE_INSTALL_PLUGINDIR}/navtracks)
endif (INSTALL_BINARIES)

add_library(dmscoordinate MODULE dmscoordinate.cpp)
target_link_libraries(dmscoordinate
  abstractcoordinate
  Qt5::Gui
  Qt5::Widgets
  KF5::I18n
  ${LIBKFDIALOG_LIBRARIES}
)
if (INSTALL_BINARIES)
  install(TARGETS dmscoordinate DESTINATION ${KDE_INSTALL_PLUGINDIR}/navtracks)
endif (INSTALL_BINARIES)

add_library(swisscoordinate MODULE swisscoordinate.cpp)
target_link_libraries(swisscoordinate
  abstractcoordinate
  Qt5::Gui
  Qt5::Widgets
  KF5::I18n
)
if (INSTALL_BINARIES)
  install(TARGETS swisscoordinate DESTINATION ${KDE_INSTALL_PLUGINDIR}/navtracks)
endif (INSTALL_BINARIES)

add_library(osgbcoordinate MODULE osgbcoordinate.cpp)
target_link_libraries(osgbcoordinate
  abstractcoordinate
  Qt5::Gui
  Qt5::Widgets
  KF5::I18n
  ${LIBKFDIALOG_LIBRARIES}
)
if (INSTALL_BINARIES)
  install(TARGETS osgbcoordinate DESTINATION ${KDE_INSTALL_PLUGINDIR}/navtracks)
endif (INSTALL_BINARIES)
