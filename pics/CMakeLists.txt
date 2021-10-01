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

set(pics_SRCS
  # Application actions
  application_side_list.png
  application_side_tree.png
  # Data item icons
  chart_track.png
  chart_track-32.png
  chart_segment.png
  chart_segment-32.png
  chart_point.png
  chart_point-32.png
  chart_route.png
  chart_route-32.png
  chart_routepoint.png
  chart_routepoint-32.png
  # Coloured waypoint icon images
  waypoint-16.png
  waypoint-24.png
  waypoint-32.png
)

install(FILES ${pics_SRCS} DESTINATION ${KDE_INSTALL_DATADIR}/${PN}/pics)
