/*
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#ifndef __qSlicerPairOfLeavesWidget_h
#define __qSlicerPairOfLeavesWidget_h

// Qt includes
#include <QAbstractSlider>

// STD includes
#include <utility>


// PairOfLeaves Widgets includes
#include "qSlicerIhepMlcControlModuleWidgetsExport.h"

#define NOF_LEAVES 4
#define NOF_LEAVES_PER_SIDE ((NOF_LEAVES) >> 1)

#define DISTANCE                                                   15.8 // cm
#define NOF_TEETH                                                    28
#define DISTANCE_PER_TOOTH                                       0.1884 // cm
#define STEPS_PER_TURN                                              200

#define COUNTS_PER_TURN                                             200
#define DISTANCE_PER_TURN                                          0.8f // mm
#define DISTANCE_PER_STEP      ((DISTANCE_PER_TURN) / (STEPS_PER_TURN))

class qSlicerAbstractPairOfLeavesWidgetPrivate;

/// \ingroup Slicer_QtModules_IhepMlcControl
class Q_SLICER_MODULE_IHEPMLCCONTROL_WIDGETS_EXPORT qSlicerAbstractPairOfLeavesWidget
  : public QAbstractSlider
{
  Q_OBJECT
  Q_PROPERTY(size_t cursorPositionMarginSize READ cursorMarginSize WRITE setCursorMarginSize)
  Q_PROPERTY(bool enableLeavesControl READ controlEnabled WRITE setControlEnabled)

public:
  typedef QAbstractSlider Superclass;
  enum ValueChangeType : int { NONE_CHANGED = -1, MIN_CHANGED = 0, MAX_CHANGED = 1 };
  explicit qSlicerAbstractPairOfLeavesWidget(Qt::Orientation orientation, QWidget *parent = 0);
  explicit qSlicerAbstractPairOfLeavesWidget(Qt::Orientation orientation, int range, int requiredMin, int requiredMax, int currentMin, int currentMax, QWidget *parent = 0);
  ~qSlicerAbstractPairOfLeavesWidget() override;

  size_t cursorMarginSize() const;
  void setCursorMarginSize(size_t size);

  bool controlEnabled() const;
  void setControlEnabled(bool enabled = true);

  void getMinMaxRequiredPositions(int& min, int& max) const;
  int getMinRequiredPosition() const;
  int getMaxRequiredPosition() const;

  void getMinMaxCurrentPositions(int& min, int& max) const;
  int getMinCurrentPosition() const;
  int getMaxCurrentPosition() const;

  int getMinRange() const;
  int getMaxRange() const;
  void setLeavesNumbers(int min, int max);
  void getLeavesNumbers(int& min, int& max) const;
  int getRequiredOpening() const;
  int getCurrentOpening() const;

public slots:
  void setMinRequiredValue(int min);
  void setMaxRequiredValue(int min);
  void setMinCurrentValue(int min);
  void setMaxCurrentValue(int max);

protected:
  virtual void mouseDoubleClickEvent(QMouseEvent *); // change leaf pair settings
  virtual void mousePressEvent(QMouseEvent* ev); // change mouse cursor icon
  virtual void mouseReleaseEvent(QMouseEvent *ev); // restore mouse cursor icon
  virtual void mouseMoveEvent(QMouseEvent *ev); // redraw to a new position
  virtual void paintEvent(QPaintEvent *ev); // redraw filling

signals:
  void pairOfLeavesDoubleClicked();
  void positionsChanged(int, int);
  void minRangeChanged(int);
  void maxRangeChanged(int);
  void minPositionChanged(int);
  void maxPositionChanged(int);
  void valuesPositionChanged(bool, bool);
  void minRequiredCurrentGapChanged(int);
  void maxRequiredCurrentGapChanged(int);
  void openingChanged(int, int);

protected:
  QScopedPointer<qSlicerAbstractPairOfLeavesWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerAbstractPairOfLeavesWidget);
  Q_DISABLE_COPY(qSlicerAbstractPairOfLeavesWidget);
};

class Q_SLICER_MODULE_IHEPMLCCONTROL_WIDGETS_EXPORT qSlicerVerticalPairOfLeavesWidget
  : public qSlicerAbstractPairOfLeavesWidget
{
  Q_OBJECT
public:
  explicit qSlicerVerticalPairOfLeavesWidget(QWidget *parent = 0);
  explicit qSlicerVerticalPairOfLeavesWidget(int range, int requiredMin, int requiredMax, int currentMin, int currentMax, QWidget *parent = 0);
  ~qSlicerVerticalPairOfLeavesWidget() override;
};

class Q_SLICER_MODULE_IHEPMLCCONTROL_WIDGETS_EXPORT qSlicerHorizontalPairOfLeavesWidget
  : public qSlicerAbstractPairOfLeavesWidget
{
  Q_OBJECT
public:
  explicit qSlicerHorizontalPairOfLeavesWidget(QWidget *parent = 0);
  explicit qSlicerHorizontalPairOfLeavesWidget(int range, int requiredMin, int requiredMax, int currentMin, int currentMax, QWidget *parent = 0);
  ~qSlicerHorizontalPairOfLeavesWidget() override;
};

#endif
