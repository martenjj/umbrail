##########################################################################
##									##
##  Project:	Umbrail - GPX track viewer and editor			##
##									##
##########################################################################
##									##
##  Copyright (c) 2014-2021 Jonathan Marten <jjm@keelhaul.me.uk>	##
##  Home and download page: <http://github.com/martenjj/umbrail>	##
##									##
##  This program is free software:  you can redistribute it and/or	##
##  modify it under the terms of the GNU General Public License as	##
##  published by the Free Software Foundation, either version 3 of	##
##  the License or (at your option) any later version.			##
##									##
##  It is distributed in the hope that it will be useful, but		##
##  WITHOUT ANY WARRANTY;  without even the implied warranty of		##
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	##
##  GNU General Public License for more details.			##
##									##
##  You should have received a copy of the GNU General Public License	##
##  along with this program;  see the file COPYING for further		##
##  details.  If not, see <http:##gnu.org/licenses/gpl>.      		##
##									##
##########################################################################


#########################################################################
#									#
#  Main application							#
#									#
#########################################################################

set(app_SRCS
  main.cpp
  commands.cpp
  filescontroller.cpp
  folderselectdialogue.cpp
  folderselectwidget.cpp
  mainwindow.cpp
  settingsdialogue.cpp
  statisticswidget.cpp
  stopdetectdialogue.cpp
  trackpropertiesdialogue.cpp
  trackpropertiespage.cpp
  trackpropertiesdetailpages.cpp
  trackpropertiesgeneralpages.cpp
  trackpropertiesmetadatapages.cpp
  trackpropertiesplotpages.cpp
  trackpropertiesstylepages.cpp
)

if (QCUSTOMPLOT_FOUND)
  add_definitions("-DHAVE_QCUSTOMPLOT")
  set(app_SRCS ${app_SRCS} profilewidget.cpp)
  if (NOT QCUSTOMPLOT_INTERNAL)
    include_directories(${QCUSTOMPLOT_INCLUDES})
  endif (NOT QCUSTOMPLOT_INTERNAL)
endif (QCUSTOMPLOT_FOUND)

if (KF5KExiv2_FOUND)
  add_definitions("-DHAVE_KEXIV2")
endif (KF5KExiv2_FOUND)

add_executable(${PN} ${app_SRCS})
target_link_libraries(${PN}
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  Qt5::PrintSupport
  KF5::I18n
  KF5::XmlGui
  KF5::Crash
  ${PN}core
  ${PN}settings
  ${PN}gui
  ${PN}map
  ${PN}media
  ${PN}io
  ${LIBKFDIALOG_LIBRARIES}
)
if (KF5KExiv2_FOUND)
  target_link_libraries(${PN} KF5::KExiv2)
endif (KF5KExiv2_FOUND)
if (QCUSTOMPLOT_FOUND)
  target_link_libraries(${PN} ${QCUSTOMPLOT_LIB})
endif (QCUSTOMPLOT_FOUND)

add_dependencies(${PN} vcsversion)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../..)

#########################################################################
#									#
#  Installation								#
#									#
#########################################################################

if (INSTALL_BINARIES)
  install(TARGETS ${PN} ${INSTALL_TARGETS_DEFAULT_ARGS})
endif (INSTALL_BINARIES)
