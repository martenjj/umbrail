
project(navtracks)

include(ECMInstallIcons)

set(navtracks_ICONS
  32-apps-navtracks.png
)

ecm_install_icons(ICONS ${navtracks_ICONS} DESTINATION ${KDE_INSTALL_ICONDIR})

install(FILES navtracksui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/navtracks)
install(FILES viewerui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/navtracks)
install(FILES navtracks.desktop DESTINATION ${KDE_INSTALL_APPDIR})
