PROJECT( BirchQtWidgets )

SET( BirchQtWidgetsLib_SRC
  QBirchAbstractView.cxx
  QBirchDicomTagWidget.cxx
  QBirchDoubleSlider.cxx
  QBirchDoubleSpinBox.cxx
  QBirchFramePlayerWidget.cxx
  QBirchImageControl.cxx
  QBirchImageWidget.cxx
  QBirchSliceView.cxx
  QBirchSliderWidget.cxx
  QBirchVTKOutputWindow.cxx
)

SET( BIRCH_WIDGET_UI_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/Resources/UI
)
SET( BirchQtWidgets_Uis
  ${BIRCH_WIDGET_UI_DIR}/QBirchDicomTagWidget.ui
  ${BIRCH_WIDGET_UI_DIR}/QBirchFramePlayerWidget.ui
  ${BIRCH_WIDGET_UI_DIR}/QBirchImageControl.ui
  ${BIRCH_WIDGET_UI_DIR}/QBirchImageWidget.ui
  ${BIRCH_WIDGET_UI_DIR}/QBirchSliderWidget.ui
)

SET( BirchQtWidgets_MocHeaders
  QBirchAbstractView.h
  QBirchAbstractView_p.h
  QBirchDicomTagWidget.h
  QBirchDoubleSlider.h
  QBirchDoubleSpinBox.h
  QBirchDoubleSpinBox_p.h
  QBirchFramePlayerWidget.h
  QBirchImageControl.h
  QBirchImageWidget.h
  QBirchSliderWidget.h
  QBirchSliceView.h
  QBirchSliceView_p.h
)

SET( BirchQtWidgets_Resources
  ${CMAKE_CURRENT_SOURCE_DIR}/Resources/QBirchWidgets.qrc
)

INCLUDE_DIRECTORIES(
  ${BIRCH_COMMON_DIR}
  ${BIRCH_VTK_DIR}
)

# Wrap QT files
QT5_WRAP_UI(       BirchQtWidgetsLib_UI  ${BirchQtWidgets_Uis} )
QT5_WRAP_CPP(      BirchQtWidgetsLib_MOC ${BirchQtWidgets_MocHeaders} )
QT5_ADD_RESOURCES( BirchQtWidgetsLib_QRC ${BirchQtWidgets_Resources} )

ADD_LIBRARY( BirchQtWidgets
  ${BirchQtWidgetsLib_SRC}
  ${BirchQtWidgetsLib_UI}
  ${BirchQtWidgetsLib_MOC}
  ${BirchQtWidgetsLib_QRC}
)

INSTALL( TARGETS BirchQtWidgets
  DESTINATION lib
)
