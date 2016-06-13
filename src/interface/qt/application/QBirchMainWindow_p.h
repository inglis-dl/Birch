/*=========================================================================

  Program:  Birch (CLSA Ultrasound Image Viewer)
  Module:   QBirchMainWindow_p.h
  Language: C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QBirchMainWindow_p_h
#define __QBirchMainWindow_p_h

// Birch includes
#include <QBirchMainWindow.h>
#include <ui_QBirchMainWindow.h>

// Qt incluides
#include <QStringList>
#include <QMainWindow>

// VTK includes
#include <vtkSmartPointer.h>

class vtkContextView;
class vtkEventQtSlotConnect;
class vtkObject;
class QAction;
class QWidget;
class QSignalMapper;

class QBirchMainWindowPrivate : public QObject, public Ui_QBirchMainWindow
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(QBirchMainWindow);
  protected:
    QBirchMainWindow* const q_ptr;

  public:
    explicit QBirchMainWindowPrivate(QBirchMainWindow& object);
    virtual ~QBirchMainWindowPrivate();

    void setupUi(QMainWindow* window);
    void updateUi();
  
    void buildHistogram();
    void buildLabels();
    void configureSharpenInterface();
 
  public slots:
    // action event functions
    virtual void slotOpen();
    virtual void slotSave();
    void openRecentFile();
    void onMapped(QWidget*);
    void sharpenImage();
    void reloadImage();

  protected:
    QStringList fileHistory;

  private:
    vtkSmartPointer<vtkContextView> ContextView;
    vtkSmartPointer<vtkEventQtSlotConnect> qvtkConnection;

    QString strippedName(const QString &fullFileName);
    void setCurrentFile(const QString &fileName);
    void loadFile(const QString &fileName);
    void saveFile(const QString &fileName);

    QString currentFile;
    enum { MaxRecentFiles = 10 };
    QAction* recentFileActs[MaxRecentFiles];
    QAction* separatorAct;

    QSignalMapper* signalMapper;
};

#endif
