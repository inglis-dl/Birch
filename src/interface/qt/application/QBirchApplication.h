/*=========================================================================

  Program:  Birch (A simple image viewer)
  Module:   QBirchApplication.h
  Language: C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QBirchApplication_h
#define __QBirchApplication_h

// Qt includes
#include <QApplication>

class Ui_QBirchApplication;

class QBirchApplication : public QApplication
{
public:
  QBirchApplication(int& argc, char** argv) : QApplication(argc, argv) {}
  bool notify(QObject* pObject, QEvent* pEvent);
};

#endif
