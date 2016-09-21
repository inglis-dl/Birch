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
#include "QBirchDoubleSpinBox.h"

// Qt includes
#include <QDoubleSpinBox>
#include <QPointer>
class QBirchDoubleSpinBoxPrivate;

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+----
class birchQDoubleSpinBox: public QDoubleSpinBox
{
  Q_OBJECT
  /// This property controls whether decreasing the value by the mouse
  /// button or mouse wheel increases the value of the widget, and inverts the
  /// control similarly in the other way round or not. The property is switched off by
  /// default.
  /// \sa invertedControls(), setInvertedControls()
  Q_PROPERTY(bool invertedControls READ invertedControls WRITE setInvertedControls)
public:
  typedef QDoubleSpinBox Superclass;
  birchQDoubleSpinBox(QBirchDoubleSpinBoxPrivate* pimpl, QWidget* widget);
  void setInvertedControls(bool invertedControls);
  bool invertedControls() const;

  /// Overrides QDoubleSpinBox::stepBy(int) and negates the step number if the
  /// invertedControls property is true.
  virtual void stepBy(int steps);

  /// Expose lineEdit() publicly.
  /// \sa QAbstractSpinBox::lineEdit()
  virtual QLineEdit* lineEdit()const;

  virtual double valueFromText(const QString &text) const;
  virtual QString textFromValue(double value) const;
  virtual int decimalsFromText(const QString &text) const;
  virtual QValidator::State	validate(QString& input, int& pos)const;

  /// Expose publicly QAbstractSpinBox::initStyleOption()
  void initStyleOptionSpinBox(QStyleOptionSpinBox* option);
protected:
  QBirchDoubleSpinBoxPrivate* const d_ptr;

  /// If the invertedControls property is false (by default) then this function
  /// behavesLike QDoubleSpinBox::stepEnabled(). If the property is true then
  /// stepping down is allowed if the value is less then the maximum, and
  /// stepping up is allowed if the value is greater then the minimum.
  virtual StepEnabled stepEnabled () const;

  bool InvertedControls;
private:
  Q_DECLARE_PRIVATE(QBirchDoubleSpinBox);
  Q_DISABLE_COPY(birchQDoubleSpinBox);
};

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+----
class QBirchDoubleSpinBoxPrivate: public QObject
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(QBirchDoubleSpinBox);
protected:
  QBirchDoubleSpinBox* const q_ptr;
public:
  QBirchDoubleSpinBoxPrivate(QBirchDoubleSpinBox& object);

  birchQDoubleSpinBox* SpinBox;
  QBirchDoubleSpinBox::SetMode Mode;
  int DefaultDecimals;
  QBirchDoubleSpinBox::DecimalsOptions DOption;
  bool InvertedControls;
  QBirchDoubleSpinBox::SizeHintPolicy SizeHintPolicy;

  double InputValue;
  double InputRange[2];

  mutable QString CachedText;
  mutable double CachedValue;
  mutable QValidator::State CachedState;
  mutable int CachedDecimals;
  mutable QSize CachedSizeHint;
  bool ForceInputValueUpdate;

  void init();
  /// Compare two double previously rounded according to the number of decimals
  bool compare(double x1, double x2) const;
  /// Return a value rounded with the number of decimals
  double round(double value, int decimals)const;

/// Return a "smart" number of decimals needed to display (in a gui) a floating
/// number. 16 is the max that can be returned, -1 for NaN numbers. When the
/// number of decimals is not obvious, it defaults to defaultDecimals if it is
/// different from -1, 16 otherwise.
/// e.g. significantDecimals(120.01) returns 2
///      significantDecimals(123456.1333333) returns 3
///      significantDecimals(123456.26999999999999996) returns 2
///      significantDecimals(123456.12345678901234567, 3) return 3
  int significantDecimals(double value, int defaultDecimals = -1)const;

  /// Remove prefix and suffix
  QString stripped(const QString& text, int* pos)const;

  /// Return the number of decimals bounded by the allowed min and max number of
  /// decimals.
  /// If -1, returns the current number of decimals.
  int boundDecimals(int decimals)const;
  /// Return the number of decimals to use to display the value.
  /// Note that if DecimalsByValue is not set, the number of decimals to use
  /// is DefaultDecimals.
  int decimalsForValue(double value)const;
  /// Set the number of decimals of the spinbox and emit the signal
  /// No check if they are the same.
  void setDecimals(int dec);
  /// Set value with a specific number of decimals. -1 means the number of
  /// decimals stays the same.
  void setValue(double value, int dec = -1);

  /// Ensure the spinbox text is meaningful.
  /// It is called multiple times when the spinbox line edit is modified,
  /// therefore values are cached.
  double validateAndInterpret(QString &input, int &pos,
                              QValidator::State &state, int &decimals) const;

  void connectSpinBoxValueChanged();
  void disconnectSpinBoxValueChanged();

public Q_SLOTS:
  void editorTextChanged(const QString& text);
  void onValueChanged();
};
