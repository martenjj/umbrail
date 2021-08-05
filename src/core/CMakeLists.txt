#########################################################################
#  Project:		Track Editor					#
#  Edit:		26-Jul-21					#
#########################################################################

project(navtracks)

#########################################################################
#									#
#  Core library								#
#									#
#########################################################################

set(navtrackscore_SRCS
  trackdata.cpp
  dataindexer.cpp
  pluginmanager.cpp
  units.cpp
  waypointimageprovider.cpp
  metadatamodel.cpp
  filesmodel.cpp
  trackfiltermodel.cpp
  applicationdata.cpp
  applicationdatainterface.cpp
)

add_library(navtrackscore STATIC ${navtrackscore_SRCS})
target_link_libraries(navtrackscore
  Qt5::Core
  Qt5::Gui
  KF5::I18n
  KF5::KIOCore
  KF5::IconThemes
  KF5::ConfigWidgets
)
