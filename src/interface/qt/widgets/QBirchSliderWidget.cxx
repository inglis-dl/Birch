/*=========================================================================

  Library:   CTK

  Copyright (c) Kitware Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=========================================================================*/

// Qt includes
#include <QDebug>
#include <QMouseEvent>
#include <QPointer>

// Birch includes
#include "QBirchSliderWidget.h"
#include "ui_QBirchSliderWidget.h"

// STD includes
#include <cmath>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+----
class QBirchSliderWidgetPrivate: public Ui_QBirchSliderWidget
{
  Q_DECLARE_PUBLIC(QBirchSliderWidget);
protected:
  QBirchSliderWidget* const q_ptr;

public:
  QBirchSliderWidgetPrivate(QBirchSliderWidget& object);
  virtual ~QBirchSliderWidgetPrivate();

  void updateSpinBoxWidth();
  void updateSpinBoxDecimals();

  int synchronizedSpinBoxWidth() const;

  void synchronizeSiblingWidth(int width);
  void synchronizeSiblingDecimals(int decimals);
  bool equal(double spinBoxValue, double sliderValue)const
  {
    return qAbs(sliderValue - spinBoxValue) < std::pow(10., -this->SpinBox->decimals());
  }

  bool   Tracking;
  bool   Changing;
  double ValueBeforeChange;
  bool   BlockSetSliderValue;
  QBirchSliderWidget::SynchronizeSiblings SynchronizeMode;
};

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchSliderWidgetPrivate::QBirchSliderWidgetPrivate(QBirchSliderWidget& object)
  :q_ptr(&object)
{
  qRegisterMetaType<QBirchSliderWidget::SynchronizeSiblings>(
    "QBirchSliderWidget::SynchronizeSiblings");
  this->Tracking = true;
  this->Changing = false;
  this->ValueBeforeChange = 0.;
  this->BlockSetSliderValue = false;
  this->SynchronizeMode = QBirchSliderWidget::SynchronizeWidth;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchSliderWidgetPrivate::~QBirchSliderWidgetPrivate()
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidgetPrivate::updateSpinBoxWidth()
{
  int spinBoxWidth = this->synchronizedSpinBoxWidth();
  if (this->SynchronizeMode.testFlag(QBirchSliderWidget::SynchronizeWidth))
    {
    this->SpinBox->setMinimumWidth(spinBoxWidth);
    }
  else
    {
    this->SpinBox->setMinimumWidth(0);
    }

  this->synchronizeSiblingWidth(spinBoxWidth);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidgetPrivate::updateSpinBoxDecimals()
{
  if (this->SynchronizeMode.testFlag(QBirchSliderWidget::SynchronizeDecimals))
    {
    this->synchronizeSiblingDecimals(this->SpinBox->decimals());
    }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int QBirchSliderWidgetPrivate::synchronizedSpinBoxWidth()const
{
  Q_Q(const QBirchSliderWidget);
  int maxWidth = this->SpinBox->sizeHint().width();
  if (!q->parent())
    {
    return maxWidth;
    }
  QList<QBirchSliderWidget*> siblings =
    q->parent()->findChildren<QBirchSliderWidget*>();
  foreach(QBirchSliderWidget* sibling, siblings)
    {
    maxWidth = qMax(maxWidth, sibling->d_func()->SpinBox->sizeHint().width());
    }
  return maxWidth;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidgetPrivate::synchronizeSiblingWidth(int width)
{
  Q_UNUSED(width);
  Q_Q(const QBirchSliderWidget);
  QList<QBirchSliderWidget*> siblings =
    q->parent()->findChildren<QBirchSliderWidget*>();
  foreach(QBirchSliderWidget* sibling, siblings)
    {
    if (sibling != q
      && sibling->synchronizeSiblings().testFlag(QBirchSliderWidget::SynchronizeWidth))
      {
      sibling->d_func()->SpinBox->setMinimumWidth(
        this->SpinBox->minimumWidth());
      }
    }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidgetPrivate::synchronizeSiblingDecimals(int decimals)
{
  Q_UNUSED(decimals);
  Q_Q(const QBirchSliderWidget);
  QList<QBirchSliderWidget*> siblings =
    q->parent()->findChildren<QBirchSliderWidget*>();
  foreach(QBirchSliderWidget* sibling, siblings)
    {
    if (sibling != q
      && sibling->synchronizeSiblings().testFlag(QBirchSliderWidget::SynchronizeDecimals))
      {
      sibling->d_func()->SpinBox->setDecimals(this->SpinBox->decimals());
      }
    }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchSliderWidget::QBirchSliderWidget(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new QBirchSliderWidgetPrivate(*this))
{
  Q_D(QBirchSliderWidget);

  d->setupUi(this);

  d->Slider->setMaximum(d->SpinBox->maximum());
  d->Slider->setMinimum(d->SpinBox->minimum());

  this->connect(d->SpinBox, SIGNAL(valueChanged(double)), this, SLOT(setSliderValue(double)));
  this->connect(d->SpinBox, SIGNAL(decimalsChanged(int)), this, SLOT(setDecimals(int)));

  this->connect(d->Slider, SIGNAL(sliderPressed()), this, SLOT(startChanging()));
  this->connect(d->Slider, SIGNAL(sliderReleased()), this, SLOT(stopChanging()));
  // setSpinBoxValue will fire the valueChanged signal.
  this->connect(d->Slider, SIGNAL(valueChanged(double)), this, SLOT(setSpinBoxValue(double)));
  d->SpinBox->installEventFilter(this);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchSliderWidget::~QBirchSliderWidget()
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchSliderWidget::minimum()const
{
  Q_D(const QBirchSliderWidget);
  Q_ASSERT(d->equal(d->SpinBox->minimum(),d->Slider->minimum()));
  return d->Slider->minimum();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchSliderWidget::maximum()const
{
  Q_D(const QBirchSliderWidget);
  Q_ASSERT(d->equal(d->SpinBox->maximum(),d->Slider->maximum()));
  return d->Slider->maximum();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget::setMinimum(double min)
{
  Q_D(QBirchSliderWidget);
  bool wasBlockSetSliderValue = d->BlockSetSliderValue;
  d->BlockSetSliderValue = true;
  d->SpinBox->setMinimum(min);
  d->BlockSetSliderValue = wasBlockSetSliderValue;

  // SpinBox can truncate min (depending on decimals).
  // use Spinbox's min to set Slider's min
  d->Slider->setMinimum(d->SpinBox->minimum());
  Q_ASSERT(d->equal(d->SpinBox->minimum(),d->Slider->minimum()));
  Q_ASSERT(d->equal(d->SpinBox->value(),d->Slider->value()));
  Q_ASSERT(d->equal(d->SpinBox->maximum(),d->Slider->maximum()));
  d->updateSpinBoxWidth();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget::setMaximum(double max)
{
  Q_D(QBirchSliderWidget);
  bool wasBlockSetSliderValue = d->BlockSetSliderValue;
  d->BlockSetSliderValue = true;
  d->SpinBox->setMaximum(max);
  d->BlockSetSliderValue = wasBlockSetSliderValue;

  // SpinBox can truncate max (depending on decimals).
  // use Spinbox's max to set Slider's max
  d->Slider->setMaximum(d->SpinBox->maximum());
  Q_ASSERT(d->equal(d->SpinBox->minimum(),d->Slider->minimum()));
  Q_ASSERT(d->equal(d->SpinBox->value(),d->Slider->value()));
  Q_ASSERT(d->equal(d->SpinBox->maximum(),d->Slider->maximum()));
  d->updateSpinBoxWidth();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget::setRange(double min, double max)
{
  Q_D(QBirchSliderWidget);

  bool wasBlockSetSliderValue = d->BlockSetSliderValue;
  d->BlockSetSliderValue = true;
  d->SpinBox->setRange(min, max);
  d->BlockSetSliderValue = wasBlockSetSliderValue;

  // SpinBox can truncate the range (depending on decimals).
  // use Spinbox's range to set Slider's range
  d->Slider->setRange(d->SpinBox->minimum(), d->SpinBox->maximum());
  Q_ASSERT(d->equal(d->SpinBox->minimum(),d->Slider->minimum()));
  Q_ASSERT(d->equal(d->SpinBox->value(),d->Slider->value()));
  Q_ASSERT(d->equal(d->SpinBox->maximum(),d->Slider->maximum()));
  d->updateSpinBoxWidth();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchSliderWidget::value()const
{
  Q_D(const QBirchSliderWidget);
  Q_ASSERT(d->equal(d->SpinBox->value(), d->Slider->value()));
  // The slider is the most precise as it does not round the value with the
  // decimals number.
  return d->Changing ? d->ValueBeforeChange : d->Slider->value();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget::setValue(double _value)
{
  Q_D(QBirchSliderWidget);
  // disable the tracking temporally to emit the
  // signal valueChanged if setSpinBoxValue() is called
  bool isChanging = d->Changing;
  d->Changing = false;
  d->SpinBox->setValue(_value);
  Q_ASSERT(d->equal(d->SpinBox->minimum(),d->Slider->minimum()));
  Q_ASSERT(d->equal(d->SpinBox->value(),d->Slider->value()));
  Q_ASSERT(d->equal(d->SpinBox->maximum(),d->Slider->maximum()));
  // restore the prop
  d->Changing = isChanging;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget::startChanging()
{
  Q_D(QBirchSliderWidget);
  if (d->Tracking)
    {
    return;
    }
  d->ValueBeforeChange = this->value();
  d->Changing = true;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget::stopChanging()
{
  Q_D(QBirchSliderWidget);
  if (d->Tracking)
    {
    return;
    }
  d->Changing = false;
  if (qAbs(this->value() - d->ValueBeforeChange) > (this->singleStep() * 0.000000001))
    {
    emit this->valueChanged(this->value());
    }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget::setSliderValue(double spinBoxValue)
{
  Q_D(QBirchSliderWidget);
  if (d->BlockSetSliderValue)
    {
    return;
    }
  d->Slider->setValue(spinBoxValue);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget::setSpinBoxValue(double sliderValue)
{
  Q_D(QBirchSliderWidget);

  bool wasBlockSetSliderValue = d->BlockSetSliderValue;
  d->BlockSetSliderValue = true;
  d->SpinBox->setValue(sliderValue);
  d->BlockSetSliderValue = wasBlockSetSliderValue;
  Q_ASSERT(d->equal(d->SpinBox->value(), d->Slider->value()));

  if (!d->Tracking)
    {
    emit this->valueIsChanging(sliderValue);
    }
  if (!d->Changing)
    {
    emit this->valueChanged(sliderValue);
    }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchSliderWidget::eventFilter(QObject *obj, QEvent *event)
 {
   if (event->type() == QEvent::MouseButtonPress)
     {
     QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
     if (mouseEvent->button() & Qt::LeftButton)
       {
       this->startChanging();
       }
     }
   else if (event->type() == QEvent::MouseButtonRelease)
     {
     QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
     if (mouseEvent->button() & Qt::LeftButton)
       {
       // here we might prevent QBirchSliderWidget::stopChanging
       // from sending a valueChanged() event as the spinbox might
       // send a valueChanged() after eventFilter() is done.
       this->stopChanging();
       }
     }
   // standard event processing
   return this->Superclass::eventFilter(obj, event);
 }

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchSliderWidget::singleStep()const
{
  Q_D(const QBirchSliderWidget);
  Q_ASSERT(d->equal(d->SpinBox->singleStep(), d->Slider->singleStep()));
  return d->Slider->singleStep();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget::setSingleStep(double newStep)
{
  Q_D(QBirchSliderWidget);
  if (!d->Slider->isValidStep(newStep))
    {
    qWarning() << "QBirchSliderWidget::setSingleStep() " << newStep << "is out of bounds." <<
      this->minimum() << this->maximum() <<this->value();
    return;
    }
  d->SpinBox->setSingleStep(newStep);
  d->Slider->setSingleStep(d->SpinBox->singleStep());
  Q_ASSERT(d->equal(d->SpinBox->minimum(),d->Slider->minimum()));
  Q_ASSERT(d->equal(d->SpinBox->value(),d->Slider->value()));
  Q_ASSERT(d->equal(d->SpinBox->maximum(),d->Slider->maximum()));
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchSliderWidget::pageStep()const
{
  Q_D(const QBirchSliderWidget);
  return d->Slider->pageStep();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget::setPageStep(double step)
{
  Q_D(QBirchSliderWidget);
  d->Slider->setPageStep(step);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int QBirchSliderWidget::decimals()const
{
  Q_D(const QBirchSliderWidget);
  return d->SpinBox->decimals();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget::setDecimals(int newDecimals)
{
  Q_D(QBirchSliderWidget);
  d->SpinBox->setDecimals(newDecimals);
  // The number of decimals can change the range values
  // i.e. 50.55 with 2 decimals -> 51 with 0 decimals
  // As the SpinBox range change doesn't fire signals,
  // we have to do the synchronization manually here
  d->Slider->setRange(d->SpinBox->minimum(), d->SpinBox->maximum());
  Q_ASSERT(d->equal(d->SpinBox->minimum(),d->Slider->minimum()));
  Q_ASSERT(d->equal(d->SpinBox->maximum(),d->Slider->maximum()));
  // Last time the value was set on the spinbox, the value might have been
  // rounded by the previous number of decimals. The slider however never rounds
  // the value. Now, if the number of decimals is higher, such rounding is lost
  // precision. The "true" value must be set again to the spinbox to "recover"
  // the precision.
  this->setSpinBoxValue(d->Slider->value());
  Q_ASSERT(d->equal(d->SpinBox->value(),d->Slider->value()));
  d->updateSpinBoxDecimals();
  emit decimalsChanged(d->SpinBox->decimals());
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QString QBirchSliderWidget::prefix()const
{
  Q_D(const QBirchSliderWidget);
  return d->SpinBox->prefix();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget::setPrefix(const QString& newPrefix)
{
  Q_D(QBirchSliderWidget);
  d->SpinBox->setPrefix(newPrefix);
  d->updateSpinBoxWidth();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QString QBirchSliderWidget::suffix()const
{
  Q_D(const QBirchSliderWidget);
  return d->SpinBox->suffix();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget::setSuffix(const QString& newSuffix)
{
  Q_D(QBirchSliderWidget);
  d->SpinBox->setSuffix(newSuffix);
  d->updateSpinBoxWidth();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchSliderWidget::tickInterval()const
{
  Q_D(const QBirchSliderWidget);
  return d->Slider->tickInterval();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget::setTickInterval(double ti)
{
  Q_D(QBirchSliderWidget);
  d->Slider->setTickInterval(ti);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QSlider::TickPosition QBirchSliderWidget::tickPosition()const
{
  Q_D(const QBirchSliderWidget);
  return d->Slider->tickPosition();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget::setTickPosition(QSlider::TickPosition newTickPosition)
{
  Q_D(QBirchSliderWidget);
  d->Slider->setTickPosition(newTickPosition);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget::reset()
{
  this->setValue(0.);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget::setSpinBoxAlignment(Qt::Alignment alignment)
{
  Q_D(QBirchSliderWidget);
  return d->SpinBox->setAlignment(alignment);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
Qt::Alignment QBirchSliderWidget::spinBoxAlignment()const
{
  Q_D(const QBirchSliderWidget);
  return d->SpinBox->alignment();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget::setTracking(bool enable)
{
  Q_D(QBirchSliderWidget);
  d->Tracking = enable;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchSliderWidget::hasTracking()const
{
  Q_D(const QBirchSliderWidget);
  return d->Tracking;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchSliderWidget::invertedAppearance()const
{
  Q_D(const QBirchSliderWidget);
  return d->Slider->invertedAppearance();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget::setInvertedAppearance(bool invertedAppearance)
{
  Q_D(QBirchSliderWidget);
  d->Slider->setInvertedAppearance(invertedAppearance);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchSliderWidget::invertedControls()const
{
  Q_D(const QBirchSliderWidget);
  return d->Slider->invertedControls() && d->SpinBox->invertedControls();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget::setInvertedControls(bool invertedControls)
{
  Q_D(QBirchSliderWidget);
  d->Slider->setInvertedControls(invertedControls);
  d->SpinBox->setInvertedControls(invertedControls);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchSliderWidget::SynchronizeSiblings
QBirchSliderWidget::synchronizeSiblings() const
{
  Q_D(const QBirchSliderWidget);
  return d->SynchronizeMode;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget
::setSynchronizeSiblings(QBirchSliderWidget::SynchronizeSiblings flag)
{
  Q_D(QBirchSliderWidget);
  d->SynchronizeMode = flag;
  d->updateSpinBoxWidth();
  d->updateSpinBoxDecimals();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchSliderWidget::isSpinBoxVisible()const
{
  Q_D(const QBirchSliderWidget);
  return d->SpinBox->isVisibleTo(const_cast<QBirchSliderWidget*>(this));
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget::setSpinBoxVisible(bool visible)
{
  Q_D(QBirchSliderWidget);
  d->SpinBox->setVisible(visible);
}
// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchDoubleSpinBox* QBirchSliderWidget::spinBox()
{
  Q_D(QBirchSliderWidget);
  return d->SpinBox;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchDoubleSlider* QBirchSliderWidget::slider()
{
  Q_D(QBirchSliderWidget);
  return d->Slider;
}
