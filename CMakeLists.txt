##########################################################################
##									##
##  Project:	Umbrail - GPX track viewer and editor			##
##									##
##########################################################################
##									##
##  Copyright (c) 2014-2024 Jonathan Marten <jjm@keelhaul.me.uk>	##
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

project(umbrail)
cmake_minimum_required(VERSION 3.6)

if (NOT DEFINED QT_MAJOR_VERSION)
  set(QT_MAJOR_VERSION 6)
endif ()

if (QT_MAJOR_VERSION EQUAL 6)
  set(QT_MIN_VERSION "6.3.0")
  set(KF_MIN_VERSION "6.1.0")
else ()
  if (QT_MAJOR_VERSION EQUAL 5)
    set(QT_MIN_VERSION "5.15.2")
    set(KF_MIN_VERSION "5.101.0")
  else ()
    message(FATAL_ERROR "Expecting to build for either Qt5 or Qt6")
  endif ()
endif ()

set(QV ${QT_MAJOR_VERSION})
set(PN ${CMAKE_PROJECT_NAME})

set(VERSION "${QV}.0.0")
message(STATUS "Configuring for project '${CMAKE_PROJECT_NAME}' version ${VERSION}")

#########################################################################
#									#
#  ECM paths and definitions						#
#									#
#########################################################################

set(ECM_MIN_VERSION ${KF_MIN_VERSION})
set(EXIV2_MIN_VERSION "0.24")

find_package(ECM ${ECM_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

include(ECMSetupVersion)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(FeatureSummary)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${KDE_INSTALL_FULL_DATAROOTDIR}/marble/cmake)

#########################################################################
#									#
#  Options								#
#									#
#########################################################################

option(INSTALL_BINARIES "Install the binaries and libraries, turn off for development in place" ON)

#########################################################################
#									#
#  QCustomPlot, system installed or internal copy			#
#									#
#########################################################################

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

#########################################################################
#									#
#  VCS version tracking							#
#									#
#########################################################################

add_custom_target(vcsversion
  ALL
  COMMENT "Checking VCS type and version"
  VERBATIM
  COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/vcsversion.sh ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${VERSION}
)

#########################################################################
#									#
#  Subdirectories							#
#									#
#########################################################################

if (QCUSTOMPLOT_INTERNAL)
  add_subdirectory(qcustomplot)
endif (QCUSTOMPLOT_INTERNAL)

add_subdirectory(src)
add_subdirectory(desktop)
add_subdirectory(pics)

#########################################################################
#									#
#  Information at end of cmake run					#
#									#
#########################################################################

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
