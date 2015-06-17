#########################################################################
#  Project:		Track Editor					#
#  Edit:		17-Jun-15					#
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

find_package(Marble REQUIRED)
find_package(Phonon)

#########################################################################
#									#
#  Building the target							#
#									#
#########################################################################

set(navtracks_SRCS
  main.cpp
  autotooltipdelegate.cpp
  commands.cpp
  createwaypointdialogue.cpp
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
  latlongwidget.cpp
  layerbase.cpp
  mainwindow.cpp
  mapcontroller.cpp
  mapthemedialogue.cpp
  mapview.cpp
  mediaplayer.cpp
  metadatamodel.cpp
  moveitemdialogue.cpp
  photoviewer.cpp
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
  trackpropertiesmetadatapages.cpp
  trackpropertiesstylepages.cpp
  trackslayer.cpp
  variableunitdisplay.cpp
  variableunitcombo.cpp
  waypointslayer.cpp
  ${QCUSTOMPLOT_DIR}/qcustomplot.cpp
)

if (PHONON_FOUND)
  set(navtracks_SRCS ${navtracks_SRCS} videoviewer.cpp)
endif (PHONON_FOUND)

kde4_add_kcfg_files(navtracks_SRCS settings.kcfgc)

add_definitions("-DVERSION=\\\"${VERSION}\\\"")
if (SORTABLE_VIEW)
  add_definitions("-DSORTABLE_VIEW=1")
endif (SORTABLE_VIEW)
if (PHONON_FOUND)
  add_definitions("-DHAVE_PHONON=1")
endif (PHONON_FOUND)

kde4_add_executable(navtracks ${navtracks_SRCS})
target_link_libraries(navtracks ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${MARBLE_LIBRARIES})
if (PHONON_FOUND)
  target_link_libraries(navtracks ${PHONON_LIBS})
endif (PHONON_FOUND)

add_dependencies(navtracks vcsversion)

#########################################################################
#									#
#  Installation								#
#									#
#########################################################################

install(TARGETS navtracks ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES settings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
