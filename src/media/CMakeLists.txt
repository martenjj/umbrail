#########################################################################
#  Project:		Track Editor					#
#  Edit:		20-Sep-21					#
#########################################################################


#########################################################################
#									#
#  Media display							#
#									#
#########################################################################

set(media_SRCS
  mediaplayer.cpp
  photoviewer.cpp
)

if (Phonon4Qt5_FOUND)
  set(media_SRCS ${media_SRCS} videoviewer.cpp)
  add_definitions("-DHAVE_PHONON=1")
endif (Phonon4Qt5_FOUND)

add_library(${PN}media STATIC ${media_SRCS})
target_link_libraries(${PN}media
  Qt5::Core
  KF5::Parts
  ${PN}core
  ${PN}settings
)

if (Phonon4Qt5_FOUND)
  target_link_libraries(${PN}media Phonon::phonon4qt5)
endif (Phonon4Qt5_FOUND)
