/*=========================================================================

  Program:  Birch (CLSA Ultrasound Image Viewer)
  Module:   QMainBirchWindow.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

#ifndef __QMainBirchWindow_h
#define __QMainBirchWindow_h

#include <QStringList>
#include <QMainWindow>

class Ui_QMainBirchWindow;
class vtkMedicalImageViewer;
class vtkContextView;
class vtkEventQtSlotConnect;
class vtkObject;
class QAction;
class QWidget;
class QSignalMapper;

#include "vtkSmartPointer.h"

class QMainBirchWindow : public QMainWindow
{
  Q_OBJECT

public:
  QMainBirchWindow( QWidget* parent = 0 );
  ~QMainBirchWindow();

  void resetImage();
  void updateInterface();
  void buildHistogram();
  void buildLabels();
  void configureSharpenInterface();

public slots:
  // action event functions
  virtual void slotOpen();
  void openRecentFile();
  void initialize();
  void processEvents();
  void onMapped(QWidget*);
  void sharpenImage();
  void reloadImage();
  

protected:
  // called whenever the main window is closed
  virtual void closeEvent( QCloseEvent* );
  QStringList fileHistory;

//protected slots:

private:
  // Designer form
  Ui_QMainBirchWindow *ui;
  vtkSmartPointer<vtkMedicalImageViewer> Viewer;
  vtkSmartPointer<vtkContextView> ContextView;
  
  vtkSmartPointer<vtkEventQtSlotConnect> Connections;

  QString strippedName(const QString &fullFileName);
  void setCurrentFile(const QString &fileName);
  void loadFile(const QString &fileName);
  void buildSharpenInterface();

  QString currentFile;
  enum { MaxRecentFiles = 10 };
  QAction* recentFileActs[MaxRecentFiles];
  QAction* separatorAct;

  QSignalMapper* signalMapper;
};

#endif
