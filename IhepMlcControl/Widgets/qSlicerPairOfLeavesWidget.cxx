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

public:
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

  std::pair< int, int > m_PairOfLeavesNumbers; // first = bottom (min), second = top (max)
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
    d->m_RequiredValues = std::make_pair( requiredMin, this->Superclass::maximum() - requiredMax);
    d->m_CurrentValues = std::make_pair( currentMin, this->Superclass::maximum() - currentMax);
  }
  else
  {
    this->Superclass::setRange(0, 99);
    d->m_RequiredValues = std::make_pair<int, int>( 5, 70);
    d->m_CurrentValues = std::make_pair<int, int>( 0, 99);
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
  int range = this->Superclass::maximum() - this->Superclass::minimum();
  int xCurrTop, yCurrTop, xCurrBottom, yCurrBottom;
  
  painter.setBrush(QBrush(Qt::blue, Qt::SolidPattern));
  
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
      
    painter.setPen(Qt::red);
    painter.drawLine(0, yCurrBottom, widget_size.width(), yCurrBottom);
    painter.drawLine(0, yCurrTop, widget_size.width(), yCurrTop);

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
  
    painter.setPen(Qt::red);
    painter.drawLine(xCurrBottom, 0, xCurrBottom, widget_size.height());
    painter.drawLine(xCurrTop, 0, xCurrTop, widget_size.height());
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

  emit this->maxRangeChanged(this->getMaxRange());
  emit this->minRangeChanged(this->getMinRange());
  emit this->positionsChanged(d->m_RequiredValues.first, this->Superclass::maximum() - d->m_RequiredValues.second);
  emit this->openingChanged(this->getRequiredOpening(), this->getCurrentOpening());

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
  QSize widget_size = this->size();
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
    switch (this->orientation())
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

    switch (this->orientation())
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
          this->setCursor(Qt::SizeHorCursor);
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

//-----------------------------------------------------------------------------
void qSlicerAbstractPairOfLeavesWidget::setMinRequiredValue(int min)
{
  Q_D(qSlicerAbstractPairOfLeavesWidget);
  d->m_RequiredValues.first = min;
  this->update();
  emit this->valuesPositionChanged( true, false);
  emit this->minRequiredCurrentGapChanged(d->m_RequiredValues.first - d->m_CurrentValues.first);
  emit this->openingChanged(this->getRequiredOpening(), this->getCurrentOpening());
}

//-----------------------------------------------------------------------------
void qSlicerAbstractPairOfLeavesWidget::setMinCurrentValue(int min)
{
  Q_D(qSlicerAbstractPairOfLeavesWidget);
  d->m_CurrentValues.first = min;
  this->update();
  emit this->minRequiredCurrentGapChanged(d->m_RequiredValues.first - min);
  emit this->openingChanged(this->getRequiredOpening(), this->getCurrentOpening());
}

//-----------------------------------------------------------------------------
void qSlicerAbstractPairOfLeavesWidget::setMaxRequiredValue(int max)
{
  Q_D(qSlicerAbstractPairOfLeavesWidget);
  d->m_RequiredValues.second = this->Superclass::maximum() - max;
  this->update();
  emit this->valuesPositionChanged( false, true);
  emit this->maxRequiredCurrentGapChanged(-1 * (d->m_RequiredValues.second - d->m_CurrentValues.second));
  emit this->openingChanged(this->getRequiredOpening(), this->getCurrentOpening());
}

//-----------------------------------------------------------------------------
void qSlicerAbstractPairOfLeavesWidget::setMaxCurrentValue(int max)
{
  Q_D(qSlicerAbstractPairOfLeavesWidget);
  d->m_CurrentValues.second = this->Superclass::maximum() - max;
  this->update();
  emit this->maxRequiredCurrentGapChanged(-1 * (d->m_RequiredValues.second - d->m_CurrentValues.second));
  emit this->openingChanged(this->getRequiredOpening(), this->getCurrentOpening());
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
