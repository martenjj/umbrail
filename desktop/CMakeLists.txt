project(navtracks)

kde4_install_icons(${ICON_INSTALL_DIR})

install(FILES navtracksui.rc DESTINATION ${DATA_INSTALL_DIR}/navtracks)
install(FILES viewerui.rc DESTINATION ${DATA_INSTALL_DIR}/navtracks)
install(FILES navtracks.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
