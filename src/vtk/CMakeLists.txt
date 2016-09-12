PROJECT( BirchVTK )

SET( BirchVTK_SRCS
  vtkAnimationPlayer.cxx
  vtkCenteredAxesActor.cxx
  vtkCustomCornerAnnotation.cxx
  vtkCustomInteractorStyleImage.cxx
  vtkFrameAnimationPlayer.cxx
  vtkImageCoordinateWidget.cxx
  vtkImageDataReader.cxx
  vtkImageDataWriter.cxx
  vtkImageWindowLevel.cxx
  vtkMedicalImageViewer.cxx
  vtkImageSharpen.cxx
)

SET_SOURCE_FILES_PROPERTIES(
  vtkAnimationPlayer.cxx
  ABSTRACT
)

INCLUDE( ${VTK_USE_FILE} )

INCLUDE_DIRECTORIES(
  ${BIRCH_COMMON_DIR}
)

ADD_LIBRARY( BirchVTK ${BirchVTK_SRCS} )

SET(VTK_LIBS
  vtkImagingMath
  vtkIOMINC
  vtkIOXML
  vtkRenderingFreeTypeOpenGL
  vtkRenderingImage
  vtkRenderingVolumeOpenGL
)

TARGET_LINK_LIBRARIES( BirchVTK
  ${VTK_LIBS}
  vtkDICOM
  BirchCommon
  vtkgdcm
)

INSTALL( TARGETS BirchVTK DESTINATION lib )
