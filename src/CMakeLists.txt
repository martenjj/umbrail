#########################################################################
#  Project:		Track Editor					#
#  Edit:		05-Aug-21					#
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
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS I18n Config XmlGui Parts IconThemes ItemViews KIO Crash Auth)

find_package(Marble REQUIRED NO_POLICY_SCOPE)
find_package(Phonon4Qt5 NO_POLICY_SCOPE)
find_package(KF5KExiv2 NO_POLICY_SCOPE)
find_package(LibKFDialog REQUIRED NO_POLICY_SCOPE)

set_package_properties("Marble" PROPERTIES DESCRIPTION "Map display library" URL "http://marble.kde.org" TYPE REQUIRED PURPOSE "Required for map display")
set_package_properties("Phonon4Qt5" PROPERTIES DESCRIPTION "Multimedia playback library" URL "http://phonon.kde.org" TYPE OPTIONAL PURPOSE "Required for playback of audio/video notes")
set_package_properties("KF5KExiv2" PROPERTIES DESCRIPTION "Wrapper around Exiv2 library" URL "http://quickgit.kde.org/?p=libkexiv2.git" TYPE OPTIONAL PURPOSE "Required for geolocation of photos")
set_package_properties("LibKFDialog" PROPERTIES DESCRIPTION "KDE Frameworks dialogue utility library" URL "https://github.com/martenjj/libkfdialog" TYPE REQUIRED PURPOSE "Required to build the project")

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
add_definitions("-DQT_DEPRECATED_WARNINGS")

remove_definitions("-DQT_NO_CAST_FROM_ASCII")
remove_definitions("-DQT_NO_SIGNALS_SLOTS_KEYWORDS")

#########################################################################
#									#
#  Build options							#
#									#
#########################################################################

################################# TEMP, enable with option above ###############
add_definitions("-DENABLE_OPEN_WITH_GOOGLE=1")
add_definitions("-DENABLE_OPEN_WITH_BING=1")
################################# TEMP #########################################

#########################################################################
#									#
#  Global settings							#
#									#
#########################################################################

kconfig_add_kcfg_files(navtrackssettings_SRCS settings.kcfgc)
add_library(navtrackssettings STATIC ${navtrackssettings_SRCS})
target_link_libraries(navtrackssettings
  KF5::ConfigCore
  KF5::ConfigGui
)

#########################################################################
#									#
#  Subdirectories							#
#									#
#########################################################################

add_subdirectory(qcustomplot)
add_subdirectory(core)
add_subdirectory(services)
add_subdirectory(media)
add_subdirectory(gui)
add_subdirectory(map)
add_subdirectory(io)
add_subdirectory(app)
add_subdirectory(plugins)

#########################################################################
#									#
#  Installation								#
#									#
#########################################################################

if (INSTALL_BINARIES)
  install(TARGETS navtracks ${INSTALL_TARGETS_DEFAULT_ARGS})
endif (INSTALL_BINARIES)
install(FILES settings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR} RENAME navtracks.kcfg)
