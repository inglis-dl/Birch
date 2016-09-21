PROJECT( BirchApplication )

SET( BIRCH_APP_SRC
  Birch.cxx
  QBirchApplication.cxx
  QBirchMainWindow.cxx
)

SET( BIRCH_APP_UI
  QBirchMainWindow.ui
)

SET( BIRCH_APP_MOC
  QBirchMainWindow.h
  QBirchMainWindow_p.h
)

QT5_WRAP_UI( BirchApp_UI   ${BIRCH_APP_UI} )
QT5_WRAP_CPP( BirchApp_MOC ${BIRCH_APP_MOC} )

ADD_DEFINITIONS( -DQT_GUI_LIBS -DQT_CORE_LIB )

SET_SOURCE_FILES_PROPERTIES( ${BIRCH_APP_SRC} PROPERTIES OBJECT_DEPENDS "${BirchApp_UI}" )

INCLUDE_DIRECTORIES(
  ${BIRCH_COMMON_DIR}
  ${BIRCH_VTK_DIR}
  ${BIRCH_QT_WIDGETS_DIR}
  ${QT_INCLUDE_DIR}
)

ADD_EXECUTABLE( birch
  ${BIRCH_APP_SRC}
  ${BirchApp_UI}
  ${BirchApp_MOC}
)

TARGET_LINK_LIBRARIES( birch
  ${BIRCH_LIBRARIES}
  ${QT_LIBRARIES}
  vtkGUISupportQt
  vtkChartsCore
  vtkImagingStatistics
  vtkRenderingContextOpenGL
  vtkViewsContext2D
)

INSTALL( TARGETS birch RUNTIME DESTINATION bin )
