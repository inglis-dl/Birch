/*=======================================================================

  Module:    QBirchSliceView_p.h
  Program:   Birch
  Language:  C++
  Author:    Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QBirchAbstractView_p_h
#define __QBirchAbstractView_p_h

// Qt includes
#include <QObject>

// Birch includes
#include <QBirchAbstractView.h>

// VTK includes
#include <vtkCenteredAxesActor.h>
#include <QVTKWidget.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

class QBirchAbstractViewPrivate : public QObject
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(QBirchAbstractView);

  protected:
    QBirchAbstractView* const q_ptr;

  public:
    explicit QBirchAbstractViewPrivate(QBirchAbstractView& object);

    virtual void init();
    virtual void setupAxesWidget();

    bool axesOverView;

    QList<vtkRenderer*> renderers()const;
    vtkRenderer* firstRenderer()const;

    QVTKWidget*                                 VTKWidget;
    vtkSmartPointer<vtkRenderer>                Renderer;
    vtkSmartPointer<vtkRenderWindow>            RenderWindow;
    vtkSmartPointer<vtkCenteredAxesActor>       AxesActor;
    vtkSmartPointer<vtkOrientationMarkerWidget> OrientationMarkerWidget;
};

#endif
