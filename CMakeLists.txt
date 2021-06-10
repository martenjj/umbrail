#########################################################################
#  Project:		Track Editor					#
#  Edit:		08-Jun-21					#
#########################################################################
#									#
#									#
#########################################################################

project(navtracks)
set(VERSION "0.0.2")
message(STATUS "Configuring for NavTracks version ${VERSION}")

#########################################################################
#									#
#  ECM paths and definitions						#
#									#
#########################################################################

set(QT_MIN_VERSION "5.10.0")
set(KF5_MIN_VERSION "5.69.0")
set(ECM_MIN_VERSION "5.69.0")
set(EXIV2_MIN_VERSION "0.24")

find_package(ECM ${ECM_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

include(ECMSetupVersion)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${KDE_INSTALL_FULL_DATAROOTDIR}/marble/cmake)

#########################################################################
#									#
#  Options								#
#									#
#########################################################################

option(INSTALL_BINARIES "Install the binaries and libraries, turn off for development in place" ON)

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

set(QCUSTOMPLOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/qcustomplot")

add_subdirectory(qcustomplot)
add_subdirectory(src)
add_subdirectory(desktop)
add_subdirectory(pics)

#########################################################################
#									#
#  Information at end of cmake run					#
#									#
#########################################################################

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
