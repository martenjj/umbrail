#########################################################################
#  Project:		Track Editor					#
#  Edit:		18-Nov-17					#
#########################################################################

project(navtracks)

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

add_library(qcustomplot STATIC qcustomplot.cpp)
target_link_libraries(qcustomplot
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
)
