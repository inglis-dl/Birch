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

// Birch includes
#include <QBirchApplication.h>
#include <QBirchMainWindow.h>
#include <QBirchVTKOutputWindow.h>
#include <Utilities.h>

// Qt includes
#include <QObject>
#include <QString>

// C++ includes
#include <iostream>
#include <stdexcept>

// main function
int main(int argc, char** argv)
{
  int status = EXIT_SUCCESS;
  try
  {
    // create the user interface
    QBirchVTKOutputWindow::Install();
    QBirchApplication qapp(argc, argv);
    QBirchMainWindow mainWindow;

    mainWindow.show();

    // execute the application, then delete the application
    int status = qapp.exec();
  }
  catch(std::exception& e)
  {
    std::cerr << "Uncaught exception: " << e.what() << std::endl;
    status = EXIT_FAILURE;
  }

  // return the result of the executed application
  return status;
}
