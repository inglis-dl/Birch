/*=========================================================================

  Program:  Birch (A simple image viewer)
  Module:   QBirchApplication.cxx
  Language: C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include "QBirchApplication.h"

// Qt includes
#include <QErrorMessage>

// C++ includes
#include <stdexcept>
#include <iostream>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QBirchApplication::notify(QObject* pObject, QEvent* pEvent)
{
  try
  {
    return QApplication::notify(pObject, pEvent);
  }
  catch(std::exception& e)
  {
    // catch any exception and display it to the user
    QErrorMessage* dialog = new QErrorMessage(this->activeWindow());
    dialog->setModal(true);
    dialog->showMessage(QDialog::tr(e.what()));
  }

  return false;
}
