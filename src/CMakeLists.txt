#########################################################################
#  Project:		Track Editor					#
#  Edit:		20-Jan-14					#
#########################################################################

project(navtracks)

#########################################################################
#									#
#  Options								#
#									#
#########################################################################

#  The only sensible ordering for tracks, segments and points is by time
#  (which is assumed to be the same as the original file order).  So there
#  is not really need for a sortable view.
option(SORTABLE_VIEW "Allow the tracks/points tree view to be sortable" OFF)

#########################################################################
#									#
#  Dependencies								#
#									#
#########################################################################

include(FindMarble)

#########################################################################
#									#
#  Building the target							#
#									#
#########################################################################

set(navtracks_SRCS
  main.cpp
  mainwindow.cpp
  project.cpp
  trackdata.cpp
  filesview.cpp
  filescontroller.cpp
  filesmodel.cpp
  commands.cpp
#  categoriesmanager.cpp
#  sourcesmanager.cpp
#  pointpropertiesdialog.cpp
#  mergepointsdialogue.cpp
#  latlongedit.cpp
#  latlongdialogue.cpp
#  categoriesedit.cpp
#  categoriesdialogue.cpp
#  addressedit.cpp
#  addressdialogue.cpp
#  categoriestooldialogue.cpp
  mapcontroller.cpp
  mapview.cpp
  mapthemedialogue.cpp
#  iconsmanager.cpp
#  iconstooldialogue.cpp
#  symboledit.cpp
#  sourcesedit.cpp
#  sourcesdialogue.cpp
#  sourcestooldialogue.cpp
#  iconselector.cpp
  importerexporterbase.cpp
  importerbase.cpp
  gpximporter.cpp
#  exporterbase.cpp
#  gpxexporter.cpp
  autotooltipdelegate.cpp
)

add_definitions("-DVERSION=\\\"${VERSION}\\\"")
if (SORTABLE_VIEW)
  add_definitions("-DSORTABLE_VIEW=1")
endif (SORTABLE_VIEW)

kde4_add_executable(navtracks ${navtracks_SRCS})
target_link_libraries(navtracks ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${MARBLE_LIBRARIES})

add_dependencies(navtracks vcsversion)

#########################################################################
#									#
#  Installation								#
#									#
#########################################################################

install(TARGETS navtracks ${INSTALL_TARGETS_DEFAULT_ARGS})
