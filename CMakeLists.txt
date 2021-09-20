#########################################################################
#  Project:		Track Editor					#
#  Edit:		20-Sep-21					#
#########################################################################
#									#
#									#
#########################################################################

project(umbrail)
set(VERSION "1.0.0")
message(STATUS "Configuring for project '${CMAKE_PROJECT_NAME}' version ${VERSION}")

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
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(FeatureSummary)

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

set(PN ${CMAKE_PROJECT_NAME})

add_subdirectory(src)
add_subdirectory(desktop)
add_subdirectory(pics)

#########################################################################
#									#
#  Information at end of cmake run					#
#									#
#########################################################################

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
