CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )
PROJECT( BirchTesting )

SET( TEST_ANIMATION_SOURCE
  TestAnimation.cxx
)

# Targets
ADD_EXECUTABLE( TestAnimation ${TEST_ANIMATION_SOURCE} )

TARGET_LINK_LIBRARIES( TestAnimation
  VTKBirch
  vtkRendering
  vtkIO
  vtkgdcm
)

INSTALL( TARGETS TestAnimation RUNTIME DESTINATION bin )
