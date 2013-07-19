/*==============================================================================

  Library: MSVTK

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

==============================================================================*/

// Qt includes
#include <QIcon>
#include <QTime>
#include <QTimer>

// Birch includes
#include <vtkBirchQFramePlayerWidget.h>
#include <ui_vtkBirchQFramePlayerWidget.h>

// VTK includes
#include <vtkMedicalImageViewer.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

//------------------------------------------------------------------------------
class vtkBirchQFramePlayerWidgetPrivate : public Ui_vtkBirchQFramePlayerWidget
{
  Q_DECLARE_PUBLIC(vtkBirchQFramePlayerWidget);
protected:
  vtkBirchQFramePlayerWidget* const q_ptr;

  vtkSmartPointer<vtkMedicalImageViewer> viewer;

  double maxFrameRate;                    // Time Playing speed factor.
  QTimer* timer;                          // Timer to process the player.
  QTime realTime;                         // Time to reference the real one.
  QAbstractAnimation::Direction direction;// Sense of direction

public:
  vtkBirchQFramePlayerWidgetPrivate(vtkBirchQFramePlayerWidget& object);
  virtual ~vtkBirchQFramePlayerWidgetPrivate();

  virtual void setupUi(QWidget*);
  virtual void updateUi();

  struct PipelineInfoType
    {
    PipelineInfoType();

    bool isConnected;
    unsigned int numberOfFrames;
    double frameRange[2];
    double currentFrame;

    void printSelf()const;
    double clampTimeInterval(double, double) const; // Transform a frameRate into a time interval
    double validateFrame(double) const;    // Validate a frame
    double nextFrame() const;     // Get the next frame.
    double previousFrame() const; // Get the previous frame.
    };
  PipelineInfoType retrievePipelineInfo();            // Get pipeline information.
  virtual void processRequest(double);                // Request Data and Update
  virtual bool isConnected();                         // Check if the pipeline is ready
  virtual void processRequest(const PipelineInfoType&, double); // Request Data and Update
  virtual void requestData(const PipelineInfoType&, double);    // Request Data by time
  virtual void updateUi(const PipelineInfoType&);               // Update the widget giving pipeline statut
};

//------------------------------------------------------------------------------
// vtkBirchQFramePlayerWidgetPrivate methods

//------------------------------------------------------------------------------
vtkBirchQFramePlayerWidgetPrivate::vtkBirchQFramePlayerWidgetPrivate
(vtkBirchQFramePlayerWidget& object)
  : q_ptr(&object)
{
  this->maxFrameRate = 60;          // 60 FPS by default
}

//------------------------------------------------------------------------------
vtkBirchQFramePlayerWidgetPrivate::~vtkBirchQFramePlayerWidgetPrivate()
{
  this->viewer = NULL;
}

//------------------------------------------------------------------------------
vtkBirchQFramePlayerWidgetPrivate::PipelineInfoType::PipelineInfoType()
  : isConnected(false)
  , numberOfFrames(0)
  , currentFrame(0)
{
  this->frameRange[0] = 0;
  this->frameRange[1] = 0;
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidgetPrivate::PipelineInfoType::printSelf()const
{
  std::cout << "---------------------------------------------------------------" << std::endl
            << "Pipeline info: " << this << std::endl
            << "Number of image frames: " << this->numberOfFrames << std::endl
            << "Frame range: " << this->frameRange[0] << " " << this->frameRange[1] << std::endl
            << "Last frame request: " << this->currentFrame << std::endl
            << "Is connected: " << this->isConnected << std::endl;

}

//------------------------------------------------------------------------------
vtkBirchQFramePlayerWidgetPrivate::PipelineInfoType
vtkBirchQFramePlayerWidgetPrivate::retrievePipelineInfo()
{
  PipelineInfoType pipeInfo;

  pipeInfo.isConnected = this->isConnected();
  if (!pipeInfo.isConnected)
    return pipeInfo;
  if( this->viewer->GetImageDimensionality() < 3 ) 
    return pipeInfo;

  pipeInfo.numberOfFrames =
    this->viewer->GetNumberOfSlices();
  pipeInfo.frameRange[0] = this->viewer->GetSliceMin();
  pipeInfo.frameRange[1] = this->viewer->GetSliceMax();
  
  pipeInfo.currentFrame = this->viewer->GetSlice();
  return pipeInfo;
}

//------------------------------------------------------------------------------
double vtkBirchQFramePlayerWidgetPrivate::PipelineInfoType::
clampTimeInterval(double playbackSpeed, double maxFrameRate) const
{
  Q_ASSERT(playbackSpeed > 0.);

  // the time interval is the time between QTimer emitting
  // timeout signals, which in turn fires onTick, wherein the
  // frame is selected and displayed by the viewer
  // the playback speed is set in frames per second: eg., 60 FPS

  // Clamp the frame rate
  double rate = qMin( playbackSpeed, maxFrameRate);
  
  // return the time interval
  return  1000. / rate;
}

//------------------------------------------------------------------------------
double vtkBirchQFramePlayerWidgetPrivate::PipelineInfoType::validateFrame(double frame) const
{
  if (this->numberOfFrames == 0)
    return vtkMath::Nan();
  else if (this->numberOfFrames == 1)
    return this->frameRange[0];
  return frame;
}

//------------------------------------------------------------------------------
double vtkBirchQFramePlayerWidgetPrivate::PipelineInfoType::previousFrame() const
{
  return this->validateFrame(this->currentFrame-1);
}

//------------------------------------------------------------------------------
double vtkBirchQFramePlayerWidgetPrivate::PipelineInfoType::nextFrame() const
{
  return this->validateFrame(this->currentFrame+1);
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidgetPrivate::setupUi(QWidget* widget)
{
  Q_Q(vtkBirchQFramePlayerWidget);

  this->Ui_vtkBirchQFramePlayerWidget::setupUi(widget);
  this->timer = new QTimer(widget);

  // Connect Menu ToolBars actions
  q->connect(this->firstFrameButton, SIGNAL(pressed()), q, SLOT(goToFirstFrame()));
  q->connect(this->previousFrameButton, SIGNAL(pressed()), q, SLOT(goToPreviousFrame()));
  q->connect(this->playButton, SIGNAL(toggled(bool)), q, SLOT(playForward(bool)));
  q->connect(this->playReverseButton, SIGNAL(toggled(bool)), q, SLOT(playBackward(bool)));
  q->connect(this->nextFrameButton, SIGNAL(pressed()), q, SLOT(goToNextFrame()));
  q->connect(this->lastFrameButton, SIGNAL(pressed()), q, SLOT(goToLastFrame()));
  q->connect(this->speedFactorSpinBox, SIGNAL(valueChanged(double)), q, SLOT(setPlaySpeed(double)));

  // Connect the time slider
  q->connect(this->frameSlider, SIGNAL(valueChanged(double)), q, SLOT(setCurrentFrame(double)));
  this->frameSlider->setSuffix("");
  this->frameSlider->setDecimals(0);
  this->frameSlider->setSingleStep(1);

  // Connect the Timer for animation
  q->connect(this->timer, SIGNAL(timeout()), q, SLOT(onTick()));
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidgetPrivate::updateUi()
{
  PipelineInfoType pipeInfo = this->retrievePipelineInfo();
  this->updateUi(pipeInfo);
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidgetPrivate::updateUi(const PipelineInfoType& pipeInfo)
{
  // Buttons
  this->firstFrameButton->setEnabled((pipeInfo.currentFrame > pipeInfo.frameRange[0]));
  this->previousFrameButton->setEnabled((pipeInfo.currentFrame > pipeInfo.frameRange[0]));
  this->playButton->setEnabled((pipeInfo.numberOfFrames > 1));
  this->playReverseButton->setEnabled((pipeInfo.numberOfFrames > 1));
  this->nextFrameButton->setEnabled((pipeInfo.currentFrame < pipeInfo.frameRange[1]));
  this->lastFrameButton->setEnabled((pipeInfo.currentFrame < pipeInfo.frameRange[1]));

  // Slider
  this->frameSlider->blockSignals(true);
  this->frameSlider->setEnabled((pipeInfo.frameRange[0]!=pipeInfo.frameRange[1]));
  this->frameSlider->setRange(pipeInfo.frameRange[0], pipeInfo.frameRange[1]);
  this->frameSlider->setValue(pipeInfo.currentFrame);
  this->frameSlider->blockSignals(false);
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidgetPrivate::requestData(const PipelineInfoType& pipeInfo,
                                              double frame)
{
  Q_Q(vtkBirchQFramePlayerWidget);

  // We clamp the time requested
  frame = qBound( pipeInfo.frameRange[0], 
            static_cast<double>(vtkMath::Round(frame)), 
            pipeInfo.frameRange[1] );

  // Abort the request
  if (!pipeInfo.isConnected || frame == pipeInfo.currentFrame)
    return;


  this->viewer->SetSlice( frame );
  emit q->currentFrameChanged(frame); // Emit the change
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidgetPrivate::processRequest(double frame)
{
  PipelineInfoType pipeInfo = this->retrievePipelineInfo();
  this->processRequest(pipeInfo, frame);
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidgetPrivate::processRequest(const PipelineInfoType& pipeInfo,
                                                 double frame)
{
  if (vtkMath::IsNan(frame))
    return;

  this->requestData(pipeInfo, frame);
  this->updateUi();
}

//------------------------------------------------------------------------------
bool vtkBirchQFramePlayerWidgetPrivate::isConnected()
{
  return this->viewer && this->viewer->GetInput();
}

//------------------------------------------------------------------------------
// vtkBirchQFramePlayerWidget methods

//------------------------------------------------------------------------------
vtkBirchQFramePlayerWidget::vtkBirchQFramePlayerWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new vtkBirchQFramePlayerWidgetPrivate(*this))
{
  Q_D(vtkBirchQFramePlayerWidget);
  d->setupUi(this);
  d->updateUi();
}

//------------------------------------------------------------------------------
vtkBirchQFramePlayerWidget::~vtkBirchQFramePlayerWidget()
{
  Q_D(vtkBirchQFramePlayerWidget);
  this->stop();
  d->viewer = 0;
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::setViewer(vtkMedicalImageViewer* viewer)
{
  Q_D(vtkBirchQFramePlayerWidget);

  d->viewer = viewer;
  d->updateUi();
}

//------------------------------------------------------------------------------
vtkMedicalImageViewer* vtkBirchQFramePlayerWidget::viewer() const
{
  Q_D(const vtkBirchQFramePlayerWidget);
  return d->viewer;
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::updateFromViewer()
{
  Q_D(vtkBirchQFramePlayerWidget);
  d->updateUi();
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::goToFirstFrame()
{
  Q_D(vtkBirchQFramePlayerWidget);

  // Fetch pipeline information
  vtkBirchQFramePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  d->processRequest(pipeInfo, pipeInfo.frameRange[0]);
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::goToPreviousFrame()
{
  Q_D(vtkBirchQFramePlayerWidget);

  // Fetch pipeline information
  vtkBirchQFramePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  d->processRequest(pipeInfo, pipeInfo.previousFrame());
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::goToNextFrame()
{
  Q_D(vtkBirchQFramePlayerWidget);

  // Fetch pipeline information
  vtkBirchQFramePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  d->processRequest(pipeInfo, pipeInfo.nextFrame());
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::goToLastFrame()
{
  Q_D(vtkBirchQFramePlayerWidget);

  // Fetch pipeline information
  vtkBirchQFramePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  d->processRequest(pipeInfo, pipeInfo.frameRange[1]);
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::play(bool playPause)
{
  if (!playPause)
    this->pause();
  if (playPause)
    this->play();
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::play()
{
  Q_D(vtkBirchQFramePlayerWidget);

  // Fetch pipeline information
  vtkBirchQFramePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();
  double period = pipeInfo.frameRange[1] - pipeInfo.frameRange[0];

  if (!d->viewer || period == 0)
    return;

  if (d->direction == QAbstractAnimation::Forward) {
      d->playReverseButton->blockSignals(true);
      d->playReverseButton->setChecked(false);
      d->playReverseButton->blockSignals(false);

      // Use when set the play by script
      if (!d->playButton->isChecked()) {
        d->playButton->blockSignals(true);
        d->playButton->setChecked(true);
        d->playButton->blockSignals(false);
      }

    // We reset the Slider to the initial value if we play from the end
    if (pipeInfo.currentFrame == pipeInfo.frameRange[1])
      d->frameSlider->setValue(pipeInfo.frameRange[0]);
  }
  else if (d->direction == QAbstractAnimation::Backward) {
      d->playButton->blockSignals(true);
      d->playButton->setChecked(false);
      d->playButton->blockSignals(false);

      // Use when set the play by script
      if (!d->playReverseButton->isChecked()) {
        d->playReverseButton->blockSignals(true);
        d->playReverseButton->setChecked(true);
        d->playReverseButton->blockSignals(false);
      }

    // We reset the Slider to the initial value if we play from the beginning
    if (pipeInfo.currentFrame == pipeInfo.frameRange[0])
      d->frameSlider->setValue(pipeInfo.frameRange[1]);
  }

  double timeInterval =
    pipeInfo.clampTimeInterval(d->speedFactorSpinBox->value(), d->maxFrameRate);

  d->realTime.start();
  d->timer->start(timeInterval);
  emit this->playing(true);
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::pause()
{
  Q_D(vtkBirchQFramePlayerWidget);

  if (d->direction == QAbstractAnimation::Forward)
    d->playButton->setChecked(false);
  else if (d->direction == QAbstractAnimation::Backward)
    d->playReverseButton->setChecked(false);

  if (d->timer->isActive()) {
    d->timer->stop();
    emit this->playing(false);
  }

  return;
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::stop()
{
  this->pause();
  this->goToFirstFrame();
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::playForward(bool play)
{
  this->setDirection(QAbstractAnimation::Forward);
  this->play(play);
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::playBackward(bool play)
{
  this->setDirection(QAbstractAnimation::Backward);
  this->play(play);
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::onTick()
{
  Q_D(vtkBirchQFramePlayerWidget);

  // Forward the internal timer timeout signal
  emit this->onTimeout();

  // Fetch pipeline information
  vtkBirchQFramePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  // currentFrame + number of milliseconds since starting x speed x direction  
  double sec = d->realTime.restart() / 1000.;
  double frameRequest = pipeInfo.currentFrame + sec *
                       d->speedFactorSpinBox->value() *
                       ((d->direction == QAbstractAnimation::Forward) ? 1 : -1);

  if (d->playButton->isChecked() && !d->playReverseButton->isChecked()) {
    if (frameRequest > pipeInfo.frameRange[1] && !d->repeatButton->isChecked()) {
      d->processRequest(pipeInfo, frameRequest);
      this->playForward(false);
      return;
    }
    else if (frameRequest > pipeInfo.frameRange[1] &&
             d->repeatButton->isChecked()) { // We Loop
      frameRequest = pipeInfo.frameRange[0];
      emit this->loop();
    }
  }
  else if (!d->playButton->isChecked() && d->playReverseButton->isChecked()) {
    if (frameRequest < pipeInfo.frameRange[0] && !d->repeatButton->isChecked()) {
      d->processRequest(pipeInfo, frameRequest);
      this->playBackward(false);
      return;
    }
    else if (frameRequest < pipeInfo.frameRange[0] &&
             d->repeatButton->isChecked()) { // We Loop
      frameRequest = pipeInfo.frameRange[1];
      emit this->loop();
    }
  }
  else
  {
    return; // Undefined status
  }

  d->processRequest(pipeInfo, frameRequest);
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::setCurrentFrame(double frame)
{
  Q_D(vtkBirchQFramePlayerWidget);
  d->processRequest(frame);
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::setPlaySpeed(double speedFactor)
{
  Q_D(vtkBirchQFramePlayerWidget);
  speedFactor = speedFactor <= 0. ? 1. : speedFactor;
  d->speedFactorSpinBox->setValue(speedFactor);

  vtkBirchQFramePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  double timeInterval =
    pipeInfo.clampTimeInterval(speedFactor, d->maxFrameRate);
  d->timer->setInterval(timeInterval);
}

//------------------------------------------------------------------------------
// vtkBirchQECGMainWindow methods -- Widgets Interface

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::setFirstFrameIcon(const QIcon& ico)
{
  Q_D(vtkBirchQFramePlayerWidget);
  d->firstFrameButton->setIcon(ico);
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::setPreviousFrameIcon(const QIcon& ico)
{
  Q_D(vtkBirchQFramePlayerWidget);
  d->previousFrameButton->setIcon(ico);
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::setPlayIcon(const QIcon& ico)
{
  Q_D(vtkBirchQFramePlayerWidget);
  d->playButton->setIcon(ico);
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::setPlayReverseIcon(const QIcon& ico)
{
  Q_D(vtkBirchQFramePlayerWidget);
  d->playReverseButton->setIcon(ico);
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::setNextFrameIcon(const QIcon& ico)
{
  Q_D(vtkBirchQFramePlayerWidget);
  d->nextFrameButton->setIcon(ico);
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::setLastFrameIcon(const QIcon& ico)
{
  Q_D(vtkBirchQFramePlayerWidget);
  d->lastFrameButton->setIcon(ico);
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::setRepeatIcon(const QIcon& ico)
{
  Q_D(vtkBirchQFramePlayerWidget);
  d->repeatButton->setIcon(ico);
}

//------------------------------------------------------------------------------
QIcon vtkBirchQFramePlayerWidget::firstFrameIcon() const
{
  Q_D(const vtkBirchQFramePlayerWidget);
  return d->firstFrameButton->icon();
}

//------------------------------------------------------------------------------
QIcon vtkBirchQFramePlayerWidget::previousFrameIcon() const
{
  Q_D(const vtkBirchQFramePlayerWidget);
  return d->previousFrameButton->icon();
}

//------------------------------------------------------------------------------
QIcon vtkBirchQFramePlayerWidget::playIcon() const
{
  Q_D(const vtkBirchQFramePlayerWidget);
  return d->playButton->icon();
}

//------------------------------------------------------------------------------
QIcon vtkBirchQFramePlayerWidget::playReverseIcon() const
{
  Q_D(const vtkBirchQFramePlayerWidget);
  return d->playReverseButton->icon();
}

//------------------------------------------------------------------------------
QIcon vtkBirchQFramePlayerWidget::nextFrameIcon() const
{
  Q_D(const vtkBirchQFramePlayerWidget);
  return d->nextFrameButton->icon();
}

//------------------------------------------------------------------------------
QIcon vtkBirchQFramePlayerWidget::lastFrameIcon() const
{
  Q_D(const vtkBirchQFramePlayerWidget);
  return d->lastFrameButton->icon();
}

//------------------------------------------------------------------------------
QIcon vtkBirchQFramePlayerWidget::repeatIcon() const
{
  Q_D(const vtkBirchQFramePlayerWidget);
  return d->repeatButton->icon();
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::setPlayReverseVisibility(bool visible)
{
  Q_D(vtkBirchQFramePlayerWidget);
  d->playReverseButton->setVisible(visible);
}
//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::setBoundFramesVisibility(bool visible)
{
  Q_D(vtkBirchQFramePlayerWidget);

  d->firstFrameButton->setVisible(visible);
  d->lastFrameButton->setVisible(visible);
}
//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::setGoToVisibility(bool visible)
{
  Q_D(vtkBirchQFramePlayerWidget);

  d->previousFrameButton->setVisible(visible);
  d->nextFrameButton->setVisible(visible);
}
//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::setFrameSpinBoxVisibility(bool visible)
{
  Q_D(vtkBirchQFramePlayerWidget);
  d->frameSlider->setSpinBoxVisible(visible);
}

//------------------------------------------------------------------------------
bool vtkBirchQFramePlayerWidget::playReverseVisibility() const
{
  Q_D(const vtkBirchQFramePlayerWidget);
  return d->playReverseButton->isVisibleTo(
    const_cast<vtkBirchQFramePlayerWidget*>(this));
}
//------------------------------------------------------------------------------
bool vtkBirchQFramePlayerWidget::boundFramesVisibility() const
{
  Q_D(const vtkBirchQFramePlayerWidget);
  return (d->firstFrameButton->isVisibleTo(
            const_cast<vtkBirchQFramePlayerWidget*>(this)) &&
          d->lastFrameButton->isVisibleTo(
            const_cast<vtkBirchQFramePlayerWidget*>(this)));
}
//------------------------------------------------------------------------------
bool vtkBirchQFramePlayerWidget::goToVisibility() const
{
  Q_D(const vtkBirchQFramePlayerWidget);
  return (d->previousFrameButton->isVisibleTo(
            const_cast<vtkBirchQFramePlayerWidget*>(this)) &&
          d->nextFrameButton->isVisibleTo(
            const_cast<vtkBirchQFramePlayerWidget*>(this)));
}
//------------------------------------------------------------------------------
bool vtkBirchQFramePlayerWidget::frameSpinBoxVisibility() const
{
  Q_D(const vtkBirchQFramePlayerWidget);
  return d->frameSlider->spinBox()->isVisibleTo(
    const_cast<vtkBirchQFramePlayerWidget*>(this));
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::setSliderDecimals(int decimals)
{
  Q_D(vtkBirchQFramePlayerWidget);
  d->frameSlider->setDecimals(decimals);
}
//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::setSliderPageStep(double pageStep)
{
  Q_D(vtkBirchQFramePlayerWidget);
  d->frameSlider->setPageStep(pageStep);
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::setSliderSingleStep(double singleStep)
{
  Q_D(vtkBirchQFramePlayerWidget);

  if (singleStep < 0) {
    return;
  }

  d->frameSlider->setSingleStep(singleStep);
}

//------------------------------------------------------------------------------
int vtkBirchQFramePlayerWidget::sliderDecimals() const
{
  Q_D(const vtkBirchQFramePlayerWidget);
  return d->frameSlider->decimals();
}
//------------------------------------------------------------------------------
double vtkBirchQFramePlayerWidget::sliderPageStep() const
{
  Q_D(const vtkBirchQFramePlayerWidget);
  return d->frameSlider->pageStep();
}

//------------------------------------------------------------------------------
double vtkBirchQFramePlayerWidget::sliderSingleStep() const
{
  Q_D(const vtkBirchQFramePlayerWidget);
  return d->frameSlider->singleStep();
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::setDirection(QAbstractAnimation::Direction direction)
{
  Q_D(vtkBirchQFramePlayerWidget);

  if (d->direction != direction) {
    d->direction = direction;
    emit this->directionChanged(direction);
  }
}

//------------------------------------------------------------------------------
QAbstractAnimation::Direction vtkBirchQFramePlayerWidget::direction() const
{
  Q_D(const vtkBirchQFramePlayerWidget);
  return d->direction;
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::setRepeat(bool repeat)
{
  Q_D(const vtkBirchQFramePlayerWidget);
  d->repeatButton->setChecked(repeat);
}

//------------------------------------------------------------------------------
bool vtkBirchQFramePlayerWidget::repeat() const
{
  Q_D(const vtkBirchQFramePlayerWidget);
  return d->repeatButton->isChecked();
}

//------------------------------------------------------------------------------
void vtkBirchQFramePlayerWidget::setMaxFramerate(double frameRate)
{
  Q_D(vtkBirchQFramePlayerWidget);
  // Clamp frameRate min value
  frameRate = (frameRate <= 0) ? 60 : frameRate;
  d->maxFrameRate = frameRate;
}

//------------------------------------------------------------------------------
double vtkBirchQFramePlayerWidget::maxFramerate() const
{
  Q_D(const vtkBirchQFramePlayerWidget);
  return d->maxFrameRate;
}

//------------------------------------------------------------------------------
double vtkBirchQFramePlayerWidget::currentFrame() const
{
  Q_D(const vtkBirchQFramePlayerWidget);
  return d->frameSlider->value();
}

//------------------------------------------------------------------------------
double vtkBirchQFramePlayerWidget::playSpeed() const
{
  Q_D(const vtkBirchQFramePlayerWidget);
  return d->speedFactorSpinBox->value();
}
