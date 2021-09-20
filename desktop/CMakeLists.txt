

include(ECMInstallIcons)

set(icons_SRCS
  32-apps-${PN}.png
)

ecm_install_icons(ICONS ${icons_SRCS} DESTINATION ${KDE_INSTALL_ICONDIR})

install(FILES mainui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/${PN} RENAME ${PN}ui.rc)
install(FILES viewerui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/${PN})
install(FILES app.desktop DESTINATION ${KDE_INSTALL_APPDIR} RENAME ${PN}.desktop)
