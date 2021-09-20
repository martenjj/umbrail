#########################################################################
#  Project:		Track Editor					#
#  Edit:		20-Sep-21					#
#########################################################################


#########################################################################
#									#
#  Import and export							#
#									#
#########################################################################

set(io_SRCS
  errorreporter.cpp
  exporterbase.cpp
  gpxexporter.cpp
  gpximporter.cpp
  importerexporterbase.cpp
  importerbase.cpp
)

add_library(${PN}io STATIC ${io_SRCS})
target_link_libraries(${PN}io
  Qt5::Core
  Qt5::Gui
  Qt5::Xml
  KF5::I18n
  KF5::ConfigWidgets
  ${PN}core
)
