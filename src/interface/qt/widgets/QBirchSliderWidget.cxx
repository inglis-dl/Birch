/*=========================================================================

  Program:   Birch
  Module:    QBirchSliderWidget.cxx
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
#include <QBirchSliderWidget.h>
#include <ui_QBirchSliderWidget.h>

// Qt includes
#include <QDebug>
#include <QMouseEvent>

// STD includes
#include <cmath>

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
class QBirchSliderWidgetPrivate: public Ui_QBirchSliderWidget
{
  Q_DECLARE_PUBLIC(QBirchSliderWidget);
  protected:
    QBirchSliderWidget* const q_ptr;

  public:
    explicit QBirchSliderWidgetPrivate(QBirchSliderWidget& object);
    virtual ~QBirchSliderWidgetPrivate();

    void updateSpinBoxWidth();
    int synchronizedSpinBoxWidth() const;
    void synchronizeSiblingSpinBox(int newWidth);
    bool equal(double spinBoxValue, double sliderValue) const
    {
      return qAbs(sliderValue - spinBoxValue) <
             std::pow(10., -this->SpinBox->decimals());
    }

    bool Tracking;
    bool Changing;
    double ValueBeforeChange;
    bool AutoSpinBoxWidth;
};

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchSliderWidgetPrivate::QBirchSliderWidgetPrivate(QBirchSliderWidget& object)
  :q_ptr(&object)
{
  this->Tracking = true;
  this->Changing = false;
  this->ValueBeforeChange = 0.;
  this->AutoSpinBoxWidth = true;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchSliderWidgetPrivate::~QBirchSliderWidgetPrivate()
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidgetPrivate::updateSpinBoxWidth()
{
  int spinBoxWidth = this->synchronizedSpinBoxWidth();
  if (this->AutoSpinBoxWidth)
    {
    this->SpinBox->setMinimumWidth(spinBoxWidth);
    }
  else
    {
    this->SpinBox->setMinimumWidth(0);
    }
  this->synchronizeSiblingSpinBox(spinBoxWidth);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int QBirchSliderWidgetPrivate::synchronizedSpinBoxWidth() const
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
void QBirchSliderWidgetPrivate::synchronizeSiblingSpinBox(int width)
{
  Q_Q(const QBirchSliderWidget);
  QList<QBirchSliderWidget*> siblings =
    q->parent()->findChildren<QBirchSliderWidget*>();
  foreach(QBirchSliderWidget* sibling, siblings)
    {
    if (sibling != q && sibling->isAutoSpinBoxWidth())
      {
      sibling->d_func()->SpinBox->setMinimumWidth(width);
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

  this->connect(d->SpinBox, SIGNAL(valueChanged(double)),
    d->Slider, SLOT(setValue(double)));
  this->connect(d->Slider, SIGNAL(sliderPressed()),
    this, SLOT(startChanging()));
  this->connect(d->Slider, SIGNAL(sliderReleased()),
    this, SLOT(stopChanging()));
  this->connect(d->Slider, SIGNAL(valueChanged(double)),
    this, SLOT(changeValue(double)));
  d->SpinBox->installEventFilter(this);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchSliderWidget::~QBirchSliderWidget()
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchSliderWidget::minimum() const
{
  Q_D(const QBirchSliderWidget);
  Q_ASSERT(d->equal(d->SpinBox->minimum(), d->Slider->minimum()));
  return d->Slider->minimum();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchSliderWidget::maximum() const
{
  Q_D(const QBirchSliderWidget);
  Q_ASSERT(d->equal(d->SpinBox->maximum(), d->Slider->maximum()));
  return d->Slider->maximum();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget::setMinimum(double min)
{
  Q_D(QBirchSliderWidget);
  bool wasBlocked = d->SpinBox->blockSignals(true);
  d->SpinBox->setMinimum(min);
  d->SpinBox->blockSignals(wasBlocked);

  // SpinBox can truncate min (depending on decimals).
  // use Spinbox's min to set Slider's min
  d->Slider->setMinimum(d->SpinBox->minimum());
  Q_ASSERT(d->equal(d->SpinBox->minimum(), d->Slider->minimum()));
  Q_ASSERT(d->equal(d->SpinBox->value(), d->Slider->value()));
  Q_ASSERT(d->equal(d->SpinBox->maximum(), d->Slider->maximum()));
  d->updateSpinBoxWidth();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget::setMaximum(double max)
{
  Q_D(QBirchSliderWidget);
  bool wasBlocked = d->SpinBox->blockSignals(true);
  d->SpinBox->setMaximum(max);
  d->SpinBox->blockSignals(wasBlocked);

  // SpinBox can truncate max (depending on decimals).
  // use Spinbox's max to set Slider's max
  d->Slider->setMaximum(d->SpinBox->maximum());
  Q_ASSERT(d->equal(d->SpinBox->minimum(), d->Slider->minimum()));
  Q_ASSERT(d->equal(d->SpinBox->value(), d->Slider->value()));
  Q_ASSERT(d->equal(d->SpinBox->maximum(), d->Slider->maximum()));
  d->updateSpinBoxWidth();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget::setRange(double min, double max)
{
  Q_D(QBirchSliderWidget);

  bool wasBlocked = d->SpinBox->blockSignals(true);
  d->SpinBox->setRange(min, max);
  d->SpinBox->blockSignals(wasBlocked);

  // SpinBox can truncate the range (depending on decimals).
  // use Spinbox's range to set Slider's range
  d->Slider->setRange(d->SpinBox->minimum(), d->SpinBox->maximum());
  Q_ASSERT(d->equal(d->SpinBox->minimum(), d->Slider->minimum()));
  Q_ASSERT(d->equal(d->SpinBox->value(), d->Slider->value()));
  Q_ASSERT(d->equal(d->SpinBox->maximum(), d->Slider->maximum()));
  d->updateSpinBoxWidth();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchSliderWidget::value() const
{
  Q_D(const QBirchSliderWidget);
  Q_ASSERT(d->equal(d->SpinBox->value(), d->Slider->value()));
  return d->Changing ? d->ValueBeforeChange : d->Slider->value();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget::setValue(double _value)
{
  Q_D(QBirchSliderWidget);
  // disable the tracking temporally to emit the
  // signal valueChanged if changeValue() is called
  bool isChanging = d->Changing;
  d->Changing = false;
  d->SpinBox->setValue(_value);
  Q_ASSERT(d->equal(d->SpinBox->minimum(), d->Slider->minimum()));
  Q_ASSERT(d->equal(d->SpinBox->value(), d->Slider->value()));
  Q_ASSERT(d->equal(d->SpinBox->maximum(), d->Slider->maximum()));
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
  d->Changing = true;
  d->ValueBeforeChange = this->value();
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
  if (qAbs(this->value() - d->ValueBeforeChange) >
      (this->singleStep() * 0.000000001))
    {
    emit this->valueChanged(this->value());
    }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget::changeValue(double newValue)
{
  Q_D(QBirchSliderWidget);

  bool wasBlocked = d->SpinBox->blockSignals(true);
  d->SpinBox->setValue(newValue);
  d->SpinBox->blockSignals(wasBlocked);
  Q_ASSERT(d->equal(d->SpinBox->value(), d->Slider->value()));

  if (!d->Tracking)
    {
    emit this->valueIsChanging(newValue);
    }
  if (!d->Changing)
    {
    emit this->valueChanged(newValue);
    }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchSliderWidget::eventFilter(QObject* obj, QEvent* event)
{
  if (event->type() == QEvent::MouseButtonPress)
    {
    QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
    if (mouseEvent->button() & Qt::LeftButton)
      {
      this->startChanging();
      }
    }
  else if (event->type() == QEvent::MouseButtonRelease)
    {
    QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
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
double QBirchSliderWidget::singleStep() const
{
  Q_D(const QBirchSliderWidget);
  Q_ASSERT(d->equal(d->SpinBox->singleStep(), d->Slider->singleStep()));
  return d->Slider->singleStep();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget::setSingleStep(double step)
{
  Q_D(QBirchSliderWidget);
  d->SpinBox->setSingleStep(step);
  d->Slider->setSingleStep(d->SpinBox->singleStep());
  Q_ASSERT(d->equal(d->SpinBox->minimum(), d->Slider->minimum()));
  Q_ASSERT(d->equal(d->SpinBox->value(), d->Slider->value()));
  Q_ASSERT(d->equal(d->SpinBox->maximum(), d->Slider->maximum()));
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchSliderWidget::pageStep() const
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
int QBirchSliderWidget::decimals() const
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
  Q_ASSERT(d->equal(d->SpinBox->minimum(), d->Slider->minimum()));
  Q_ASSERT(d->equal(d->SpinBox->value(), d->Slider->value()));
  Q_ASSERT(d->equal(d->SpinBox->maximum(), d->Slider->maximum()));
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QString QBirchSliderWidget::prefix() const
{
  Q_D(const QBirchSliderWidget);
  return d->SpinBox->prefix();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget::setPrefix(const QString& newPrefix)
{
  Q_D(QBirchSliderWidget);
  d->SpinBox->setPrefix(newPrefix);
#if QT_VERSION < 0x040800
  /// Setting the prefix doesn't recompute the sizehint, do it manually here:
  /// See: http://bugreports.qt.nokia.com/browse/QTBUG-9530
  d->SpinBox->setRange(d->SpinBox->minimum(), d->SpinBox->maximum());
#endif
  d->updateSpinBoxWidth();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QString QBirchSliderWidget::suffix() const
{
  Q_D(const QBirchSliderWidget);
  return d->SpinBox->suffix();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget::setSuffix(const QString& newSuffix)
{
  Q_D(QBirchSliderWidget);
  d->SpinBox->setSuffix(newSuffix);
#if QT_VERSION < 0x040800
  /// Setting the suffix doesn't recompute the sizehint, do it manually here:
  /// See: http://bugreports.qt.nokia.com/browse/QTBUG-9530
  d->SpinBox->setRange(d->SpinBox->minimum(), d->SpinBox->maximum());
#endif
  d->updateSpinBoxWidth();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchSliderWidget::tickInterval() const
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
Qt::Alignment QBirchSliderWidget::spinBoxAlignment() const
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
bool QBirchSliderWidget::hasTracking() const
{
  Q_D(const QBirchSliderWidget);
  return d->Tracking;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchSliderWidget::isAutoSpinBoxWidth() const
{
  Q_D(const QBirchSliderWidget);
  return d->AutoSpinBoxWidth;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliderWidget::setAutoSpinBoxWidth(bool autoWidth)
{
  Q_D(QBirchSliderWidget);
  d->AutoSpinBoxWidth = autoWidth;
  d->updateSpinBoxWidth();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchSliderWidget::isSpinBoxVisible() const
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
QDoubleSpinBox* QBirchSliderWidget::spinBox()
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
