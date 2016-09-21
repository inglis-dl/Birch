/*==============================================================================

  Module:    QBirchFramePlayerWidget.h
  Program:   Birch
  Language:  C++
  Author:    Dean Inglis <inglisd AT mcmaster DOT ca>

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
#include <QBirchFramePlayerWidget.h>
#include <ui_QBirchFramePlayerWidget.h>

// Birch includes
#include <Common.h>
#include <vtkMedicalImageViewer.h>
#include <QBirchSliceView.h>
#include <QBirchDoubleSpinBox.h>

// Qt includes
#include <QIcon>
#include <QTime>
#include <QTimer>

// VTK includes
#include <vtkEventQtSlotConnect.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
class QBirchFramePlayerWidgetPrivate : public Ui_QBirchFramePlayerWidget
{
  Q_DECLARE_PUBLIC(QBirchFramePlayerWidget);
  protected:
    QBirchFramePlayerWidget* const q_ptr;

    vtkSmartPointer<vtkMedicalImageViewer> viewer;
    vtkSmartPointer<vtkEventQtSlotConnect> connector;

    // Time Playing speed factor.
    double maxFrameRate;
    // Timer to process the player.
    QTimer* timer;
    // Time to reference the real one.
    QTime realTime;
    // Sense of direction
    QAbstractAnimation::Direction direction;

  public:
    explicit QBirchFramePlayerWidgetPrivate(QBirchFramePlayerWidget& object);
    virtual ~QBirchFramePlayerWidgetPrivate();

    virtual void setupUi(QWidget* widget);
    virtual void updateUi();

    struct PipelineInfoType
    {
      PipelineInfoType();

      bool isConnected;
      unsigned int numberOfFrames;
      double frameRange[2];
      double currentFrame;
      int maxFrameRate;

      void printSelf() const;
      // Transform a frameRate into a time interval
      double clampTimeInterval(double speed, double rate) const;
      // Validate a frame
      double validateFrame(double frame) const;
      // Get the next frame.
      double nextFrame() const;
      // Get the previous frame.
      double previousFrame() const;
    };

    // Get pipeline information.
    PipelineInfoType retrievePipelineInfo();
    // Request Data and Update
    virtual void processRequest(double frame);
    // Check if the pipeline is ready
    virtual bool isConnected();
    // Request Data and Update
    virtual void processRequest(const PipelineInfoType& info, double frame);
    // Request Data by time
    virtual void requestData(const PipelineInfoType& info, double frame);
    // Update the widget giving pipeline statut
    virtual void updateUi(const PipelineInfoType& info);
};

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QBirchFramePlayerWidgetPrivate methods
//
// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchFramePlayerWidgetPrivate::QBirchFramePlayerWidgetPrivate
(QBirchFramePlayerWidget& object)
  : q_ptr(&object)
{
  this->maxFrameRate = 60;  // 60 FPS by default
  this->viewer = 0;
  this->connector = 0;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchFramePlayerWidgetPrivate::~QBirchFramePlayerWidgetPrivate()
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchFramePlayerWidgetPrivate::PipelineInfoType::PipelineInfoType()
  : isConnected(false)
  , numberOfFrames(0)
  , currentFrame(0)
  , maxFrameRate(60)
{
  this->frameRange[0] = 0;
  this->frameRange[1] = 0;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidgetPrivate::PipelineInfoType::printSelf() const
{
  std::cout << "---------------------------------------------------------------"
            << std::endl
            << "Pipeline info: " << this << std::endl
            << "Number of image frames: " << this->numberOfFrames << std::endl
            << "Frame range: " << this->frameRange[0] << " "
            << this->frameRange[1] << std::endl
            << "Last frame request: " << this->currentFrame << std::endl
            << "Maximum frame rate: " << this->maxFrameRate << std::endl
            << "Is connected: " << this->isConnected << std::endl;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchFramePlayerWidgetPrivate::PipelineInfoType
QBirchFramePlayerWidgetPrivate::retrievePipelineInfo()
{
  Q_Q(QBirchFramePlayerWidget);
  PipelineInfoType pipeInfo;

  if (this->viewer)
  {
    pipeInfo.isConnected = this->isConnected();
    if (!pipeInfo.isConnected)
    {
      return pipeInfo;
    }
    if (3 > this->viewer->GetImageDimensionality())
    {
      return pipeInfo;
    }

    pipeInfo.frameRange[0] = this->viewer->GetSliceMin();
    pipeInfo.frameRange[1] = this->viewer->GetSliceMax();
    pipeInfo.numberOfFrames = this->viewer->GetNumberOfSlices();
    pipeInfo.currentFrame = this->viewer->GetSlice();
    pipeInfo.maxFrameRate = this->viewer->GetMaxFrameRate();
  }
  else if (!q->sliceViewPointer.isNull())
  {
    pipeInfo.isConnected = !q->sliceViewPointer.isNull();
    if (!pipeInfo.isConnected)
    {
      return pipeInfo;
    }
    if (3 > q->sliceViewPointer.data()->dimensionality())
    {
      pipeInfo.isConnected = false;
      return pipeInfo;
    }

    pipeInfo.frameRange[0] = q->sliceViewPointer->sliceMin();
    pipeInfo.frameRange[1] = q->sliceViewPointer->sliceMax();
    pipeInfo.numberOfFrames =
      pipeInfo.frameRange[1] - pipeInfo.frameRange[0] + 1;
    pipeInfo.currentFrame = q->sliceViewPointer->slice();
    pipeInfo.maxFrameRate = q->sliceViewPointer->frameRate();
  }
  return pipeInfo;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchFramePlayerWidgetPrivate::PipelineInfoType::
  clampTimeInterval(double playbackSpeed, double maxFrameRate) const
{
  Q_ASSERT(playbackSpeed > 0.);

  // the time interval is the time between QTimer Q_EMITting
  // timeout signals, which in turn fires onTick, wherein the
  // frame is selected and displayed by the viewer
  // the playback speed is set in frames per second: eg., 60 FPS

  // Clamp the frame rate
  double rate = qMin(playbackSpeed, maxFrameRate);

  // return the time interval
  return  1000. / rate;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchFramePlayerWidgetPrivate::PipelineInfoType::validateFrame(
  double frame) const
{
  if (0 == this->numberOfFrames)
    return vtkMath::Nan();
  else if (1 == this->numberOfFrames)
    return this->frameRange[0];
  return frame;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchFramePlayerWidgetPrivate::PipelineInfoType::previousFrame() const
{
  return this->validateFrame(this->currentFrame - 1);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchFramePlayerWidgetPrivate::PipelineInfoType::nextFrame() const
{
  return this->validateFrame(this->currentFrame + 1);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidgetPrivate::setupUi(QWidget* widget)
{
  Q_Q(QBirchFramePlayerWidget);

  this->Ui_QBirchFramePlayerWidget::setupUi(widget);
  this->timer = new QTimer(widget);

  // Connect Menu ToolBars actions
  q->connect(this->firstFrameButton, SIGNAL(pressed()),
    q, SLOT(goToFirstFrame()));
  q->connect(this->previousFrameButton, SIGNAL(pressed()),
    q, SLOT(goToPreviousFrame()));
  q->connect(this->playButton, SIGNAL(toggled(bool)),
    q, SLOT(playForward(bool)));
  q->connect(this->playReverseButton, SIGNAL(toggled(bool)),
    q, SLOT(playBackward(bool)));
  q->connect(this->nextFrameButton, SIGNAL(pressed()),
    q, SLOT(goToNextFrame()));
  q->connect(this->lastFrameButton, SIGNAL(pressed()),
    q, SLOT(goToLastFrame()));
  q->connect(this->speedSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(setPlaySpeed(double)));

  // Connect the time slider
  q->connect(this->frameSlider, SIGNAL(valueChanged(double)),
    q, SLOT(setCurrentFrame(double)));
  this->frameSlider->setSuffix("");
  this->frameSlider->setDecimals(0);
  this->frameSlider->setSingleStep(1);

  // Connect the Timer for animation
  q->connect(this->timer, SIGNAL(timeout()), q, SLOT(onTick()));
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidgetPrivate::updateUi()
{
  PipelineInfoType pipeInfo = this->retrievePipelineInfo();
  this->updateUi(pipeInfo);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidgetPrivate::updateUi(const PipelineInfoType& pipeInfo)
{
  // Buttons
  this->firstFrameButton->setEnabled(
    (pipeInfo.currentFrame > pipeInfo.frameRange[0]));
  this->previousFrameButton->setEnabled(
    (pipeInfo.currentFrame > pipeInfo.frameRange[0]));
  this->playButton->setEnabled((pipeInfo.numberOfFrames > 1));
  this->playReverseButton->setEnabled((pipeInfo.numberOfFrames > 1));
  this->nextFrameButton->setEnabled(
    (pipeInfo.currentFrame < pipeInfo.frameRange[1]));
  this->lastFrameButton->setEnabled(
    (pipeInfo.currentFrame < pipeInfo.frameRange[1]));
  this->repeatButton->setEnabled((pipeInfo.numberOfFrames > 1));
  this->speedSpinBox->setEnabled((pipeInfo.numberOfFrames > 1));

  // Slider
  this->frameSlider->blockSignals(true);
  this->frameSlider->setEnabled(
    (pipeInfo.frameRange[0] != pipeInfo.frameRange[1]));
  this->frameSlider->setRange(
    pipeInfo.frameRange[0]+1, pipeInfo.frameRange[1]+1);
  this->frameSlider->setValue(pipeInfo.currentFrame+1);
  this->frameSlider->blockSignals(false);

  // SpinBox
  // the max frame rate from the pipeinfo object is set from the viewer's frame
  // rate information. The value of the speed spin box set here is a suggested
  // value. The speed can be set and is clamped between 1 and whatever the max
  // frame rate set through the QBirchFramePlayerWidget's maxFrameRate property.
  this->speedSpinBox->blockSignals(true);
  double frameRate = pipeInfo.maxFrameRate;
  if (this->speedSpinBox->maximum() < frameRate)
    this->speedSpinBox->setMaximum(frameRate);
  this->speedSpinBox->setValue(
    qMin(this->speedSpinBox->value(), frameRate));
  this->speedSpinBox->blockSignals(false);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidgetPrivate::requestData(
  const PipelineInfoType& pipeInfo, double frame)
{
  Q_Q(QBirchFramePlayerWidget);

  // We clamp the time requested
  frame = qBound(pipeInfo.frameRange[0],
            static_cast<double>(vtkMath::Round(frame)),
            pipeInfo.frameRange[1]);

  // Abort the request
  if (!pipeInfo.isConnected || frame == pipeInfo.currentFrame)
    return;

  if (this->viewer)
  {
    this->viewer->SetSlice(frame);
  }
  else if (!q->sliceViewPointer.isNull())
  {
    q->sliceViewPointer.data()->setSlice(frame);
  }

  Q_EMIT q->currentFrameChanged(frame);  // Emit the change
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidgetPrivate::processRequest(double frame)
{
  PipelineInfoType pipeInfo = this->retrievePipelineInfo();
  this->processRequest(pipeInfo, frame);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidgetPrivate::processRequest(
  const PipelineInfoType& pipeInfo, double frame)
{
  if (vtkMath::IsNan(frame))
    return;

  this->requestData(pipeInfo, frame);
  this->updateUi();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchFramePlayerWidgetPrivate::isConnected()
{
  return this->viewer && this->viewer->GetInput() && this->connector;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QBirchFramePlayerWidget methods
//
// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchFramePlayerWidget::QBirchFramePlayerWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new QBirchFramePlayerWidgetPrivate(*this))
{
  Q_D(QBirchFramePlayerWidget);
  d->setupUi(this);
  d->updateUi();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchFramePlayerWidget::~QBirchFramePlayerWidget()
{
  Q_D(QBirchFramePlayerWidget);
  this->stop();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::setViewer(vtkMedicalImageViewer* viewer)
{
  Q_D(QBirchFramePlayerWidget);
  if (d->viewer && d->connector)
  {
    d->connector->Disconnect(
      d->viewer, Birch::Common::OrientationChangedEvent,
      this, SLOT(goToCurrentFrame()));
  }
  d->viewer = viewer;
  if (viewer)
  {
    if (!d->connector)
      d->connector =
        vtkSmartPointer<vtkEventQtSlotConnect>::New();

    d->connector->Connect(
      viewer, Birch::Common::OrientationChangedEvent,
      this, SLOT(goToCurrentFrame()));
  }
  d->updateUi();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkMedicalImageViewer* QBirchFramePlayerWidget::viewer() const
{
  Q_D(const QBirchFramePlayerWidget);
  return d->viewer;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::update()
{
  Q_D(QBirchFramePlayerWidget);
  if (!this->sliceViewPointer.isNull())
  {
    int frameRate = this->sliceViewPointer.data()->frameRate();
    this->setMaxFrameRate(frameRate);
    this->setPlaySpeed(frameRate);
  }
  d->updateUi();
  this->goToCurrentFrame();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::setSliceView(QBirchSliceView* view)
{
  Q_D(QBirchFramePlayerWidget);

  if (d->viewer)
    this->setViewer(0);

  this->sliceViewPointer = view;
  if (!this->sliceViewPointer.isNull())
  {
    connect(this->sliceViewPointer.data(), SIGNAL(imageDataChanged()),
             this, SLOT(update()));
  }
  d->updateUi();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::goToCurrentFrame()
{
  Q_D(QBirchFramePlayerWidget);

  // Fetch pipeline information
  QBirchFramePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  d->processRequest(pipeInfo, pipeInfo.currentFrame);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::goToFirstFrame()
{
  Q_D(QBirchFramePlayerWidget);

  // Fetch pipeline information
  QBirchFramePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  d->processRequest(pipeInfo, pipeInfo.frameRange[0]);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::goToPreviousFrame()
{
  Q_D(QBirchFramePlayerWidget);

  // Fetch pipeline information
  QBirchFramePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  d->processRequest(pipeInfo, pipeInfo.previousFrame());
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::goToNextFrame()
{
  Q_D(QBirchFramePlayerWidget);

  // Fetch pipeline information
  QBirchFramePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  d->processRequest(pipeInfo, pipeInfo.nextFrame());
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::goToLastFrame()
{
  Q_D(QBirchFramePlayerWidget);

  // Fetch pipeline information
  QBirchFramePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  d->processRequest(pipeInfo, pipeInfo.frameRange[1]);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::play(bool playPause)
{
  if (!playPause)
    this->pause();
  if (playPause)
    this->play();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::play()
{
  Q_D(QBirchFramePlayerWidget);

  // Fetch pipeline information
  QBirchFramePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();
  double period = pipeInfo.frameRange[1] - pipeInfo.frameRange[0];

  if (!pipeInfo.isConnected || 0 == period)
    return;

  if (QAbstractAnimation::Forward == d->direction)
  {
    d->playReverseButton->blockSignals(true);
    d->playReverseButton->setChecked(false);
    d->playReverseButton->blockSignals(false);

    // Use when set the play by script
    if (!d->playButton->isChecked())
    {
      d->playButton->blockSignals(true);
      d->playButton->setChecked(true);
      d->playButton->blockSignals(false);
    }

    // We reset the Slider to the initial value if we play from the end
    if (pipeInfo.currentFrame == pipeInfo.frameRange[1])
      d->frameSlider->setValue(pipeInfo.frameRange[0]+1);
  }
  else if (QAbstractAnimation::Backward == d->direction)
  {
    d->playButton->blockSignals(true);
    d->playButton->setChecked(false);
    d->playButton->blockSignals(false);

    // Use when set the play by script
    if (!d->playReverseButton->isChecked())
    {
      d->playReverseButton->blockSignals(true);
      d->playReverseButton->setChecked(true);
      d->playReverseButton->blockSignals(false);
    }

    // We reset the Slider to the initial value if we play from the beginning
    if (pipeInfo.currentFrame == pipeInfo.frameRange[0])
      d->frameSlider->setValue(pipeInfo.frameRange[1]+1);
  }

  double timeInterval =
    pipeInfo.clampTimeInterval(d->speedSpinBox->value(), d->maxFrameRate);

  d->realTime.start();
  d->timer->start(timeInterval);
  Q_EMIT this->playing(true);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::pause()
{
  Q_D(QBirchFramePlayerWidget);

  if (QAbstractAnimation::Forward == d->direction)
    d->playButton->setChecked(false);
  else if (QAbstractAnimation::Backward == d->direction)
    d->playReverseButton->setChecked(false);

  if (d->timer->isActive())
  {
    d->timer->stop();
    Q_EMIT this->playing(false);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::stop()
{
  this->pause();
  this->goToFirstFrame();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::playForward(bool play)
{
  this->setDirection(QAbstractAnimation::Forward);
  this->play(play);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::playBackward(bool play)
{
  this->setDirection(QAbstractAnimation::Backward);
  this->play(play);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::onTick()
{
  Q_D(QBirchFramePlayerWidget);

  // Forward the internal timer timeout signal
  Q_EMIT this->onTimeout();

  // Fetch pipeline information
  QBirchFramePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  // currentFrame + number of milliseconds since starting x speed x direction
  double sec = d->realTime.restart() / 1000.;
  double frameRequest = pipeInfo.currentFrame + sec *
                       d->speedSpinBox->value() *
                       ((d->direction == QAbstractAnimation::Forward) ? 1 : -1);

  if (d->playButton->isChecked() && !d->playReverseButton->isChecked())
  {
    if (frameRequest > pipeInfo.frameRange[1] && !d->repeatButton->isChecked())
    {
      d->processRequest(pipeInfo, frameRequest);
      this->playForward(false);
      return;
    }
    else if (frameRequest > pipeInfo.frameRange[1] &&
             d->repeatButton->isChecked())
    { // We Loop
      frameRequest = pipeInfo.frameRange[0];
      Q_EMIT this->loop();
    }
  }
  else if (!d->playButton->isChecked() && d->playReverseButton->isChecked())
  {
    if (frameRequest < pipeInfo.frameRange[0] && !d->repeatButton->isChecked())
    {
      d->processRequest(pipeInfo, frameRequest);
      this->playBackward(false);
      return;
    }
    else if (frameRequest < pipeInfo.frameRange[0] &&
             d->repeatButton->isChecked())
    { // We Loop
      frameRequest = pipeInfo.frameRange[1];
      Q_EMIT this->loop();
    }
  }
  else
  {
    return;  // Undefined status
  }

  d->processRequest(pipeInfo, frameRequest);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::setCurrentFrame(double frame)
{
  Q_D(QBirchFramePlayerWidget);
  if (d->frameSlider == qobject_cast<QBirchSliderWidget*>(sender()))
    d->processRequest(frame-1);
  else
    d->processRequest(frame);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::setPlaySpeed(double speed)
{
  Q_D(QBirchFramePlayerWidget);
  speed = speed <= 0. ? 1. : speed;
  d->speedSpinBox->setValue(speed);

  QBirchFramePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  double timeInterval =
    pipeInfo.clampTimeInterval(speed, d->maxFrameRate);
  d->timer->setInterval(timeInterval);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QBirchFramePlayerWidget methods -- Widgets Interface
//
// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::setFirstFrameIcon(const QIcon& ico)
{
  Q_D(QBirchFramePlayerWidget);
  d->firstFrameButton->setIcon(ico);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::setPreviousFrameIcon(const QIcon& ico)
{
  Q_D(QBirchFramePlayerWidget);
  d->previousFrameButton->setIcon(ico);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::setPlayIcon(const QIcon& ico)
{
  Q_D(QBirchFramePlayerWidget);
  d->playButton->setIcon(ico);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::setPlayReverseIcon(const QIcon& ico)
{
  Q_D(QBirchFramePlayerWidget);
  d->playReverseButton->setIcon(ico);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::setNextFrameIcon(const QIcon& ico)
{
  Q_D(QBirchFramePlayerWidget);
  d->nextFrameButton->setIcon(ico);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::setLastFrameIcon(const QIcon& ico)
{
  Q_D(QBirchFramePlayerWidget);
  d->lastFrameButton->setIcon(ico);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::setRepeatIcon(const QIcon& ico)
{
  Q_D(QBirchFramePlayerWidget);
  d->repeatButton->setIcon(ico);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QBirchFramePlayerWidget::firstFrameIcon() const
{
  Q_D(const QBirchFramePlayerWidget);
  return d->firstFrameButton->icon();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QBirchFramePlayerWidget::previousFrameIcon() const
{
  Q_D(const QBirchFramePlayerWidget);
  return d->previousFrameButton->icon();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QBirchFramePlayerWidget::playIcon() const
{
  Q_D(const QBirchFramePlayerWidget);
  return d->playButton->icon();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QBirchFramePlayerWidget::playReverseIcon() const
{
  Q_D(const QBirchFramePlayerWidget);
  return d->playReverseButton->icon();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QBirchFramePlayerWidget::nextFrameIcon() const
{
  Q_D(const QBirchFramePlayerWidget);
  return d->nextFrameButton->icon();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QBirchFramePlayerWidget::lastFrameIcon() const
{
  Q_D(const QBirchFramePlayerWidget);
  return d->lastFrameButton->icon();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QBirchFramePlayerWidget::repeatIcon() const
{
  Q_D(const QBirchFramePlayerWidget);
  return d->repeatButton->icon();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::setPlayReverseVisibility(bool visible)
{
  Q_D(QBirchFramePlayerWidget);
  d->playReverseButton->setVisible(visible);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::setBoundFramesVisibility(bool visible)
{
  Q_D(QBirchFramePlayerWidget);

  d->firstFrameButton->setVisible(visible);
  d->lastFrameButton->setVisible(visible);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::setGoToVisibility(bool visible)
{
  Q_D(QBirchFramePlayerWidget);

  d->previousFrameButton->setVisible(visible);
  d->nextFrameButton->setVisible(visible);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::setFrameSpinBoxVisibility(bool visible)
{
  Q_D(QBirchFramePlayerWidget);
  d->frameSlider->setSpinBoxVisible(visible);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchFramePlayerWidget::playReverseVisibility() const
{
  Q_D(const QBirchFramePlayerWidget);
  return d->playReverseButton->isVisibleTo(
    const_cast<QBirchFramePlayerWidget*>(this));
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchFramePlayerWidget::boundFramesVisibility() const
{
  Q_D(const QBirchFramePlayerWidget);
  return (d->firstFrameButton->isVisibleTo(
            const_cast<QBirchFramePlayerWidget*>(this)) &&
          d->lastFrameButton->isVisibleTo(
            const_cast<QBirchFramePlayerWidget*>(this)));
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchFramePlayerWidget::goToVisibility() const
{
  Q_D(const QBirchFramePlayerWidget);
  return (d->previousFrameButton->isVisibleTo(
            const_cast<QBirchFramePlayerWidget*>(this)) &&
          d->nextFrameButton->isVisibleTo(
            const_cast<QBirchFramePlayerWidget*>(this)));
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchFramePlayerWidget::frameSpinBoxVisibility() const
{
  Q_D(const QBirchFramePlayerWidget);
  return d->frameSlider->spinBox()->isVisibleTo(
    const_cast<QBirchFramePlayerWidget*>(this));
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::setSliderDecimals(int decimals)
{
  Q_D(QBirchFramePlayerWidget);
  d->frameSlider->setDecimals(decimals);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::setSliderPageStep(double pageStep)
{
  Q_D(QBirchFramePlayerWidget);
  d->frameSlider->setPageStep(pageStep);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::setSliderSingleStep(double singleStep)
{
  Q_D(QBirchFramePlayerWidget);

  if (singleStep < 0.)
    return;

  d->frameSlider->setSingleStep(singleStep);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int QBirchFramePlayerWidget::sliderDecimals() const
{
  Q_D(const QBirchFramePlayerWidget);
  return d->frameSlider->decimals();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchFramePlayerWidget::sliderPageStep() const
{
  Q_D(const QBirchFramePlayerWidget);
  return d->frameSlider->pageStep();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchFramePlayerWidget::sliderSingleStep() const
{
  Q_D(const QBirchFramePlayerWidget);
  return d->frameSlider->singleStep();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::setDirection(
  QAbstractAnimation::Direction direction)
{
  Q_D(QBirchFramePlayerWidget);

  if (direction != d->direction)
  {
    d->direction = direction;
    Q_EMIT this->directionChanged(direction);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAbstractAnimation::Direction QBirchFramePlayerWidget::direction() const
{
  Q_D(const QBirchFramePlayerWidget);
  return d->direction;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::setRepeat(bool repeat)
{
  Q_D(const QBirchFramePlayerWidget);
  d->repeatButton->setChecked(repeat);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchFramePlayerWidget::repeat() const
{
  Q_D(const QBirchFramePlayerWidget);
  return d->repeatButton->isChecked();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchFramePlayerWidget::setMaxFrameRate(double frameRate)
{
  Q_D(QBirchFramePlayerWidget);
  // Clamp frameRate min value
  frameRate = (0 >= frameRate) ? 60 : frameRate;
  d->maxFrameRate = frameRate;
  d->speedSpinBox->blockSignals(true);
  if (d->speedSpinBox->maximum() < frameRate)
    d->speedSpinBox->setMaximum(frameRate);
  d->speedSpinBox->setValue(
    qMin(d->speedSpinBox->value(), frameRate));
  d->speedSpinBox->blockSignals(false);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchFramePlayerWidget::maxFrameRate() const
{
  Q_D(const QBirchFramePlayerWidget);
  return d->maxFrameRate;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchFramePlayerWidget::currentFrame() const
{
  Q_D(const QBirchFramePlayerWidget);
  return d->frameSlider->value()-1;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QBirchFramePlayerWidget::playSpeed() const
{
  Q_D(const QBirchFramePlayerWidget);
  return d->speedSpinBox->value();
}
