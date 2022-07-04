##########################################################################
##									##
##  Project:	Umbrail - GPX track viewer and editor			##
##									##
##########################################################################
##									##
##  Copyright (c) 2014-2021 Jonathan Marten <jjm@keelhaul.me.uk>	##
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

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core Gui Widgets PrintSupport)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS I18n Config XmlGui Parts IconThemes ItemViews KIO Crash Auth)

find_package(Marble REQUIRED NO_POLICY_SCOPE)
find_package(Phonon4Qt5 NO_POLICY_SCOPE)
find_package(KF5KExiv2 NO_POLICY_SCOPE)
find_package(LibKFDialog REQUIRED NO_POLICY_SCOPE)

# find_package(QCustomPlot) is currently untested, my distro
# does not provide any CMake or package config for QCustomPlot.
find_package(QCustomPlot NO_POLICY_SCOPE QUIET)
if (QCustomPlot_FOUND)
  message(STATUS "Found external QCustomPlot package")
  set(QCUSTOMPLOT_FOUND TRUE)
  set(QCUSTOMPLOT_LIB ${QCUSTOMPLOT_LIBRARIES})
else (QCustomPlot_FOUND)
  find_library(QCUSTOMPLOT_LIB qcustomplot)
  find_file(QCUSTOMPLOT_HDR qcustomplot.h)

  if (QCUSTOMPLOT_LIB AND QCUSTOMPLOT_HDR)
    message(STATUS "Found external QCustomPlot library")
    set(QCUSTOMPLOT_FOUND TRUE)
    get_filename_component(QCUSTOMPLOT_INCLUDES ${QCUSTOMPLOT_HDR} DIRECTORY)
  else (QCUSTOMPLOT_LIB AND QCUSTOMPLOT_HDR)
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/qcustomplot")
      message(STATUS "Could not find external QCustomPlot, using internal source")
      set(QCUSTOMPLOT_FOUND TRUE)
      set(QCUSTOMPLOT_INTERNAL TRUE)
      set(QCUSTOMPLOT_LIB "qcustomplot")
      # No need to set QCUSTOMPLOT_INCLUDES, linking to QCUSTOMPLOT_LIB
      # will add the internal source directory to the search path.
    endif (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/qcustomplot")
  endif (QCUSTOMPLOT_LIB AND QCUSTOMPLOT_HDR)
endif (QCustomPlot_FOUND)

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
set_package_properties("Phonon4Qt5" PROPERTIES DESCRIPTION "Multimedia playback library" URL "http://phonon.kde.org" TYPE OPTIONAL PURPOSE "Required for playback of audio/video notes")
set_package_properties("KF5KExiv2" PROPERTIES DESCRIPTION "Wrapper around Exiv2 library" URL "http://quickgit.kde.org/?p=libkexiv2.git" TYPE OPTIONAL PURPOSE "Required for geolocation of photos")
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

remove_definitions("-DQT_NO_CAST_FROM_ASCII")
remove_definitions("-DQT_NO_SIGNALS_SLOTS_KEYWORDS")

#########################################################################
#									#
#  Global build options							#
#									#
#########################################################################

add_definitions("-DPROJECT_NAME=\\\"${PN}\\\"")

set(ACH_PLUGIN_STRING "uk.me.keelhaul.${PN}.AbstractCoordinateHandler")
# I don't know exactly where or by what the quoting is being
# handled differently, but these separate definitions are needed
# for compliation and automoc!
set(CMAKE_AUTOMOC_MOC_OPTIONS "-DACH_PLUGIN_IID=\"${ACH_PLUGIN_STRING}\"")
add_definitions("-DACH_PLUGIN_IID=\\\"${ACH_PLUGIN_STRING}\\\"")

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
  KF5::ConfigCore
  KF5::ConfigGui
)

#########################################################################
#									#
#  Subdirectories							#
#									#
#########################################################################

if (QCUSTOMPLOT_INTERNAL)
  add_subdirectory(qcustomplot)
endif (QCUSTOMPLOT_INTERNAL)

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
