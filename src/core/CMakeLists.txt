#########################################################################
#  Project:		Track Editor					#
#  Edit:		20-Sep-21					#
#########################################################################


#########################################################################
#									#
#  Core library								#
#									#
#########################################################################

set(core_SRCS
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

add_library(${PN}core STATIC ${core_SRCS})
target_link_libraries(${PN}core
  Qt5::Core
  Qt5::Gui
  KF5::I18n
  KF5::KIOCore
  KF5::IconThemes
  KF5::ConfigWidgets
)
