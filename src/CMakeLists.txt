#########################################################################
#  Project:		Track Editor					#
#  Edit:		22-Apr-15					#
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
  autotooltipdelegate.cpp
  commands.cpp
  dataindexer.cpp
  errorreporter.cpp
  exporterbase.cpp
  filescontroller.cpp
  filesmodel.cpp
  filesview.cpp
  gpxexporter.cpp
  gpximporter.cpp
  importerexporterbase.cpp
  importerbase.cpp
  itemtypecombo.cpp
  latlongdialogue.cpp
  layerbase.cpp
  mainwindow.cpp
  mapcontroller.cpp
  mapthemedialogue.cpp
  mapview.cpp
  moveitemdialogue.cpp
  profilewidget.cpp
  project.cpp
  settingsdialogue.cpp
  statisticswidget.cpp
  style.cpp
  timezonedialogue.cpp
  timezoneselector.cpp
  trackdata.cpp
  trackdatalabel.cpp
  trackfiltermodel.cpp
  trackpropertiesdialogue.cpp
  trackpropertiespage.cpp
  trackpropertiesdetailpages.cpp
  trackpropertiesgeneralpages.cpp
  trackpropertiesstylepages.cpp
  trackslayer.cpp
  variableunitdisplay.cpp
  variableunitcombo.cpp
  waypointslayer.cpp
  ${QCUSTOMPLOT_DIR}/qcustomplot.cpp
)

kde4_add_kcfg_files(navtracks_SRCS settings.kcfgc)

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
install(FILES settings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
