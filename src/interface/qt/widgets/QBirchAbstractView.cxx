/*=======================================================================

  Module:    QBirchAbstractView.cxx
  Program:   Birch
  Language:  C++
  Author:    Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QBirchAbstractView.h>
#include <QBirchAbstractView_p.h>

// Qt includes
#include <QVBoxLayout>
#include <QDebug>

// VTK includes
#include <vtkCaptionActor2D.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
// QBirchAbstractViewPrivate methods

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchAbstractViewPrivate::QBirchAbstractViewPrivate(QBirchAbstractView& object)
  : q_ptr(&object)
{
  this->Renderer = vtkSmartPointer<vtkRenderer>::New();
  this->RenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
  this->AxesActor = vtkSmartPointer<vtkCenteredAxesActor>::New();
  this->AxesActor->PickableOff();
  this->AxesActor->DragableOff();
  // Set up axes actor label size to scale with view size
  vtkCaptionActor2D* captionActors[3] =
  {
    this->AxesActor->GetXAxisCaptionActor2D(),
    this->AxesActor->GetYAxisCaptionActor2D(),
    this->AxesActor->GetZAxisCaptionActor2D()
  };
  for (int i = 0; i < 3; ++i)
  {
    captionActors[i]->GetTextActor()->SetTextScaleModeToViewport();
    captionActors[i]->GetTextActor()->SetNonLinearFontScale(0.9, 24);
    captionActors[i]->GetTextActor()->GetTextProperty()->SetFontSize(36);
  }
  this->OrientationMarkerWidget =
    vtkSmartPointer<vtkOrientationMarkerWidget>::New();
  this->OrientationMarkerWidget->SetOrientationMarker(this->AxesActor);
  this->OrientationMarkerWidget->KeyPressActivationOff();
  this->OrientationMarkerWidget->SetViewport(0.8, 0.0, 1.0, 0.2);
  this->axesOverView = true;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchAbstractViewPrivate::init()
{
  Q_Q(QBirchAbstractView);

  this->setParent(q);

  this->VTKWidget = new QVTKWidget;
  q->setLayout(new QVBoxLayout);
  q->layout()->setMargin(0);
  q->layout()->setSpacing(0);
  q->layout()->addWidget(this->VTKWidget);

  this->RenderWindow->AddRenderer(this->Renderer);
  this->VTKWidget->SetRenderWindow(this->RenderWindow);
  this->Renderer->GradientBackgroundOn();
  double color[3] = {0., 0., 0.};
  this->Renderer->SetBackground(color);  // black (lower part of gradient)
  color[2] = 1.;
  this->Renderer->SetBackground2(color);  // blue (upper part of gradient)

  q->setInteractor(this->RenderWindow->GetInteractor());
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+--
void QBirchAbstractViewPrivate::setupAxesWidget()
{
  Q_ASSERT(this->RenderWindow);
  Q_ASSERT(this->Renderer);
  if (this->axesOverView && this->RenderWindow->GetInteractor())
  {
    this->OrientationMarkerWidget->SetDefaultRenderer(this->Renderer);
    this->OrientationMarkerWidget->SetInteractor(
      this->RenderWindow->GetInteractor());
    this->OrientationMarkerWidget->On();
    this->OrientationMarkerWidget->InteractiveOff();
  }
  else
  {
    if (this->OrientationMarkerWidget->GetInteractor())
      this->OrientationMarkerWidget->Off();
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+--
QList<vtkRenderer*> QBirchAbstractViewPrivate::renderers() const
{
  QList<vtkRenderer*> rendererList;
  vtkRendererCollection* rendererCollection =
    this->RenderWindow->GetRenderers();
  vtkCollectionSimpleIterator rendererIterator;
  rendererCollection->InitTraversal(rendererIterator);
  vtkRenderer* renderer;
  while ((renderer = rendererCollection->GetNextRenderer(rendererIterator)))
  {
    rendererList << renderer;
  }
  return rendererList;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+--
// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkRenderer* QBirchAbstractViewPrivate::firstRenderer() const
{
  return static_cast<vtkRenderer*>(
    this->RenderWindow->GetRenderers()->GetItemAsObject(0));
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
// QBirchAbstractView methods
// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchAbstractView::QBirchAbstractView(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new QBirchAbstractViewPrivate(*this))
{
  Q_D(QBirchAbstractView);
  d->init();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchAbstractView::QBirchAbstractView(
  QBirchAbstractViewPrivate* pimpl, QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(pimpl)
{
  // derived classes must call init manually. Calling init() here may results in
  // actions on a derived public class not yet finished to be created
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchAbstractView::~QBirchAbstractView()
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchAbstractView::forceRender()
{
  Q_D(QBirchAbstractView);

  if (!this->isVisible())
  {
    return;
  }
  d->RenderWindow->Render();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkRenderWindow* QBirchAbstractView::renderWindow() const
{
  Q_D(const QBirchAbstractView);
  return d->RenderWindow;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
void QBirchAbstractView::setInteractor(vtkRenderWindowInteractor* newInteractor)
{
  Q_D(QBirchAbstractView);
  if (newInteractor != d->RenderWindow->GetInteractor())
    d->RenderWindow->SetInteractor(newInteractor);

  if (newInteractor != d->OrientationMarkerWidget->GetInteractor())
    d->OrientationMarkerWidget->SetInteractor(newInteractor);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
vtkRenderWindowInteractor* QBirchAbstractView::interactor() const
{
  Q_D(const QBirchAbstractView);
  return d->RenderWindow->GetInteractor();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
void QBirchAbstractView::setOrientationDisplay(bool display)
{
  Q_D(QBirchAbstractView);
  if (d->OrientationMarkerWidget->GetInteractor())
    d->OrientationMarkerWidget->SetEnabled(display);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
bool QBirchAbstractView::orientationDisplay() const
{
  Q_D(const QBirchAbstractView);
  return d->OrientationMarkerWidget->GetEnabled();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
vtkInteractorObserver* QBirchAbstractView::interactorStyle() const
{
  return this->interactor() ?
    this->interactor()->GetInteractorStyle() : 0;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
QVTKWidget* QBirchAbstractView::VTKWidget() const
{
  Q_D(const QBirchAbstractView);
  return d->VTKWidget;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
QSize QBirchAbstractView::minimumSizeHint() const
{
  // Arbitrary size. 50x50 because smaller seems too small.
  return QSize(50, 50);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
QSize QBirchAbstractView::sizeHint() const
{
  // Arbitrary size. 300x300 is the default vtkRenderWindow size.
  return QSize(300, 300);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
bool QBirchAbstractView::hasHeightForWidth() const
{
  return true;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
int QBirchAbstractView::heightForWidth(int width) const
{
  // typically VTK render window tend to be square...
  return width;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
vtkRenderer* QBirchAbstractView::renderer()
{
  Q_D(const QBirchAbstractView);
  return d->firstRenderer();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
void QBirchAbstractView::setBackgroundColor(const QColor& qcolor)
{
  Q_D(QBirchAbstractView);
  double color[3];
  color[0] = qcolor.redF();
  color[1] = qcolor.greenF();
  color[2] = qcolor.blueF();
  foreach(vtkRenderer* renderer, d->renderers())
  {
    renderer->SetBackground(color);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
QColor QBirchAbstractView::backgroundColor() const
{
  Q_D(const QBirchAbstractView);
  vtkRenderer* firstRenderer = d->firstRenderer();
  return firstRenderer ? QColor::fromRgbF(firstRenderer->GetBackground()[0],
                                          firstRenderer->GetBackground()[1],
                                          firstRenderer->GetBackground()[2])
                       : QColor();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
void QBirchAbstractView::setForegroundColor(const QColor& qcolor)
{
  Q_D(QBirchAbstractView);
  double color[3];
  color[0] = qcolor.redF();
  color[1] = qcolor.greenF();
  color[2] = qcolor.blueF();
  foreach(vtkRenderer* renderer, d->renderers())
  {
    renderer->SetBackground2(color);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
QColor QBirchAbstractView::foregroundColor() const
{
  Q_D(const QBirchAbstractView);
  vtkRenderer* firstRenderer = d->firstRenderer();
  return firstRenderer ? QColor::fromRgbF(firstRenderer->GetBackground2()[0],
                                          firstRenderer->GetBackground2()[1],
                                          firstRenderer->GetBackground2()[2])
                       : QColor();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
void QBirchAbstractView::setGradientBackground(bool enable)
{
  Q_D(QBirchAbstractView);
  foreach(vtkRenderer* renderer, d->renderers())
  {
    renderer->SetGradientBackground(enable);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
bool QBirchAbstractView::gradientBackground() const
{
  Q_D(const QBirchAbstractView);
  vtkRenderer* firstRenderer = d->firstRenderer();
  return firstRenderer ? firstRenderer->GetGradientBackground() : false;
}
