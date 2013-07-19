/*=========================================================================

  Program:  Birch (CLSA Medical Image Quality Assessment Tool)
  Module:   QBirchApplication.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
#include "QBirchApplication.h"

#include <QErrorMessage>

#include <stdexcept>
#include <iostream>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchApplication::notify( QObject *pObject, QEvent *pEvent )
{
  try
  {
    return QApplication::notify( pObject,pEvent );
  }
  catch( std::exception &e )
  {
    // catch any exception and display it to the user
    QErrorMessage *dialog = new QErrorMessage( this->activeWindow() );
    dialog->setModal( true );
    dialog->showMessage( tr( e.what() ) );
  }

  return false;
}
