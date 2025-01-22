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
#  GUI widgets								#
#									#
#########################################################################

set(gui_SRCS
  autotooltipdelegate.cpp
  itemtypecombo.cpp
  latlongdialogue.cpp
  latlongwidget.cpp
  ploteditwidget.cpp
  timezonelistdialogue.cpp
  timezonedisplay.cpp
  timezonelistwidget.cpp
  timezonesettingdialogue.cpp
  trackdatalabel.cpp
  valueslider.cpp
  variableunitdisplay.cpp
  variableunitcombo.cpp
  waypointselectdialogue.cpp
  addresseditdialogue.cpp
  categorieseditdialogue.cpp
  categoriesmanagedialogue.cpp
  flagseditdialogue.cpp
  symboliconselector.cpp
  symboliconbutton.cpp
  symbolshapecombo.cpp
  importfiledialogue.cpp
  mergepointsdialogue.cpp
  listeditwidget.cpp
  exportfiledialogue.cpp
)

add_library(${PN}gui STATIC ${gui_SRCS})
target_include_directories(${PN}gui PRIVATE ${LIBKFDIALOG_INCLUDES})
target_link_libraries(${PN}gui
  Qt${QV}::Core
  Qt${QV}::Gui
  Qt${QV}::Widgets
  KF${QV}::I18n
  KF${QV}::Completion
  KF${QV}::ConfigCore
  KF${QV}::ConfigWidgets
  KF${QV}::GuiAddons
  KF${QV}::ItemViews
  KF${QV}::IconWidgets
  KF${QV}::XmlGui
  ${PN}core
  ${PN}models
  ${PN}io
  ${PN}settings
  ${PN}services
  ${PN}coordinate
  ${LIBKFDIALOG_LIBRARIES}
)
