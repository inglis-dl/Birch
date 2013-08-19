/*=========================================================================

  Program:  Birch (A simple image viewer)
  Module:   Birch.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
//
// .SECTION Description
// The main function which launches the application.
//

#include <Utilities.h>

#include "QMainBirchWindow.h"
#include "QBirchApplication.h"
#include <QObject>
#include <QString>
#include <QTimer>

//#include "vtkSmartPointer.h"

#include <stdexcept>

using namespace Birch;

// main function
int main( int argc, char** argv )
{
  int status = EXIT_FAILURE;

  try
  {
    // create the user interface
    QBirchApplication qapp( argc, argv );
    QMainBirchWindow mainWindow;

    QTimer::singleShot(0,&mainWindow, SLOT(initialize()));

    mainWindow.show();

    // execute the application, then delete the application
    int status = qapp.exec();
  }
  catch( std::exception &e )
  {
    cerr << "Uncaught exception: " << e.what() << endl;
    return EXIT_FAILURE;
  }

  // return the result of the executed application
  return status;
}
