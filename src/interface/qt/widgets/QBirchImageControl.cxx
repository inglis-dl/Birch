/*==============================================================================

  Module:    QBirchImageControl.h
  Program:   Birch
  Language:  C++
  Author:    Dean Inglis <inglisd AT mcmaster DOT ca>

==============================================================================*/
#include <QBirchImageControl.h>
#include <ui_QBirchImageControl.h>

// Birch includes
#include <Common.h>
#include <QBirchSliceView.h>

// Qt includes
#include <QIcon>
#include <QColor>
#include <QColorDialog>
#include <QPainter>
#include <QPen>

// VTK includes
#include <vtkEventQtSlotConnect.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
class QBirchImageControlPrivate : public Ui_QBirchImageControl
{
  Q_DECLARE_PUBLIC(QBirchImageControl);
  protected:
    QBirchImageControl* const q_ptr;

  public:
    explicit QBirchImageControlPrivate(QBirchImageControl& object);
    virtual ~QBirchImageControlPrivate();

    virtual void setupUi(QWidget* widget);
    virtual void updateUi();

    // 1 => foreground, 0 => background, 2 => annotation
    QColor color(const int& which) const;
    void setColor(const int& which, const QColor& color);

  private:
    QColor backgroundColor;
    QColor foregroundColor;
    QColor annotationColor;
    bool interpolation;

    void drawButton(const int& which);
};

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QBirchImageControlPrivate methods
//
// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchImageControlPrivate::QBirchImageControlPrivate
(QBirchImageControl& object)
  : q_ptr(&object)
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchImageControlPrivate::~QBirchImageControlPrivate()
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchImageControlPrivate::setupUi(QWidget* widget)
{
  Q_Q(QBirchImageControl);

  this->Ui_QBirchImageControl::setupUi(widget);

  this->foregroundColorButton->setProperty("which", QVariant(1));
  this->backgroundColorButton->setProperty("which", QVariant(0));
  this->annotationColorButton->setProperty("which", QVariant(2));

  this->viewXButton->setProperty("orientation", QVariant(
    QBirchSliceView::OrientationYZ));
  this->viewYButton->setProperty("orientation", QVariant(
    QBirchSliceView::OrientationXZ));
  this->viewZButton->setProperty("orientation", QVariant(
    QBirchSliceView::OrientationXY));

  QColor qcolor;
  qcolor.setRgbF(0., 0., 0.);
  this->setColor(0, qcolor);  // black background
  qcolor.setRgbF(0., 0., 1.);
  this->setColor(1, qcolor);  // blue foreground
  qcolor.setRgbF(1., 1., 1.);
  this->setColor(2, qcolor);  // white annotation text
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchImageControlPrivate::updateUi()
{
  Q_Q(QBirchImageControl);
  if (!q->sliceViewPointer.isNull())
  {
    QBirchSliceView* view = q->sliceViewPointer.data();
    this->setColor(2, view->annotationColor());
    this->setColor(1, view->foregroundColor());
    this->setColor(0, view->backgroundColor());
    this->interpolationButton->blockSignals(true);
    q->setInterpolation(view->interpolation());
    this->interpolationButton->blockSignals(false);
    bool enable = view->hasImageData();
    QList<QToolButton*> buttons = q->findChildren<QToolButton*>();
    QListIterator<QToolButton*> it(buttons);
    while (it.hasNext())
    {
      it.next()->setEnabled(enable);
    }
  }
  else
  {
    q->setEnabled(false);
  }
}

// set the color(s) of the SliceView and this widget's buttons
// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchImageControlPrivate::setColor(const int& which, const QColor& color)
{
  Q_Q(QBirchImageControl);

  QBirchSliceView* view =
    q->sliceViewPointer.isNull() ? 0 : q->sliceViewPointer.data();
  if (2 == which)
  {
    this->annotationColor = color;
    if (view)
    {
      view->setAnnotationColor(color);
    }
    this->drawButton(which);
    return;
  }

  if (1 == which)
    this->foregroundColor = color;
  else
    this->backgroundColor = color;

  bool gradient = true;
  if (view)
  {
    gradient = view->gradientBackground();
    if (gradient)
    {
      if (1 == which)
        view->setForegroundColor(this->foregroundColor);
      else
        view->setBackgroundColor(this->backgroundColor);
    }
    else
    {
      this->foregroundColor = color;
      this->backgroundColor = color;
      view->setForegroundColor(color);
      view->setBackgroundColor(color);
    }
  }
  if (gradient)
  {
    this->drawButton(which);
  }
  else
  {
    this->drawButton(0);
    this->drawButton(1);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchImageControlPrivate::drawButton(const int& which)
{
  QToolButton* button;
  QColor color;
  if (1 == which)
  {
    color = this->foregroundColor;
    button = this->foregroundColorButton;
  }
  else if (0 == which)
  {
    color = this->backgroundColor;
    button = this->backgroundColorButton;
  }
  else if (2 == which)
  {
    color = this->annotationColor;
    button = this->annotationColorButton;
  }

  int iconSize = button->style()->pixelMetric(QStyle::PM_SmallIconSize);
  QPixmap pix(iconSize, iconSize);
  pix.fill(
    color.isValid() ? button->palette().button().color() : Qt::transparent);
  QPainter painter(&pix);
  painter.setPen(QPen(Qt::gray));
  painter.setBrush(color.isValid() ? color : QBrush(Qt::NoBrush));
  painter.drawRect(0, 0, pix.width(), pix.height());
  button->setIcon(QIcon(pix));
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QColor QBirchImageControlPrivate::color(const int& which) const
{
  QColor qcolor;
  switch (which)
  {
    case 0: qcolor = this->backgroundColor; break;
    case 1: qcolor = this->foregroundColor; break;
    case 2: qcolor = this->annotationColor; break;
  }
  return qcolor;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QBirchImageControl methods
//
// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchImageControl::QBirchImageControl(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new QBirchImageControlPrivate(*this))
{
  Q_D(QBirchImageControl);
  d->setupUi(this);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchImageControl::~QBirchImageControl()
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchImageControl::update()
{
  Q_D(QBirchImageControl);
  d->updateUi();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchImageControl::setSliceView(QBirchSliceView* view)
{
  Q_D(QBirchImageControl);

  if (!this->sliceViewPointer.isNull())
  {
    disconnect(view, SIGNAL(imageDataChanged()),
      this, SLOT(update()));
    disconnect(d->flipVerticalButton, SIGNAL(pressed()),
      view, SLOT(flipCameraVertical()));
    disconnect(d->flipHorizontalButton, SIGNAL(pressed()),
      view, SLOT(flipCameraHorizontal()));
    disconnect(d->rotateClockwiseButton, SIGNAL(pressed()),
      view, SLOT(rotateCameraClockwise()));
    disconnect(d->rotateCounterClockwiseButton, SIGNAL(pressed()),
      view, SLOT(rotateCameraCounterClockwise()));
    disconnect(d->interpolationButton, SIGNAL(toggled(bool)),
      this, SLOT(interpolate(bool)));
    disconnect(d->invertWindowLevelButton, SIGNAL(pressed()),
      view, SLOT(invertColorWindowLevel()));
    disconnect(d->backgroundColorButton, SIGNAL(pressed()),
      this, SLOT(selectColor()));
    disconnect(d->foregroundColorButton, SIGNAL(pressed()),
      this, SLOT(selectColor()));
    disconnect(d->annotationColorButton, SIGNAL(pressed()),
      this, SLOT(selectColor()));
    disconnect(d->viewXButton, SIGNAL(pressed()),
      this, SLOT(viewToAxis()));
    disconnect(d->viewYButton, SIGNAL(pressed()),
      this, SLOT(viewToAxis()));
    disconnect(d->viewZButton, SIGNAL(pressed()),
      this, SLOT(viewToAxis()));
  }
  this->sliceViewPointer = view;

  if (!this->sliceViewPointer.isNull())
  {
    connect(view, SIGNAL(imageDataChanged()),
      this, SLOT(update()));
    connect(d->flipVerticalButton, SIGNAL(pressed()),
      view, SLOT(flipCameraVertical()));
    connect(d->flipHorizontalButton, SIGNAL(pressed()),
      view, SLOT(flipCameraHorizontal()));
    connect(d->rotateClockwiseButton, SIGNAL(pressed()),
      view, SLOT(rotateCameraClockwise()));
    connect(d->rotateCounterClockwiseButton, SIGNAL(pressed()),
      view, SLOT(rotateCameraCounterClockwise()));
    connect(d->interpolationButton, SIGNAL(toggled(bool)),
      this, SLOT(interpolate(bool)));
    connect(d->invertWindowLevelButton, SIGNAL(pressed()),
      view, SLOT(invertColorWindowLevel()));
    connect(d->backgroundColorButton, SIGNAL(pressed()),
      this, SLOT(selectColor()));
    connect(d->foregroundColorButton, SIGNAL(pressed()),
      this, SLOT(selectColor()));
    connect(d->annotationColorButton, SIGNAL(pressed()),
      this, SLOT(selectColor()));
    connect(d->viewXButton, SIGNAL(pressed()),
      this, SLOT(viewToAxis()));
    connect(d->viewYButton, SIGNAL(pressed()),
      this, SLOT(viewToAxis()));
    connect(d->viewZButton, SIGNAL(pressed()),
      this, SLOT(viewToAxis()));
  }
  d->updateUi();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchImageControl::selectColor()
{
  Q_D(QBirchImageControl);

  QToolButton* button = qobject_cast<QToolButton*>(sender());
  QColorDialog dialog(this);
  if (button && dialog.exec())
  {
    QColor qcolor = dialog.selectedColor();
    QVariant v = button->property("which");
    if (v.isValid())
      d->setColor(v.toInt(), qcolor);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchImageControl::interpolate(bool state)
{
  Q_D(QBirchImageControl);
  QBirchSliceView* view =
    this->sliceViewPointer.isNull() ? 0 : this->sliceViewPointer.data();
  if (state)
  {
    if (view)
    {
      view->setInterpolation(VTK_LINEAR_INTERPOLATION);
    }
    d->interpolationButton->setIcon(QIcon(":/icons/linearicon"));
  }
  else
  {
    if (view)
    {
      view->setInterpolation(VTK_NEAREST_INTERPOLATION);
    }
    d->interpolationButton->setIcon(QIcon(":/icons/nearesticon"));
  }
  d->interpolation = state;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchImageControl::interpolation() const
{
  Q_D(const QBirchImageControl);
  return d->interpolation;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchImageControl::setInterpolation(const bool& state)
{
  this->interpolate(state);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchImageControl::viewToAxis()
{
  Q_D(QBirchImageControl);
  QBirchSliceView* view =
  this->sliceViewPointer.isNull() ? 0 : this->sliceViewPointer.data();
  QToolButton* button = qobject_cast<QToolButton*>(sender());
  if (button && view)
  {
    QVariant v = button->property("orientation");
    if (v.isValid())
      view->setOrientation(
        static_cast<QBirchSliceView::Orientation>(v.toInt()));
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QColor QBirchImageControl::backgroundColor() const
{
  Q_D(const QBirchImageControl);
  return d->color(0);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QColor QBirchImageControl::foregroundColor() const
{
  Q_D(const QBirchImageControl);
  return d->color(1);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QColor QBirchImageControl::annotationColor() const
{
  Q_D(const QBirchImageControl);
  return d->color(2);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchImageControl::setForegroundColor(const QColor& color)
{
  Q_D(QBirchImageControl);
  d->setColor(0, color);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchImageControl::setBackgroundColor(const QColor& color)
{
  Q_D(QBirchImageControl);
  d->setColor(1, color);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchImageControl::setAnnotationColor(const QColor& color)
{
  Q_D(QBirchImageControl);
  d->setColor(2, color);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchImageControl::setFlipVerticalIcon(const QIcon& ico)
{
  Q_D(QBirchImageControl);
  d->flipVerticalButton->setIcon(ico);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchImageControl::setFlipHorizontalIcon(const QIcon& ico)
{
  Q_D(QBirchImageControl);
  d->flipHorizontalButton->setIcon(ico);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchImageControl::setRotateClockwiseIcon(const QIcon& ico)
{
  Q_D(QBirchImageControl);
  d->rotateClockwiseButton->setIcon(ico);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchImageControl::setRotateCounterClockwiseIcon(const QIcon& ico)
{
  Q_D(QBirchImageControl);
  d->rotateCounterClockwiseButton->setIcon(ico);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchImageControl::setInterpolationIcon(const QIcon& ico)
{
  Q_D(QBirchImageControl);
  d->interpolationButton->setIcon(ico);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchImageControl::setViewXIcon(const QIcon& ico)
{
  Q_D(QBirchImageControl);
  d->viewXButton->setIcon(ico);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchImageControl::setViewZIcon(const QIcon& ico)
{
  Q_D(QBirchImageControl);
  d->viewZButton->setIcon(ico);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchImageControl::setViewYIcon(const QIcon& ico)
{
  Q_D(QBirchImageControl);
  d->viewYButton->setIcon(ico);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchImageControl::setInvertWindowLevelIcon(const QIcon& ico)
{
  Q_D(QBirchImageControl);
  d->invertWindowLevelButton->setIcon(ico);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QBirchImageControl::flipVerticalIcon() const
{
  Q_D(const QBirchImageControl);
  return d->flipVerticalButton->icon();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QBirchImageControl::flipHorizontalIcon() const
{
  Q_D(const QBirchImageControl);
  return d->flipHorizontalButton->icon();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QBirchImageControl::rotateClockwiseIcon() const
{
  Q_D(const QBirchImageControl);
  return d->rotateClockwiseButton->icon();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QBirchImageControl::rotateCounterClockwiseIcon() const
{
  Q_D(const QBirchImageControl);
  return d->rotateCounterClockwiseButton->icon();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QBirchImageControl::interpolationIcon() const
{
  Q_D(const QBirchImageControl);
  return d->interpolationButton->icon();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QBirchImageControl::viewXIcon() const
{
  Q_D(const QBirchImageControl);
  return d->viewXButton->icon();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QBirchImageControl::viewYIcon() const
{
  Q_D(const QBirchImageControl);
  return d->viewYButton->icon();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QBirchImageControl::viewZIcon() const
{
  Q_D(const QBirchImageControl);
  return d->viewZButton->icon();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QBirchImageControl::invertWindowLevelIcon() const
{
  Q_D(const QBirchImageControl);
  return d->invertWindowLevelButton->icon();
}
