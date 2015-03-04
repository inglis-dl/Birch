/*=========================================================================

  Program:  Birch (A Simple Image Viewer)
  Module:   vtkBirchQDicomTagWidget.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

#ifndef __vtkBirchQDicomTagWidget_h
#define __vtkBirchQDicomTagWidget_h

#include <QWidget>

class vtkBirchQDicomTagWidgetPrivate;

class vtkBirchQDicomTagWidget : public QWidget
{
  Q_OBJECT

public:
  typedef QWidget Superclass;
  vtkBirchQDicomTagWidget( QWidget* parent = 0 );
  virtual ~vtkBirchQDicomTagWidget();

public Q_SLOTS:
  virtual void updateTableWidget( const std::string& fileName );

protected:
  QScopedPointer<vtkBirchQDicomTagWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(vtkBirchQDicomTagWidget);
  Q_DISABLE_COPY(vtkBirchQDicomTagWidget);
};

#endif
