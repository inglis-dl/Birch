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

// QT includes
#include <QDebug>
#include <QHBoxLayout>
#include <QHelpEvent>
#include <QPointer>
#include <QStyle>
#include <QStyleOptionSlider>
#include <QToolTip>

// Birch includes
#include "QBirchDoubleSlider.h"

// STD includes
#include <limits>

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
// birchSlider

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
class birchSlider: public QSlider
{
public:
  birchSlider(QWidget* parent);
  using QSlider::initStyleOption;
};

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
birchSlider::birchSlider(QWidget* parent): QSlider(parent)
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
// QBirchDoubleSliderPrivate

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
class QBirchDoubleSliderPrivate
{
  Q_DECLARE_PUBLIC(QBirchDoubleSlider);
protected:
  QBirchDoubleSlider* const q_ptr;
public:
  QBirchDoubleSliderPrivate(QBirchDoubleSlider& object);
  int toInt(double value)const;
  double fromInt(int value)const;
  double safeFromInt(int value)const;
  void init();
  void updateOffset(double value);

  birchSlider*    Slider;
  QString       HandleToolTip;
  double      Minimum;
  double      Maximum;
  bool        SettingRange;
  // we should have a Offset and SliderPositionOffset (and MinimumOffset?)
  double      Offset;
  double      SingleStep;
  double      PageStep;
  double      Value;
};

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchDoubleSliderPrivate::QBirchDoubleSliderPrivate(QBirchDoubleSlider& object)
  :q_ptr(&object)
{
  this->Slider = 0;
  this->Minimum = 0.;
  this->Maximum = 100.;
  this->SettingRange = false;
  this->Offset = 0.;
  this->SingleStep = 1.;
  this->PageStep = 10.;
  this->Value = 0.;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSliderPrivate::init()
{
  Q_Q(QBirchDoubleSlider);
  this->Slider = new birchSlider(q);
  this->Slider->installEventFilter(q);
  QHBoxLayout* l = new QHBoxLayout(q);
  l->addWidget(this->Slider);
  l->setContentsMargins(0,0,0,0);

  this->Minimum = this->Slider->minimum();
  this->Maximum = this->Slider->maximum();
  // this->Slider->singleStep is always 1
  this->SingleStep = this->Slider->singleStep();
  this->PageStep = this->Slider->pageStep();
  this->Value = this->Slider->value();

  q->connect(this->Slider, SIGNAL(valueChanged(int)), q, SLOT(onValueChanged(int)));
  q->connect(this->Slider, SIGNAL(sliderMoved(int)), q, SLOT(onSliderMoved(int)));
  q->connect(this->Slider, SIGNAL(sliderPressed()), q, SIGNAL(sliderPressed()));
  q->connect(this->Slider, SIGNAL(sliderReleased()), q, SIGNAL(sliderReleased()));
  q->connect(this->Slider, SIGNAL(rangeChanged(int,int)),
             q, SLOT(onRangeChanged(int,int)));

  q->setSizePolicy(this->Slider->sizePolicy());
  q->setAttribute(Qt::WA_WState_OwnSizePolicy, false);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int QBirchDoubleSliderPrivate::toInt(double doubleValue)const
{
  double tmp = doubleValue / this->SingleStep;
  static const double minInt = std::numeric_limits<int>::min();
  static const double maxInt = std::numeric_limits<int>::max();
#ifndef QT_NO_DEBUG
  static const double maxDouble = std::numeric_limits<double>::max();
  if ( (tmp < minInt || tmp > maxInt) &&
       // If the value is the min or max double, there is no need
       // to warn. It is expected that the number is outside of bounds.
       (doubleValue != -maxDouble && doubleValue != maxDouble) )
    {
    qWarning() << __FUNCTION__ << ": value " << doubleValue
               << " for singleStep " << this->SingleStep
               << " is out of integer bounds !";
    }
#endif
  tmp = qBound(minInt, tmp, maxInt);
  int intValue = qRound(tmp);
  //qDebug() << __FUNCTION__ << doubleValue << tmp << intValue;
  return intValue;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchDoubleSliderPrivate::fromInt(int intValue)const
{
  double doubleValue = this->SingleStep * (this->Offset + intValue) ;
  return doubleValue;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchDoubleSliderPrivate::safeFromInt(int intValue)const
{
  return qBound(this->Minimum, this->fromInt(intValue), this->Maximum);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSliderPrivate::updateOffset(double value)
{
  this->Offset = (value / this->SingleStep) - this->toInt(value);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
// QBirchDoubleSlider

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchDoubleSlider::QBirchDoubleSlider(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new QBirchDoubleSliderPrivate(*this))
{
  Q_D(QBirchDoubleSlider);
  d->init();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchDoubleSlider::QBirchDoubleSlider(Qt::Orientation _orientation, QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new QBirchDoubleSliderPrivate(*this))
{
  Q_D(QBirchDoubleSlider);
  d->init();
  this->setOrientation(_orientation);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchDoubleSlider::~QBirchDoubleSlider()
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSlider::setMinimum(double min)
{
  this->setRange(min, qMax(min, this->maximum()));
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSlider::setMaximum(double max)
{
  this->setRange(qMin(this->minimum(), max), max);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSlider::setRange(double newMin, double newMax)
{
  Q_D(QBirchDoubleSlider);

  if (newMin > newMax)
    {
    qSwap(newMin, newMax);
    }

  double oldMin = d->Minimum;
  double oldMax = d->Maximum;

  d->Minimum = newMin;
  d->Maximum = newMax;

  if (d->Minimum >= d->Value)
    {
    d->updateOffset(d->Minimum);
    }
  if (d->Maximum <= d->Value)
    {
    d->updateOffset(d->Maximum);
    }
  bool wasSettingRange = d->SettingRange;
  d->SettingRange = true;
  d->Slider->setRange(d->toInt(newMin), d->toInt(newMax));
  d->SettingRange = wasSettingRange;
  if (!wasSettingRange && (d->Minimum != oldMin || d->Maximum != oldMax))
    {
    emit this->rangeChanged(this->minimum(), this->maximum());
    }
  /// In case QSlider::setRange(...) didn't notify the value
  /// has changed.
  this->setValue(this->value());
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchDoubleSlider::minimum()const
{
  Q_D(const QBirchDoubleSlider);
  double min = d->Minimum;
  double max = d->Maximum;
  return qMin(min, max);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchDoubleSlider::maximum()const
{
  Q_D(const QBirchDoubleSlider);
  double min = d->Minimum;
  double max = d->Maximum;
  return qMax(min, max);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchDoubleSlider::sliderPosition()const
{
  Q_D(const QBirchDoubleSlider);
  int intPosition = d->Slider->sliderPosition();
  double position = d->safeFromInt(intPosition);
  return position;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSlider::setSliderPosition(double newPosition)
{
  Q_D(QBirchDoubleSlider);
  int newIntPosition = d->toInt(newPosition);
  d->Slider->setSliderPosition(newIntPosition);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchDoubleSlider::value()const
{
  Q_D(const QBirchDoubleSlider);
  double val = d->Value;
  return val;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSlider::setValue(double newValue)
{
  Q_D(QBirchDoubleSlider);

  newValue = qBound(d->Minimum, newValue, d->Maximum);
  d->updateOffset(newValue);
  int newIntValue = d->toInt(newValue);
  if (newIntValue != d->Slider->value())
    {
    // d->Slider will emit a valueChanged signal that is connected to
    // QBirchDoubleSlider::onValueChanged
    d->Slider->setValue(newIntValue);
    }
  else
    {
    double oldValue = d->Value;
    d->Value = newValue;
    // don't emit a valuechanged signal if the new value is quite
    // similar to the old value.
    if (qAbs(newValue - oldValue) > (d->SingleStep * 0.000000001))
      {
      emit this->valueChanged(this->value());
      }
    }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchDoubleSlider::singleStep()const
{
  Q_D(const QBirchDoubleSlider);
  double step = d->SingleStep;
  return step;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSlider::setSingleStep(double newStep)
{
  Q_D(QBirchDoubleSlider);
  if (!this->isValidStep(newStep))
    {
    qWarning() << "QBirchDoubleSlider::setSingleStep("<< newStep <<")"
               << "is outside of valid bounds.";
    return;
    }
  d->SingleStep = newStep;
  d->updateOffset(d->Value);
  // update the new values of the QSlider
  bool oldBlockSignals = d->Slider->blockSignals(true);
  d->Slider->setRange(d->toInt(d->Minimum), d->toInt(d->Maximum));
  d->Slider->setValue(d->toInt(d->Value));
  d->Slider->setPageStep(d->toInt(d->PageStep));
  d->Slider->blockSignals(oldBlockSignals);
  Q_ASSERT(qFuzzyCompare(d->Value,d->safeFromInt(d->Slider->value())));
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchDoubleSlider::isValidStep(double step)const
{
  Q_D(const QBirchDoubleSlider);
  if (d->Minimum == d->Maximum)
    {
    return true;
    }
  const double minStep = qMax(d->Maximum / std::numeric_limits<double>::max(),
                              std::numeric_limits<double>::epsilon());
  const double maxStep = qMin(d->Maximum - d->Minimum,
                              static_cast<double>(std::numeric_limits<int>::max()));
  return step >= minStep && step <= maxStep;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchDoubleSlider::pageStep()const
{
  Q_D(const QBirchDoubleSlider);
  return d->PageStep;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSlider::setPageStep(double newStep)
{
  Q_D(QBirchDoubleSlider);
  d->PageStep = newStep;
  int intPageStep = d->toInt(d->PageStep);
  d->Slider->setPageStep(intPageStep);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchDoubleSlider::tickInterval()const
{
  Q_D(const QBirchDoubleSlider);
  // No need to apply Offset
  double interval = d->SingleStep * d->Slider->tickInterval();
  return interval;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSlider::setTickInterval(double newInterval)
{
  Q_D(QBirchDoubleSlider);
  int newIntInterval = d->toInt(newInterval);
  d->Slider->setTickInterval(newIntInterval);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QSlider::TickPosition QBirchDoubleSlider::tickPosition()const
{
  Q_D(const QBirchDoubleSlider);
  return d->Slider->tickPosition();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSlider::setTickPosition(QSlider::TickPosition newTickPosition)
{
  Q_D(QBirchDoubleSlider);
  d->Slider->setTickPosition(newTickPosition);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchDoubleSlider::hasTracking()const
{
  Q_D(const QBirchDoubleSlider);
  return d->Slider->hasTracking();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSlider::setTracking(bool enable)
{
  Q_D(QBirchDoubleSlider);
  d->Slider->setTracking(enable);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchDoubleSlider::invertedAppearance()const
{
  Q_D(const QBirchDoubleSlider);
  return d->Slider->invertedAppearance();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSlider::setInvertedAppearance(bool invertedAppearance)
{
  Q_D(QBirchDoubleSlider);
  d->Slider->setInvertedAppearance(invertedAppearance);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchDoubleSlider::invertedControls()const
{
  Q_D(const QBirchDoubleSlider);
  return d->Slider->invertedControls();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSlider::setInvertedControls(bool invertedControls)
{
  Q_D(QBirchDoubleSlider);
  d->Slider->setInvertedControls(invertedControls);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSlider::triggerAction( QAbstractSlider::SliderAction action)
{
  Q_D(QBirchDoubleSlider);
  d->Slider->triggerAction(action);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
Qt::Orientation QBirchDoubleSlider::orientation()const
{
  Q_D(const QBirchDoubleSlider);
  return d->Slider->orientation();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSlider::setOrientation(Qt::Orientation newOrientation)
{
  Q_D(QBirchDoubleSlider);
  if (this->orientation() == newOrientation)
    {
    return;
    }
  if (!testAttribute(Qt::WA_WState_OwnSizePolicy))
    {
    QSizePolicy sp = this->sizePolicy();
    sp.transpose();
    this->setSizePolicy(sp);
    this->setAttribute(Qt::WA_WState_OwnSizePolicy, false);
    }
  // d->Slider will take care of calling updateGeometry
  d->Slider->setOrientation(newOrientation);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QString QBirchDoubleSlider::handleToolTip()const
{
  Q_D(const QBirchDoubleSlider);
  return d->HandleToolTip;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSlider::setHandleToolTip(const QString& toolTip)
{
  Q_D(QBirchDoubleSlider);
  d->HandleToolTip = toolTip;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSlider::onValueChanged(int newValue)
{
  Q_D(QBirchDoubleSlider);
  double doubleNewValue = d->safeFromInt(newValue);
  if (d->Value == doubleNewValue)
    {
    return;
    }
  d->Value = doubleNewValue;
  emit this->valueChanged(this->value());
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSlider::onSliderMoved(int newPosition)
{
  Q_D(const QBirchDoubleSlider);
  emit this->sliderMoved(d->safeFromInt(newPosition));
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSlider::onRangeChanged(int newIntMin, int newIntMax)
{
  Q_D(const QBirchDoubleSlider);
  if (d->SettingRange)
    {
    return;
    }
  double newMin = d->fromInt(newIntMin);
  double newMax = d->fromInt(newIntMax);
  this->setRange(newMin, newMax);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchDoubleSlider::eventFilter(QObject* watched, QEvent* event)
{
  Q_D(QBirchDoubleSlider);
  if (watched == d->Slider)
    {
    switch(event->type())
      {
      case QEvent::ToolTip:
        {
        QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
        QStyleOptionSlider opt;
        d->Slider->initStyleOption(&opt);
        QStyle::SubControl hoveredControl =
          d->Slider->style()->hitTestComplexControl(
            QStyle::CC_Slider, &opt, helpEvent->pos(), this);
        if (!d->HandleToolTip.isEmpty() &&
            hoveredControl == QStyle::SC_SliderHandle)
          {
          QToolTip::showText(helpEvent->globalPos(), d->HandleToolTip.arg(this->value()));
          event->accept();
          return true;
          }
        }
      default:
        break;
      }
    }
  return this->Superclass::eventFilter(watched, event);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QSlider* QBirchDoubleSlider::slider()const
{
  Q_D(const QBirchDoubleSlider);
  return d->Slider;
}
