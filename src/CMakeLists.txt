#########################################################################
#  Project:		Track Editor					#
#  Edit:		02-Jun-17					#
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

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core Gui Widgets PrintSupport)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS I18n Config XmlGui Parts IconThemes ItemViews KIO)

find_package(Marble REQUIRED NO_POLICY_SCOPE)
find_package(Phonon4Qt5 NO_POLICY_SCOPE)
find_package(KF5KExiv2 NO_POLICY_SCOPE)
find_package(DialogUtil REQUIRED NO_POLICY_SCOPE)

set_package_properties("Marble" PROPERTIES DESCRIPTION "Map display library" URL "http://marble.kde.org" TYPE REQUIRED PURPOSE "Required for map display")
set_package_properties("Phonon4Qt5" PROPERTIES DESCRIPTION "Multimedia playback library" URL "http://phonon.kde.org" TYPE OPTIONAL PURPOSE "Required for playback of audio/video notes")
set_package_properties("KF5KExiv2" PROPERTIES DESCRIPTION "Wrapper around Exiv2 library" URL "http://quickgit.kde.org/?p=libkexiv2.git" TYPE OPTIONAL PURPOSE "Required for geolocation of photos")
set_package_properties("DialogUtil" PROPERTIES DESCRIPTION "Dialogue utility library" TYPE REQUIRED PURPOSE "Required to build the project")

#########################################################################
#									#
#  Rigourousness							#
#									#
#########################################################################

add_definitions("-DQT_USE_FAST_CONCATENATION")
add_definitions("-DQT_USE_FAST_OPERATOR_PLUS")
add_definitions("-DQT_NO_CAST_FROM_BYTEARRAY")
add_definitions("-DQT_NO_URL_CAST_FROM_STRING")
add_definitions("-DQT_NO_CAST_TO_ASCII")
add_definitions("-DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT")

remove_definitions("-DQT_NO_CAST_FROM_ASCII")
remove_definitions("-DQT_NO_SIGNALS_SLOTS_KEYWORDS")

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
  createroutepointdialogue.cpp
  dataindexer.cpp
  elevationmanager.cpp
  elevationtile.cpp
  errorreporter.cpp
  exporterbase.cpp
  filescontroller.cpp
  filesmodel.cpp
  filesview.cpp
  folderselectdialogue.cpp
  folderselectwidget.cpp
  gpxexporter.cpp
  gpximporter.cpp
  importerexporterbase.cpp
  importerbase.cpp
  itemselectdialogue.cpp
  itemtypecombo.cpp
  latlongdialogue.cpp
  latlongwidget.cpp
  layerbase.cpp
  mainwindow.cpp
  mainwindowinterface.cpp
  mapcontroller.cpp
  mapthemedialogue.cpp
  mapview.cpp
  mediaplayer.cpp
  metadatamodel.cpp
  moveitemdialogue.cpp
  photoviewer.cpp
  positioninfodialogue.cpp
  profilewidget.cpp
  project.cpp
  routeslayer.cpp
  settingsdialogue.cpp
  statisticswidget.cpp
  stopdetectdialogue.cpp
  stopslayer.cpp
  style.cpp
  timezonedialogue.cpp
  timezoneprovider.cpp
  timezoneselector.cpp
  timezonewidget.cpp
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
  valueslider.cpp
  variableunitdisplay.cpp
  variableunitcombo.cpp
  waypointimageprovider.cpp
  waypointslayer.cpp
  ${QCUSTOMPLOT_DIR}/qcustomplot.cpp
)

if (Phonon4Qt5_FOUND)
  set(navtracks_SRCS ${navtracks_SRCS} videoviewer.cpp)
endif (Phonon4Qt5_FOUND)

kconfig_add_kcfg_files(navtracks_SRCS settings.kcfgc)

add_definitions("-DVERSION=\\\"${VERSION}\\\"")
if (SORTABLE_VIEW)
  add_definitions("-DSORTABLE_VIEW=1")
endif (SORTABLE_VIEW)
if (Phonon4Qt5_FOUND)
  add_definitions("-DHAVE_PHONON=1")
  include_directories(${PHONON_INCLUDES})
endif (Phonon4Qt5_FOUND)
if (KF5KExiv2_FOUND)
  add_definitions("-DHAVE_KEXIV2=1")
  include_directories(${KF5KExiv2_INCLUDE_DIR})
endif (KF5KExiv2_FOUND)

# see https://cgit.kde.org/marble.git/tree/examples/cpp/hello-marble/CMakeLists.txt.external
set(MARBLE_LIBRARIES "Marble")

add_executable(navtracks ${navtracks_SRCS})
target_link_libraries(navtracks
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  Qt5::PrintSupport
  KF5::I18n
  KF5::ConfigCore
  KF5::XmlGui
  KF5::KIOCore KF5::KIOWidgets
  KF5::IconThemes
  KF5::ItemViews
  KF5::Parts
  ${DIALOGUTIL_LIBRARIES}
  ${MARBLE_LIBRARIES})
if (Phonon4Qt5_FOUND)
  target_link_libraries(navtracks Phonon::phonon4qt5)
endif (Phonon4Qt5_FOUND)
if (KF5KExiv2_FOUND)
  target_link_libraries(navtracks KF5::KExiv2)
endif (KF5KExiv2_FOUND)

add_dependencies(navtracks vcsversion)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/..)
include_directories(${QCUSTOMPLOT_DIR})
include_directories(${HDRDIALOGUTIL_DIR})

#########################################################################
#									#
#  Installation								#
#									#
#########################################################################

install(TARGETS navtracks ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES settings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR}/navtracks.kcfg)
