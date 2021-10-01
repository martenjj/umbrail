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

include(ECMInstallIcons)

set(icons_SRCS
  32-apps-${PN}.png
  64-apps-${PN}.png
)

ecm_install_icons(ICONS ${icons_SRCS} DESTINATION ${KDE_INSTALL_ICONDIR})

install(FILES mainui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/${PN} RENAME ${PN}ui.rc)
install(FILES viewerui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/${PN})
install(FILES app.desktop DESTINATION ${KDE_INSTALL_APPDIR} RENAME ${PN}.desktop)
