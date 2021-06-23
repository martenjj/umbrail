#########################################################################
#  Project:		Track Editor					#
#  Edit:		23-Jun-21					#
#########################################################################

project(navtracks)

#########################################################################
#									#
#  Media display							#
#									#
#########################################################################

set(navtracksmedia_SRCS
  mediaplayer.cpp
  photoviewer.cpp
)

if (Phonon4Qt5_FOUND)
  set(navtracksmedia_SRCS ${navtracksmedia_SRCS} videoviewer.cpp)
  add_definitions("-DHAVE_PHONON=1")
endif (Phonon4Qt5_FOUND)

add_library(navtracksmedia STATIC ${navtracksmedia_SRCS})
target_link_libraries(navtracksmedia
  Qt5::Core
  KF5::Parts
  navtrackscore
  navtrackssettings
)

if (Phonon4Qt5_FOUND)
  target_link_libraries(navtracksmedia Phonon::phonon4qt5)
endif (Phonon4Qt5_FOUND)
