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

// CTK includes
//#include "ctkPopupWidget.h"
#include "vtkBirchSliderWidget.h"
#include "ui_vtkBirchSliderWidget.h"

// STD includes 
#include <cmath>

//-----------------------------------------------------------------------------
class vtkBirchSliderWidgetPrivate: public Ui_vtkBirchSliderWidget
{
  Q_DECLARE_PUBLIC(vtkBirchSliderWidget);
protected:
  vtkBirchSliderWidget* const q_ptr;

public:
  vtkBirchSliderWidgetPrivate(vtkBirchSliderWidget& object);
  virtual ~vtkBirchSliderWidgetPrivate();

  void updateSpinBoxWidth();
  int synchronizedSpinBoxWidth()const;
  void synchronizeSiblingSpinBox(int newWidth);
  bool equal(double spinBoxValue, double sliderValue)const
  {
    return qAbs(sliderValue - spinBoxValue) < std::pow(10., -this->SpinBox->decimals());
  }

  bool   Tracking;
  bool   Changing;
  double ValueBeforeChange;
  bool   AutoSpinBoxWidth;
  //ctkPopupWidget* SliderPopup;
};

// --------------------------------------------------------------------------
vtkBirchSliderWidgetPrivate::vtkBirchSliderWidgetPrivate(vtkBirchSliderWidget& object)
  :q_ptr(&object)
{
  this->Tracking = true;
  this->Changing = false;
  this->ValueBeforeChange = 0.;
  this->AutoSpinBoxWidth = true;
  //this->SliderPopup = 0;
}


// --------------------------------------------------------------------------
vtkBirchSliderWidgetPrivate::~vtkBirchSliderWidgetPrivate()
{
}

// --------------------------------------------------------------------------
void vtkBirchSliderWidgetPrivate::updateSpinBoxWidth()
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

// --------------------------------------------------------------------------
int vtkBirchSliderWidgetPrivate::synchronizedSpinBoxWidth()const
{
  Q_Q(const vtkBirchSliderWidget);
  int maxWidth = this->SpinBox->sizeHint().width();
  if (!q->parent())
    {
    return maxWidth;
    }
  QList<vtkBirchSliderWidget*> siblings =
    q->parent()->findChildren<vtkBirchSliderWidget*>();
  foreach(vtkBirchSliderWidget* sibling, siblings)
    {
    maxWidth = qMax(maxWidth, sibling->d_func()->SpinBox->sizeHint().width());
    }
  return maxWidth;
}

// --------------------------------------------------------------------------
void vtkBirchSliderWidgetPrivate::synchronizeSiblingSpinBox(int width)
{
  Q_Q(const vtkBirchSliderWidget);
  QList<vtkBirchSliderWidget*> siblings =
    q->parent()->findChildren<vtkBirchSliderWidget*>();
  foreach(vtkBirchSliderWidget* sibling, siblings)
    {
    if (sibling != q && sibling->isAutoSpinBoxWidth())
      {
      sibling->d_func()->SpinBox->setMinimumWidth(width);
      }
    }
}

// --------------------------------------------------------------------------
vtkBirchSliderWidget::vtkBirchSliderWidget(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new vtkBirchSliderWidgetPrivate(*this))
{
  Q_D(vtkBirchSliderWidget);
  
  d->setupUi(this);

  d->Slider->setMaximum(d->SpinBox->maximum());
  d->Slider->setMinimum(d->SpinBox->minimum());

  this->connect(d->SpinBox, SIGNAL(valueChanged(double)), d->Slider, SLOT(setValue(double)));

  //this->connect(d->Slider, SIGNAL(valueChanged(double)), SIGNAL(valueChanged(double)));
  this->connect(d->Slider, SIGNAL(sliderPressed()), this, SLOT(startChanging()));
  this->connect(d->Slider, SIGNAL(sliderReleased()), this, SLOT(stopChanging()));
  this->connect(d->Slider, SIGNAL(valueChanged(double)), this, SLOT(changeValue(double)));
  d->SpinBox->installEventFilter(this);
}

// --------------------------------------------------------------------------
vtkBirchSliderWidget::~vtkBirchSliderWidget()
{
}

// --------------------------------------------------------------------------
double vtkBirchSliderWidget::minimum()const
{
  Q_D(const vtkBirchSliderWidget);
  Q_ASSERT(d->equal(d->SpinBox->minimum(),d->Slider->minimum()));
  return d->Slider->minimum();
}

// --------------------------------------------------------------------------
double vtkBirchSliderWidget::maximum()const
{
  Q_D(const vtkBirchSliderWidget);
  Q_ASSERT(d->equal(d->SpinBox->maximum(),d->Slider->maximum()));
  return d->Slider->maximum();
}

// --------------------------------------------------------------------------
void vtkBirchSliderWidget::setMinimum(double min)
{
  Q_D(vtkBirchSliderWidget);
  bool wasBlocked = d->SpinBox->blockSignals(true);
  d->SpinBox->setMinimum(min);
  d->SpinBox->blockSignals(wasBlocked);

  // SpinBox can truncate min (depending on decimals).
  // use Spinbox's min to set Slider's min
  d->Slider->setMinimum(d->SpinBox->minimum());
  Q_ASSERT(d->equal(d->SpinBox->minimum(),d->Slider->minimum()));
  Q_ASSERT(d->equal(d->SpinBox->value(),d->Slider->value()));
  Q_ASSERT(d->equal(d->SpinBox->maximum(),d->Slider->maximum()));
  d->updateSpinBoxWidth();
}

// --------------------------------------------------------------------------
void vtkBirchSliderWidget::setMaximum(double max)
{
  Q_D(vtkBirchSliderWidget);
  bool wasBlocked = d->SpinBox->blockSignals(true);
  d->SpinBox->setMaximum(max);
  d->SpinBox->blockSignals(wasBlocked);

  // SpinBox can truncate max (depending on decimals).
  // use Spinbox's max to set Slider's max
  d->Slider->setMaximum(d->SpinBox->maximum());
  Q_ASSERT(d->equal(d->SpinBox->minimum(),d->Slider->minimum()));
  Q_ASSERT(d->equal(d->SpinBox->value(),d->Slider->value()));
  Q_ASSERT(d->equal(d->SpinBox->maximum(),d->Slider->maximum()));
  d->updateSpinBoxWidth();
}

// --------------------------------------------------------------------------
void vtkBirchSliderWidget::setRange(double min, double max)
{
  Q_D(vtkBirchSliderWidget);
  
  bool wasBlocked = d->SpinBox->blockSignals(true);
  d->SpinBox->setRange(min, max);
  d->SpinBox->blockSignals(wasBlocked);
  
  // SpinBox can truncate the range (depending on decimals).
  // use Spinbox's range to set Slider's range
  d->Slider->setRange(d->SpinBox->minimum(), d->SpinBox->maximum());
  Q_ASSERT(d->equal(d->SpinBox->minimum(),d->Slider->minimum()));
  Q_ASSERT(d->equal(d->SpinBox->value(),d->Slider->value()));
  Q_ASSERT(d->equal(d->SpinBox->maximum(),d->Slider->maximum()));
  d->updateSpinBoxWidth();
}
/*
// --------------------------------------------------------------------------
double vtkBirchSliderWidget::sliderPosition()const
{
  return d->Slider->sliderPosition();
}

// --------------------------------------------------------------------------
void vtkBirchSliderWidget::setSliderPosition(double position)
{
  d->Slider->setSliderPosition(position);
}
*/
/*
// --------------------------------------------------------------------------
double vtkBirchSliderWidget::previousSliderPosition()
{
  return d->Slider->previousSliderPosition();
}
*/

// --------------------------------------------------------------------------
double vtkBirchSliderWidget::value()const
{
  Q_D(const vtkBirchSliderWidget);
  Q_ASSERT(d->equal(d->SpinBox->value(), d->Slider->value()));
  return d->Changing ? d->ValueBeforeChange : d->Slider->value();
}

// --------------------------------------------------------------------------
void vtkBirchSliderWidget::setValue(double _value)
{
  Q_D(vtkBirchSliderWidget);
  // disable the tracking temporally to emit the
  // signal valueChanged if changeValue() is called
  bool isChanging = d->Changing;
  d->Changing = false;
  d->SpinBox->setValue(_value);
  // Why do we need to set the value to the slider ?
  //d->Slider->setValue(d->SpinBox->value());
  //double spinBoxValue = d->SpinBox->value();
  Q_ASSERT(d->equal(d->SpinBox->minimum(),d->Slider->minimum()));
  Q_ASSERT(d->equal(d->SpinBox->value(),d->Slider->value()));
  Q_ASSERT(d->equal(d->SpinBox->maximum(),d->Slider->maximum()));
  // restore the prop
  d->Changing = isChanging;
}

// --------------------------------------------------------------------------
void vtkBirchSliderWidget::startChanging()
{
  Q_D(vtkBirchSliderWidget);
  if (d->Tracking)
    {
    return;
    }
  d->Changing = true;
  d->ValueBeforeChange = this->value();
}

// --------------------------------------------------------------------------
void vtkBirchSliderWidget::stopChanging()
{
  Q_D(vtkBirchSliderWidget);
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

// --------------------------------------------------------------------------
void vtkBirchSliderWidget::changeValue(double newValue)
{
  Q_D(vtkBirchSliderWidget);
  
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

// --------------------------------------------------------------------------
bool vtkBirchSliderWidget::eventFilter(QObject *obj, QEvent *event)
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
       // here we might prevent vtkBirchSliderWidget::stopChanging
       // from sending a valueChanged() event as the spinbox might
       // send a valueChanged() after eventFilter() is done.
       this->stopChanging();
       }
     } 
   // standard event processing
   return this->Superclass::eventFilter(obj, event);
 }

// --------------------------------------------------------------------------
double vtkBirchSliderWidget::singleStep()const
{
  Q_D(const vtkBirchSliderWidget);
  Q_ASSERT(d->equal(d->SpinBox->singleStep(), d->Slider->singleStep()));
  return d->Slider->singleStep();
}

// --------------------------------------------------------------------------
void vtkBirchSliderWidget::setSingleStep(double step)
{
  Q_D(vtkBirchSliderWidget);
  d->SpinBox->setSingleStep(step);
  d->Slider->setSingleStep(d->SpinBox->singleStep());
  Q_ASSERT(d->equal(d->SpinBox->minimum(),d->Slider->minimum()));
  Q_ASSERT(d->equal(d->SpinBox->value(),d->Slider->value()));
  Q_ASSERT(d->equal(d->SpinBox->maximum(),d->Slider->maximum()));
}

// --------------------------------------------------------------------------
double vtkBirchSliderWidget::pageStep()const
{
  Q_D(const vtkBirchSliderWidget);
  return d->Slider->pageStep();
}

// --------------------------------------------------------------------------
void vtkBirchSliderWidget::setPageStep(double step)
{
  Q_D(vtkBirchSliderWidget);
  d->Slider->setPageStep(step);
}

// --------------------------------------------------------------------------
int vtkBirchSliderWidget::decimals()const
{
  Q_D(const vtkBirchSliderWidget);
  return d->SpinBox->decimals();
}

// --------------------------------------------------------------------------
void vtkBirchSliderWidget::setDecimals(int newDecimals)
{
  Q_D(vtkBirchSliderWidget);
  d->SpinBox->setDecimals(newDecimals);
  // The number of decimals can change the range values
  // i.e. 50.55 with 2 decimals -> 51 with 0 decimals
  // As the SpinBox range change doesn't fire signals, 
  // we have to do the synchronization manually here
  d->Slider->setRange(d->SpinBox->minimum(), d->SpinBox->maximum());
  Q_ASSERT(d->equal(d->SpinBox->minimum(),d->Slider->minimum()));
  Q_ASSERT(d->equal(d->SpinBox->value(),d->Slider->value()));
  Q_ASSERT(d->equal(d->SpinBox->maximum(),d->Slider->maximum()));
}

// --------------------------------------------------------------------------
QString vtkBirchSliderWidget::prefix()const
{
  Q_D(const vtkBirchSliderWidget);
  return d->SpinBox->prefix();
}

// --------------------------------------------------------------------------
void vtkBirchSliderWidget::setPrefix(const QString& newPrefix)
{
  Q_D(vtkBirchSliderWidget);
  d->SpinBox->setPrefix(newPrefix);
#if QT_VERSION < 0x040800
  /// Setting the prefix doesn't recompute the sizehint, do it manually here:
  /// See: http://bugreports.qt.nokia.com/browse/QTBUG-9530
  d->SpinBox->setRange(d->SpinBox->minimum(), d->SpinBox->maximum());
#endif
  d->updateSpinBoxWidth();
}

// --------------------------------------------------------------------------
QString vtkBirchSliderWidget::suffix()const
{
  Q_D(const vtkBirchSliderWidget);
  return d->SpinBox->suffix();
}

// --------------------------------------------------------------------------
void vtkBirchSliderWidget::setSuffix(const QString& newSuffix)
{
  Q_D(vtkBirchSliderWidget);
  d->SpinBox->setSuffix(newSuffix);
#if QT_VERSION < 0x040800
  /// Setting the suffix doesn't recompute the sizehint, do it manually here:
  /// See: http://bugreports.qt.nokia.com/browse/QTBUG-9530
  d->SpinBox->setRange(d->SpinBox->minimum(), d->SpinBox->maximum());
#endif
  d->updateSpinBoxWidth();
}

// --------------------------------------------------------------------------
double vtkBirchSliderWidget::tickInterval()const
{
  Q_D(const vtkBirchSliderWidget);
  return d->Slider->tickInterval();
}

// --------------------------------------------------------------------------
void vtkBirchSliderWidget::setTickInterval(double ti)
{ 
  Q_D(vtkBirchSliderWidget);
  d->Slider->setTickInterval(ti);
}

// -------------------------------------------------------------------------
void vtkBirchSliderWidget::reset()
{
  this->setValue(0.);
}

// -------------------------------------------------------------------------
void vtkBirchSliderWidget::setSpinBoxAlignment(Qt::Alignment alignment)
{
  Q_D(vtkBirchSliderWidget);
  return d->SpinBox->setAlignment(alignment);
}

// -------------------------------------------------------------------------
Qt::Alignment vtkBirchSliderWidget::spinBoxAlignment()const
{
  Q_D(const vtkBirchSliderWidget);
  return d->SpinBox->alignment();
}

// -------------------------------------------------------------------------
void vtkBirchSliderWidget::setTracking(bool enable)
{
  Q_D(vtkBirchSliderWidget);
  d->Tracking = enable;
}

// -------------------------------------------------------------------------
bool vtkBirchSliderWidget::hasTracking()const
{
  Q_D(const vtkBirchSliderWidget);
  return d->Tracking;
}

// -------------------------------------------------------------------------
bool vtkBirchSliderWidget::isAutoSpinBoxWidth()const
{
  Q_D(const vtkBirchSliderWidget);
  return d->AutoSpinBoxWidth;
}

// -------------------------------------------------------------------------
void vtkBirchSliderWidget::setAutoSpinBoxWidth(bool autoWidth)
{
  Q_D(vtkBirchSliderWidget);
  d->AutoSpinBoxWidth = autoWidth;
  d->updateSpinBoxWidth();
}

// -------------------------------------------------------------------------
bool vtkBirchSliderWidget::isSpinBoxVisible()const
{
  Q_D(const vtkBirchSliderWidget);
  return d->SpinBox->isVisibleTo(const_cast<vtkBirchSliderWidget*>(this));
}

// -------------------------------------------------------------------------
void vtkBirchSliderWidget::setSpinBoxVisible(bool visible)
{
  Q_D(vtkBirchSliderWidget);
  d->SpinBox->setVisible(visible);
}
/*
// --------------------------------------------------------------------------
bool vtkBirchSliderWidget::hasPopupSlider()const
{
  Q_D(const vtkBirchSliderWidget);
  return d->SliderPopup != 0;
}

// --------------------------------------------------------------------------
void vtkBirchSliderWidget::setPopupSlider(bool popup)
{
  Q_D(vtkBirchSliderWidget);
  if (this->hasPopupSlider() == popup)
    {
    return;
    }
  if (popup)
    {
    d->SliderPopup = new ctkPopupWidget(this);
    d->SliderPopup->setObjectName("DoubleSliderPopup");

    QHBoxLayout* layout = new QHBoxLayout(d->SliderPopup);
    layout->setContentsMargins(0,0,0,0);
    /// If the Slider has already been created, it will try to keep its
    /// size.
    layout->addWidget(d->Slider);

    d->SliderPopup->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->SliderPopup->setOrientation(Qt::Horizontal);
    d->SliderPopup->setHorizontalDirection(Qt::RightToLeft);
    }
  else
    {
    qobject_cast<QHBoxLayout*>(this->layout())->insertWidget(0,d->Slider);
    d->SliderPopup->deleteLater();
    d->SliderPopup = 0;
    }
}

// --------------------------------------------------------------------------
ctkPopupWidget* vtkBirchSliderWidget::popup()const
{
  Q_D(const vtkBirchSliderWidget);
  return d->SliderPopup;
}
*/
// --------------------------------------------------------------------------
QDoubleSpinBox* vtkBirchSliderWidget::spinBox()
{
  Q_D(vtkBirchSliderWidget);
  return d->SpinBox;
}

// --------------------------------------------------------------------------
vtkBirchDoubleSlider* vtkBirchSliderWidget::slider()
{
  Q_D(vtkBirchSliderWidget);
  return d->Slider;
}
