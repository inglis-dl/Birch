/*=========================================================================

  Program:   Birch (CLSA Medical Image Quality Assessment Tool)
  Module:    vtkFrameAnimationPlayer.h
  Language:  C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

/**
 * @class vtkFrameAnimationPlayer

 * @author Patrick Emond <emondpd@mcmaster.ca>
 * @author Dean Inglis <inglisd@mcmaster.ca>
 *
 * @brief Concrete implementation of an animation player.
 *
 * This class was adapted from Paraview's vtkSequenceAnimationPlayer to work
 * with VTK's vtkAnimationCue and vtkAnimationScene since Paraview has
 * its own implementation of these two classes. It has been further adapted
 * to play back images frame by frame.
 */

#ifndef __vtkFrameAnimationPlayer_h
#define __vtkFrameAnimationPlayer_h

#include <vtkAnimationPlayer.h>

class vtkFrameAnimationPlayer : public vtkAnimationPlayer
{
public:
  static vtkFrameAnimationPlayer* New();
  vtkTypeMacro(vtkFrameAnimationPlayer, vtkAnimationPlayer);
  void PrintSelf(ostream& os, vtkIndent indent);

  /**
   * Set/Get the number of frames. Default 0 and must be set.
   */
  vtkSetClampMacro(NumberOfFrames, int, 2, VTK_INT_MAX);
  vtkGetMacro(NumberOfFrames, int);

  /** Get the frame number currently in play */
  vtkGetMacro(FrameNo, int);

protected:
  vtkFrameAnimationPlayer();
  ~vtkFrameAnimationPlayer();

  virtual void StartLoop(double, double, double*);
  virtual void EndLoop() {};

  /** Return the next time given the current time. */
  virtual double GetNextTime(double currentime);

  virtual double GoToNextTime(double start, double end, double currenttime);
  virtual double GoToPreviousTime(double start, double end, double currenttime);

  int NumberOfFrames;
  double StartTime;
  double EndTime;
  int FrameNo;

private:
  vtkFrameAnimationPlayer(const vtkFrameAnimationPlayer&); /** Not implemented */
  void operator=(const vtkFrameAnimationPlayer&); /** Not implemented */
};

#endif
