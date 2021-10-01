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
