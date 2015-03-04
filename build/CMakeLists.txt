CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )
PROJECT( Birch )

# Set the version
SET( BIRCH_VERSION_MAJOR 0 )
SET( BIRCH_VERSION_MINOR 1 )
SET( BIRCH_VERSION_PATCH 0 )

# Look in the build directory for cmake modules
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/" )

# We need VTK
FIND_PACKAGE( VTK REQUIRED )
INCLUDE( ${VTK_USE_FILE} )

# We need GDCM
FIND_PACKAGE( GDCM REQUIRED )
INCLUDE( ${GDCM_USE_FILE} )

# We need QT4
SET( QT_MIN_VERSION 4.0.0 )
FIND_PACKAGE( Qt4 REQUIRED )
INCLUDE( ${QT_USE_FILE} )

SET( BIRCH_ROOT_DIR ${PROJECT_SOURCE_DIR}/.. )
SET( BIRCH_SRC_DIR ${BIRCH_ROOT_DIR}/src )
SET( BIRCH_QT_DIR ${BIRCH_SRC_DIR}/interface/qt )
SET( BIRCH_VTK_DIR ${BIRCH_SRC_DIR}/vtk )

# Configure the utitlities header
CONFIGURE_FILE( ${BIRCH_SRC_DIR}/Utilities.h.in
                ${PROJECT_BINARY_DIR}/Utilities.h @ONLY IMMEDIATE )

# Set up include directories
SET( BIRCH_INCLUDE_DIR
  ${BIRCH_QT_DIR}
  ${BIRCH_VTK_DIR}
)

SET( BIRCH_VTK_LIB_SOURCE 
  ${BIRCH_VTK_DIR}/vtkAnimationPlayer.cxx
  ${BIRCH_VTK_DIR}/vtkCustomCornerAnnotation.cxx
  ${BIRCH_VTK_DIR}/vtkCustomInteractorStyleImage.cxx
  ${BIRCH_VTK_DIR}/vtkFrameAnimationPlayer.cxx
  ${BIRCH_VTK_DIR}/vtkFrameAnimationPlayer.cxx
  ${BIRCH_VTK_DIR}/vtkImageCoordinateWidget.cxx
  ${BIRCH_VTK_DIR}/vtkImageDataReader.cxx
  ${BIRCH_VTK_DIR}/vtkImageDataWriter.cxx
  ${BIRCH_VTK_DIR}/vtkImageWindowLevel.cxx
  ${BIRCH_VTK_DIR}/vtkImageSharpen.cxx
  ${BIRCH_VTK_DIR}/vtkMedicalImageViewer.cxx
)

SET_SOURCE_FILES_PROPERTIES( 
  ${BIRCH_VTK_DIR}/vtkAnimationPlayer.cxx
  ABSTRACT 
)

ADD_LIBRARY( VTKBirch ${BIRCH_VTK_LIB_SOURCE} )

SET( BIRCH_APP_SOURCE
  ${BIRCH_SRC_DIR}/Birch.cxx
  
  ${BIRCH_QT_DIR}/QBirchApplication.cxx
  ${BIRCH_QT_DIR}/QMainBirchWindow.cxx
  ${BIRCH_QT_DIR}/widgets/vtkBirchQDicomTagWidget.cxx
  ${BIRCH_QT_DIR}/widgets/vtkBirchDoubleSlider.cxx
  ${BIRCH_QT_DIR}/widgets/vtkBirchSliderWidget.cxx
  ${BIRCH_QT_DIR}/widgets/vtkBirchQFramePlayerWidget.cxx
)

SET( BIRCH_QT_UIS
  ${BIRCH_QT_DIR}/QMainBirchWindow.ui
  ${BIRCH_QT_DIR}/widgets/Resources/UI/vtkBirchQDicomTagWidget.ui
  ${BIRCH_QT_DIR}/widgets/Resources/UI/vtkBirchQFramePlayerWidget.ui
  ${BIRCH_QT_DIR}/widgets/Resources/UI/vtkBirchSliderWidget.ui
)

SET( BIRCH_QT_HEADERS
  ${BIRCH_QT_DIR}/QMainBirchWindow.h
  ${BIRCH_QT_DIR}/widgets/vtkBirchQDicomTagWidget.h
  ${BIRCH_QT_DIR}/widgets/vtkBirchQFramePlayerWidget.h
  ${BIRCH_QT_DIR}/widgets/vtkBirchSliderWidget.h
  ${BIRCH_QT_DIR}/widgets/vtkBirchDoubleSlider.h
)

SET( BIRCH_QT_QRC
  ${BIRCH_QT_DIR}/widgets/Resources/vtkBirchQWidgets.qrc
)

# Wrap QT files
QT4_WRAP_UI( UIHeaders ${BIRCH_QT_UIS} )
QT4_WRAP_CPP( MOCSrcs ${BIRCH_QT_HEADERS} )
QT4_ADD_RESOURCES( QRCSrcs ${BIRCH_QT_QRC} )

ADD_DEFINITIONS( -DQT_GUI_LIBS -DQT_CORE_LIB )

SET_SOURCE_FILES_PROPERTIES( ${BIRCH_APP_SOURCE} PROPERTIES OBJECT_DEPENDS "${UIHeaders}" )

# Use the include path and library for Qt
INCLUDE_DIRECTORIES(
  ${BIRCH_QT_DIR}/widgets
  ${BIRCH_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# Targets
ADD_EXECUTABLE( Birch ${BIRCH_APP_SOURCE} ${MOCSrcs} ${QRCSrcs})

TARGET_LINK_LIBRARIES( Birch
  VTKBirch
  QVTK
  vtkRendering
  vtkGraphics
  vtkIO
  vtkCommon
  vtkgdcm
  gdcmDSED
  gdcmMSFF
  gdcmDICT
  vtkCharts
  ${QT_LIBRARIES}
)

INSTALL( TARGETS Birch RUNTIME DESTINATION bin )

#SUBDIRS(${BIRCH_ROOT_DIR}/testing)

