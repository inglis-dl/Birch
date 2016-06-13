/*=========================================================================

  Program:  Birch
  Module:   QBirchDicomTagWidget.h
  Language: C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QBirchDicomTagWidget_h
#define __QBirchDicomTagWidget_h

// Qt includes
#include <QWidget>

class QBirchDicomTagWidgetPrivate;

class QBirchDicomTagWidget : public QWidget
{
  Q_OBJECT

  public:
    typedef QWidget Superclass;
    explicit QBirchDicomTagWidget(QWidget* parent = 0);
    virtual ~QBirchDicomTagWidget();

  public Q_SLOTS:
    virtual void load(const QString& aFileName);

  protected:
    QScopedPointer<QBirchDicomTagWidgetPrivate> d_ptr;

  private:
    Q_DECLARE_PRIVATE(QBirchDicomTagWidget);
    Q_DISABLE_COPY(QBirchDicomTagWidget);
};

#endif
