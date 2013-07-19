/*=========================================================================

  Program:  Birch (CLSA Ultrasound Image Viewer)
  Module:   QMainBirchWindow.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
#include "QMainBirchWindow.h"
#include "ui_QMainBirchWindow.h"

#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkMath.h"
#include "vtkPNGWriter.h"
#include "vtkSmartPointer.h"
#include "vtkWindowToImageFilter.h"
#include "vtkSmartPointer.h"
#include "vtkGDCMImageReader.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkMedicalImageViewer.h"

#include "vtkEventQtSlotConnect.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QString>
#include <QMessageBox>
#include <QScrollBar>
#include <QSettings>

#include "vtkImageHistogram.h"
#include "vtkIdTypeArray.h"
#include "vtkIntArray.h"
#include "vtkDataArrayCollection.h"
#include "vtkPointData.h"
#include "vtkChartXY.h"
#include "vtkPlot.h"
#include "vtkTable.h"
#include "vtkContextView.h"
#include "vtkContextScene.h"

 
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()
 

/*
class QMainBirchWindowProgressCommand : public vtkCommand
{
public:
  static QMainBirchWindowProgressCommand *New() { return new QMainBirchWindowProgressCommand; }
  void Execute( vtkObject *caller, unsigned long eventId, void *callData );
  Ui_QMainBirchWindow *ui;

protected:
  QMainBirchWindowProgressCommand() { this->ui = NULL; }
};
*/

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QMainBirchWindow::QMainBirchWindow( QWidget* parent )
  : QMainWindow( parent )
{
  QMenu *menu;
  
  this->ui = new Ui_QMainBirchWindow;
  this->ui->setupUi( this );
  
  // connect the file menu items
  // connect the Open item to open a DICOM image
  QObject::connect(
    this->ui->actionOpen, SIGNAL( triggered() ),
    this, SLOT( slotOpen() ) );

  for (int i = 0; i < MaxRecentFiles; ++i) 
  {
    this->recentFileActs[i] = new QAction(this);
    this->recentFileActs[i]->setVisible(false);
    connect(this->recentFileActs[i], SIGNAL(triggered()),
            this, SLOT(openRecentFile()));
  }
  this->separatorAct = this->ui->menuFile->addSeparator();
  for (int i = 0; i < MaxRecentFiles; ++i)
    this->ui->menuFile->addAction(this->recentFileActs[i]);

  // connect the Exit item to close the application  
  QObject::connect(
    this->ui->actionExit, SIGNAL( triggered() ),
    qApp, SLOT( closeAllWindows() ) );

  this->Viewer = vtkSmartPointer<vtkMedicalImageViewer>::New();

  vtkRenderWindow* renwin = this->ui->imageView->GetRenderWindow();

  vtkRenderer* renderer = this->Viewer->GetRenderer();
  renderer->GradientBackgroundOn();
  renderer->SetBackground( 0, 0, 0 );
  renderer->SetBackground2( 0, 0, 1 );
  this->Viewer->SetRenderWindow( renwin  );
  this->Viewer->InterpolateOff();

  renwin = this->ui->histogramView->GetRenderWindow();
  
  VTK_CREATE(vtkRenderer, ren);
  ren->SetBackground(1,1,1);
  renwin->AddRenderer( ren );
  // Set up a 2D scene, add an XY chart to it
  this->ContextView = vtkSmartPointer<vtkContextView>::New();
  
  this->ContextView->SetRenderWindow(renwin);
  VTK_CREATE(vtkChartXY, chart);
  this->ContextView->GetScene()->AddItem(chart);
  this->resetImage();

  this->ui->framePlayerWidget->setViewer(this->Viewer);
/*
  this->Connections = vtkSmartPointer<vtkEventQtSlotConnect>::New();
  this->Connections->Connect(this->Viewer->GetRenderWindow()->GetInteractor(),
    vtkCommand::MouseMoveEvent, this, SLOT(processEvents()) );
*/ 
  this->currentFile = "";

  this->setCorner( Qt::BottomLeftCorner, Qt::BottomDockWidgetArea );
  this->setCorner( Qt::BottomRightCorner, Qt::RightDockWidgetArea );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QMainBirchWindow::~QMainBirchWindow()
{
  this->ui->framePlayerWidget->play(false);
}

void QMainBirchWindow::processEvents()
{
  QApplication::processEvents();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainBirchWindow::initialize()
{
  QStringList args = QCoreApplication::arguments();

  if (args.size() > 1 && QFile::exists(args.last())) 
  {
    this->loadFile(args.last());
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainBirchWindow::closeEvent( QCloseEvent *event )
{
  this->ui->framePlayerWidget->setViewer(0);
  event->accept();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainBirchWindow::slotOpen()
{
  QFileDialog dialog( this );

  // this addresses a known and unfixable problem with native dialogs in KDE
  dialog.setOption( QFileDialog::DontUseNativeDialog );

  dialog.setNameFilter( tr( "Images (*.dcm *.png *.jpg *.jpeg *.tif *.tiff *.gif)" ) );
  dialog.setFileMode( QFileDialog::ExistingFile );
  dialog.setModal( true );
  
  if( dialog.exec() )
  {
    QStringList fileNames = dialog.selectedFiles();
    if( fileNames.isEmpty() ) return;
    QString fileName = fileNames.first();
    this->loadFile( fileName );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainBirchWindow::loadFile(const QString &fileName )
{
    try
    {
      this->Viewer->Load( fileName.toStdString() );
    }
    catch( std::exception &e )
    {
      QMessageBox errorMessage( this );
      errorMessage.setWindowModality( Qt::WindowModal );
      errorMessage.setIcon( QMessageBox::Warning );
      errorMessage.setText( "There was an error while attempting to open the image." );
      errorMessage.exec();

      this->Viewer->SetImageToSinusoid();
    }
  this->setCurrentFile( fileName );
  this->updateInterface();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainBirchWindow::setCurrentFile(const QString &fileName)
{
     this->currentFile = fileName;

     QSettings settings;
     QStringList files = settings.value("recentFileList").toStringList();
     files.removeAll(fileName);
     files.prepend(fileName);
     while (files.size() > MaxRecentFiles)
         files.removeLast();

     settings.setValue("recentFileList", files);

     int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

     for (int i = 0; i < numRecentFiles; ++i) {
         QString text = tr("&%1 %2").arg(i + 1).arg(this->strippedName(files[i]));
         this->recentFileActs[i]->setText(text);
         this->recentFileActs[i]->setData(files[i]);
         this->recentFileActs[i]->setVisible(true);
     }
     for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
         this->recentFileActs[j]->setVisible(false);

     this->separatorAct->setVisible(numRecentFiles > 0);
 }

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
 void QMainBirchWindow::openRecentFile()
 {
     QAction *action = qobject_cast<QAction *>(sender());
     if (action)
         loadFile(action->data().toString());
 }

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QString QMainBirchWindow::strippedName(const QString &fullFileName)
{
  return QFileInfo(fullFileName).fileName();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainBirchWindow::updateInterface()
{
  this->ui->framePlayerWidget->updateFromViewer();
  this->buildHistogram();
  this->buildLabels();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainBirchWindow::resetImage()
{
  this->Viewer->SetImageToSinusoid();
  this->updateInterface();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainBirchWindow::buildLabels()
{
  vtkImageData* image = this->Viewer->GetInput();
  if(!image) return;

  QString str;
  int n = image->GetNumberOfScalarComponents();
  str =  vtkVariant(n).ToString();
  this->ui->label_7->setText( str );

  double* o = image->GetOrigin();
  str =  vtkVariant(o[0]).ToString();
  str += ", ";
  str +=  vtkVariant(o[1]).ToString();
  str += ", ";
  str +=  vtkVariant(o[2]).ToString();
  this->ui->label_8->setText( str );

  double* s = image->GetSpacing();
  str =  vtkVariant(s[0]).ToString();
  str += ", ";
  str +=  vtkVariant(s[1]).ToString();
  str += ", ";
  str +=  vtkVariant(s[2]).ToString();
  this->ui->label_9->setText( str );

  int* d = image->GetDimensions();
  str =  vtkVariant(d[0]).ToString();
  str += " x ";
  str +=  vtkVariant(d[1]).ToString();
  str += " x ";
  str +=  vtkVariant(d[2]).ToString();
  this->ui->label_10->setText( str );

  str = image->GetScalarTypeAsString();
  this->ui->label_11->setText( str );

  double range[2];
  double min = image->GetScalarTypeMax();
  double max = image->GetScalarTypeMin();
  for(int i = 0; i < n; i++)
  {
    image->GetPointData()->GetScalars()->GetRange(range,i);
    min = min < range[0] ? min : range[0];
    max = max > range[1] ? max : range[1];    
  }
  str = "[";
  str += vtkVariant(min).ToString();
  str += ", ";
  str += vtkVariant(max).ToString();
  str += "]";
  this->ui->label_12->setText( str );

  QFileInfo info(this->currentFile);
  this->ui->label_14->setText( info.fileName() );
  str = vtkVariant( info.size() ).ToString();
  this->ui->label_16->setText( str ); 
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainBirchWindow::buildHistogram()
{
  vtkChartXY* chart = vtkChartXY::SafeDownCast(this->ContextView->GetScene()->GetItem(0));
  if(!chart) return;

  vtkImageData* image = this->Viewer->GetInput();
  if(!image) return;

  int nc = image->GetNumberOfScalarComponents();
  double range[2];
  double min = image->GetScalarTypeMax();
  double max = image->GetScalarTypeMin();

  for(int i = 0; i < nc; i++)
  {
    image->GetPointData()->GetScalars()->GetRange(range,i);
    min = min < range[0] ? min : range[0];
    max = max > range[1] ? max : range[1];    
  } 
   
  VTK_CREATE(vtkImageHistogram, histogram);
  histogram->GenerateHistogramImageOff();
  histogram->SetInput(image);
  histogram->SetBinOrigin(min);
  histogram->SetBinSpacing(1);
  histogram->SetNumberOfBins( static_cast<int>(max-min) + 1 );
  histogram->AutomaticBinningOff();

  VTK_CREATE(vtkDataArrayCollection, channels);
  for(int i = 0; i < nc; i++)
  { 
    histogram->SetActiveComponent(i);
    histogram->Update();
    
    VTK_CREATE(vtkIdTypeArray, channel);

    channel->DeepCopy( histogram->GetHistogram() );
    channel->SetName(vtkVariant(i).ToString());
    channels->AddItem( channel);
  }

  VTK_CREATE(vtkTable, table);
  table->SetNumberOfRows(histogram->GetNumberOfBins());
 
  VTK_CREATE(vtkIntArray, xvalues);
  xvalues->SetNumberOfValues(histogram->GetNumberOfBins());
  xvalues->SetName("values");
  table->AddColumn(xvalues);
 
  int start = static_cast<int>(histogram->GetBinOrigin());

  for (int i = 0; i < table->GetNumberOfRows(); i++)
  {
    table->SetValue(i,0,start+i);     
  }

  vtkPlot *line = 0;
  chart->ClearPlots();
  vtkIdTypeArray* dataArray = 0;
  vtkCollectionSimpleIterator it;
  int i = 0;
  unsigned char r[3] = {255,0,0};
  unsigned char g[3] = {0,255,0};
  unsigned char b[3] = {0,0,255};

  for( channels->InitTraversal(it);
      (dataArray = vtkIdTypeArray::SafeDownCast(channels->GetNextItemAsObject(it)));i++)
  {
    table->AddColumn(dataArray);
    line = chart->AddPlot(vtkChart::LINE);
    line->SetInput(table, 0, i+1);
    line->SetColor(r[i], g[i], b[i], 255);
  }
}

