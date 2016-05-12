#########################################################################
#  Project:		Track Editor					#
#  Edit:		12-May-16					#
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

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core Gui Widgets)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS I18n Config XmlGui Parts KIO KDELibs4Support)

find_package(Marble REQUIRED NO_POLICY_SCOPE)
find_package(Phonon4Qt5 NO_POLICY_SCOPE)
find_package(KF5KExiv2 NO_POLICY_SCOPE)

set_package_properties("Marble" PROPERTIES DESCRIPTION "Map display library" URL "http://marble.kde.org" TYPE REQUIRED PURPOSE "Required for map display")
set_package_properties("Phonon4Qt5" PROPERTIES DESCRIPTION "Multimedia playback library" URL "http://phonon.kde.org" TYPE OPTIONAL PURPOSE "Required for playback of audio/video notes")
set_package_properties("KF5KExiv2" PROPERTIES DESCRIPTION "Wrapper around Exiv2 library" URL "http://www.digikam.org/sharedlibs" TYPE OPTIONAL PURPOSE "Required for geolocation of photos")

find_library(LIBDIALOGUTIL_FOUND "libdialogutil.so")
if (LIBDIALOGUTIL_FOUND)
  get_filename_component(LIBDIALOGUTIL_DIR ${LIBDIALOGUTIL_FOUND} DIRECTORY)
  get_filename_component(LIBDIALOGUTIL_LIBRARY ${LIBDIALOGUTIL_FOUND} NAME)
  message(STATUS "Found libdialogutil library: ${LIBDIALOGUTIL_DIR}")

  find_file(HDRDIALOGUTIL_FOUND "dialogbase.h" HINTS ${KDE_INSTALL_FULL_INCLUDEDIR} PATH_SUFFIXES "KF5")
  get_filename_component(HDRDIALOGUTIL_DIR ${HDRDIALOGUTIL_FOUND} DIRECTORY)
  message(STATUS "Found libdialogutil headers: ${HDRDIALOGUTIL_DIR}")
else (LIBDIALOGUTIL_FOUND)
  message(FATAL_ERROR "This project requires the libdialogutil library")
endif (LIBDIALOGUTIL_FOUND)

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
  dataindexer.cpp
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
  profilewidget.cpp
  project.cpp
  settingsdialogue.cpp
  statisticswidget.cpp
  stopdetectdialogue.cpp
  stopslayer.cpp
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
  valueslider.cpp
  variableunitdisplay.cpp
  variableunitcombo.cpp
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

add_executable(navtracks ${navtracks_SRCS})
target_link_libraries(navtracks
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  KF5::I18n
  KF5::ConfigCore
  KF5::XmlGui
  KF5::KIOCore KF5::KIOWidgets
  KF5::Parts
  KF5::KDELibs4Support
  ${MARBLE_LIBRARIES})
if (Phonon4Qt5_FOUND)
  target_link_libraries(navtracks Phonon::phonon4qt5)
endif (Phonon4Qt5_FOUND)
if (KF5KExiv2_FOUND)
  target_link_libraries(navtracks KF5::KExiv2)
endif (KF5KExiv2_FOUND)

target_link_libraries(navtracks ${LIBDIALOGUTIL_LIBRARY})

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
