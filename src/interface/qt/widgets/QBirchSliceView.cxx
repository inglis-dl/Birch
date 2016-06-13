/*=======================================================================

  Module:    QBirchSliceView.cxx
  Program:   Birch
  Language:  C++
  Author:    Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QBirchSliceView.h>
#include <QBirchSliceView_p.h>

// Alder includes
#include <vtkImageDataReader.h>
#include <vtkImageDataWriter.h>

// Qt includes
#include <QDebug>

// VTK includes
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkDataArray.h>
#include <vtkEventForwarderCommand.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageClip.h>
#include <vtkImageData.h>
#include <vtkImagePermute.h>
#include <vtkImageProperty.h>
#include <vtkImageSinusoidSource.h>
#include <vtkMedicalImageProperties.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkTextProperty.h>

// C++ includes
#include <map>
#include <string>

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
// QBirchSliceViewPrivate methods
// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
class QBirchWindowLevelCallback : public vtkCommand
{
  public:
    static QBirchWindowLevelCallback* New()
      { return new QBirchWindowLevelCallback; }

    void Execute(vtkObject* vtkNotUsed(caller), unsigned long event,
                  void* vtkNotUsed(callData))
    {
      if (!this->pimpl) return;
      switch (event)
      {
        case vtkCommand::ResetWindowLevelEvent:
          this->pimpl->doResetWindowLevelEvent();
          break;
        case vtkCommand::StartWindowLevelEvent:
          this->pimpl->doStartWindowLevelEvent();
          break;
        case vtkCommand::WindowLevelEvent:
          this->pimpl->doWindowLevelEvent();
          break;
        case vtkCommand::EndWindowLevelEvent:
          break;
      }
    }

    QBirchWindowLevelCallback():pimpl(0){}
    ~QBirchWindowLevelCallback(){ this->pimpl = 0; }
    QBirchSliceViewPrivate* pimpl;
};

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
class QBirchAnnotationUpdateCallback : public vtkCommand
{
  public:
    static QBirchAnnotationUpdateCallback* New()
      { return new QBirchAnnotationUpdateCallback; }

    void Execute(vtkObject* caller, unsigned long vtkNotUsed(event),
                  void* vtkNotUsed(callData))
    {
      vtkImageCoordinateWidget* self =
        reinterpret_cast<vtkImageCoordinateWidget*>(caller);
      if (!self || !this->pimpl) return;

      this->pimpl->CornerAnnotation->SetText(0, self->GetMessageString());
      this->pimpl->RenderWindow->Render();
    }

    QBirchAnnotationUpdateCallback():pimpl(0){}
    ~QBirchAnnotationUpdateCallback(){ this->pimpl = 0; }
    QBirchSliceViewPrivate* pimpl;
};

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
class QBirchOrientationCallback : public vtkCommand {
  public:
    static QBirchOrientationCallback* New()
      { return new QBirchOrientationCallback; }

    void Execute(vtkObject* vtkNotUsed(caller), unsigned long vtkNotUsed(event),
      void* vtkNotUsed(callData))
    {
       vtkRenderWindowInteractor* rwi =
         this->pimpl->RenderWindow->GetInteractor();
       if (3 != this->pimpl->dimensionality) return;
       if (rwi)
       {
        switch (rwi->GetKeyCode())
        {
          case 'x':
          case 'X':
            {
            this->pimpl->setOrientation(QBirchSliceView::OrientationYZ);
            }
            break;

          case 'y':
          case 'Y':
            {
            this->pimpl->setOrientation(QBirchSliceView::OrientationXZ);
            }
            break;

          case 'z':
          case 'Z':
            {
            this->pimpl->setOrientation(QBirchSliceView::OrientationXY);
            }
            break;
        }
      }
    }

    QBirchOrientationCallback():pimpl(0){}
    ~QBirchOrientationCallback(){ this->pimpl = 0; }
    QBirchSliceViewPrivate* pimpl;
};

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchSliceViewPrivate::QBirchSliceViewPrivate(QBirchSliceView& object)
  : QBirchAbstractViewPrivate(object)
{
  this->ImageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
  this->ImageSliceMapper->SliceFacesCameraOff();
  this->ImageSliceMapper->SliceAtFocalPointOff();
  this->ImageSliceMapper->BorderOff();
  this->ImageSliceMapper->CroppingOff();

  this->ImageSlice = vtkSmartPointer<vtkImageSlice>::New();
  vtkImageProperty* property = this->ImageSlice->GetProperty();
  property->SetInterpolationTypeToNearest();
  this->interpolation = VTK_NEAREST_INTERPOLATION;
  this->ImageSlice->SetMapper(this->ImageSliceMapper);

  this->WindowLevel = vtkSmartPointer<vtkImageWindowLevel>::New();
  this->CoordinateWidget = vtkSmartPointer<vtkImageCoordinateWidget>::New();

  this->CornerAnnotation = vtkSmartPointer<vtkCustomCornerAnnotation>::New();
  this->CornerAnnotation->VisibilityOff();
  this->CornerAnnotation->SetMaximumFontSize(15);
  this->CornerAnnotation->SetLinearFontScaleFactor(100);
  this->CornerAnnotation->SetMaximumLineHeight(0.07);
  this->CornerAnnotation->SetText(3, "<window>\n<level>");

  this->originalColorWindow = 255.;
  this->originalColorLevel = 127.5;
  this->initialColorWindow = 255.;
  this->initialColorLevel = 127.5;
  this->orientation = QBirchSliceView::OrientationXY;
  this->dimensionality = 0;
  this->frameRate = 25;
  this->annotateOverView = true;
  this->cursorOverView = true;

  this->InteractorStyle =
    vtkSmartPointer<vtkCustomInteractorStyleImage>::New();

  this->slice = 0;
  double p[3] = {1.0, -1.0, 1.0};
  for (int i = 0; i < 3; ++i)
  {
    for (int j = 0; j < 3; ++j)
    {
      this->cameraPosition[i][j] = (i == j ? p[j] : 0.0);
      this->cameraFocalPoint[i][j] = 0.0;
      this->cameraViewUp[i][j] = 0.0;
    }
    this->cameraViewUp[i][(i != 2 ? 2 : 1)] = 1.0;
    this->cameraParallelScale[i] = 1.0;
    this->cameraDistance[i] = 1.0;
    this->cameraClippingRange[i][0] = 0.001;
    this->cameraClippingRange[i][1] = 1000.;
    this->lastSlice[i] = 0;
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int* QBirchSliceViewPrivate::sliceRange()
{
  vtkImageData* input =
    vtkImageData::SafeDownCast(this->WindowLevel->GetInput());
  if (input)
  {
    this->WindowLevel->Update();
    return input->GetExtent() + 2*this->orientation;
  }
  return 0;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int QBirchSliceViewPrivate::sliceMin()
{
  int* range = this->sliceRange();
  if (range)
  {
    return range[0];
  }
  return 0;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int QBirchSliceViewPrivate::sliceMax()
{
  int* range = this->sliceRange();
  if (range)
  {
    return range[1];
  }
  return 0;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceViewPrivate::setSlice(const int& _slice)
{
  int* range = this->sliceRange();
  int slice = _slice;
  if (range)
  {
    if (slice < range[0])
    {
      slice = range[0];
    }
    else if (slice > range[1])
    {
      slice = range[1];
    }
  }

  this->recordCameraView();
  this->lastSlice[this->orientation] = this->slice;
  this->slice = slice;

  this->ImageSliceMapper->SetSliceNumber(this->slice);
  this->ImageSliceMapper->Update();

  this->computeCameraFromCurrentSlice();
  this->updateCameraView();
  this->RenderWindow->Render();

  if (this->cursorOverView && this->annotateOverView)
  {
    this->CoordinateWidget->UpdateMessageString();
    this->CornerAnnotation->SetText(
      0, this->CoordinateWidget->GetMessageString());
    this->CornerAnnotation->Modified();
    this->RenderWindow->Render();
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceViewPrivate::setImageData(vtkImageData* input)
{
  this->setupRendering(false);
  this->dimensionality = 0;
  this->frameRate = 25;
  if (input)
  {
    int dims[3];
    input->GetDimensions(dims);
    this->dimensionality = 3;
    for (int i = 0; i < 3; ++i)
    {
      if (1 == dims[i])
      {
        this->dimensionality = 2;
        break;
      }
    }
    if (2 == this->dimensionality &&
        QBirchSliceView::OrientationXY != this->orientation)
      this->orientation = QBirchSliceView::OrientationXY;

    bool initCamera = true;
    vtkImageData* lastInput =
      vtkImageData::SafeDownCast(this->WindowLevel->GetInput());
    if (lastInput)
    {
      double* lastSpacing = lastInput->GetSpacing();
      double* spacing = input->GetSpacing();
      double* lastOrigin = lastInput->GetOrigin();
      double* origin = input->GetOrigin();
      int* lastExtent = lastInput->GetExtent();
      int* extent = input->GetExtent();
      initCamera = false;
      for (int i = 0; i < 3; ++i)
      {
        if (lastSpacing[i]    != spacing[i]  ||
            lastOrigin[i]     != origin[i]   ||
            lastExtent[2*i]   != extent[2*i] ||
            lastExtent[2*i+1] != extent[2*i+1])
        {
          initCamera = true;
          break;
        }
      }
    }

    this->WindowLevel->SetInputData(input);
    this->ImageSliceMapper->SetInputConnection(
      this->WindowLevel->GetOutputPort());
    int components = input->GetNumberOfScalarComponents();
    switch (components)
    {
      case 1:
        this->WindowLevel->SetActiveComponent(0);
        this->WindowLevel->PassAlphaToOutputOff();
        this->WindowLevel->SetOutputFormatToLuminance();
        break;
      case 2:
      case 3:
        this->WindowLevel->SetOutputFormatToRGB();
        this->WindowLevel->PassAlphaToOutputOff();
        break;
      case 4:
        this->WindowLevel->SetOutputFormatToRGBA();
        this->WindowLevel->PassAlphaToOutputOn();
        break;
    }
    this->WindowLevel->Modified();
    this->setupRendering(true, initCamera);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceViewPrivate::setupRendering(
  const bool& display, const bool& initCamera)
{
  vtkImageData* input =
    vtkImageData::SafeDownCast(this->WindowLevel->GetInput());
  if (display && input)
  {
    this->RenderWindow->GetInteractor()->SetInteractorStyle(
      this->InteractorStyle);
    if (3 == this->dimensionality)
      this->InteractorStyle->SetInteractionModeToImage3D();
    else
      this->InteractorStyle->SetInteractionModeToImage2D();

    vtkSmartPointer<QBirchWindowLevelCallback> cbk =
      vtkSmartPointer<QBirchWindowLevelCallback>::New();
    cbk->pimpl = this;

    this->callbackTags[
      vtkCommand::GetStringFromEventId(vtkCommand::StartWindowLevelEvent)] =
        this->InteractorStyle->AddObserver(
          vtkCommand::StartWindowLevelEvent, cbk);

    this->callbackTags[
      vtkCommand::GetStringFromEventId(vtkCommand::WindowLevelEvent)] =
        this->InteractorStyle->AddObserver(
          vtkCommand::WindowLevelEvent, cbk);

    this->callbackTags[
      vtkCommand::GetStringFromEventId(vtkCommand::EndWindowLevelEvent)] =
        this->InteractorStyle->AddObserver(
          vtkCommand::EndWindowLevelEvent, cbk);

    this->callbackTags[
      vtkCommand::GetStringFromEventId(vtkCommand::ResetWindowLevelEvent)] =
        this->InteractorStyle->AddObserver(
          vtkCommand::ResetWindowLevelEvent, cbk);

    vtkSmartPointer<QBirchOrientationCallback> charCbk =
      vtkSmartPointer<QBirchOrientationCallback>::New();
    charCbk->pimpl = this;
    this->callbackTags[
      vtkCommand::GetStringFromEventId(vtkCommand::CharEvent)] =
        this->RenderWindow->GetInteractor()->AddObserver(
          vtkCommand::CharEvent, charCbk);

    this->Renderer->AddViewProp(this->ImageSlice);
    this->Renderer->GetActiveCamera()->ParallelProjectionOn();

    this->setupCornerAnnotation();
    this->setupCoordinateWidget();
    this->setupAxesWidget();

    this->initializeWindowLevel();
    if (initCamera)
      this->initializeCameraViews();

    this->setSlice(this->lastSlice[this->orientation]);
  }
  else
  {
    std::map<std::string, unsigned long>::iterator it;
    it = this->callbackTags.find(
      vtkCommand::GetStringFromEventId(vtkCommand::StartWindowLevelEvent));
    if (it != this->callbackTags.end())
    {
      this->InteractorStyle->RemoveObserver(it->second);
      this->callbackTags.erase(it);
    }
    it = this->callbackTags.find(
      vtkCommand::GetStringFromEventId(vtkCommand::WindowLevelEvent));
    if (it != this->callbackTags.end())
    {
      this->InteractorStyle->RemoveObserver(it->second);
      this->callbackTags.erase(it);
    }
    it = this->callbackTags.find(
      vtkCommand::GetStringFromEventId(vtkCommand::EndWindowLevelEvent));
    if (it != this->callbackTags.end())
    {
      this->InteractorStyle->RemoveObserver(it->second);
      this->callbackTags.erase(it);
    }
    it = this->callbackTags.find(
      vtkCommand::GetStringFromEventId(vtkCommand::ResetWindowLevelEvent));
    if (it != this->callbackTags.end())
    {
      this->InteractorStyle->RemoveObserver(it->second);
      this->callbackTags.erase(it);
    }
    it = this->callbackTags.find(
      vtkCommand::GetStringFromEventId(vtkCommand::CharEvent));
    if (it != this->callbackTags.end())
    {
      this->RenderWindow->GetInteractor()->RemoveObserver(it->second);
      this->callbackTags.erase(it);
    }

    bool oldcursorOverView = this->cursorOverView;
    bool oldannotateOverView = this->annotateOverView;
    bool oldaxesOverView = this->axesOverView;

    this->cursorOverView = false;
    this->annotateOverView = false;
    this->axesOverView = false;

    this->setupCornerAnnotation();
    this->setupCoordinateWidget();
    this->setupAxesWidget();

    this->cursorOverView = oldcursorOverView;
    this->annotateOverView = oldannotateOverView;
    this->axesOverView = oldcursorOverView;

    this->Renderer->RemoveViewProp(this->ImageSlice);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceViewPrivate::doStartWindowLevelEvent()
{
  this->initialColorWindow = this->WindowLevel->GetWindow();
  this->initialColorLevel = this->WindowLevel->GetLevel();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceViewPrivate::doResetWindowLevelEvent()
{
  this->setColorWindowLevel(
    this->originalColorWindow, this->originalColorLevel);
  this->RenderWindow->Render();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceViewPrivate::doWindowLevelEvent()
{
  if (!this->InteractorStyle) return;

  int* size = this->RenderWindow->GetSize();
  double window = this->initialColorWindow;
  double level = this->initialColorLevel;

  // Compute normalized delta

  double dx = 4.0 *
    (this->InteractorStyle->GetWindowLevelCurrentPosition()[0] -
      this->InteractorStyle->GetWindowLevelStartPosition()[0]) / size[0];
  double dy = 4.0 *
    (this->InteractorStyle->GetWindowLevelStartPosition()[1] -
      this->InteractorStyle->GetWindowLevelCurrentPosition()[1]) / size[1];

  // Scale by current values

  if (fabs(window) > 0.01)
  {
    dx = dx * window;
  }
  else
  {
    dx = dx * (window < 0 ? -0.01 : 0.01);
  }
  if (fabs(level) > 0.01)
  {
    dy = dy * level;
  }
  else
  {
    dy = dy * (level < 0 ? -0.01 : 0.01);
  }

  // Abs so that direction does not flip

  if (window < 0.0)
  {
    dx = -1 * dx;
  }
  if (level < 0.0)
  {
    dy = -1 * dy;
  }

  // Compute new window level

  double newWindow = dx + window;
  double newLevel;

  newLevel = level - dy;

  if (fabs(newWindow) < 0.01)
  {
    newWindow = 0.01 * (newWindow < 0 ? -1 : 1);
  }
  if (fabs(newLevel) < 0.01)
  {
    newLevel = 0.01 * (newLevel < 0 ? -1 : 1);
  }

  this->setColorWindowLevel(newWindow, newLevel);
  this->RenderWindow->Render();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceViewPrivate::initializeCameraViews()
{
  vtkImageData* input =
    vtkImageData::SafeDownCast(this->WindowLevel->GetInput());
  if (!input)
    return;
  this->WindowLevel->Update();
  double* origin = input->GetOrigin();
  double* spacing = input->GetSpacing();
  int* extent = input->GetExtent();
  int u, v;
  double fpt[3];
  double pos[3];

  for (int w = 0; w < 3; ++w)
  {
    double vup[3] = {0.0, 0.0, 0.0};
    double vpn[3] = {0.0, 0.0, 0.0};
    switch (w)
    {
      case 0: u = 1; v = 2; break;  // YZ
      case 1: u = 0; v = 2; break;  // XZ
      case 2: u = 0; v = 1; break;  // XY
    }
    vup[v] = 1.0;
    vpn[w] = 1 == w ? -1.0 : 1.0;

    // compute the bounds for the first slice of each orientation
    double bounds[6];
    bounds[2*u]   = origin[u] + spacing[u]*extent[2*u];
    bounds[2*u+1] = origin[u] + spacing[u]*extent[2*u+1];
    bounds[2*v]   = origin[v] + spacing[v]*extent[2*v];
    bounds[2*v+1] = origin[v] + spacing[v]*extent[2*v+1];
    bounds[2*w]   = origin[w] + spacing[w]*extent[2*w];
    bounds[2*w+1] = bounds[2*w];

    fpt[u] = pos[u] = origin[u] + 0.5*spacing[u]*(extent[2*u] + extent[2*u+1]);
    fpt[v] = pos[v] = origin[v] + 0.5*spacing[v]*(extent[2*v] + extent[2*v+1]);
    fpt[w] = origin[w] + spacing[w] * (1 == w ? extent[2*w+1] : extent[2*w]);
    if (3 == this->dimensionality)
      pos[w] = fpt[w] + vpn[w]*spacing[w]*(extent[2*w+1] - extent[2*w]);
    else
      pos[w] = fpt[w] + vpn[w]*spacing[w];

    vtkCamera * camera = this->Renderer->GetActiveCamera();
    camera->SetFocalPoint(fpt);
    camera->SetPosition(pos);
    camera->SetViewUp(vup);

    this->Renderer->ResetCamera(bounds);
    this->recordCameraView(w);

    this->lastSlice[w] = extent[2*w];
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceViewPrivate::recordCameraView(const int& specified)
{
  vtkCamera* camera = this->Renderer->GetActiveCamera();
  if (camera)
  {
    double pos[3];
    double fpt[3];
    double v[3];
    camera->GetPosition(pos);
    camera->GetFocalPoint(fpt);
    camera->GetViewUp(v);
    int w = -1 == specified ? this->orientation : specified;
    for (int i = 0; i < 3; ++i)
    {
      this->cameraPosition[w][i]   = pos[i];
      this->cameraFocalPoint[w][i] = fpt[i];
      this->cameraViewUp[w][i]     = v[i];
    }
    this->cameraParallelScale[w]    = camera->GetParallelScale();
    this->cameraDistance[w]         = camera->GetDistance();
    this->cameraClippingRange[w][0] = camera->GetClippingRange()[0];
    this->cameraClippingRange[w][1] = camera->GetClippingRange()[1];
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceViewPrivate::initializeWindowLevel()
{
  vtkImageData* input =
    vtkImageData::SafeDownCast(this->WindowLevel->GetInput());
  if (!input) return;
  this->WindowLevel->Update();

  int components = input->GetNumberOfScalarComponents();
  double dataMin = input->GetScalarTypeMax();
  double dataMax = input->GetScalarTypeMin();
  if (1 == components)
  {
    dataMin = input->GetScalarRange()[0];
    dataMax = input->GetScalarRange()[1];
  }
  else
  {
    vtkDataArray* data = 0;
    if (input->GetPointData() &&
        0 != (data = input->GetPointData()->GetScalars()))
    {
      for (int i = 0; i < components; ++i)
      {
        double min = data->GetRange(i)[0];
        double max = data->GetRange(i)[1];
        if (dataMin > min) dataMin = min;
        if (dataMax < max) dataMax = max;
      }
    }
  }

  this->originalColorWindow = dataMax - dataMin;
  this->originalColorLevel =  0.5*(dataMin + dataMax);

  double tol = 0.001;
  if (tol > fabs(this->originalColorWindow))
  {
    this->originalColorWindow =
      this->originalColorWindow < 0.0 ? -tol : tol;
  }

  if (tol > fabs(this->originalColorLevel))
  {
    this->originalColorLevel =
      this->originalColorLevel < 0.0 ? -tol : tol;
  }

  // VTK_LUMINANCE is defined in vtkSystemIncludes.h
  if (VTK_LUMINANCE != this->WindowLevel->GetOutputFormat())
  {
    this->originalColorWindow = 255.;
    this->originalColorLevel = 127.5;
  }

  this->setColorWindowLevel(
    this->originalColorWindow, this->originalColorLevel);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceViewPrivate::setColorWindowLevel(
  const double& window, const double& level)
{
  this->WindowLevel->SetWindow(window);
  this->WindowLevel->SetLevel(level);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceViewPrivate::setupCornerAnnotation()
{
  vtkImageData* input =
    vtkImageData::SafeDownCast(this->WindowLevel->GetInput());
  if (this->annotateOverView && input)
  {
    this->CornerAnnotation->SetImageSlice(this->ImageSlice);
    this->CornerAnnotation->SetWindowLevel(this->WindowLevel);
    this->CornerAnnotation->VisibilityOn();
    this->Renderer->AddViewProp(this->CornerAnnotation);

    if (3 == this->dimensionality)
      this->CornerAnnotation->SetText(2, "<slice_and_max>");
    else
      this->CornerAnnotation->SetText(2, "");
  }
  else
  {
    this->CornerAnnotation->VisibilityOff();
    this->Renderer->RemoveViewProp(this->CornerAnnotation);
    this->CornerAnnotation->SetImageSlice(0);
    this->CornerAnnotation->SetWindowLevel(0);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceViewPrivate::updateCameraView()
{
  vtkCamera* camera = this->Renderer->GetActiveCamera();
  if (camera)
  {
    double pos[3];
    double fpt[3];
    double v[3];
    int w = this->orientation;
    for (int i = 0; i < 3; ++i)
    {
      pos[i] = this->cameraPosition[w][i];
      fpt[i] = this->cameraFocalPoint[w][i];
      v[i]   = this->cameraViewUp[w][i];
    }
    camera->SetViewUp(v);
    camera->SetFocalPoint(fpt);
    camera->SetPosition(pos);
    camera->SetParallelScale(this->cameraParallelScale[w]);
    camera->SetClippingRange(this->cameraClippingRange[w]);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceViewPrivate::computeCameraFromCurrentSlice(
  const bool& useCamera)
{
  vtkImageData* input =
    vtkImageData::SafeDownCast(this->WindowLevel->GetInput());
  if (!input) return;

  vtkCamera* camera = this->Renderer->GetActiveCamera();
  if (camera)
  {
    int u, v, w = this->orientation;
    switch (w)
    {
      case 0: u = 1; v = 2; break;
      case 1: u = 0; v = 2; break;
      case 2: u = 0; v = 1; break;
    }

    double* origin = input->GetOrigin();
    double* spacing = input->GetSpacing();
    int* extent = input->GetExtent();
    double fpt[3];
    fpt[u] = origin[u] + 0.5*spacing[u]*(extent[2*u] + extent[2*u+1]);
    fpt[v] = origin[v] + 0.5*spacing[v]*(extent[2*v] + extent[2*v+1]);
    fpt[w] = origin[w] + spacing[w]*this->slice;
    if (useCamera)
    {
      double* vpn = camera->GetViewPlaneNormal();
      double d = camera->GetDistance();
      for (int i = 0; i < 3; ++i)
      {
        this->cameraFocalPoint[w][i] = fpt[i];
        this->cameraPosition[w][i] = fpt[i] + d*vpn[i];
      }
      this->cameraDistance[w] = d;
    }
    else
    {
      double pos[3];
      double vpn[3] = {0.0, 0.0, 0.0};
      vpn[w] = 1 == w ? -1.0 : 1.0;
      pos[u] = fpt[u];
      pos[v] = fpt[v];
      pos[w] = fpt[w] + this->cameraDistance[w]*vpn[w];
      for (int i = 0; i < 3; ++i)
      {
        this->cameraFocalPoint[w][i] = fpt[i];
        this->cameraPosition[w][i] = pos[i];
      }
    }
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceViewPrivate::setOrientation(
  const QBirchSliceView::Orientation& _orientation)
{
  if (_orientation == this->orientation) return;

  this->recordCameraView();
  this->lastSlice[this->orientation] = this->slice;
  this->orientation = _orientation;
  this->slice = this->lastSlice[this->orientation];

  this->ImageSliceMapper->SetOrientation(this->orientation);
  this->ImageSliceMapper->SetSliceNumber(this->slice);
  this->ImageSliceMapper->Update();

  this->computeCameraFromCurrentSlice(false);
  this->updateCameraView();
  this->RenderWindow->Render();

  if (this->annotateOverView && this->CoordinateWidget->GetEnabled()
      && this->CornerAnnotation->GetVisibility())
  {
    this->CoordinateWidget->UpdateMessageString();
    this->CornerAnnotation->SetText(
      0, this->CoordinateWidget->GetMessageString());
    this->CornerAnnotation->Modified();
    this->RenderWindow->Render();
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceViewPrivate::setInterpolation(const int& newInterpolation)
{
  if (newInterpolation != this->interpolation &&
      (VTK_NEAREST_INTERPOLATION == newInterpolation ||
       VTK_LINEAR_INTERPOLATION == newInterpolation))
  {
    this->interpolation = newInterpolation;
    this->ImageSlice->GetProperty()->SetInterpolationType(this->interpolation);
    this->CoordinateWidget->SetCursoringMode(
      VTK_NEAREST_INTERPOLATION == this->interpolation ?
      vtkImageCoordinateWidget::Discrete :
      vtkImageCoordinateWidget::Continuous);
    this->RenderWindow->Render();
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceViewPrivate::setupCoordinateWidget()
{
  vtkImageData* image =
    vtkImageData::SafeDownCast(this->WindowLevel->GetInput());
  if (this->cursorOverView && image)
  {
    this->CoordinateWidget->SetDefaultRenderer(this->Renderer);
    this->CoordinateWidget->SetInteractor(
      this->RenderWindow->GetInteractor());
    this->CoordinateWidget->SetInputData(image);
    this->CoordinateWidget->AddViewProp(this->ImageSlice);

    if (this->annotateOverView && this->CornerAnnotation->GetVisibility())
    {
      vtkSmartPointer<QBirchAnnotationUpdateCallback> cbk =
        vtkSmartPointer<QBirchAnnotationUpdateCallback>::New();
      cbk->pimpl = this;
      this->callbackTags[
        vtkCommand::GetStringFromEventId(vtkCommand::InteractionEvent)] =
          this->CoordinateWidget->AddObserver(
            vtkCommand::InteractionEvent, cbk);
    }

    this->CoordinateWidget->On();
  }
  else
  {
    if (this->CoordinateWidget->GetEnabled())
      this->CoordinateWidget->Off();
    this->CoordinateWidget->RemoveAllProps();
    this->CoordinateWidget->SetInputData(0);
    std::map<std::string, unsigned long>::iterator it;
    it = this->callbackTags.find(
      vtkCommand::GetStringFromEventId(vtkCommand::InteractionEvent));
    if (it != this->callbackTags.end())
    {
      this->CoordinateWidget->RemoveObserver(it->second);
      this->callbackTags.erase(it);
    }
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceViewPrivate::rotateCamera(const double& angle)
{
  if (this->Renderer)
  {
    this->recordCameraView();
    this->Renderer->GetActiveCamera()->Roll(-angle);
    this->RenderWindow->Render();
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceViewPrivate::flipCameraVertical()
{
  if (this->Renderer)
  {
    this->recordCameraView();
    int w = this->orientation;
    for (int i = 0; i < 3; ++i)
    {
      this->cameraViewUp[w][i] =
        -this->cameraViewUp[w][i];
      this->cameraPosition[w][i] =
        2.0*this->cameraFocalPoint[w][i] -
        this->cameraPosition[w][i];
    }
    this->Renderer->GetActiveCamera()->SetPosition(
      this->cameraPosition[w]);
    this->Renderer->GetActiveCamera()->SetViewUp(
      this->cameraViewUp[w]);
    this->Renderer->ResetCameraClippingRange();
    this->RenderWindow->Render();
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceViewPrivate::flipCameraHorizontal()
{
  if (this->Renderer)
  {
    this->recordCameraView();
    int w = this->orientation;
    for (int i = 0; i < 3; ++i)
    {
      this->cameraPosition[w][i] =
        2.0*this->cameraFocalPoint[w][i] -
        this->cameraPosition[w][i];
    }
    this->Renderer->GetActiveCamera()->SetPosition(
      this->cameraPosition[w]);
    this->Renderer->ResetCameraClippingRange();
    this->RenderWindow->Render();
  }
}

//---------------------------------------------------------------------------
// QBirchSliceView methods

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchSliceView::QBirchSliceView(QWidget* parentWidget)
  : Superclass(new QBirchSliceViewPrivate(*this), parentWidget)
{
  Q_D(QBirchSliceView);
  d->init();
  d->setupRendering(false);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchSliceView::~QBirchSliceView()
{
  Q_D(QBirchSliceView);
  d->setupRendering(false);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchSliceView::hasImageData() const
{
  Q_D(const QBirchSliceView);
  vtkImageData* input = 0;
  return (input = vtkImageData::SafeDownCast(d->WindowLevel->GetInput()));
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceView::invertColorWindowLevel()
{
  Q_D(QBirchSliceView);
  d->WindowLevel->SetWindow(-1.0*this->colorWindow());
  d->RenderWindow->Render();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchSliceView::colorLevel() const
{
  Q_D(const QBirchSliceView);
  return d->WindowLevel->GetLevel();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceView::setColorLevel(double level)
{
  Q_D(QBirchSliceView);
  d->WindowLevel->SetLevel(level);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchSliceView::colorWindow() const
{
  Q_D(const QBirchSliceView);
  return d->WindowLevel->GetWindow();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceView::setColorWindow(double window)
{
  Q_D(QBirchSliceView);
  d->WindowLevel->SetWindow(window);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceView::setOrientation(Orientation orientation)
{
  Q_D(QBirchSliceView);

  if (orientation < QBirchSliceView::OrientationYZ ||
      orientation > QBirchSliceView::OrientationXY)
  {
    return;
  }

  d->setOrientation(orientation);
  Q_EMIT orientationChanged(orientation);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchSliceView::Orientation QBirchSliceView::orientation() const
{
  Q_D(const QBirchSliceView);
  return d->orientation;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceView::setCursorOverView(bool view)
{
  Q_D(QBirchSliceView);
  d->cursorOverView = view;
  d->setupCoordinateWidget();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceView::setAnnotateOverView(bool view)
{
  Q_D(QBirchSliceView);
  d->annotateOverView = view;
  d->setupCornerAnnotation();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchSliceView::cursorOverView() const
{
  Q_D(const QBirchSliceView);
  return d->cursorOverView;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchSliceView::annotateOverView() const
{
  Q_D(const QBirchSliceView);
  return d->annotateOverView;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceView::setSlice(int slice)
{
  Q_D(QBirchSliceView);
  d->setSlice(slice);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int QBirchSliceView::slice() const
{
  Q_D(const QBirchSliceView);
  return d->slice;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int QBirchSliceView::sliceMin()
{
  Q_D(QBirchSliceView);
  return d->sliceMin();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int QBirchSliceView::sliceMax()
{
  Q_D(QBirchSliceView);
  return d->sliceMax();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceView::setInterpolation(int newInterpolation)
{
  Q_D(QBirchSliceView);
  d->setInterpolation(newInterpolation);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int QBirchSliceView::interpolation() const
{
  Q_D(const QBirchSliceView);
  return d->interpolation;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int QBirchSliceView::dimensionality() const
{
  Q_D(const QBirchSliceView);
  return d->dimensionality;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int QBirchSliceView::frameRate() const
{
  Q_D(const QBirchSliceView);
  return d->frameRate;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceView::rotateCamera(double angle)
{
  Q_D(QBirchSliceView);
  d->rotateCamera(angle);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceView::rotateCameraClockwise()
{
  Q_D(QBirchSliceView);
  d->rotateCamera(90.);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceView::rotateCameraCounterClockwise()
{
  Q_D(QBirchSliceView);
  d->rotateCamera(-90.);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceView::flipCameraHorizontal()
{
  Q_D(QBirchSliceView);
  d->flipCameraHorizontal();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceView::flipCameraVertical()
{
  Q_D(QBirchSliceView);
  d->flipCameraVertical();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceView::writeSlice(const QString& fileName)
{
  Q_D(QBirchSliceView);
  vtkImageData* input = vtkImageData::SafeDownCast(d->WindowLevel->GetInput());
  if (input &&
      vtkImageDataWriter::IsValidFileName(fileName.toStdString().c_str()))
  {
    vtkNew<vtkImageDataWriter> writer;
    writer->SetFileName(fileName.toStdString().c_str());
    vtkNew<vtkImageClip> clip;

    int extent[6];
    int u, v, w = d->orientation;
    switch (w)
    {
      case 0: u = 1; v = 2; break;
      case 1: u = 0; v = 2; break;
      case 2: u = 0; v = 1; break;
    }
    int* w_ext = input->GetExtent();
    extent[2*u] = w_ext[2*u];
    extent[2*u+1] = w_ext[2*u+1];
    extent[2*v] = w_ext[2*v];
    extent[2*v+1] = w_ext[2*v+1];
    extent[2*w] = d->slice;
    extent[2*w+1] = d->slice;

    clip->SetOutputWholeExtent(extent);
    clip->ClipDataOn();
    clip->SetInputData(input);
    clip->Update();

    int ec[6];
    clip->GetOutput()->GetExtent(ec);

    int et[] = {0, 0, 0};  // extent translation
    int op[] = {0, 1, 2};  // output axes permutation

    // translate to an x-y plane:
    // 1) change extents via vtkImageChangeInformation
    // 2) permute the axes
    switch (w)
    {
      case 0: et[0] = -ec[0];
              op[0] = 2; op[1] = 0; op[2] = 1;
              break;  // YZ
      case 1: et[1] = -ec[2];
              op[0] = 0; op[1] = 2; op[2] = 1;
              break;  // XZ
      case 2: et[2] = -ec[4];
              break;  // XY
    }

    vtkNew<vtkImageChangeInformation> change;
    change->SetExtentTranslation(et);
    change->SetInputConnection(clip->GetOutputPort());

    vtkNew<vtkImagePermute> permute;
    permute->SetFilteredAxes(op);
    permute->SetInputConnection(change->GetOutputPort());

    writer->SetInputData(permute->GetOutput());
    writer->Write();
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchSliceView::load(
  const QString& fileName, vtkEventForwarderCommand* forward)
{
  Q_D(QBirchSliceView);
  bool success = false;
  if (vtkImageDataReader::IsValidFileName(fileName.toStdString().c_str()))
  {
    vtkNew<vtkImageDataReader> reader;
    reader->SetFileName(fileName.toStdString().c_str());
    if (forward)
    {
      reader->GetReader()->AddObserver(
        vtkCommand::ProgressEvent, forward);
    }
    vtkImageData* image = reader->GetOutput();
    if (image)
    {
      d->setImageData(image);
      if (3 == d->dimensionality)
      {
        vtkMedicalImageProperties* properties =
          reader->GetMedicalImageProperties();
        if (properties && NULL != properties->GetUserDefinedValue("CineRate"))
        {
          std::string s = properties->GetUserDefinedValue("CineRate");
          int rate = vtkVariant(s.c_str()).ToInt();
          d->frameRate = rate;
        }
      }
      emit imageDataChanged();
      success = true;
    }
  }

  return success;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkImageData* QBirchSliceView::imageData()
{
  Q_D(QBirchSliceView);
  return vtkImageData::SafeDownCast(d->WindowLevel->GetInput());
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceView::setImageData(vtkImageData* data)
{
  Q_D(QBirchSliceView);
  d->setImageData(data);
  emit imageDataChanged();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchSliceView::setImageToSinusoid()
{
  Q_D(QBirchSliceView);
  // Create the sinusoid default image like MicroView does
  vtkNew<vtkImageSinusoidSource> sinusoid;
  sinusoid->SetPeriod(32);
  sinusoid->SetPhase(0);
  sinusoid->SetAmplitude(255);
  sinusoid->SetWholeExtent(0, 127, 0, 127, 0, 31);
  sinusoid->SetDirection(0.5, -0.5, 1.0 / sqrt(2.0));
  sinusoid->Update();

  d->setImageData(sinusoid->GetOutput());
  d->setSlice(15);
  emit imageDataChanged();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
void QBirchSliceView::setAnnotationColor(const QColor& qcolor)
{
  Q_D(QBirchSliceView);
  double color[3];
  color[0] = qcolor.redF();
  color[1] = qcolor.greenF();
  color[2] = qcolor.blueF();
  vtkTextProperty* prop = d->CornerAnnotation->GetTextProperty();
  prop->SetColor(color);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
QColor QBirchSliceView::annotationColor() const
{
  Q_D(const QBirchSliceView);
  vtkTextProperty* prop = d->CornerAnnotation->GetTextProperty();
  double color[3];
  prop->GetColor(color);
  return QColor::fromRgbF(color[0], color[1], color[2]);
}
