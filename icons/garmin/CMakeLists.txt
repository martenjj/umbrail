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

set(garmin_FILES
  Waypoints_001-040.png
  Waypoints_041-080.png
  Waypoints_081-120.png
  Waypoints_121-160.png
  Waypoints_161-200.png
  Waypoints_201-240.png
  Waypoints_241-248.png
)

# from https://cmake.org/cmake/help/book/mastering-cmake/chapter/Custom Commands.html
# section "Adding a Custom Target"

foreach(F ${garmin_FILES})
  add_custom_command(
    OUTPUT ${F}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND wget -N "http://www.ushighway66.com/images/${F}"
    COMMENT "Downloading ${F}"
    VERBATIM
  )
endforeach()

add_custom_target(download
  DEPENDS ${garmin_FILES}
  VERBATIM
)

#########################################################################
#									#
#  Installation								#
#									#
#########################################################################

install(FILES ${garmin_FILES} DESTINATION ${KDE_INSTALL_DATADIR}/${PN}/icons/garmin)
