/*=========================================================================

  Program:  Birch
  Module:   QBirchMainWindow.h
  Language: C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QBirchMainWindow_h
#define __QBirchMainWindow_h

// Qt includes
#include <QMainWindow>

class QBirchMainWindowPrivate;

class QBirchMainWindow : public QMainWindow
{
  Q_OBJECT

  public:
    typedef QMainWindow Superclass;
    explicit QBirchMainWindow(QWidget* parent = 0);
    virtual ~QBirchMainWindow();

  protected:
    QScopedPointer<QBirchMainWindowPrivate> d_ptr;

    // called whenever the main window is closed
    virtual void closeEvent(QCloseEvent* event);

  private:
    Q_DECLARE_PRIVATE(QBirchMainWindow);
    Q_DISABLE_COPY(QBirchMainWindow);
};

#endif
