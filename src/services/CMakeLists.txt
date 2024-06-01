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
  Qt${QV}::Core
  Qt${QV}::Core5Compat
  Qt${QV}::Gui
  KF${QV}::I18n
  KF${QV}::KIOCore KF${QV}::KIOWidgets
  KF${QV}::WidgetsAddons
  ${PN}core
  ${PN}settings
)
