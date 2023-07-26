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

// PairOfLeaves Widgets includes
#include "qSlicerPairOfLeavesWidget.h"

#include <QDebug>
#include <QSize>
#include <QBrush>
#include <QPainter>
#include <QMouseEvent>

#include <iostream>
#include <climits>
#include <cstdlib>

//-----------------------------------------------------------------------------
// qSlicerAbstractPairOfLeavesWidgetPrivate methods

class qSlicerAbstractPairOfLeavesWidgetPrivate
{
  Q_DECLARE_PUBLIC(qSlicerAbstractPairOfLeavesWidget);

protected:
  qSlicerAbstractPairOfLeavesWidget* const q_ptr;
public:
  qSlicerAbstractPairOfLeavesWidgetPrivate(qSlicerAbstractPairOfLeavesWidget& object);
  void init();

  /// Required position of a pair of leaves
  /// min - bottom leaf
  /// max - top leaf
  std::pair< int, int > m_RequiredValues{ 5, 70 }; // first = min, second = max
  /// Current position (RED color line)
  /// min - bottom leaf
  /// max - top leaf
  std::pair< int, int > m_CurrentValues{ 20, 60 }; // first = min, second = max

  /// margin between cursor position and leaf value
  int m_MarginSize{ 5 };
  qSlicerAbstractPairOfLeavesWidget::ValueChangeType m_ChangeType{ qSlicerAbstractPairOfLeavesWidget::NONE_CHANGED };

  std::pair< int, int > m_PairOfLeavesNumbers{ 28, 29 }; // first = bottom (min), second = top (max)
  bool m_LeavesControlEnabled{ true }; // if leaves control is enabled
};

//-----------------------------------------------------------------------------
qSlicerAbstractPairOfLeavesWidgetPrivate::qSlicerAbstractPairOfLeavesWidgetPrivate(qSlicerAbstractPairOfLeavesWidget& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
void qSlicerAbstractPairOfLeavesWidgetPrivate::init()
{
  Q_Q(qSlicerAbstractPairOfLeavesWidget);
  q->setRange(0, 99);
}


//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// qSlicerAbstractPairOfLeavesWidget methods

qSlicerAbstractPairOfLeavesWidget::qSlicerAbstractPairOfLeavesWidget(Qt::Orientation orientation, QWidget *parent)
  :
  Superclass(parent),
  d_ptr( new qSlicerAbstractPairOfLeavesWidgetPrivate(*this) )
{
  Q_D(qSlicerAbstractPairOfLeavesWidget);
  d->init();

  this->Superclass::setOrientation(orientation);

  switch (orientation)
  {
  case Qt::Horizontal:
    this->Superclass::setMinimumHeight(20);
    break;
  case Qt::Vertical:
    this->Superclass::setMinimumWidth(20);
    break;
  default:
    break;
  }
}

//-----------------------------------------------------------------------------
qSlicerAbstractPairOfLeavesWidget::qSlicerAbstractPairOfLeavesWidget(Qt::Orientation orientation,
  int range, int requiredMin, int requiredMax, int currentMin, int currentMax, QWidget *parent)
  :
  Superclass(parent),
  d_ptr( new qSlicerAbstractPairOfLeavesWidgetPrivate(*this) )
{
  Q_D(qSlicerAbstractPairOfLeavesWidget);
  d->init();

  this->Superclass::setOrientation(orientation);

  switch (orientation)
  {
  case Qt::Horizontal:
    this->Superclass::setMinimumHeight(20);
    break;
  case Qt::Vertical:
    this->Superclass::setMinimumWidth(20);
    break;
  default:
    break;
  }

  if (range > 0)
  {
    this->Superclass::setRange(0, range);
    d->m_RequiredValues = { requiredMin, this->Superclass::maximum() - requiredMax };
    d->m_CurrentValues = { currentMin, this->Superclass::maximum() - currentMax };
  }
}

//-----------------------------------------------------------------------------
qSlicerAbstractPairOfLeavesWidget::~qSlicerAbstractPairOfLeavesWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerAbstractPairOfLeavesWidget::paintEvent(QPaintEvent *event)
{
  Q_D(qSlicerAbstractPairOfLeavesWidget);
  Q_UNUSED(event);
  
  QPainter painter(this);
  painter.setPen(Qt::gray);
  
  QSize widget_size = this->Superclass::size();
  
  painter.setBrush(QBrush(Qt::white, Qt::SolidPattern));
  painter.drawRect(0, 0, widget_size.width(), widget_size.height());
  
  int x1top, x2top, y1top, y2top;
  int x1bottom, x2bottom, y1bottom, y2bottom;
  int range = this->maximum() - this->minimum();
  int xCurrTop, yCurrTop, xCurrBottom, yCurrBottom;
  
  painter.setBrush(QBrush(Qt::blue, Qt::SolidPattern));
  QPen redPen(Qt::red, 3, Qt::SolidLine);
///  qDebug() << Q_FUNC_INFO << "Current : Side1: " << d->m_CurrentValues.first << ", Side2: " << d->m_CurrentValues.second;
  switch (this->orientation())
  {
  case Qt::Vertical:
    x1bottom = 0;
    x2bottom = widget_size.width();
    y1bottom = static_cast<int>(widget_size.height() * double(this->Superclass::maximum() - d->m_RequiredValues.first) / double(range));
    y2bottom = widget_size.height();
    yCurrBottom = static_cast<int>(widget_size.height() * double(this->Superclass::maximum() - d->m_CurrentValues.first) / double(range));
  
    x1top = 0;
    x2top = widget_size.width();
    y1top = 0;
    y2top = static_cast<int>(widget_size.height() * double(this->Superclass::maximum() - d->m_RequiredValues.second) / double(range));
    yCurrTop = static_cast<int>(widget_size.height() * double(this->Superclass::maximum() - d->m_CurrentValues.second) / double(range));
  
    painter.drawRect(x1bottom, y1bottom, x2bottom, y2bottom);
    painter.drawRect(x1top, y1top, x2top, y2top);
  
    painter.drawLine(0, widget_size.height() / 2, widget_size.width(), widget_size.height() / 2);
///    qDebug() << Q_FUNC_INFO << ": yCurrTop: " << yCurrTop << ", yCurrBottom: " << yCurrBottom;
    painter.setPen(redPen);
    painter.drawLine(0, yCurrBottom, widget_size.width(), yCurrBottom);
    painter.drawLine(0, yCurrTop, widget_size.width(), yCurrTop);
    painter.setBrush(QBrush(Qt::red, Qt::SolidPattern));
    painter.drawRect(x1bottom + widget_size.width() / 2, yCurrBottom, x2bottom, y2bottom);
    painter.drawRect(x1top + widget_size.width() / 2, y1top, x2top, yCurrTop);
    break;
  case Qt::Horizontal:
    x1bottom = 0;
    x2bottom = static_cast<int>(widget_size.width() * (d->m_RequiredValues.first - this->Superclass::minimum()) / double(range));
    y1bottom = 0;
    y2bottom = widget_size.height();
    xCurrBottom = static_cast<int>(widget_size.width() * (d->m_CurrentValues.first - this->Superclass::minimum()) / double(range));
  
    x1top = static_cast<int>(widget_size.width() * d->m_RequiredValues.second / double(range));
    x2top = widget_size.width();
    y1top = 0;
    y2top = widget_size.height();
    xCurrTop = static_cast<int>(widget_size.width() * d->m_CurrentValues.second / double(range));
  
    painter.drawRect(x1bottom, y1bottom, x2bottom, y2bottom);
    painter.drawRect(x1top, y1top, x2top, y2top);
  
    painter.drawLine(widget_size.width() / 2, 0, widget_size.width() / 2, widget_size.height());
  
    painter.setPen(redPen);
    painter.drawLine(xCurrBottom, 0, xCurrBottom, widget_size.height());
    painter.drawLine(xCurrTop, 0, xCurrTop, widget_size.height());
    painter.setBrush(QBrush(Qt::red, Qt::SolidPattern));
    painter.drawRect(x1bottom, y1bottom, xCurrBottom + widget_size.height() / 2, y2bottom);
    painter.drawRect(xCurrTop, y1top, x2top + widget_size.height() / 2, y2top);
    break;
  default:
    break;
  }
}

//-----------------------------------------------------------------------------
void qSlicerAbstractPairOfLeavesWidget::mouseMoveEvent(QMouseEvent *event)
{
  Q_D(qSlicerAbstractPairOfLeavesWidget);
  if (!d->m_LeavesControlEnabled)
  {
    return;
  }

  QSize widget_size = this->Superclass::size();

  int v = 0;
  switch (this->Superclass::orientation())
  {
  case Qt::Horizontal:
    v = static_cast<int>(this->Superclass::maximum() * double(event->x()) / double(widget_size.width()));
    break;
  case Qt::Vertical:
  default:
    v = static_cast<int>(this->Superclass::maximum() * double(widget_size.height() - event->y()) / double(widget_size.height()));
    break;
  }

  switch (d->m_ChangeType)
  {
  case MIN_CHANGED:
    if (d->m_RequiredValues.second <= v)
    {
      v = d->m_RequiredValues.second;
    }
    if (v <= this->Superclass::minimum())
    {
      v = this->Superclass::minimum();
    }

    d->m_RequiredValues.first = v;

    break;
  case MAX_CHANGED:
    if (d->m_RequiredValues.first >= v)
    {
      v = d->m_RequiredValues.first;
    }
    if (v >= this->Superclass::maximum())
    {
      v = this->Superclass::maximum();
    }

    d->m_RequiredValues.second = v;

    break;
  default:
    break;
  }

  int minValue = d->m_RequiredValues.first;
  int minRange = this->getMinRange();
  int maxValue = this->Superclass::maximum() - d->m_RequiredValues.second;
  int maxRange = this->getMaxRange();

  emit this->maxRangeChanged(maxRange);
  emit this->minRangeChanged(minRange);
  emit this->positionsChanged(minValue, maxValue);
  emit this->openingChanged(this->getRequiredOpening(), this->getCurrentOpening());
  emit this->positionsRangesChanged(minValue, minRange, maxValue, maxRange);

  this->update();
}

//-----------------------------------------------------------------------------
void qSlicerAbstractPairOfLeavesWidget::mouseReleaseEvent(QMouseEvent *event)
{
  Q_D(qSlicerAbstractPairOfLeavesWidget);
  if (!d->m_LeavesControlEnabled)
  {
    return;
  }

  if (event->button() == Qt::LeftButton)
  {
    this->Superclass::setCursor(Qt::ArrowCursor);
    d->m_ChangeType = NONE_CHANGED;
  }
}

//-----------------------------------------------------------------------------
void qSlicerAbstractPairOfLeavesWidget::mousePressEvent(QMouseEvent *event)
{
  Q_D(qSlicerAbstractPairOfLeavesWidget);
  if (!d->m_LeavesControlEnabled)
  {
    return;
  }

  int bottom = 0, top = 0;
  int pos1 = 0, pos2 = 0;
  QSize widget_size = this->Superclass::size();
  int range = this->Superclass::maximum() - this->Superclass::minimum();

  switch (this->Superclass::orientation())
  {
  case Qt::Vertical:
    bottom = static_cast<int>(widget_size.height() * double(this->Superclass::maximum() - d->m_RequiredValues.first) / double(range));
    top = static_cast<int>(widget_size.height() * double(this->Superclass::maximum() - d->m_RequiredValues.second) / double(range));
    break;
  case Qt::Horizontal:
    bottom = static_cast<int>(widget_size.width() * d->m_RequiredValues.first / double(range));
    top = static_cast<int>(widget_size.width() * d->m_RequiredValues.second / double(range));
    break;
  default:
    break;
  }

  if (event->button() == Qt::LeftButton)
  {
    switch (this->Superclass::orientation())
    {
    case Qt::Vertical:
      pos1 = abs(event->y() - bottom);
      pos2 = abs(event->y() - top);
      break;
    case Qt::Horizontal:
      pos1 = abs(event->x() - bottom);
      pos2 = abs(event->x() - top);
      break;
    default:
      break;
    }

    switch (this->Superclass::orientation())
    {
    case Qt::Vertical:
      {
        if (pos1 < d->m_MarginSize) // min
        {
          this->Superclass::setCursor(Qt::SizeVerCursor);
          d->m_ChangeType = MIN_CHANGED;
          emit this->valuesPositionChanged( true, false);
        }
        else if (pos2 < d->m_MarginSize) // max
        {
          this->Superclass::setCursor(Qt::SizeVerCursor);
          d->m_ChangeType = MAX_CHANGED;
          emit this->valuesPositionChanged( false, true);
        }
        else if (pos1 < d->m_MarginSize && pos2 < d->m_MarginSize) // any
        {
        }
      }
      break;
    case Qt::Horizontal:
      {
        if (pos1 < d->m_MarginSize) // min
        {
          this->Superclass::setCursor(Qt::SizeHorCursor);
          d->m_ChangeType = MIN_CHANGED;
          emit this->valuesPositionChanged( true, false);
        }
        else if (pos2 < d->m_MarginSize) // max
        {
          this->Superclass::setCursor(Qt::SizeHorCursor);
          d->m_ChangeType = MAX_CHANGED;
          emit this->valuesPositionChanged( false, true);
        }
        else if (pos1 < d->m_MarginSize && pos2 < d->m_MarginSize) // any
        {
        }
      }
      break;
    default:
      break;
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerAbstractPairOfLeavesWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
  Q_D(qSlicerAbstractPairOfLeavesWidget);
  Q_UNUSED(event);
  emit this->pairOfLeavesDoubleClicked();
}

void
qSlicerAbstractPairOfLeavesWidget::setMinRequiredValue(int min)
{
  Q_D(qSlicerAbstractPairOfLeavesWidget);
  d->m_RequiredValues.first = min;

  emit this->valuesPositionChanged( true, false);
  emit this->minRequiredCurrentGapChanged(min - d->m_CurrentValues.first);
  emit this->openingChanged(this->getRequiredOpening(), this->getCurrentOpening());
  this->update();
}

void
qSlicerAbstractPairOfLeavesWidget::setMinCurrentValue(int min)
{
  Q_D(qSlicerAbstractPairOfLeavesWidget);
  d->m_CurrentValues.first = min;

  emit this->minRequiredCurrentGapChanged(d->m_RequiredValues.first - min);
  emit this->openingChanged(this->getRequiredOpening(), this->getCurrentOpening());
  this->update();
}

void
qSlicerAbstractPairOfLeavesWidget::setMaxRequiredValue(int max)
{
  Q_D(qSlicerAbstractPairOfLeavesWidget);
  d->m_RequiredValues.second = this->Superclass::maximum() - max;

  emit this->valuesPositionChanged( false, true);
  emit this->maxRequiredCurrentGapChanged(-1 * (d->m_RequiredValues.second - d->m_CurrentValues.second));
  emit this->openingChanged(this->getRequiredOpening(), this->getCurrentOpening());
  this->update();
}

void
qSlicerAbstractPairOfLeavesWidget::setMaxCurrentValue(int max)
{
  Q_D(qSlicerAbstractPairOfLeavesWidget);
  d->m_CurrentValues.second = this->Superclass::maximum() - max;

  emit this->maxRequiredCurrentGapChanged(-1 * (d->m_RequiredValues.second - d->m_CurrentValues.second));
  emit this->openingChanged(this->getRequiredOpening(), this->getCurrentOpening());
  this->update();
}

//-----------------------------------------------------------------------------
void qSlicerAbstractPairOfLeavesWidget::setMinRequiredValueFromLeafData(int min)
{
  Q_D(qSlicerAbstractPairOfLeavesWidget);
  d->m_RequiredValues.first = min;
  this->update();
}

//-----------------------------------------------------------------------------
void qSlicerAbstractPairOfLeavesWidget::setMaxRequiredValueFromLeafData(int max)
{
  Q_D(qSlicerAbstractPairOfLeavesWidget);
  d->m_RequiredValues.second = this->Superclass::maximum() - max;
  this->update();
}

//-----------------------------------------------------------------------------
void qSlicerAbstractPairOfLeavesWidget::setMinCurrentValueFromLeafData(int min)
{
  Q_D(qSlicerAbstractPairOfLeavesWidget);
  d->m_CurrentValues.first = min;
  this->update();
}

//-----------------------------------------------------------------------------
void qSlicerAbstractPairOfLeavesWidget::setMaxCurrentValueFromLeafData(int max)
{
  Q_D(qSlicerAbstractPairOfLeavesWidget);
  d->m_CurrentValues.second = this->Superclass::maximum() - max;
  this->update();
}

//-----------------------------------------------------------------------------
size_t qSlicerAbstractPairOfLeavesWidget::cursorMarginSize() const
{ 
  Q_D(const qSlicerAbstractPairOfLeavesWidget);
  return d->m_MarginSize;
}

//-----------------------------------------------------------------------------
void qSlicerAbstractPairOfLeavesWidget::setCursorMarginSize(size_t size)
{
  Q_D(qSlicerAbstractPairOfLeavesWidget);
  d->m_MarginSize = size;
}

//-----------------------------------------------------------------------------
bool qSlicerAbstractPairOfLeavesWidget::controlEnabled() const
{
  Q_D(const qSlicerAbstractPairOfLeavesWidget);
  return d->m_LeavesControlEnabled;
}

//-----------------------------------------------------------------------------
void qSlicerAbstractPairOfLeavesWidget::setControlEnabled(bool enabled)
{
  Q_D(qSlicerAbstractPairOfLeavesWidget);
  d->m_LeavesControlEnabled = enabled;
}

//-----------------------------------------------------------------------------
void qSlicerAbstractPairOfLeavesWidget::getMinMaxRequiredPositions(int& min, int& max) const
{
  Q_D(const qSlicerAbstractPairOfLeavesWidget);
  min = this->getMinRequiredPosition();
  max = this->getMaxRequiredPosition();
}

//-----------------------------------------------------------------------------
int qSlicerAbstractPairOfLeavesWidget::getMinRequiredPosition() const
{
  Q_D(const qSlicerAbstractPairOfLeavesWidget);
  return d->m_RequiredValues.first;
}

//-----------------------------------------------------------------------------
int qSlicerAbstractPairOfLeavesWidget::getMaxRequiredPosition() const
{
  Q_D(const qSlicerAbstractPairOfLeavesWidget);
  return (this->maximum() - d->m_RequiredValues.second);
}

//-----------------------------------------------------------------------------
void qSlicerAbstractPairOfLeavesWidget::getMinMaxCurrentPositions(int& min, int& max) const
{
  Q_D(const qSlicerAbstractPairOfLeavesWidget);
  min = this->getMinCurrentPosition();
  max = this->getMaxCurrentPosition();
}

//-----------------------------------------------------------------------------
int qSlicerAbstractPairOfLeavesWidget::getMinCurrentPosition() const
{
  Q_D(const qSlicerAbstractPairOfLeavesWidget);
  return d->m_CurrentValues.first;
}

//-----------------------------------------------------------------------------
int qSlicerAbstractPairOfLeavesWidget::getMaxCurrentPosition() const
{
  Q_D(const qSlicerAbstractPairOfLeavesWidget);
  return (this->maximum() - d->m_CurrentValues.second);
}

//-----------------------------------------------------------------------------
int qSlicerAbstractPairOfLeavesWidget::getMinRange() const
{
  Q_D(const qSlicerAbstractPairOfLeavesWidget);
  return d->m_RequiredValues.second;
}

//-----------------------------------------------------------------------------
int qSlicerAbstractPairOfLeavesWidget::getMaxRange() const
{
  Q_D(const qSlicerAbstractPairOfLeavesWidget);
  return (this->maximum() - d->m_RequiredValues.first);
}

//-----------------------------------------------------------------------------
void qSlicerAbstractPairOfLeavesWidget::setLeavesNumbers(int min, int max)
{
  Q_D(qSlicerAbstractPairOfLeavesWidget);
  d->m_PairOfLeavesNumbers.first = min;
  d->m_PairOfLeavesNumbers.second = max;
}
//-----------------------------------------------------------------------------
void qSlicerAbstractPairOfLeavesWidget::getLeavesNumbers(int& min, int& max) const
{
  Q_D(const qSlicerAbstractPairOfLeavesWidget);
  min = d->m_PairOfLeavesNumbers.first;
  max = d->m_PairOfLeavesNumbers.second;
}

//-----------------------------------------------------------------------------
int qSlicerAbstractPairOfLeavesWidget::getRequiredOpening() const
{
  Q_D(const qSlicerAbstractPairOfLeavesWidget);
  return (d->m_RequiredValues.second - d->m_RequiredValues.first);
}

//-----------------------------------------------------------------------------
int qSlicerAbstractPairOfLeavesWidget::getCurrentOpening() const
{
  Q_D(const qSlicerAbstractPairOfLeavesWidget);
  return (d->m_CurrentValues.second - d->m_CurrentValues.first);
}

//-----------------------------------------------------------------------------
qSlicerVerticalPairOfLeavesWidget::qSlicerVerticalPairOfLeavesWidget(QWidget *parent)
  :
  qSlicerAbstractPairOfLeavesWidget(Qt::Vertical, parent)
{
}

//-----------------------------------------------------------------------------
qSlicerVerticalPairOfLeavesWidget::qSlicerVerticalPairOfLeavesWidget(int range, int requiredMin, int requiredMax, int currentMin, int currentMax, QWidget *parent)
  :
  qSlicerAbstractPairOfLeavesWidget(Qt::Vertical, range, requiredMin, requiredMax, currentMin, currentMax, parent)
{
}

//-----------------------------------------------------------------------------
qSlicerVerticalPairOfLeavesWidget::~qSlicerVerticalPairOfLeavesWidget()
{
}

//-----------------------------------------------------------------------------
qSlicerHorizontalPairOfLeavesWidget::qSlicerHorizontalPairOfLeavesWidget(QWidget *parent)
  :
  qSlicerAbstractPairOfLeavesWidget(Qt::Horizontal, parent)
{
}

//-----------------------------------------------------------------------------
qSlicerHorizontalPairOfLeavesWidget::qSlicerHorizontalPairOfLeavesWidget(int range, int requiredMin, int requiredMax, int currentMin, int currentMax, QWidget *parent)
  :
  qSlicerAbstractPairOfLeavesWidget(Qt::Horizontal, range, requiredMin, requiredMax, currentMin, currentMax, parent)
{
}

//-----------------------------------------------------------------------------
qSlicerHorizontalPairOfLeavesWidget::~qSlicerHorizontalPairOfLeavesWidget()
{
}
