##########################################################################
##									##
##  Project:	Umbrail - GPX track viewer and editor			##
##									##
##########################################################################
##									##
##  Copyright (c) 2014-2022 Jonathan Marten <jjm@keelhaul.me.uk>	##
##  Home and download page:  <http://github.com/martenjj/umbrail>	##
##									##
##  This program is free software;  you can redistribute it and/or	##
##  modify it under the terms of the GNU General Public License as	##
##  published by the Free Software Foundation, either version 3 of	##
##  the License or (at your option) any later version.			##
##									##
##  It is distributed in the hope that it will be useful, but		##
##  WITHOUT ANY WARRANTY; without even the implied warranty of		##
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	##
##  GNU General Public License for more details.			##
##									##
##  You should have received a copy of the GNU General Public License	##
##  along with this program;  see the file COPYING for further		##
##  details.  If not, see <http://gnu.org/licenses/gpl>.      		##
##									##
##########################################################################


#########################################################################
#									#
#  Options								#
#									#
#########################################################################

#  These map services are useful, but the options are here to disable them
#  in order that the application can be built without any dependency on or
#  recommendation of closed map services.  Enabling them does not actually
#  make the application non-free or embed any proprietary or copyright material.
option(OPEN_WITH_GOOGLE "Enable external map browsing with Google Maps" ON)
option(OPEN_WITH_BING "Enable external map browsing with Bing Maps" ON)

#########################################################################
#									#
#  Dependencies								#
#									#
#########################################################################

find_package(Qt${QV} ${QT_MIN_VERSION} REQUIRED COMPONENTS Core Gui Widgets PrintSupport)
find_package(KF${QV} ${KF5_MIN_VERSION} REQUIRED COMPONENTS I18n Config XmlGui Parts IconThemes ItemViews KIO Crash TextWidgets)

# Needed to build with Marble, otherwise
#
#  CMake Error at /usr/lib64/cmake/Marble/MarbleTargets.cmake:61 (set_target_properties):
#    The link interface of target "Marble" contains:
#
#      Qt6::Core5Compat
#
#    but the target was not found.  Possible reasons include:
#
find_package(Qt${QV} ${QT_MIN_VERSION} REQUIRED COMPONENTS Core5Compat)

find_package(Marble REQUIRED NO_POLICY_SCOPE)
find_package(Phonon4Qt${QV} NO_POLICY_SCOPE)
find_package(LibKFDialog REQUIRED NO_POLICY_SCOPE)

if (${QV} GREATER_EQUAL 6)
  set (KEXIV2_PACKAGE_NAME "KExiv2Qt${QV}")
else ()
  set (KEXIV2_PACKAGE_NAME "KF${QV}KExiv2")
endif ()
find_package(${KEXIV2_PACKAGE_NAME} NO_POLICY_SCOPE)
if (${${KEXIV2_PACKAGE_NAME}_FOUND})
  set (KEXIV2_FOUND ON)
endif ()

if (QCUSTOMPLOT_FOUND)
  message(STATUS "QCustomPlot library: ${QCUSTOMPLOT_LIB}")
  message(STATUS "QCustomPlot include: ${QCUSTOMPLOT_INCLUDES}")
else (QCUSTOMPLOT_FOUND)
  message(STATUS "QCustomPlot not found, profile plotting will not be available")
endif (QCUSTOMPLOT_FOUND)

if (NOT WIN32)
  # This sems to be necessary as a consequence of commit cb40e3d6
  # to tier1/kwindowsystem, "Make link against X11 and XCB public".
  # Otherwise the final link fails with "cannot find -lX11::X11".
  find_package(X11 REQUIRED NO_POLICY_SCOPE)
endif (NOT WIN32)

set_package_properties("Marble" PROPERTIES DESCRIPTION "Map display library" URL "http://marble.kde.org" TYPE REQUIRED PURPOSE "Required for map display")
set_package_properties("Phonon4Qt${QV}" PROPERTIES DESCRIPTION "Multimedia playback library" URL "https://invent.kde.org/libraries/phonon" TYPE OPTIONAL PURPOSE "Required for playback of audio/video notes")
set_package_properties(${KEXIV2_PACKAGE_NAME} PROPERTIES DESCRIPTION "Wrapper around Exiv2 library" URL "https://invent.kde.org/graphics/libkexiv2" TYPE OPTIONAL PURPOSE "Required for geolocation of photos")
set_package_properties("LibKFDialog" PROPERTIES DESCRIPTION "KDE Frameworks dialogue utility library" URL "https://github.com/martenjj/libkfdialog" TYPE REQUIRED PURPOSE "Required to build the project")
set_package_properties("QCustomPlot" PROPERTIES DESCRIPTION "Qt plotting and data visualization" URL "https://www.qcustomplot.com" TYPE OPTIONAL PURPOSE "Required for the profile display, internal source will be used if not found")

#########################################################################
#									#
#  Rigourousness							#
#									#
#########################################################################

add_definitions("-DQT_USE_FAST_CONCATENATION")
add_definitions("-DQT_USE_FAST_OPERATOR_PLUS")
add_definitions("-DQT_NO_CAST_FROM_BYTEARRAY")
add_definitions("-DQT_NO_URL_CAST_FROM_STRING")
add_definitions("-DQT_NO_CAST_TO_ASCII")
add_definitions("-DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT")
add_definitions("-DQT_DEPRECATED_WARNINGS")
add_definitions("-DQT_NO_CONTEXTLESS_CONNECT")

remove_definitions("-DQT_NO_CAST_FROM_ASCII")
remove_definitions("-DQT_NO_SIGNALS_SLOTS_KEYWORDS")
remove_definitions("-DQT_NO_KEYWORDS")
remove_definitions("-DQT_USE_QSTRINGBUILDER")
# TODO: remove this removal when QCustomPlot has been updated
# to not use 'foreach'
remove_definitions("-DQT_NO_FOREACH")

#########################################################################
#									#
#  Global build options							#
#									#
#########################################################################

add_definitions("-DPROJECT_NAME=\"${PN}\"")
add_definitions("-DKF_DIR=\"kf${QV}\"")

set(ACH_PLUGIN_STRING "uk.me.keelhaul.${PN}.AbstractCoordinateHandler")
# I don't know exactly where or by what the quoting is being
# handled differently, but these separate definitions are needed
# for compliation and automoc!
set(CMAKE_AUTOMOC_MOC_OPTIONS "-DACH_PLUGIN_IID=\"${ACH_PLUGIN_STRING}\"")
add_definitions("-DACH_PLUGIN_IID=\"${ACH_PLUGIN_STRING}\"")

if (OPEN_WITH_GOOGLE)
  add_definitions("-DENABLE_OPEN_WITH_GOOGLE")
endif (OPEN_WITH_GOOGLE)
if (OPEN_WITH_BING)
  add_definitions("-DENABLE_OPEN_WITH_BING")
endif (OPEN_WITH_BING)

#########################################################################
#									#
#  Global settings							#
#									#
#########################################################################

kconfig_add_kcfg_files(settings_SRCS settings.kcfgc)
add_library(${PN}settings STATIC ${settings_SRCS})
target_link_libraries(${PN}settings
  KF${QV}::ConfigCore
  KF${QV}::ConfigGui
)

#########################################################################
#									#
#  Subdirectories							#
#									#
#########################################################################

add_subdirectory(core)
add_subdirectory(services)
add_subdirectory(media)
add_subdirectory(gui)
add_subdirectory(map)
add_subdirectory(io)
add_subdirectory(app)
add_subdirectory(plugins)

#########################################################################
#									#
#  Installation								#
#									#
#########################################################################

install(FILES settings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR} RENAME ${PN}.kcfg)
