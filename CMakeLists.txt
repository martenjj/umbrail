#########################################################################
#  Project:		Track Editor					#
#  Edit:		20-Jan-14					#
#########################################################################
#									#
#									#
#########################################################################

project(navtracks)
set(VERSION "0.0.1")
message(STATUS "Configuring for NavTracks version ${VERSION}")

#########################################################################
#									#
#  KDE paths and definitions						#
#									#
#########################################################################

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} )
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

#########################################################################
#									#
#  VCS version tracking							#
#									#
#########################################################################

add_custom_target(vcsversion
  ALL
  COMMENT "Checking VCS type and version"
  VERBATIM
  COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/genversion.sh ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${VERSION}
)

#########################################################################
#									#
#  Subdirectories							#
#									#
#########################################################################

macro_optional_add_subdirectory(src)
macro_optional_add_subdirectory(desktop)
macro_optional_add_subdirectory(pics)

#########################################################################
#									#
#  Information at end of cmake run					#
#									#
#########################################################################

macro_display_feature_log()
