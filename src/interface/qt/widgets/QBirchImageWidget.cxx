/*=========================================================================

  Program:  Birch
  Module:   QBirchImageWidget.cxx
  Language: C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QBirchImageWidget.h>
#include <ui_QBirchImageWidget.h>

// Birch includes
#include <QBirchSliceView.h>

// VTK includes
#include <vtkEventForwarderCommand.h>
#include <vtkEventQtSlotConnect.h>
#include <vtkImageData.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

// C++ includes
#include <sstream>
#include <stdexcept>

class QBirchImageWidgetPrivate : public Ui_QBirchImageWidget
{
  Q_DECLARE_PUBLIC(QBirchImageWidget);
  protected:
    QBirchImageWidget* const q_ptr;

  public:
    explicit QBirchImageWidgetPrivate(QBirchImageWidget& object);
    virtual ~QBirchImageWidgetPrivate();

    virtual void setupUi(QWidget* widget);
};

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QBirchImageWidgetPrivate methods
//
// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchImageWidgetPrivate::QBirchImageWidgetPrivate(
  QBirchImageWidget& object)
  : q_ptr(&object)
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchImageWidgetPrivate::~QBirchImageWidgetPrivate()
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchImageWidgetPrivate::setupUi(QWidget* widget)
{
  Q_Q(QBirchImageWidget);

  this->Ui_QBirchImageWidget::setupUi(widget);
  this->imageControl->setSliceView(this->sliceView);
  this->framePlayerWidget->setSliceView(this->sliceView);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchImageWidget::QBirchImageWidget(QWidget* parent)
  : Superclass(parent)
  , d_ptr(new QBirchImageWidgetPrivate(*this))
{
  Q_D(QBirchImageWidget);
  d->setupUi(this);
  this->reset();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchImageWidget::~QBirchImageWidget()
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchImageWidget::eventFilter(QObject *obj, QEvent *event)
{
  Q_D(QBirchImageWidget);
  if (obj == d->sliceView->VTKWidget())
  {
    if (QEvent::Enter == event->type())
    {
      d->frame->setStyleSheet("border : 3px solid red");
    }
    if (QEvent::Leave == event->type())
    {
      d->frame->setStyleSheet("border : 3px solid green");
    }
  }
  return false;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchImageWidget::reset()
{
  Q_D(QBirchImageWidget);
  d->sliceView->setImageToSinusoid();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchImageWidget::load(
  const QString& fileName, vtkEventForwarderCommand* forward)
{
  Q_D(QBirchImageWidget);
  if (!d->sliceView->load(fileName, forward))
  {
    std::stringstream stream;
    stream << "Unable to load image file \"" << fileName.toStdString() << "\"";
    throw std::runtime_error(stream.str());
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchImageWidget::save(const QString& fileName)
{
  Q_D(QBirchImageWidget);
  d->sliceView->writeSlice(fileName);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchImageWidget::setImageData(vtkImageData* data)
{
  Q_D(QBirchImageWidget);
  d->sliceView->setImageData(data);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkImageData* QBirchImageWidget::imageData()
{
  Q_D(QBirchImageWidget);
  return d->sliceView->imageData();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchSliceView* QBirchImageWidget::sliceView()
{
  Q_D(QBirchImageWidget);
  return d->sliceView;
}
