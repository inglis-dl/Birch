/*=======================================================================

  Module:    QBirchSliceView.h
  Program:   Birch
  Language:  C++
  Author:    Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QBirchSliceView_h
#define __QBirchSliceView_h

// Birch includes
#include <QBirchAbstractView.h>

class QBirchSliceViewPrivate;
class vtkEventForwarderCommand;
class vtkImageData;

class QBirchSliceView : public QBirchAbstractView
{
  Q_OBJECT
  Q_PROPERTY(bool annotateOverView READ annotateOverView
    WRITE setAnnotateOverView)
  Q_PROPERTY(bool cursorOverView READ cursorOverView
    WRITE setCursorOverView)
  Q_PROPERTY(double colorLevel READ colorLevel WRITE setColorLevel)
  Q_PROPERTY(double colorWindow READ colorWindow WRITE setColorWindow)
  Q_PROPERTY(Orientation orientation READ orientation
    WRITE setOrientation NOTIFY orientationChanged)
  Q_PROPERTY(int interpolation READ interpolation WRITE setInterpolation)
  Q_PROPERTY(QColor annotationColor READ annotationColor
    WRITE setAnnotationColor)
  Q_ENUMS(Orientation)

  public:
    typedef QBirchAbstractView Superclass;
    explicit QBirchSliceView(QWidget* parent = 0);
    virtual ~QBirchSliceView();

    enum Orientation { OrientationYZ, OrientationXZ, OrientationXY };

    Orientation orientation() const;
    double colorLevel() const;
    double colorWindow() const;
    int slice() const;
    int sliceMin();
    int sliceMax();
    int interpolation() const;
    bool annotateOverView() const;
    bool cursorOverView() const;
    bool hasImageData() const;
    int dimensionality() const;
    void setImageToSinusoid();
    bool load(const QString& fileName, vtkEventForwarderCommand* forward = 0);
    void writeSlice(const QString& fileName);
    virtual QColor annotationColor() const;
    vtkImageData* imageData();
    void setImageData(vtkImageData* data);
    int frameRate() const;

  public slots:
    void setColorLevel(double newColorLevel);
    void setColorWindow(double newColorWindow);
    void setOrientation(Orientation orientation);
    void setSlice(int slice);
    void invertColorWindowLevel();
    void setCursorOverView(bool view);
    void setAnnotateOverView(bool view);
    void rotateCamera(double angle);
    void setInterpolation(int newInterpolation);
    void flipCameraHorizontal();
    void flipCameraVertical();
    void rotateCameraClockwise();
    void rotateCameraCounterClockwise();
    void setAnnotationColor(const QColor& qcolor);

  Q_SIGNALS:
    void orientationChanged(QBirchSliceView::Orientation orientation);
    void imageDataChanged();

  private:
    Q_DECLARE_PRIVATE(QBirchSliceView);
    Q_DISABLE_COPY(QBirchSliceView);
};

#endif
