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
#  Compile options for QCustomPlot only					#
#									#
#########################################################################

# Normally these warnings are useful, but until the upstream source
# has been cleaned of them they are too noisy.
#
# warning: declaration of 'name' shadows a member of 'class'
add_compile_options(-Wno-shadow)
# warning: 'virtual type class::member()' can be marked override
add_compile_options(-Wno-suggest-override)

#########################################################################
#									#
#  Building the target							#
#									#
#########################################################################

add_library(qcustomplot STATIC qcustomplot.cpp)
target_link_libraries(qcustomplot
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
)
