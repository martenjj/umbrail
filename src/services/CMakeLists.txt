#########################################################################
#  Project:		Track Editor					#
#  Edit:		15-Aug-21					#
#########################################################################

project(navtracks)

#########################################################################
#									#
#  Data service providers						#
#									#
#########################################################################

set(navtracksservices_SRCS
  elevationmanager.cpp
  elevationtile.cpp
  timezoneprovider.cpp
  mapbrowser.cpp
)

add_library(navtracksservices STATIC ${navtracksservices_SRCS})
target_link_libraries(navtracksservices
  Qt5::Core
  Qt5::Gui
  KF5::I18n
  KF5::KIOCore KF5::KIOWidgets
  KF5::WidgetsAddons
  navtrackscore
  navtrackssettings
)
