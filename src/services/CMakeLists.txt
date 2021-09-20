#########################################################################
#  Project:		Track Editor					#
#  Edit:		20-Sep-21					#
#########################################################################


#########################################################################
#									#
#  Data service providers						#
#									#
#########################################################################

set(services_SRCS
  elevationmanager.cpp
  elevationtile.cpp
  timezoneprovider.cpp
  mapbrowser.cpp
)

add_library(${PN}services STATIC ${services_SRCS})
target_link_libraries(${PN}services
  Qt5::Core
  Qt5::Gui
  KF5::I18n
  KF5::KIOCore KF5::KIOWidgets
  KF5::WidgetsAddons
  ${PN}core
  ${PN}settings
)
