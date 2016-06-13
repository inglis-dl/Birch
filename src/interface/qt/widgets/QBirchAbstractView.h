/*=======================================================================

  Module:    QBirchAbstractView.h
  Program:   Birch
  Language:  C++
  Author:    Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

#ifndef __QBirchAbstractView_h
#define __QBirchAbstractView_h

// Qt includes
#include <QWidget>

// VTK includes
#include <QVTKWidget.h>

class QBirchAbstractViewPrivate;

class vtkInteractorObserver;
class vtkRenderWindowInteractor;
class vtkRenderWindow;

class QBirchAbstractView : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(bool orientationDisplay READ orientationDisplay
    WRITE setOrientationDisplay)
  Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
  Q_PROPERTY(QColor foregroundColor READ foregroundColor WRITE setForegroundColor)
  Q_PROPERTY(bool gradientBackground READ gradientBackground WRITE setGradientBackground)

  public:
    typedef QWidget Superclass;
    explicit QBirchAbstractView(QWidget* parent = 0);
    virtual ~QBirchAbstractView();

    vtkRenderer* renderer();

  public Q_SLOTS:
    virtual void forceRender();
    virtual void setForegroundColor(const QColor& qcolor);
    virtual void setBackgroundColor(const QColor& qcolor);
    virtual void setGradientBackground(bool enable);
    void setOrientationDisplay(bool display);

  public:
    bool orientationDisplay() const;
    vtkRenderWindow* renderWindow() const;
    vtkRenderWindowInteractor* interactor() const;
    virtual void setInteractor(vtkRenderWindowInteractor* interactor);
    vtkInteractorObserver* interactorStyle() const;
    QVTKWidget* VTKWidget() const;
    virtual QColor backgroundColor() const;
    virtual QColor foregroundColor() const;
    virtual bool gradientBackground() const;
    virtual QSize minimumSizeHint() const;
    virtual QSize sizeHint() const;
    virtual bool hasHeightForWidth() const;
    virtual int heightForWidth(int width) const;

  protected:
    QScopedPointer<QBirchAbstractViewPrivate> d_ptr;
    QBirchAbstractView(QBirchAbstractViewPrivate* pimpl, QWidget* parent);

  private:
    Q_DECLARE_PRIVATE(QBirchAbstractView);
    Q_DISABLE_COPY(QBirchAbstractView);
};

#endif
