CMAKE_MINIMUM_REQUIRED( VERSION 2.8.7 )
PROJECT( Birch )

# The major version number is incremented after major milestones are met
SET( BIRCH_VERSION_MAJOR 2 )
# The minor version number is incremented after a development branch is merged to qa
SET( BIRCH_VERSION_MINOR 0 )
# The patch version number is incremented when changes are made to the release branch
SET( BIRCH_VERSION_PATCH 0 )

# Look in the build directory for cmake modules
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/" )

# We're using cbegin and cend so we need c++11
SET( CMAKE_CXX_FLAGS "-std=c++0x -Wno-deprecated" )

# We need VTK
FIND_PACKAGE( VTK REQUIRED )
INCLUDE( ${VTK_USE_FILE} )
IF("${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}" VERSION_LESS 6.0)
  MESSAGE(FATAL_ERROR "VTK 6.0 or later is required.")
ENDIF()

# We need GDCM
FIND_PACKAGE( GDCM REQUIRED )
INCLUDE( ${GDCM_USE_FILE} )

# We need vtk-dicom
FIND_PACKAGE( DICOM REQUIRED )
INCLUDE( ${DICOM_USE_FILE} )
SET(VTK_DICOM_LBRARIES vtkDICOM)

# We need QT4
SET( QT_MIN_VERSION 4.0.0 )
FIND_PACKAGE( Qt4 REQUIRED )
INCLUDE( ${QT_USE_FILE} )

FIND_PACKAGE( MySQL REQUIRED )

SET( BIRCH_ROOT_DIR       ${PROJECT_SOURCE_DIR}/.. )
SET( BIRCH_SRC_DIR        ${BIRCH_ROOT_DIR}/src )
SET( BIRCH_COMMON_DIR     ${BIRCH_SRC_DIR}/common )
SET( BIRCH_QT_DIR         ${BIRCH_SRC_DIR}/interface/qt )
SET( BIRCH_QT_WIDGETS_DIR ${BIRCH_QT_DIR}/widgets )
SET( BIRCH_VTK_DIR        ${BIRCH_SRC_DIR}/vtk )
SET( BIRCH_APP_DIR        ${BIRCH_QT_DIR}/application )

# Set default locations for exe and lib files
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# Make sure to include RPATH in the installed binary to support linking to libraries
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Set up include directories
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

SET(BUILD_SHARED_LIBS ON)

# The libraries need BirchConfig.h
INCLUDE_DIRECTORIES( BEFORE
  ${PROJECT_BINARY_DIR}
)

SET( BIRCH_LIBRARIES
  BirchQtWidgets
  BirchVTK
  BirchCommon
)

# Birch libraries for use in other applications
ADD_SUBDIRECTORY(
  ${BIRCH_COMMON_DIR} ${PROJECT_BINARY_DIR}/common
)
ADD_SUBDIRECTORY(
  ${BIRCH_VTK_DIR} ${PROJECT_BINARY_DIR}/vtk
)
ADD_SUBDIRECTORY(
  ${BIRCH_QT_WIDGETS_DIR} ${PROJECT_BINARY_DIR}/widgets
)
ADD_SUBDIRECTORY(
  ${BIRCH_APP_DIR} ${PROJECT_BINARY_DIR}/application
)
