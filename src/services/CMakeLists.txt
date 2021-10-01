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
#  Data service providers						#
#									#
#########################################################################

set(services_SRCS
  elevationmanager.cpp
  elevationtile.cpp
  timezoneprovider.cpp
  mapbrowser.cpp
)

add_library(${PN}services STATIC ${services_SRCS})
target_link_libraries(${PN}services
  Qt5::Core
  Qt5::Gui
  KF5::I18n
  KF5::KIOCore KF5::KIOWidgets
  KF5::WidgetsAddons
  ${PN}core
  ${PN}settings
)
