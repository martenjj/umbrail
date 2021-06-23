#########################################################################
#  Project:		Track Editor					#
#  Edit:		23-Jun-21					#
#########################################################################

project(navtracks)

#########################################################################
#									#
#  Import and export							#
#									#
#########################################################################

set(navtracksio_SRCS
  errorreporter.cpp
  exporterbase.cpp
  gpxexporter.cpp
  gpximporter.cpp
  importerexporterbase.cpp
  importerbase.cpp
)

add_library(navtracksio STATIC ${navtracksio_SRCS})
target_link_libraries(navtracksio
  Qt5::Core
  Qt5::Gui
  Qt5::Xml
  KF5::I18n
  KF5::ConfigWidgets
  navtrackscore
)
