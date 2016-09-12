/*=========================================================================

  Program:  Birch
  Module:   Common.h
  Language: C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class Common
 *
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief Common class includes typedefs, macros, global events, etc.
 *
 */
#ifndef __Common_h
#define __Common_h

#include <vtkCommand.h>

/**
 * @addtogroup Birch
 * @{
 */

namespace Birch
{
  class Common
  {
  public:
    enum CustomEvents
    {
      ImageChangedEvent = vtkCommand::UserEvent + 100,
      SliceChangedEvent,
      OrientationChangedEvent
    };

  protected:
    Common() {}
    ~Common() {}
  };
}  // namespace Birch

#endif  // __Common_h
