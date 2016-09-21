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

// Birch includes
#include "QBirchDoubleSpinBox_p.h"

// Qt includes
#include <QApplication>
#include <QDebug>
#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include <QShortcut>
#include <QSizePolicy>
#include <QStyle>
#include <QStyleOptionSpinBox>
#include <QVariant>

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
// birchQDoubleSpinBox
//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
birchQDoubleSpinBox::birchQDoubleSpinBox(QBirchDoubleSpinBoxPrivate* pimpl,
                                     QWidget* spinBoxParent)
  : QDoubleSpinBox(spinBoxParent)
  , d_ptr(pimpl)
{
  this->InvertedControls = false;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
QLineEdit* birchQDoubleSpinBox::lineEdit()const
{
  return this->QDoubleSpinBox::lineEdit();
}
//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
void birchQDoubleSpinBox::initStyleOptionSpinBox(QStyleOptionSpinBox* option)
{
  this->initStyleOption(option);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
void birchQDoubleSpinBox::setInvertedControls(bool invertedControls)
{
  this->InvertedControls = invertedControls;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
bool birchQDoubleSpinBox::invertedControls() const
{
  return this->InvertedControls;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
void birchQDoubleSpinBox::stepBy(int steps)
{
  if (this->InvertedControls)
    {
    steps = -steps;
    }
  this->Superclass::stepBy(steps);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
QAbstractSpinBox::StepEnabled birchQDoubleSpinBox::stepEnabled() const
{
  if (!this->InvertedControls)
    {
    return this->Superclass::stepEnabled();
    }

  if (this->isReadOnly())
    {
    return StepNone;
    }

  if (this->wrapping())
    {
    return StepEnabled(StepUpEnabled | StepDownEnabled);
    }

  StepEnabled ret = StepNone;
  double value = this->value();
  if (value < this->maximum())
    {
    ret |= StepDownEnabled;
    }
  if (value > this->minimum())
    {
    ret |= StepUpEnabled;
    }
  return ret;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double birchQDoubleSpinBox::valueFromText(const QString &text) const
{
  Q_D(const QBirchDoubleSpinBox);

  QString copy = text;
  int pos = this->lineEdit()->cursorPosition();
  QValidator::State state = QValidator::Acceptable;
  int decimals = 0;
  double value = d->validateAndInterpret(copy, pos, state, decimals);
  return value;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QString birchQDoubleSpinBox::textFromValue(double value) const
{
  Q_D(const QBirchDoubleSpinBox);
  QString text = this->QDoubleSpinBox::textFromValue(value);
  if (text.isEmpty())
    {
    text = "0";
    }
  // If there is no decimal, it does not mean there won't be any.
  if (d->DOption & QBirchDoubleSpinBox::DecimalPointAlwaysVisible &&
      text.indexOf(this->locale().decimalPoint()) == -1)
    {
    text += this->locale().decimalPoint();
    }
  return text;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int birchQDoubleSpinBox::decimalsFromText(const QString &text) const
{
  Q_D(const QBirchDoubleSpinBox);

  QString copy = text;
  int pos = this->lineEdit()->cursorPosition();
  int decimals = 0;
  QValidator::State state = QValidator::Acceptable;
  d->validateAndInterpret(copy, pos, state, decimals);
  return decimals;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QValidator::State birchQDoubleSpinBox::validate(QString &text, int &pos) const
{
  Q_D(const QBirchDoubleSpinBox);

  QValidator::State state = QValidator::Acceptable;
  int decimals = 0;
  d->validateAndInterpret(text, pos, state, decimals);
  return state;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
// QBirchDoubleSpinBoxPrivate
// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchDoubleSpinBoxPrivate::QBirchDoubleSpinBoxPrivate(QBirchDoubleSpinBox& object)
  : q_ptr(&object)
{
  qRegisterMetaType<QBirchDoubleSpinBox::SetMode>("QBirchDoubleSpinBox::SetMode");
  qRegisterMetaType<QBirchDoubleSpinBox::DecimalsOptions>("QBirchDoubleSpinBox::DecimalsOption");
  this->SpinBox = 0;
  this->Mode = QBirchDoubleSpinBox::SetIfDifferent;
  this->DefaultDecimals = 2;
  // InsertDecimals is not a great default, but it is QDoubleSpinBox's default.
  this->DOption = QBirchDoubleSpinBox::DecimalsByShortcuts
    | QBirchDoubleSpinBox::InsertDecimals;
  this->InvertedControls = false;
  this->SizeHintPolicy = QBirchDoubleSpinBox::SizeHintByMinMax;
  this->InputValue = 0.;
  this->InputRange[0] = 0.;
  this->InputRange[1] = 99.99;
  this->ForceInputValueUpdate = false;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSpinBoxPrivate::init()
{
  Q_Q(QBirchDoubleSpinBox);
  this->SpinBox = new birchQDoubleSpinBox(this, q);
  this->SpinBox->setInvertedControls(this->InvertedControls);
  // birchQDoubleSpinBox needs to be first to receive textChanged() signals.
  QLineEdit* lineEdit = new QLineEdit(q);
  QObject::connect(lineEdit, SIGNAL(textChanged(QString)),
                   this, SLOT(editorTextChanged(QString)));
  this->SpinBox->setLineEdit(lineEdit);
  lineEdit->setObjectName(QLatin1String("qt_spinbox_lineedit"));
  this->InputValue = this->SpinBox->value();
  this->InputRange[0] = this->SpinBox->minimum();
  this->InputRange[1] = this->SpinBox->maximum();

  QObject::connect(this->SpinBox, SIGNAL(valueChanged(double)),
    this, SLOT(onValueChanged()));
  QObject::connect(this->SpinBox, SIGNAL(editingFinished()),
    q, SIGNAL(editingFinished()));

  QHBoxLayout* l = new QHBoxLayout(q);
  l->addWidget(this->SpinBox);
  l->setContentsMargins(0,0,0,0);
  q->setLayout(l);
  q->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,
    QSizePolicy::Fixed, QSizePolicy::ButtonBox));

  this->SpinBox->installEventFilter(q);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchDoubleSpinBoxPrivate::compare(double x1, double x2) const
{
  Q_Q(const QBirchDoubleSpinBox);
  return q->round(x1) == q->round(x2);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchDoubleSpinBoxPrivate::round(double value, int decimals) const
{
  return QString::number(value, 'f', decimals).toDouble();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QString QBirchDoubleSpinBoxPrivate::stripped(const QString& text, int* pos) const
{
  Q_Q(const QBirchDoubleSpinBox);
  QString strip(text);
  if (strip.startsWith(q->prefix()))
    {
    strip.remove(0, q->prefix().size());
    }
  if (strip.endsWith(q->suffix()))
    {
    strip.chop(q->suffix().size());
    }
  strip = strip.trimmed();
  if (pos)
    {
    int stripInText = text.indexOf(strip);
    *pos = qBound(0, *pos - stripInText, strip.size());
    }
  return strip;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int QBirchDoubleSpinBoxPrivate::boundDecimals(int dec)const
{
  Q_Q(const QBirchDoubleSpinBox);
  if (dec == -1)
    {
    return q->decimals();
    }
  int min = (this->DOption & QBirchDoubleSpinBox::DecimalsAsMin) ?
    this->DefaultDecimals : 0;
  int max = (this->DOption & QBirchDoubleSpinBox::DecimalsAsMax) ?
    this->DefaultDecimals : 323; // see QDoubleSpinBox::decimals doc
  return qBound(min, dec, max);
}


// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int QBirchDoubleSpinBoxPrivate::significantDecimals(double value, int defaultDecimals)const
{
  Q_Q(const QBirchDoubleSpinBox);
  if (value == 0.
      || qAbs(value) == std::numeric_limits<double>::infinity())
    {
    return 0;
    }
  if (value != value) // is NaN
    {
    return -1;
    }
  QString number = QString::number(value, 'f', 16);
  QString fractional = number.section('.', 1, 1);
  Q_ASSERT(fractional.length() == 16);
  QChar previous;
  int previousRepeat=0;
  bool only0s = true;
  bool isUnit = value > -1. && value < 1.;
  for (int i = 0; i < fractional.length(); ++i)
    {
    QChar digit = fractional.at(i);
    if (digit != '0')
      {
      only0s = false;
      }
    // Has the digit been repeated too many times ?
    if (digit == previous && previousRepeat == 2 &&
        !only0s)
      {
      if (digit == '0' || digit == '9')
        {
        return i - previousRepeat;
        }
      return i;
      }
    // Last digit
    if (i == fractional.length() - 1)
      {
      // If we are here, that means that the right number of significant
      // decimals for the number has not been figured out yet.
      if (previousRepeat > 2 && !(only0s && isUnit) )
        {
        return i - previousRepeat;
        }
      // If defaultDecimals has been provided, just use it.
      if (defaultDecimals >= 0)
        {
        return defaultDecimals;
        }
      return fractional.length();
      }
    // get ready for next
    if (previous != digit)
      {
      previous = digit;
      previousRepeat = 1;
      }
    else
      {
      ++previousRepeat;
      }
    }
  Q_ASSERT(false);
  return fractional.length();
}


// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int QBirchDoubleSpinBoxPrivate::decimalsForValue(double value) const
{
  int decimals = this->DefaultDecimals;
  if (this->DOption & QBirchDoubleSpinBox::DecimalsByValue)
    {
    decimals = this->significantDecimals(value, decimals);
    }
  return this->boundDecimals(decimals);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSpinBoxPrivate::setValue(double value, int dec)
{
  Q_Q(QBirchDoubleSpinBox);
  dec = this->boundDecimals(dec);
  const bool changeDecimals = dec != q->decimals();
  if (changeDecimals)
    {
    // don't fire valueChanged signal because we will change the value
    // right after anyway.
    const bool blockValueChangedSignal = (this->round(this->SpinBox->value(), dec) != value);
    bool wasBlocked = false;
    if (blockValueChangedSignal)
      {
      wasBlocked = this->SpinBox->blockSignals(true);
      }
    // don't fire decimalsChanged signal yet, wait for the value to be
    // up-to-date.
    this->SpinBox->setDecimals(dec);
    if (blockValueChangedSignal)
      {
      this->SpinBox->blockSignals(wasBlocked);
      }
    }
  this->SpinBox->setValue(value); // re-do the text (calls textFromValue())
  if (changeDecimals)
    {
    emit q->decimalsChanged(dec);
    }
  if (this->SizeHintPolicy == QBirchDoubleSpinBox::SizeHintByValue)
    {
    this->CachedSizeHint = QSize();
    q->updateGeometry();
    }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSpinBoxPrivate::setDecimals(int dec)
{
  Q_Q(QBirchDoubleSpinBox);
  dec = this->boundDecimals(dec);
  this->SpinBox->setDecimals(dec);
  emit q->decimalsChanged(dec);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSpinBoxPrivate::editorTextChanged(const QString& text)
{
  if (this->SpinBox->keyboardTracking())
    {
    QString tmp = text;
    int pos = this->SpinBox->lineEdit()->cursorPosition();
    QValidator::State state = QValidator::Invalid;
    int decimals = 0;
    this->validateAndInterpret(tmp, pos, state, decimals);
    if (state == QValidator::Acceptable)
      {
      double newValue = this->SpinBox->valueFromText(tmp);
      int decimals = this->boundDecimals(this->SpinBox->decimalsFromText(tmp));
      bool changeDecimals = this->DOption & QBirchDoubleSpinBox::DecimalsByKey &&
        decimals != this->SpinBox->decimals();
      if (changeDecimals)
        {
        this->ForceInputValueUpdate = true;
        this->setValue(newValue, decimals);
        this->ForceInputValueUpdate = false;
        }
      // else, let QDoubleSpinBox process the validation.
      }
    }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchDoubleSpinBoxPrivate
::validateAndInterpret(QString &input, int &pos,
                       QValidator::State &state, int &decimals) const
{
  Q_Q(const QBirchDoubleSpinBox);
  if (this->CachedText == input)
    {
    state = this->CachedState;
    decimals = this->CachedDecimals;
    return this->CachedValue;
    }
  const double max = this->SpinBox->maximum();
  const double min = this->SpinBox->minimum();

  int posInValue = pos;
  QString text = this->stripped(input, &posInValue);
  // posInValue can change, track the offset.
  const int oldPosInValue = posInValue;
  state = QValidator::Acceptable;
  decimals = 0;

  double value = min;
  const int dec = text.indexOf(q->locale().decimalPoint());

  bool ok = false;
  value = q->locale().toDouble(text, &ok);

  // could be in an intermediate state
  if (!ok  && state == QValidator::Acceptable)
    {
    if (text.isEmpty() ||
        text == "." ||
        text == "-" ||
        text == "+" ||
        text == "-." ||
        text == "+.")
      {
      state = QValidator::Intermediate;
      }
    }
  // could be because of group separators:
  if (!ok && state == QValidator::Acceptable)
    {
    if (q->locale().groupSeparator().isPrint())
      {
      int start = (dec == -1 ? text.size() : dec)- 1;
      int lastGroupSeparator = start;
      for (int digit = start; digit >= 0; --digit)
        {
        if (text.at(digit) == q->locale().groupSeparator())
          {
          if (digit != lastGroupSeparator - 3)
            {
            state = QValidator::Invalid;
            break;
            }
          text.remove(digit, 1);
          lastGroupSeparator = digit;
          }
        }
      }
    // try again without the group separators
    value = q->locale().toDouble(text, &ok);
    }
  // test the decimalPoint
  if (!ok && state == QValidator::Acceptable)
    {
    // duplicate decimal points probably means the user typed another decimal points,
    // move the cursor pos to the right then
    if (dec + 1 < text.size() &&
        text.at(dec + 1) == q->locale().decimalPoint() &&
        posInValue == dec + 1)
      {
      text.remove(dec + 1, 1);
      value = q->locale().toDouble(text, &ok);
      }
    }
  if (ok && state != QValidator::Invalid)
    {
    if (dec != -1)
      {
      decimals = text.size() - (dec + 1);
      if (decimals > q->decimals())
        {
        // With ReplaceDecimals on, key strokes replace decimal digits
        if (posInValue > dec && posInValue < text.size())
          {
          const int extraDecimals = decimals - q->decimals();
          if (this->DOption & QBirchDoubleSpinBox::ReplaceDecimals)
            {
            text.remove(posInValue, extraDecimals);
            decimals = q->decimals();
            value = q->locale().toDouble(text, &ok);
            }
          else if (!(this->DOption & QBirchDoubleSpinBox::InsertDecimals))
            {
            text.remove(text.size() - extraDecimals, extraDecimals);
            decimals = q->decimals();
            value = q->locale().toDouble(text, &ok);
            }
          }
        }
      // When DecimalsByKey is set, it is possible to extend the number of decimals
      if (decimals > q->decimals() &&
          !(this->DOption & QBirchDoubleSpinBox::DecimalsByKey) )
        {
        state = QValidator::Invalid;
        }
      }
    }
  if (state == QValidator::Acceptable)
    {
    if (!ok)
      {
      state = QValidator::Invalid;
      }
    else if (value >= min && value <= max)
      {
      state = QValidator::Acceptable;
      }
    else if (max == min)
      { // when max and min is the same the only non-Invalid input is max (or min)
      state = QValidator::Invalid;
      }
    else if ((value >= 0 && value > max) || (value < 0 && value < min))
      {
      state = QValidator::Invalid;
      }
    else
      {
      state = QValidator::Intermediate;
      }
    }

  if (state != QValidator::Acceptable)
    {
    value = max > 0 ? min : max;
    }

  pos += posInValue - oldPosInValue;
  input = q->prefix() + text + q->suffix();
  this->CachedText = input;
  this->CachedState = state;
  this->CachedValue = value;
  this->CachedDecimals = decimals;
  return value;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSpinBoxPrivate::onValueChanged()
{
  Q_Q(QBirchDoubleSpinBox);
  double newValue = this->SpinBox->value();
  double oldValue = q->value();
  // Don't trigger value changed signal if the difference only happened on the
  // precision.
  if (this->compare(oldValue, newValue) && !this->ForceInputValueUpdate)
    {
    return;
    }
  // Force it only once (when the user typed a new number that could have change
  // the number of decimals which could have make the compare test always pass.
  this->ForceInputValueUpdate = false;

  double minimum = q->minimum();
  double maximum = q->maximum();
  // Special case to return max precision
  if (this->compare(minimum, newValue))
    {
    newValue = q->minimum();
    }
  else if (this->compare(maximum, newValue))
    {
    newValue = q->maximum();
    }
  this->InputValue = newValue;
  emit q->valueChanged(newValue);
  // \tbd The string might not make much sense when using proxies.
  emit q->valueChanged(
    QString::number(newValue, 'f', this->SpinBox->decimals()));
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
// QBirchDoubleSpinBox
// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchDoubleSpinBox::QBirchDoubleSpinBox(QWidget* newParent)
  : QWidget(newParent)
  , d_ptr(new QBirchDoubleSpinBoxPrivate(*this))
{
  Q_D(QBirchDoubleSpinBox);
  d->init();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchDoubleSpinBox::QBirchDoubleSpinBox(QBirchDoubleSpinBox::SetMode mode, QWidget* newParent)
  : QWidget(newParent)
  , d_ptr(new QBirchDoubleSpinBoxPrivate(*this))
{
  Q_D(QBirchDoubleSpinBox);
  d->init();
  this->setSetMode(mode);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchDoubleSpinBox::~QBirchDoubleSpinBox()
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchDoubleSpinBox::value() const
{
  Q_D(const QBirchDoubleSpinBox);
  return d->InputValue;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchDoubleSpinBox::displayedValue() const
{
  Q_D(const QBirchDoubleSpinBox);
  return d->SpinBox->value();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
void QBirchDoubleSpinBox::setDisplayedValue(double value)
{
  Q_D(QBirchDoubleSpinBox);
  d->SpinBox->setValue(value);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QString QBirchDoubleSpinBox::text() const
{
  Q_D(const QBirchDoubleSpinBox);
  return d->SpinBox->text();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QString QBirchDoubleSpinBox::cleanText() const
{
  Q_D(const QBirchDoubleSpinBox);
  return d->SpinBox->cleanText();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
Qt::Alignment QBirchDoubleSpinBox::alignment() const
{
  Q_D(const QBirchDoubleSpinBox);
  return d->SpinBox->alignment();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSpinBox::setAlignment(Qt::Alignment flag)
{
  Q_D(const QBirchDoubleSpinBox);
  if (d->Mode == QBirchDoubleSpinBox::SetIfDifferent && flag == d->SpinBox->alignment())
    {
    return;
    }

  d->SpinBox->setAlignment(flag);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSpinBox::setFrame(bool frame)
{
  Q_D(const QBirchDoubleSpinBox);
  if (d->Mode == QBirchDoubleSpinBox::SetIfDifferent && frame == d->SpinBox->hasFrame())
    {
    return;
    }

  d->SpinBox->setFrame(frame);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchDoubleSpinBox::hasFrame() const
{
  Q_D(const QBirchDoubleSpinBox);
  return d->SpinBox->hasFrame();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QString QBirchDoubleSpinBox::prefix() const
{
  Q_D(const QBirchDoubleSpinBox);
  return d->SpinBox->prefix();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSpinBox::setPrefix(const QString &prefix)
{
  Q_D(const QBirchDoubleSpinBox);
  if (d->Mode == QBirchDoubleSpinBox::SetIfDifferent && prefix == d->SpinBox->prefix())
    {
    return;
    }

#if QT_VERSION < 0x040800
  /// Setting the prefix doesn't recompute the sizehint, do it manually here:
  /// See: http://bugreports.qt.nokia.com/browse/QTBUG-9530
  d->SpinBox->setRange(d->SpinBox->minimum(), d->SpinBox->maximum());
#endif

  d->SpinBox->setPrefix(prefix);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QString QBirchDoubleSpinBox::suffix() const
{
  Q_D(const QBirchDoubleSpinBox);
  return d->SpinBox->suffix();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSpinBox::setSuffix(const QString &suffix)
{
  Q_D(const QBirchDoubleSpinBox);
  if (d->Mode == QBirchDoubleSpinBox::SetIfDifferent && suffix == d->SpinBox->suffix())
    {
    return;
    }

#if QT_VERSION < 0x040800
  /// Setting the suffix doesn't recompute the sizehint, do it manually here:
  /// See: http://bugreports.qt.nokia.com/browse/QTBUG-9530
  d->SpinBox->setRange(d->SpinBox->minimum(), d->SpinBox->maximum());
#endif

  d->SpinBox->setSuffix(suffix);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchDoubleSpinBox::singleStep() const
{
  Q_D(const QBirchDoubleSpinBox);
  double step = d->SpinBox->singleStep();
  return step;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSpinBox::setSingleStep(double newStep)
{
  Q_D(QBirchDoubleSpinBox);
  if (d->Mode == QBirchDoubleSpinBox::SetIfDifferent
    && d->compare(newStep, this->singleStep()))
    {
    return;
    }

  d->SpinBox->setSingleStep(newStep);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchDoubleSpinBox::minimum() const
{
  Q_D(const QBirchDoubleSpinBox);
  return d->InputRange[0];
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSpinBox::setMinimum(double newMin)
{
  this->setRange(newMin, qMax(newMin, this->maximum()));
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchDoubleSpinBox::maximum() const
{
  Q_D(const QBirchDoubleSpinBox);
  return d->InputRange[1];
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSpinBox::setMaximum(double newMax)
{
  this->setRange(qMin(newMax, this->minimum()), newMax);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSpinBox::setRange(double newMin, double newMax)
{
  Q_D(QBirchDoubleSpinBox);
  if (newMin > newMax)
    {
    qSwap(newMin, newMax);
    }
  if (d->Mode == QBirchDoubleSpinBox::SetIfDifferent
      && newMin == d->InputRange[0]
      && newMax == d->InputRange[1])
    {
    return;
    }
  d->InputRange[0] = newMin;
  d->InputRange[1] = newMax;
  d->SpinBox->setRange(newMin, newMax);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int QBirchDoubleSpinBox::decimals() const
{
  Q_D(const QBirchDoubleSpinBox);
  return d->SpinBox->decimals();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSpinBox::setDecimals(int dec)
{
  Q_D(QBirchDoubleSpinBox);
  if (d->Mode == QBirchDoubleSpinBox::SetIfDifferent
      && dec == this->decimals()
      && dec == d->DefaultDecimals)
    {
    return;
    }

  d->DefaultDecimals = dec;
  // The number of decimals may or may not depend on the value. Recompute the
  // new number of decimals.
  double currentValue = this->value();
  int newDecimals = d->decimalsForValue(currentValue);
  d->setValue(currentValue, newDecimals);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchDoubleSpinBox::round(double value) const
{
  Q_D(const QBirchDoubleSpinBox);
  return QString::number(value, 'f', d->SpinBox->decimals()).toDouble();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QDoubleSpinBox* QBirchDoubleSpinBox::spinBox() const
{
  Q_D(const QBirchDoubleSpinBox);
  return d->SpinBox;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QLineEdit* QBirchDoubleSpinBox::lineEdit() const
{
  Q_D(const QBirchDoubleSpinBox);
  return d->SpinBox->lineEdit();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSpinBox::setValue(double value)
{
  Q_D(QBirchDoubleSpinBox);
  if (d->Mode == QBirchDoubleSpinBox::SetIfDifferent)
    {
    this->setValueIfDifferent(value);
    }
  else
    {
    this->setValueAlways(value);
    }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSpinBox::setValueIfDifferent(double newValue)
{
  Q_D(QBirchDoubleSpinBox);
  if (newValue == d->InputValue)
    {
    return;
    }
  this->setValueAlways(newValue);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSpinBox::setValueAlways(double newValue)
{
  Q_D(QBirchDoubleSpinBox);
  newValue = qBound(d->InputRange[0], newValue, d->InputRange[1]);
  const bool valueModified = d->InputValue != newValue;
  d->InputValue = newValue;
  double newValueToDisplay = newValue;
  const int decimals = d->decimalsForValue(newValueToDisplay);
  // setValueAlways already fires the valueChanged() signal if needed, same
  // thing for d->setValue() with decimalsChanged(). There is no need to
  // propagate the valueChanged/decimalsChanged signals from the spinbox.
  // Alternatively we could also have set a flag that prevents onValueChanged()
  // to trigger the valueChanged() signal.
  //bool wasBlocking = d->SpinBox->blockSignals(true);
  d->setValue(newValueToDisplay, decimals);
  const bool signalsEmitted = (newValue != d->InputValue);
  // Fire the valueChanged signal only if d->setValue() did not fire it
  // already..
  if (valueModified && !signalsEmitted)
    {
    emit valueChanged(d->InputValue);
    emit valueChanged(QString::number(d->InputValue, 'f', d->SpinBox->decimals()));
    }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSpinBox::stepUp()
{
  Q_D(const QBirchDoubleSpinBox);
  d->SpinBox->stepUp();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSpinBox::stepDown()
{
  Q_D(const QBirchDoubleSpinBox);
  d->SpinBox->stepDown();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchDoubleSpinBox::SetMode QBirchDoubleSpinBox::setMode() const
{
  Q_D(const QBirchDoubleSpinBox);
  return d->Mode;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSpinBox::setSetMode(QBirchDoubleSpinBox::SetMode newMode)
{
  Q_D(QBirchDoubleSpinBox);
  d->Mode = newMode;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchDoubleSpinBox::DecimalsOptions QBirchDoubleSpinBox::decimalsOption()
{
  Q_D(const QBirchDoubleSpinBox);
  return d->DOption;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSpinBox::setDecimalsOption(QBirchDoubleSpinBox::DecimalsOptions option)
{
  Q_D(QBirchDoubleSpinBox);
  if (d->Mode == QBirchDoubleSpinBox::SetIfDifferent && option == d->DOption)
    {
    return;
    }

  d->DOption = option;
  this->setValueAlways(this->value());
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
void QBirchDoubleSpinBox::setInvertedControls(bool invertedControls)
{
  Q_D(QBirchDoubleSpinBox);
  d->InvertedControls = invertedControls;
  d->SpinBox->setInvertedControls(d->InvertedControls);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
bool QBirchDoubleSpinBox::invertedControls() const
{
  Q_D(const QBirchDoubleSpinBox);
  return d->InvertedControls;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
void QBirchDoubleSpinBox
::setSizeHintPolicy(QBirchDoubleSpinBox::SizeHintPolicy newSizeHintPolicy)
{
  Q_D(QBirchDoubleSpinBox);
  if (d->Mode == QBirchDoubleSpinBox::SetIfDifferent
      && newSizeHintPolicy == d->SizeHintPolicy)
    {
    return;
    }
  d->SizeHintPolicy = newSizeHintPolicy;
  d->CachedSizeHint = QSize();
  this->updateGeometry();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
QBirchDoubleSpinBox::SizeHintPolicy QBirchDoubleSpinBox::sizeHintPolicy() const
{
  Q_D(const QBirchDoubleSpinBox);
  return d->SizeHintPolicy;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
QSize QBirchDoubleSpinBox::sizeHint() const
{
  Q_D(const QBirchDoubleSpinBox);
  if (d->SizeHintPolicy == QBirchDoubleSpinBox::SizeHintByMinMax)
    {
    return this->Superclass::sizeHint();
    }
  if (!d->CachedSizeHint.isEmpty())
    {
    return d->CachedSizeHint;
    }

  QSize newSizeHint;
  newSizeHint.setHeight(this->lineEdit()->sizeHint().height());

  QString extraString = " "; // give some room
  QString s = this->text() + extraString;
  s.truncate(18);
  int extraWidth = 2; // cursor width

  this->ensurePolished(); // ensure we are using the right font
  const QFontMetrics fm(this->fontMetrics());
  newSizeHint.setWidth(fm.width(s + extraString) + extraWidth);

  QStyleOptionSpinBox opt;
  d->SpinBox->initStyleOptionSpinBox(&opt);

  QSize extraSize(35, 6);
  opt.rect.setSize(newSizeHint + extraSize);
  extraSize += newSizeHint - this->style()->subControlRect(
    QStyle::CC_SpinBox, &opt,
    QStyle::SC_SpinBoxEditField, this).size();
  // Converging size hint...
  opt.rect.setSize(newSizeHint + extraSize);
  extraSize += newSizeHint - this->style()->subControlRect(
    QStyle::CC_SpinBox, &opt,
    QStyle::SC_SpinBoxEditField, this).size();
  newSizeHint += extraSize;

  opt.rect = this->rect();
  d->CachedSizeHint = this->style()->sizeFromContents(
    QStyle::CT_SpinBox, &opt, newSizeHint, this)
    .expandedTo(QApplication::globalStrut());
  return d->CachedSizeHint;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
QSize QBirchDoubleSpinBox::minimumSizeHint() const
{
  // For some reasons, Superclass::minimumSizeHint() returns the spinbox
  // sizeHint()
  return this->spinBox()->minimumSizeHint();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchDoubleSpinBox::keyPressEvent(QKeyEvent* event)
{
  Q_D(QBirchDoubleSpinBox);
  const bool accept = this->eventFilter(d->SpinBox, event);
  event->setAccepted(accept);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchDoubleSpinBox::eventFilter(QObject* obj, QEvent* event)
{
  Q_D(QBirchDoubleSpinBox);
  if (d->DOption & QBirchDoubleSpinBox::DecimalsByShortcuts &&
    obj == d->SpinBox && event->type() == QEvent::KeyPress)
    {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
    Q_ASSERT(keyEvent);
    int newDecimals = -1;
    if (keyEvent->modifiers() & Qt::ControlModifier)
      {
      if (keyEvent->key() == Qt::Key_Plus
        || keyEvent->key() == Qt::Key_Equal)
        {
        newDecimals = this->decimals() + 1;
        }
      else if (keyEvent->key() == Qt::Key_Minus)
        {
        newDecimals = this->decimals() - 1;
        }
      else if (keyEvent->key() == Qt::Key_0)
        {
        newDecimals = d->DefaultDecimals;
        }
      }
    if (newDecimals != -1)
      {
      double currentValue = this->value();
      // increasing the number of decimals should restore lost precision
      d->setValue(currentValue, newDecimals);
      return true;
      }
    return QWidget::eventFilter(obj, event);
    }
  else
    {
    // pass the event on to the parent class
    return QWidget::eventFilter(obj, event);
    }
}
