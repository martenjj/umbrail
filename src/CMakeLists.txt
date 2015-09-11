#########################################################################
#  Project:		Track Editor					#
#  Edit:		10-Sep-15					#
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
macro_optional_find_package(Phonon)
macro_optional_find_package(Exiv2)
macro_optional_find_package(Kexiv2)

macro_log_feature(MARBLE_FOUND "Marble" "Map display library" "http://marble.kde.org" TRUE "" "Required for map display")
macro_log_feature(PHONON_FOUND "Phonon" "Multimedia playback library" "http://phonon.kde.org" FALSE "" "Required for playback of audio/video notes")
macro_log_feature(EXIV2_FOUND "Exiv2" "Image metadata support" "http://www.exiv2.org" FALSE "0.19")
macro_log_feature(KEXIV2_FOUND "LibKExiv2" "Wrapper around Exiv2 library" "http://www.digikam.org/sharedlibs" FALSE "" "Required for geolocation of photos")

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
if (EXIV2_FOUND)
  if (KEXIV2_FOUND)
    add_definitions("-DHAVE_KEXIV2=1")
    include_directories(${KEXIV2_INCLUDE_DIR})
  endif (KEXIV2_FOUND)
endif (EXIV2_FOUND)

kde4_add_executable(navtracks ${navtracks_SRCS})
target_link_libraries(navtracks ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${MARBLE_LIBRARIES})
if (PHONON_FOUND)
  target_link_libraries(navtracks ${PHONON_LIBS})
endif (PHONON_FOUND)
if (EXIV2_FOUND)
  if (KEXIV2_FOUND)
    target_link_libraries(navtracks ${KEXIV2_LIBRARIES})
  endif (KEXIV2_FOUND)
endif (EXIV2_FOUND)

add_dependencies(navtracks vcsversion)

#########################################################################
#									#
#  Installation								#
#									#
#########################################################################

install(TARGETS navtracks ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES settings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
