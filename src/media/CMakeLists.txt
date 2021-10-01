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
#  Media display							#
#									#
#########################################################################

set(media_SRCS
  mediaplayer.cpp
  photoviewer.cpp
)

if (Phonon4Qt5_FOUND)
  set(media_SRCS ${media_SRCS} videoviewer.cpp)
  add_definitions("-DHAVE_PHONON=1")
endif (Phonon4Qt5_FOUND)

add_library(${PN}media STATIC ${media_SRCS})
target_link_libraries(${PN}media
  Qt5::Core
  KF5::Parts
  ${PN}core
  ${PN}settings
)

if (Phonon4Qt5_FOUND)
  target_link_libraries(${PN}media Phonon::phonon4qt5)
endif (Phonon4Qt5_FOUND)
