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

#ifndef __vtkBirchQFramePlayerWidget_h
#define __vtkBirchQFramePlayerWidget_h

// Qt includes
#include <QIcon>
#include <QWidget>
#include <QAbstractAnimation>


// VTK includes
class vtkMedicalImageViewer;
class vtkBirchQFramePlayerWidgetPrivate;

/// \brief A Qt slider for controlling time of filter pipelines.
///
/// It encapsulates the VTK time animation control functionality and provides
/// slots and signals to manage them in a Qt application. The widget connects
/// itself to a *Viewer*, which controls display of an image.

class vtkBirchQFramePlayerWidget : public QWidget
{
  Q_OBJECT
  /// This property holds the firstFrame button's icon.
  /// \sa firstFrameIcon(), setFirstFrameIcon()
  Q_PROPERTY(QIcon firstFrameIcon READ firstFrameIcon WRITE setFirstFrameIcon)
  /// This property holds the previousFrame button's icon.
  /// \sa previousFrameIcon(), setPreviousFrameIcon()
  Q_PROPERTY(QIcon previousFrameIcon READ previousFrameIcon WRITE setPreviousFrameIcon)
  /// This property holds the play button's icon.
  /// \sa playIcon(), setPlayIcon()
  Q_PROPERTY(QIcon playIcon READ playIcon WRITE setPlayIcon)
  /// This property holds the play reverse button's icon.
  /// \sa playReverseIcon(), setPlayReverseIcon()
  Q_PROPERTY(QIcon playReverseIcon READ playReverseIcon WRITE setPlayReverseIcon)
  /// This property holds the nextFrame button's icon.
  /// \sa nextFrameIcon(), setNextFrameIcon()
  Q_PROPERTY(QIcon nextFrameIcon READ nextFrameIcon WRITE setNextFrameIcon)
  /// This property holds the lastFrame button's icon.
  /// \sa lastFrameIcon(), setLastFrameIcon()
  Q_PROPERTY(QIcon lastFrameIcon READ lastFrameIcon WRITE setLastFrameIcon)
  /// This property holds the repeat button's icon.
  /// \sa repeatIcon(), setRepeatIcon()
  Q_PROPERTY(QIcon repeatIcon READ repeatIcon WRITE setRepeatIcon)

  /// Enable/Disable the visibility of play reverse button.
  /// \sa playReverseVisibility(), setPlayReverseVisibility()
  Q_PROPERTY(bool playReverseVisibility READ playReverseVisibility WRITE setPlayReverseVisibility)
  /// Enable/Disable the visibility of the firstFrame and lastFrame buttons.
  /// \sa boundFramesVisibility(), setBoundFramesVisibility()
  Q_PROPERTY(bool boundFramesVisibility READ boundFramesVisibility WRITE setBoundFramesVisibility)
  /// Enable/Disable the visibility of the seekBackward and seekForward buttons.
  /// \sa goToVisibility(), setGoToVisibility()
  Q_PROPERTY(bool goToVisibility READ goToVisibility WRITE setGoToVisibility)
  /// Enable/Disable the visibility of time spinBox
  /// \sa frameSpinBoxVisibility(), setFrameSpinBoxVisibility()
  Q_PROPERTY(bool frameSpinBoxVisibility READ frameSpinBoxVisibility WRITE setFrameSpinBoxVisibility)

  /// This property holds the number of decimal digits for the frameSlider.
  /// \sa sliderDecimals(), setSliderDecimals()
  Q_PROPERTY(int sliderDecimals READ sliderDecimals WRITE setSliderDecimals)
  /// This property holds the page step for the frameSlider.
  /// \sa sliderPageStep(), setSliderPageStep()
  Q_PROPERTY(double sliderPageStep READ sliderPageStep WRITE setSliderPageStep)
  /// This property holds the single step for the frameSlider.
  /// The automatic mode (by default) compute the step corresponding to one
  /// frame. To get back to the automatic mode, set a negative value.
  /// \sa sliderSingleStep(), setSliderSingleStep()
  Q_PROPERTY(double sliderSingleStep READ sliderSingleStep WRITE setSliderSingleStep)

  /// This property holds the number of the higher frame per seconds rate the
  /// the player will be willing to handle.
  /// \sa maxFramerate(), setMaxFramerate()
  Q_PROPERTY(double maxFramerate READ maxFramerate WRITE setMaxFramerate)
  /// This property holds the direction on which the widget will do the
  /// animation.
  /// \sa direction(), setDirection()
  Q_PROPERTY(QAbstractAnimation::Direction direction READ direction WRITE setDirection NOTIFY directionChanged)
  /// This property holds if the animation is repeated when we reach the end.
  /// \sa repeat(), setRepeat()
  Q_PROPERTY(bool repeat READ repeat WRITE setRepeat)
  /// This property holds the speed factor of the animation.
  /// \sa playSpeed(), setPlaySpeed()
  Q_PROPERTY(double playSpeed READ playSpeed WRITE setPlaySpeed)
  /// This property is an accessor to the widget's current time.
  /// \sa currentTime(), setCurrentTime()
  Q_PROPERTY(double currentFrame READ currentFrame WRITE setCurrentFrame NOTIFY currentFrameChanged)

public:
  typedef QWidget Superclass;
  vtkBirchQFramePlayerWidget(QWidget* parent=0);
  virtual ~vtkBirchQFramePlayerWidget();

  /// Set the viewer to connect with.
  void setViewer(vtkMedicalImageViewer* viewer);
  /// Return the viewer.
  vtkMedicalImageViewer* viewer() const;

  /// Set the first frame icon.
  /// \sa firstFrameIcon
  void setFirstFrameIcon(const QIcon&);
  /// Return the first frame icon.
  /// \sa firstFrameIcon
  QIcon firstFrameIcon() const;
  /// Set the previous frame icon.
  /// \sa previousFrameIcon
  void setPreviousFrameIcon(const QIcon&);
  /// Return the previous frame icon.
  /// \sa previousFrameIcon
  QIcon previousFrameIcon() const;
  /// Set the play icon.
  /// \sa playIcon
  void setPlayIcon(const QIcon&);
  /// Return the play icon.
  /// \sa playIcon
  QIcon playIcon() const;
  /// Set the reverse icon.
  /// \sa playReverseIcon
  void setPlayReverseIcon(const QIcon&);
  /// Return the play reverse icon
  /// \sa playReverseIcon
  QIcon playReverseIcon() const;
  /// Set the icon of the next frame button.
  /// \sa nextFrameIcon
  void setNextFrameIcon(const QIcon&);
  /// Return the icon of the next frame button.
  /// \sa nextFrameIcon
  QIcon nextFrameIcon() const;
  /// Set the icon of the last frame button.
  /// \sa lastFrameIcon
  void setLastFrameIcon(const QIcon&);
  /// Return the icon of the last frame button.
  /// \sa lastFrameIcon
  QIcon lastFrameIcon() const;
  /// Set the icon of the repeat button.
  /// \sa repeatIcon
  void setRepeatIcon(const QIcon&);
  /// Return the icon of the repeat button.
  /// \sa repeatIcon
  QIcon repeatIcon() const;

  /// \sa playReverseVisibility
  void setPlayReverseVisibility(bool visible);
  /// \sa playReverseVisibility
  bool playReverseVisibility() const;
  /// \sa boundFramesVisibility
  void setBoundFramesVisibility(bool visible);
  /// \sa boundFramesVisibility
  bool boundFramesVisibility() const;
  /// \sa goToVisibility
  void setGoToVisibility(bool visible);
  /// \sa goToVisibility
  bool goToVisibility() const;
  /// \sa frameSpinBoxVisibility
  void setFrameSpinBoxVisibility(bool visible);
  /// \sa frameSpinBoxVisibility
  bool frameSpinBoxVisibility() const;

  /// \sa sliderDecimals
  void setSliderDecimals(int decimals);
  /// \sa sliderDecimals
  int sliderDecimals() const;
  /// \sa sliderPageStep
  void setSliderPageStep(double pageStep);
  /// \sa sliderPageStep
  double sliderPageStep() const;
  /// \sa sliderSingleStep
  void setSliderSingleStep(double singleStep);
  /// \sa sliderSingleStep
  double sliderSingleStep() const;

  /// Playback
  /// \sa direction
  void setDirection(QAbstractAnimation::Direction playDirection);
  /// \sa direction
  QAbstractAnimation::Direction direction() const;
  /// \sa repeat
  void setRepeat(bool repeat);
  /// \sa repeat
  bool repeat() const;
  /// \sa maxFramerate
  void setMaxFramerate(double);
  /// \sa maxFramerate
  double maxFramerate() const;
  /// \sa currentFrame
  double currentFrame() const;
  /// \sa playSpeed
  double playSpeed() const;

public slots:
  /// \sa currentTime
  virtual void setCurrentFrame(double frame);
  /// \sa playSpeed
  virtual void setPlaySpeed(double speedCoef);
  /// Set the current time to the first frame.
  /// \sa goToPreviousFrame(), goToNextFrame(), goToLastFrame()
  virtual void goToFirstFrame();
  /// Set the current time to the previous frame.
  /// \sa goToFirstFrame(), goToNextFrame(), goToLastFrame()
  virtual void goToPreviousFrame();
  /// Set the current time to the next frame.
  /// \sa goToFirstFrame(), goToPreviousFrame(), goToLastFrame()
  virtual void goToNextFrame();
  /// Set the current time to the last frame.
  /// \sa goToFirstFrame(), goToPreviousFrame(), goToNextFrame(), goToLastFrame()
  virtual void goToLastFrame();

  /// Automatically browse all the time steps from the pipeline in the
  /// \a direction order.
  /// \sa pause(), stop(), direction
  virtual void play();
  /// Pause the browse of the time steps. To resume, call play().
  /// \sa play(), pause()
  virtual void pause();
  /// Call play() on true, pause() on false.
  /// \sa play(), pause()
  void play(bool playPause);
  /// Browse the time steps in the forward order.
  /// \sa playBackward(), play(), orientation
  void playForward(bool playPause);
  /// Browse the time steps in the backward order.
  /// \sa playForward(), play(), orientation
  void playBackward(bool playPause);
  /// Stop the browsing of the time steps and go back to the first frame.
  /// \sa play(), pause()
  void stop();

  virtual void updateFromViewer();

protected slots:
  virtual void onTick();

signals:
  /// Emitted when the frame has been changed
  void currentFrameChanged(double);

  /// Emitted when the internal timer sends a timeout
  void onTimeout();

  /// Emitted with playing(true) when play begins and
  /// playing(false) when play ends.
  void playing(bool);

  /// Emitted when the player loops the animation
  void loop();

  /// Emitted when the playback direction is changed
  void directionChanged(QAbstractAnimation::Direction);

protected:
  QScopedPointer<vtkBirchQFramePlayerWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(vtkBirchQFramePlayerWidget);
  Q_DISABLE_COPY(vtkBirchQFramePlayerWidget);
};

#endif
