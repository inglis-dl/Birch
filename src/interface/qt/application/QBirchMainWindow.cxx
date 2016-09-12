/*=========================================================================

  Program:  Birch
  Module:   QBirchMainWindow.cxx
  Language: C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QBirchMainWindow.h>
#include <QBirchMainWindow_p.h>

// Birch includes
#include <QBirchDoubleSlider.h>
#include <QBirchSliceView.h>

// VTK includes
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkChartXY.h>
#include <vtkContextScene.h>
#include <vtkContextView.h>
#include <vtkDataArrayCollection.h>
#include <vtkEventForwarderCommand.h>
#include <vtkEventQtSlotConnect.h>
#include <vtkGDCMImageReader.h>
#include <vtkIdTypeArray.h>
#include <vtkImageHistogram.h>
#include <vtkImageSharpen.h>
#include <vtkIntArray.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkPNGWriter.h>
#include <vtkPlot.h>
#include <vtkPointData.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkTable.h>
#include <vtkWindowToImageFilter.h>

// Qt includes
#include <QCloseEvent>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QScrollBar>
#include <QSettings>
#include <QSignalMapper>
#include <QSlider>
#include <QString>
#include <QWidgetItem>

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QBirchMainWindowPrivate methods
//
// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchMainWindowPrivate::QBirchMainWindowPrivate(QBirchMainWindow& object)
  : QObject(&object), q_ptr(&object)
{
  this->qvtkConnection = vtkSmartPointer<vtkEventQtSlotConnect>::New();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchMainWindowPrivate::~QBirchMainWindowPrivate()
{
  this->qvtkConnection->Disconnect();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchMainWindowPrivate::showProgress(
  vtkObject*, unsigned long, void*, void* call_data)
{
  QProgressBar* progress = this->statusbar->findChild<QProgressBar*>();
  if (progress)
  {
    progress->setVisible(true);
    progress->setValue(0);
  }
  QPushButton* button = this->statusbar->findChild<QPushButton*>();
  if (button)
    button->setVisible(true);
  QString message = reinterpret_cast<const char*>(call_data);
  if (!message.isEmpty())
    this->statusbar->showMessage(message);

  this->statusbar->repaint();
  QApplication::processEvents();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchMainWindowPrivate::hideProgress()
{
  QProgressBar* progress = this->statusbar->findChild<QProgressBar*>();
  if (progress)
  {
    progress->setVisible(false);
    this->statusbar->clearMessage();
  }
  QPushButton* button = this->statusbar->findChild<QPushButton*>();
  if (button)
    button->setVisible(false);
  this->statusbar->clearMessage();
  this->statusbar->repaint();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchMainWindowPrivate::updateProgress(
  vtkObject*, unsigned long, void*, void* call_data)
{
  QProgressBar* progress = this->statusbar->findChild<QProgressBar*>();
  double value = *(reinterpret_cast<double*>(call_data));
  progress->setValue(static_cast<int>(100*value));
  QApplication::processEvents();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchMainWindowPrivate::setupUi(QMainWindow* window)
{
  Q_Q(QBirchMainWindow);

  this->Ui_QBirchMainWindow::setupUi(window);

  // connect the file menu items
  // connect the Open item to open a DICOM image
  connect(this->actionOpen, SIGNAL(triggered()),
    this, SLOT(slotOpen()));

  connect(this->actionSave, SIGNAL(triggered()),
    this, SLOT(slotSave()));

  for (int i = 0; i < this->MaxRecentFiles; ++i)
  {
    this->recentFileActs[i] = new QAction(this);
    this->recentFileActs[i]->setVisible(false);
    connect(this->recentFileActs[i], SIGNAL(triggered()),
            this, SLOT(openRecentFile()));
  }
  this->separatorAct = this->menuFile->addSeparator();
  for (int i = 0; i < this->MaxRecentFiles; ++i)
    this->menuFile->addAction(this->recentFileActs[i]);

  // connect the Exit item to close the application
  connect(this->actionExit, SIGNAL(triggered()),
    qApp, SLOT(closeAllWindows()));

  vtkRenderWindow* renwin =
    this->imageHistogramView->GetRenderWindow();
  vtkNew<vtkRenderer> ren;
  ren->SetBackground(1, 1, 1);
  renwin->AddRenderer(ren.GetPointer());

  // Set up a 2D scene, add an XY chart to it
  this->ContextView = vtkSmartPointer<vtkContextView>::New();
  this->ContextView->SetRenderWindow(renwin);
  vtkNew<vtkChartXY> chart;
  this->ContextView->GetScene()->AddItem(chart.GetPointer());

  this->imageWidget->reset();
  this->setCurrentFile("");

  q->setCorner(Qt::BottomLeftCorner, Qt::BottomDockWidgetArea);
  q->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

  this->radiusSlider->setSingleStep(0.1);
  this->radiusSlider->setPageStep(1.0);
  this->radiusSlider->setMaximum(10.0);
  this->stddevSlider->setSingleStep(0.1);
  this->stddevSlider->setPageStep(1.0);
  this->stddevSlider->setMaximum(10.0);

  this->signalMapper = new QSignalMapper(this);
  connect(this->radiusSlider, SIGNAL(valueChanged(double)),
    signalMapper, SLOT(map()));
  this->signalMapper->setMapping(this->radiusSlider, this->radiusLabel);
  connect(this->weightSlider, SIGNAL(valueChanged(int)),
    signalMapper, SLOT(map()));
  this->signalMapper->setMapping(this->weightSlider, this->weightLabel);
  connect(this->stddevSlider, SIGNAL(valueChanged(double)),
    signalMapper, SLOT(map()));
  this->signalMapper->setMapping(this->stddevSlider, this->stddevLabel);
  connect(this->signalMapper, SIGNAL(mapped(QWidget*)),
    this, SLOT(onMapped(QWidget*)));

  this->radiusSlider->setValue(this->radiusSlider->singleStep());
  this->weightSlider->setValue(this->weightSlider->singleStep());
  this->stddevSlider->setValue(this->stddevSlider->singleStep());

  connect(this->doSharpenPushButton, SIGNAL(clicked()),
    this, SLOT(sharpenImage()));
  connect(this->undoSharpenPushButton, SIGNAL(clicked()),
    this, SLOT(reloadImage()));

  QStringList args = QCoreApplication::arguments();
  if (1 < args.size() && QFile::exists(args.last()))
  {
    this->loadFile(args.last());
  }

  QProgressBar* progress = new QProgressBar();
  this->statusbar->addPermanentWidget(progress);
  progress->setVisible(false);
  QPushButton* button = new QPushButton("Abort");
  this->statusbar->addPermanentWidget(button);
  button->setVisible(false);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchMainWindowPrivate::onMapped(QWidget* widget)
{
  QObject* mappedWidget = this->signalMapper->mapping(widget);
  QLabel* label = qobject_cast<QLabel*>(widget);
  if (label && mappedWidget)
  {
    QString str = label->objectName();
    int size = str.size();
    int index = str.lastIndexOf(QString("Label"));
    str.chop(size - index);
    QString title = str.toLower();
    title[0] = str[0].toUpper();
    title.append(": ");
    QString widgetName = mappedWidget->metaObject()->className();
    if (widgetName == QString("QSlider"))
    {
      QSlider* slider = qobject_cast<QSlider*>(mappedWidget);
      title.append(QString::number(slider->value()));
    }
    else if (widgetName == QString("QBirchDoubleSlider"))
    {
      QBirchDoubleSlider* slider = qobject_cast<QBirchDoubleSlider*>(mappedWidget);
      title.append(QString::number(slider->value()));
      double radius = this->radiusSlider->value();
      double stddev = this->stddevSlider->value();
      QString kernelVal = QString::number(2 * static_cast<int>(stddev*radius) + 1);
      QString kernelStr("Kernel Size: ");
      kernelStr.append(kernelVal);
      kernelStr.append(" x ");
      kernelStr.append(kernelVal);
      this->kernelSizeLabel->setText(kernelStr);
    }
    label->setText(title);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchMainWindowPrivate::slotOpen()
{
  Q_Q(QBirchMainWindow);
  QFileDialog dialog(q);

  // this addresses a known and unfixable problem with native dialogs in KDE
  dialog.setOption(QFileDialog::DontUseNativeDialog);
  dialog.setNameFilter(
    tr("Images (*.dcm *.png *.jpg *.jpeg *.tif *.tiff *.gif *.mhd *.vti);;All files(*.*)"));
  dialog.setFileMode(QFileDialog::ExistingFile);
  dialog.setModal(true);
  if (dialog.exec())
  {
    QStringList fileNames = dialog.selectedFiles();
    if (fileNames.isEmpty()) return;
    QString fileName = fileNames.first();
    this->loadFile(fileName);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchMainWindowPrivate::slotSave()
{
  Q_Q(QBirchMainWindow);
  QString fileName = QFileDialog::getSaveFileName(q,
    QDialog::tr("Save Image to File"), "",
    QDialog::tr("Images (*.png *.pnm *.bmp *.jpg *.jpeg *.tif *.tiff)"));

  if (fileName.isEmpty())
  {
    return;
  }
  else
  {
    this->imageWidget->save(fileName);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchMainWindowPrivate::loadFile(const QString& fileName)
{
  Q_Q(QBirchMainWindow);
  bool success = true;
  try
  {
    vtkNew<vtkEventForwarderCommand> forward;
    vtkNew<vtkObject> dummy;
    forward->SetTarget(dummy.GetPointer());
    this->qvtkConnection->Connect(dummy.GetPointer(), vtkCommand::StartEvent,
      this, SLOT(showProgress(vtkObject*, unsigned long, void*, void*)));
    this->qvtkConnection->Connect(dummy.GetPointer(), vtkCommand::EndEvent,
      this, SLOT(hideProgress()));
    this->qvtkConnection->Connect(dummy.GetPointer(), vtkCommand::ProgressEvent,
      this, SLOT(updateProgress(vtkObject*, unsigned long, void*, void*)));
    this->imageWidget->load(fileName, forward.GetPointer());
  }
  catch (std::exception& e)
  {
    QMessageBox errorMessage(q);
    errorMessage.setWindowModality(Qt::WindowModal);
    errorMessage.setIcon(QMessageBox::Warning);
    errorMessage.setText(
      "There was an error while attempting to open the image.");
    errorMessage.exec();
    this->imageWidget->reset();
    success = false;
  }
  if (success)
    this->setCurrentFile(fileName);
  this->updateUi();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchMainWindowPrivate::setCurrentFile(const QString& fileName)
{
  this->currentFile = fileName;

  QSettings settings;
  QStringList files = settings.value("recentFileList").toStringList();
  files.removeAll(fileName);

  QFileInfo checkFile(fileName);
  if (checkFile.exists() && checkFile.isFile())
    files.prepend(fileName);
  while (files.size() > MaxRecentFiles)
    files.removeLast();

  settings.setValue("recentFileList", files);

  int numRecentFiles = qMin(files.size(), static_cast<int>(MaxRecentFiles));

  for (int i = 0; i < numRecentFiles; ++i)
  {
    QString text = tr("&%1 %2").arg(i + 1).arg(this->strippedName(files[i]));
    this->recentFileActs[i]->setText(text);
    this->recentFileActs[i]->setData(files[i]);
    this->recentFileActs[i]->setVisible(true);
  }
  for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
    this->recentFileActs[j]->setVisible(false);

  this->separatorAct->setVisible(numRecentFiles > 0);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchMainWindowPrivate::openRecentFile()
{
  QAction* action = qobject_cast<QAction*>(sender());
  if (action)
    this->loadFile(action->data().toString());
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QString QBirchMainWindowPrivate::strippedName(const QString& fullFileName)
{
  return QFileInfo(fullFileName).fileName();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchMainWindowPrivate::updateUi()
{
  this->buildHistogram();
  this->buildLabels();
  this->dicomTagWidget->load(this->currentFile);
  this->configureSharpenInterface();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchMainWindowPrivate::buildLabels()
{
  vtkImageData* image = this->imageWidget->imageData();
  if (!image) return;

  QString str;
  int n = image->GetNumberOfScalarComponents();
  str =  vtkVariant(n).ToString();
  this->labelImageChannelsValue->setText(str);

  double* o = image->GetOrigin();
  str =  vtkVariant(o[0]).ToString();
  str += ", ";
  str +=  vtkVariant(o[1]).ToString();
  str += ", ";
  str +=  vtkVariant(o[2]).ToString();
  this->labelImageOriginValue->setText(str);

  double* s = image->GetSpacing();
  str =  vtkVariant(s[0]).ToString();
  str += ", ";
  str +=  vtkVariant(s[1]).ToString();
  str += ", ";
  str +=  vtkVariant(s[2]).ToString();
  this->labelImageSpacingValue->setText(str);

  int* d = image->GetDimensions();
  str =  vtkVariant(d[0]).ToString();
  str += " x ";
  str +=  vtkVariant(d[1]).ToString();
  str += " x ";
  str +=  vtkVariant(d[2]).ToString();
  this->labelImageDimensionsValue->setText(str);

  str = image->GetScalarTypeAsString();
  this->labelImageScalarTypeValue->setText(str);

  double range[2];
  double min = image->GetScalarTypeMax();
  double max = image->GetScalarTypeMin();
  for (int i = 0; i < n; ++i)
  {
    image->GetPointData()->GetScalars()->GetRange(range, i);
    min = min < range[0] ? min : range[0];
    max = max > range[1] ? max : range[1];
  }
  str = "[";
  str += vtkVariant(min).ToString();
  str += ", ";
  str += vtkVariant(max).ToString();
  str += "]";
  this->labelImageScalarRangeValue->setText(str);

  QFileInfo info(this->currentFile);
  this->labelFileNameValue->setText(info.fileName());
  str = vtkVariant(info.size()).ToString();
  this->labelFileByteSizeValue->setText(str);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchMainWindowPrivate::buildHistogram()
{
  vtkChartXY* chart = vtkChartXY::SafeDownCast(
    this->ContextView->GetScene()->GetItem(0));
  if (!chart) return;

  vtkImageData* image = this->imageWidget->imageData();
  if (!image) return;

  int nc = image->GetNumberOfScalarComponents();
  double range[2];
  double min = image->GetScalarTypeMax();
  double max = image->GetScalarTypeMin();

  for (int i = 0; i < nc; ++i)
  {
    image->GetPointData()->GetScalars()->GetRange(range, i);
    min = min < range[0] ? min : range[0];
    max = max > range[1] ? max : range[1];
  }

  vtkNew<vtkImageHistogram> histogram;
  histogram->GenerateHistogramImageOff();
  histogram->SetInputData(image);
  histogram->SetBinOrigin(min);
  histogram->SetBinSpacing(1);
  histogram->SetNumberOfBins(static_cast<int>(max-min) + 1);
  histogram->AutomaticBinningOff();

  vtkNew<vtkDataArrayCollection> channels;
  for (int i = 0; i < nc; ++i)
  {
    histogram->SetActiveComponent(i);
    histogram->Update();
    vtkNew<vtkIdTypeArray> channel;
    channel->DeepCopy(histogram->GetHistogram());
    channel->SetName(vtkVariant(i).ToString());
    channels->AddItem(channel.GetPointer());
  }

  vtkNew<vtkTable> table;
  table->SetNumberOfRows(histogram->GetNumberOfBins());

  vtkNew<vtkIntArray> xvalues;
  xvalues->SetNumberOfValues(histogram->GetNumberOfBins());
  xvalues->SetName("values");
  table->AddColumn(xvalues.GetPointer());

  int start = static_cast<int>(histogram->GetBinOrigin());
  for (int i = 0; i < table->GetNumberOfRows(); ++i)
  {
    table->SetValue(i, 0, start + i);
  }

  vtkPlot* line = 0;
  chart->ClearPlots();
  vtkIdTypeArray* dataArray = 0;
  vtkCollectionSimpleIterator it;
  int i = 0;
  unsigned char r[3] = {255, 0, 0};
  unsigned char g[3] = {0, 255, 0};
  unsigned char b[3] = {0, 0, 255};

  for (channels->InitTraversal(it);
       (dataArray = vtkIdTypeArray::SafeDownCast(
        channels->GetNextItemAsObject(it))); ++i)
  {
    table->AddColumn(dataArray);
    line = chart->AddPlot(vtkChart::LINE);
    line->SetInputData(table.GetPointer(), 0, i+1);
    line->SetColor(r[i], g[i], b[i], 255);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchMainWindowPrivate::sharpenImage()
{
  vtkImageData* image = this->imageWidget->imageData();
  if (!image) return;

  double radius = this->radiusSlider->value();
  double weight = this->weightSlider->value();
  double stddev = this->stddevSlider->value();
  if (0.0 == weight) return;

  vtkNew<vtkImageSharpen> sharpen;
  sharpen->SetRadius(radius);
  sharpen->SetWeight(weight);
  sharpen->SetStandardDeviation(stddev);
  sharpen->SetInputData(image);

  this->qvtkConnection->Connect(sharpen.GetPointer(), vtkCommand::StartEvent,
    this, SLOT(showProgress(vtkObject*, unsigned long, void*, void*)));
  this->qvtkConnection->Connect(sharpen.GetPointer(), vtkCommand::EndEvent,
    this, SLOT(hideProgress()));
  this->qvtkConnection->Connect(sharpen.GetPointer(), vtkCommand::ProgressEvent,
    this, SLOT(updateProgress(vtkObject*, unsigned long, void*, void*)));

  sharpen->Update();
  this->imageWidget->setImageData(sharpen->GetOutput());
  this->updateUi();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchMainWindowPrivate::reloadImage()
{
  this->loadFile(this->currentFile);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchMainWindowPrivate::configureSharpenInterface()
{
  vtkImageData* image = this->imageWidget->imageData();
  bool enable = 2 == this->imageWidget->sliceView()->dimensionality() &&
                image && 1 == image->GetNumberOfScalarComponents() &&
                VTK_FLOAT != image->GetScalarType() &&
                VTK_DOUBLE != image->GetScalarType();
  QLayoutItem* child;
  for (int i = 0; i < this->sharpenGridLayout->count(); ++i)
  {
    QLayoutItem* const item = this->sharpenGridLayout->itemAt(i);
    if (item->widget())
      item->widget()->setEnabled(enable);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QBirchMainWindow methods
//
// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchMainWindow::QBirchMainWindow(QWidget* parent)
  : Superclass(parent)
  , d_ptr(new QBirchMainWindowPrivate(*this))
{
  Q_D(QBirchMainWindow);
  d->setupUi(this);
  d->updateUi();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QBirchMainWindow::~QBirchMainWindow()
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QBirchMainWindow::closeEvent(QCloseEvent* event)
{
  event->accept();
}
