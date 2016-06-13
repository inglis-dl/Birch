/*==============================================================================

  Program:   Birch
  Module:    QBirchImageControl.cxx
  Language:  C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

==============================================================================*/
#ifndef __QBirchImageControl_h
#define __QBirchImageControl_h

/**
 * @class QBirchImageControl
 *
 * @brief Qt widget for controlling images in a QBirchSliceView.
 *
 */

// Qt includes
#include <QColor>
#include <QIcon>
#include <QPointer>
#include <QWidget>

class QBirchSliceView;
class QBirchImageControlPrivate;

class QBirchImageControl : public QWidget
{
  Q_OBJECT

  /**
   * This property holds the flipVertical button's icon.
   * @see flipVerticalIcon(), setFlipVerticalIcon()
   */
  Q_PROPERTY(QIcon flipVerticalIcon READ flipVerticalIcon
    WRITE setFlipVerticalIcon)

  /**
   * This property holds the flipHorizontal button's icon.
   * @see flipHorizontalIcon(), setFlipHorizontalIcon()
   */
  Q_PROPERTY(QIcon flipHorizontalIcon READ flipHorizontalIcon
    WRITE setFlipHorizontalIcon)

  /**
   * This property holds the rotateClockwise button's icon.
   * @see rotateClockwiseIcon(), setRotateClockwiseIcon()
   */
  Q_PROPERTY(QIcon rotateClockwiseIcon READ rotateClockwiseIcon
    WRITE setRotateClockwiseIcon)

  /**
   * This property holds the rotateCounterClockwise button's icon.
   * @see rotateCounterClockwiseIcon(), setRotateCounterClockwiseIcon()
   */
  Q_PROPERTY(QIcon rotateCounterClockwiseIcon READ rotateCounterClockwiseIcon
    WRITE setRotateCounterClockwiseIcon)

  /**
   * This property holds the interpolation button's icon.
   * @see interpolationIcon(), setInterpolationIcon()
   */
  Q_PROPERTY(QIcon interpolationIcon READ interpolationIcon
    WRITE setInterpolationIcon)

  /**
   * This property holds the invertWindowLevel button's icon.
   * @see invertWindowLevelIcon(), setInvertWindowLevelIcon()
   */
  Q_PROPERTY(QIcon invertWindowLevelIcon READ invertWindowLevelIcon
    WRITE setInvertWindowLevelIcon)

  /**
   * This property holds the viewX button's icon.
   * @see viewXIcon(), setViewXIcon()
   */
  Q_PROPERTY(QIcon viewXIcon READ viewXIcon
    WRITE setViewXIcon)

  /**
   * This property holds the viewY button's icon.
   * @see viewYIcon(), setViewYIcon()
   */
  Q_PROPERTY(QIcon viewYIcon READ viewYIcon
    WRITE setViewYIcon)

  /**
   * This property holds the viewZ button's icon.
   * @see viewZIcon(), setViewZIcon()
   */
  Q_PROPERTY(QIcon viewZIcon READ viewZIcon
    WRITE setViewZIcon)

  /**
   * This property holds the color of the upper portion of the viewing area.
   */
  Q_PROPERTY(QColor foregroundColor READ foregroundColor
    WRITE setForegroundColor)

  /**
   * This property holds the color of the lower portion of the viewing area.
   */
  Q_PROPERTY(QColor backgroundColor READ backgroundColor
    WRITE setBackgroundColor)

  /**
   * This property holds the color of the annotations.
   */
  Q_PROPERTY(QColor annotationColor READ annotationColor
    WRITE setAnnotationColor)

  /**
   * This property holds the interpolation status (on or off).
   */
  Q_PROPERTY(bool interpolation READ interpolation WRITE setInterpolation)

  public:
    typedef QWidget Superclass;
    explicit QBirchImageControl(QWidget* parent = 0);
    virtual ~QBirchImageControl();

    //@{
    /**
      * Convenience method to set up signals and slots with a QBirchSliceView.
      */
    void setSliceView(QBirchSliceView* view);
    //@}

    //@{
    /** Set/Get the flip vertical icon. */
    void setFlipVerticalIcon(const QIcon& icon);
    QIcon flipVerticalIcon() const;
    //@}

    //@{
    /** Set/Get the flip horizontal icon. */
    void setFlipHorizontalIcon(const QIcon& icon);
    QIcon flipHorizontalIcon() const;
    //@}

    /** Set/Get the rotate clockwise icon. */
    void setRotateClockwiseIcon(const QIcon& icon);
    QIcon rotateClockwiseIcon() const;
    //@}

    //@{
    /** Set/Get the rotate counter clockwise icon. */
    void setRotateCounterClockwiseIcon(const QIcon& icon);
    QIcon rotateCounterClockwiseIcon() const;
    //@}

    //@{
    /** Set/Get the interpolation icon. */
    void setInterpolationIcon(const QIcon& icon);
    QIcon interpolationIcon() const;
    //@}

    //@{
    /** Set/Get the viewX icon. */
    void setViewXIcon(const QIcon& icon);
    QIcon viewXIcon() const;
    //@}

    //@{
    /** Set/Get the viewY icon. */
    void setViewYIcon(const QIcon& icon);
    QIcon viewYIcon() const;
    //@}

    //@{
    /** Set/Get the viewZ icon. */
    void setViewZIcon(const QIcon& icon);
    QIcon viewZIcon() const;
    //@}

    //@{
    /** Set/Get the invert window level icon. */
    void setInvertWindowLevelIcon(const QIcon& icon);
    QIcon invertWindowLevelIcon() const;
    //@}

    //@{
    /** Set/Get the background color. */
    void setBackgroundColor(const QColor& color);
    QColor backgroundColor() const;
    //@}

    //@{
    /** Set/Get the foreground color. */
    void setForegroundColor(const QColor& color);
    QColor foregroundColor() const;
    //@}

    //@{
    /** Set/Get the annotation color. */
    void setAnnotationColor(const QColor& color);
    QColor annotationColor() const;
    //@}

    //@{
    /** Set/Get the interpolation. */
    void setInterpolation(const bool& interp);
    bool interpolation() const;
    //@}

  public Q_SLOTS:
    void selectColor();
    void interpolate(bool interp);
    void update();
    void viewToAxis();

  protected:
    QScopedPointer<QBirchImageControlPrivate> d_ptr;
    QPointer<QBirchSliceView> sliceViewPointer;

  private:
    Q_DECLARE_PRIVATE(QBirchImageControl);
    Q_DISABLE_COPY(QBirchImageControl);
};

#endif
