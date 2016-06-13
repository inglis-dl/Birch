/*=========================================================================

  Program:   Birch
  Module:    QBirchDoubleSlider.cxx
  Language:  C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

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
#include <QBirchDoubleSlider.h>

// QT includes
#include <QDebug>
#include <QHBoxLayout>
#include <QHelpEvent>
#include <QStyle>
#include <QStyleOptionSlider>
#include <QToolTip>

// STD includes
#include <limits>

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
class QBirchSlider: public QSlider
{
  public:
    explicit QBirchSlider(QWidget* parent);
    using QSlider::initStyleOption;
};

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchSlider::QBirchSlider(QWidget* parent): QSlider(parent)
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
class QBirchDoubleSliderPrivate
{
  Q_DECLARE_PUBLIC(QBirchDoubleSlider);
  protected:
    QBirchDoubleSlider* const q_ptr;
  public:
    explicit QBirchDoubleSliderPrivate(QBirchDoubleSlider& object);
    int toInt(double value) const;
    double fromInt(int value) const;
    double safeFromInt(int value) const;
    void init();
    void updateOffset(double value);

    QBirchSlider* Slider;
    QString HandleToolTip;
    double Minimum;
    double Maximum;
    bool SettingRange;
    // we should have a Offset and SliderPositionOffset (and MinimumOffset?)
    double Offset;
    double SingleStep;
    double PageStep;
    double Value;
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
  this->Slider = new QBirchSlider(q);
  this->Slider->installEventFilter(q);
  QHBoxLayout* l = new QHBoxLayout(q);
  l->addWidget(this->Slider);
  l->setContentsMargins(0, 0, 0, 0);

  this->Minimum = this->Slider->minimum();
  this->Maximum = this->Slider->maximum();
  // this->Slider->singleStep is always 1
  this->SingleStep = this->Slider->singleStep();
  this->PageStep = this->Slider->pageStep();
  this->Value = this->Slider->value();

  q->connect(this->Slider, SIGNAL(valueChanged(int)),
    q, SLOT(onValueChanged(int)));
  q->connect(this->Slider, SIGNAL(sliderMoved(int)),
    q, SLOT(onSliderMoved(int)));
  q->connect(this->Slider, SIGNAL(sliderPressed()),
    q, SIGNAL(sliderPressed()));
  q->connect(this->Slider, SIGNAL(sliderReleased()),
    q, SIGNAL(sliderReleased()));
  q->connect(this->Slider, SIGNAL(rangeChanged(int, int)),
             q, SLOT(onRangeChanged(int, int)));

  q->setSizePolicy(this->Slider->sizePolicy());
  q->setAttribute(Qt::WA_WState_OwnSizePolicy, false);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int QBirchDoubleSliderPrivate::toInt(double doubleValue) const
{
  double tmp = doubleValue / this->SingleStep;
  static const double minInt = std::numeric_limits<int>::min();
  static const double maxInt = std::numeric_limits<int>::max();
#ifndef QT_NO_DEBUG
  if (tmp < minInt || tmp > maxInt)
    {
    qWarning() << __FUNCTION__ << ": value " << doubleValue
              << " for singleStep " << this->SingleStep
              << " is out of integer bounds !";
    }
#endif
  tmp = qBound(minInt, tmp, maxInt);
  int intValue = qRound(tmp);
  // qDebug() << __FUNCTION__ << doubleValue << tmp << intValue;
  return intValue;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchDoubleSliderPrivate::fromInt(int intValue) const
{
  double doubleValue = this->SingleStep * (this->Offset + intValue);
  // qDebug() << __FUNCTION__ << intValue << doubleValue;
  return doubleValue;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchDoubleSliderPrivate::safeFromInt(int intValue) const
{
  return qBound(this->Minimum, this->fromInt(intValue), this->Maximum);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSliderPrivate::updateOffset(double value)
{
  this->Offset = (value / this->SingleStep) - this->toInt(value);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchDoubleSlider::QBirchDoubleSlider(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new QBirchDoubleSliderPrivate(*this))
{
  Q_D(QBirchDoubleSlider);
  d->init();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchDoubleSlider::QBirchDoubleSlider(
  Qt::Orientation _orientation, QWidget* _parent)
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
  Q_D(QBirchDoubleSlider);
  this->setRange(min, qMax(min, d->Maximum));
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSlider::setMaximum(double max)
{
  Q_D(QBirchDoubleSlider);
  this->setRange(qMin(d->Minimum, max), max);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSlider::setRange(double min, double max)
{
  Q_D(QBirchDoubleSlider);
  d->Minimum = min;
  d->Maximum = max;

  if (d->Minimum >= d->Value)
    {
    d->updateOffset(d->Minimum);
    }
  if (d->Maximum <= d->Value)
    {
    d->updateOffset(d->Maximum);
    }
  d->SettingRange = true;
  d->Slider->setRange(d->toInt(min), d->toInt(max));
  d->SettingRange = false;
  emit this->rangeChanged(d->Minimum, d->Maximum);
  /// In case QSlider::setRange(...) didn't notify the value
  /// has changed.
  this->setValue(d->Value);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchDoubleSlider::minimum() const
{
  Q_D(const QBirchDoubleSlider);
  return d->Minimum;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchDoubleSlider::maximum() const
{
  Q_D(const QBirchDoubleSlider);
  return d->Maximum;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchDoubleSlider::sliderPosition() const
{
  Q_D(const QBirchDoubleSlider);
  return d->safeFromInt(d->Slider->sliderPosition());
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSlider::setSliderPosition(double newSliderPosition)
{
  Q_D(QBirchDoubleSlider);
  d->Slider->setSliderPosition(d->toInt(newSliderPosition));
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchDoubleSlider::value() const
{
  Q_D(const QBirchDoubleSlider);
  return d->Value;
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
      emit this->valueChanged(newValue);
      }
    }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchDoubleSlider::singleStep() const
{
  Q_D(const QBirchDoubleSlider);
  return d->SingleStep;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSlider::setSingleStep(double newStep)
{
  Q_D(QBirchDoubleSlider);
  d->SingleStep = newStep;
  d->updateOffset(d->Value);
  // update the new values of the QSlider
  bool oldBlockSignals = d->Slider->blockSignals(true);
  d->Slider->setRange(d->toInt(d->Minimum), d->toInt(d->Maximum));
  d->Slider->setValue(d->toInt(d->Value));
  d->Slider->setPageStep(d->toInt(d->PageStep));
  d->Slider->blockSignals(oldBlockSignals);
  Q_ASSERT(qFuzzyCompare(d->Value, d->safeFromInt(d->Slider->value())));
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchDoubleSlider::pageStep() const
{
  Q_D(const QBirchDoubleSlider);
  return d->PageStep;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSlider::setPageStep(double newStep)
{
  Q_D(QBirchDoubleSlider);
  d->PageStep = newStep;
  d->Slider->setPageStep(d->toInt(d->PageStep));
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchDoubleSlider::tickInterval() const
{
  Q_D(const QBirchDoubleSlider);
  // No need to apply Offset
  return d->SingleStep * d->Slider->tickInterval();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSlider::setTickInterval(double newTickInterval)
{
  Q_D(QBirchDoubleSlider);
  d->Slider->setTickInterval(d->toInt(newTickInterval));
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchDoubleSlider::hasTracking() const
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
void QBirchDoubleSlider::triggerAction(QAbstractSlider::SliderAction action)
{
  Q_D(QBirchDoubleSlider);
  d->Slider->triggerAction(action);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
Qt::Orientation QBirchDoubleSlider::orientation() const
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
QString QBirchDoubleSlider::handleToolTip() const
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
/*
  qDebug() << "onValueChanged: " << newValue << "->"<< d->fromInt(newValue+d->Offset)
           << " old: " << d->Value << "->" << d->toInt(d->Value)
           << "offset:" << d->Offset << doubleNewValue;
*/
  if (d->Value == doubleNewValue)
    {
    return;
    }
  d->Value = doubleNewValue;
  emit this->valueChanged(d->Value);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSlider::onSliderMoved(int newPosition)
{
  Q_D(const QBirchDoubleSlider);
  emit this->sliderMoved(d->safeFromInt(newPosition));
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSlider::onRangeChanged(int min, int max)
{
  Q_D(const QBirchDoubleSlider);
  if (!d->SettingRange)
    {
    this->setRange(d->fromInt(min), d->fromInt(max));
    }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchDoubleSlider::eventFilter(QObject* watched, QEvent* event)
{
  Q_D(QBirchDoubleSlider);
  if (watched == d->Slider)
    {
    switch (event->type())
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
          QToolTip::showText(helpEvent->globalPos(),
            d->HandleToolTip.arg(this->value()));
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
