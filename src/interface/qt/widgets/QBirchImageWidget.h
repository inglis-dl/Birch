/*=========================================================================

  Program:  Birch
  Module:   QBirchImageWidget.h
  Language: C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QBirchImageWidget_h
#define __QBirchImageWidget_h

// Qt includes
#include <QWidget>

class QBirchImageWidgetPrivate;
class QBirchSliceView;
class vtkEventForwarderCommand;
class vtkImageData;

class QBirchImageWidget : public QWidget
{
  Q_OBJECT

  public:
    typedef QWidget Superclass;
    // constructor
    explicit QBirchImageWidget(QWidget* parent = 0);
    // destructor
    virtual ~QBirchImageWidget();

    void reset();
    void load(const QString& filename, vtkEventForwarderCommand* forward = 0);
    void save(const QString& fileName);
    QBirchSliceView* sliceView();
    vtkImageData* imageData();
    void setImageData(vtkImageData* data);

  protected:
    QScopedPointer<QBirchImageWidgetPrivate> d_ptr;

    bool eventFilter(QObject* object, QEvent* event);

  private:
    Q_DECLARE_PRIVATE(QBirchImageWidget);
    Q_DISABLE_COPY(QBirchImageWidget);
};

#endif
