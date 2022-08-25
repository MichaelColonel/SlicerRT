
/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#include <QDebug>
#include <QRadioButton>

// SlicerRT IhepMlcControl MRML includes
#include <vtkMRMLIhepMlcControlNode.h>

// qSlicerIhepPairOfLeavesControlDialog Widgets includes
#include "qSlicerIhepPairOfLeavesControlDialog.h"
#include "ui_qSlicerIhepPairOfLeavesControlDialog.h"

#include "qSlicerPairOfLeavesWidget.h"

// STD includes
#include <cmath>

#define DISTANCE                                                   15.8 // cm
#define NOF_TEETH                                                    28
#define DISTANCE_PER_TOOTH                                       0.1884 // cm
#define STEPS_PER_TURN                                              200

#define COUNTS_PER_TURN                                             200
#define DISTANCE_PER_TURN                                          0.8f // mm
#define DISTANCE_PER_STEP      ((DISTANCE_PER_TURN) / (STEPS_PER_TURN))

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_IhepMlcControl
class qSlicerIhepPairOfLeavesControlDialogPrivate
  : public Ui_qSlicerIhepPairOfLeavesControlDialog
{
  Q_DECLARE_PUBLIC(qSlicerIhepPairOfLeavesControlDialog);
protected:
  qSlicerIhepPairOfLeavesControlDialog* const q_ptr;

public:
  qSlicerIhepPairOfLeavesControlDialogPrivate(
    qSlicerIhepPairOfLeavesControlDialog& object);
  virtual void setupUi(qSlicerIhepPairOfLeavesControlDialog*);
  void init();

  std::pair< int, int > LeavesInitialPosition;
};

// --------------------------------------------------------------------------
qSlicerIhepPairOfLeavesControlDialogPrivate::qSlicerIhepPairOfLeavesControlDialogPrivate(
  qSlicerIhepPairOfLeavesControlDialog& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerIhepPairOfLeavesControlDialogPrivate::setupUi(qSlicerIhepPairOfLeavesControlDialog* widget)
{
  this->Ui_qSlicerIhepPairOfLeavesControlDialog::setupUi(widget);
}

// --------------------------------------------------------------------------
void qSlicerIhepPairOfLeavesControlDialogPrivate::init()
{
  Q_Q(qSlicerIhepPairOfLeavesControlDialog);
}

//-----------------------------------------------------------------------------
// qSlicerPairOfLeavesControlDialog methods

//-----------------------------------------------------------------------------
qSlicerIhepPairOfLeavesControlDialog::qSlicerIhepPairOfLeavesControlDialog(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerIhepPairOfLeavesControlDialogPrivate(*this) )
{
  Q_D(qSlicerIhepPairOfLeavesControlDialog);
  d->setupUi(this);
  d->init();
}

qSlicerIhepPairOfLeavesControlDialog::qSlicerIhepPairOfLeavesControlDialog(int side1Address, int side2Address,
  int side1Max, int side1CurrentPosition, int side1RequiredPosition,
  int side2Max, int side2CurrentPosition, int side2RequiredPosition,
  QWidget *parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerIhepPairOfLeavesControlDialogPrivate(*this) )
{
  Q_D(qSlicerIhepPairOfLeavesControlDialog);
  d->setupUi(this);
  d->init();

  d->LeavesInitialPosition.first = side1RequiredPosition;
  d->LeavesInitialPosition.second = side2RequiredPosition;

  this->fillLeavesControlWidgetContainer( side1Address, side2Address,
    side1Max, side1CurrentPosition, side1RequiredPosition,
    side2Max, side2CurrentPosition, side2RequiredPosition);

  connect( d->PositionButtonGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(onRequiredPositionChanged(QAbstractButton*)));
  connect( d->DoubleSpinBox_Side1Required, SIGNAL(valueChanged(double)), this, SLOT(onSide1RequiredValueChanged(double)));
  connect( d->DoubleSpinBox_Side2Required, SIGNAL(valueChanged(double)), this, SLOT(onSide2RequiredValueChanged(double)));

  connect( d->PairOfLeavesWidget, SIGNAL(minRangeChanged(int)), this, SLOT(onSide1LeafRangeChanged(int)));
  connect( d->PairOfLeavesWidget, SIGNAL(maxRangeChanged(int)), this, SLOT(onSide2LeafRangeChanged(int)));
  connect( d->PairOfLeavesWidget, SIGNAL(minRequiredCurrentGapChanged(int)), this, SLOT(onSide1GapChanged(int)));
  connect( d->PairOfLeavesWidget, SIGNAL(maxRequiredCurrentGapChanged(int)), this, SLOT(onSide2GapChanged(int)));

  connect( d->PairOfLeavesWidget, SIGNAL(positionsChanged(int,int)), this, SLOT(onLeafPairPositionChanged(int,int)));
  connect( d->PairOfLeavesWidget, SIGNAL(valuesPositionChanged(bool,bool)), this, SLOT(onLeafPositionInPairChanged(bool,bool)));
}

qSlicerIhepPairOfLeavesControlDialog::~qSlicerIhepPairOfLeavesControlDialog()
{
}

void qSlicerIhepPairOfLeavesControlDialog::fillLeavesControlWidgetContainer(int side1Address, int side2Address,
  int side1Max, int side1CurrentPosition, int side1RequiredPosition,
  int side2Max, int side2CurrentPosition, int side2RequiredPosition)
{
  Q_D(qSlicerIhepPairOfLeavesControlDialog);

  qDebug() << Q_FUNC_INFO << side1Max << " " << side2Max << " " << side1RequiredPosition << " " << side2RequiredPosition;
  d->Label_Side1LeafNumber->setText(QObject::tr("Leaf\n#%1").arg(side1Address));
  d->Label_Side2LeafNumber->setText(QObject::tr("Leaf\n#%1").arg(side2Address));

  d->DoubleSpinBox_Side1Required->setRange(0, side1Max * DISTANCE_PER_STEP);
  d->DoubleSpinBox_Side2Required->setRange(0, side2Max * DISTANCE_PER_STEP);

  d->DoubleSpinBox_Side1Required->setValue(side1RequiredPosition * DISTANCE_PER_STEP);
  d->DoubleSpinBox_Side2Required->setValue(side2RequiredPosition * DISTANCE_PER_STEP);

  d->PairOfLeavesWidget->setRange(0, side1Max + side2Max);
  d->PairOfLeavesWidget->setLeavesNumbers(side1Address, side2Address);
  d->PairOfLeavesWidget->setMinRequiredValue(side1RequiredPosition);
  d->PairOfLeavesWidget->setMaxRequiredValue(side2RequiredPosition);
  d->PairOfLeavesWidget->setMinCurrentValue(side1CurrentPosition);
  d->PairOfLeavesWidget->setMaxCurrentValue(side2CurrentPosition);

  this->onSide1CurrentValueChanged(side1CurrentPosition);
  this->onSide2CurrentValueChanged(side2CurrentPosition);

  d->PairOfLeavesWidget->setControlEnabled(true);

  this->onLeafPositionInPairChanged(true, false);
  this->onLeafPositionInPairChanged(false, true);
}

void qSlicerIhepPairOfLeavesControlDialog::getSidePositions(int& side1, int& side2)
{
  Q_D(qSlicerIhepPairOfLeavesControlDialog);
  d->PairOfLeavesWidget->getMinMaxPositions(side1, side2);
}

void qSlicerIhepPairOfLeavesControlDialog::getSide1Range(int& min, int& max)
{
  Q_D(qSlicerIhepPairOfLeavesControlDialog);
  min = 0;
  max = d->PairOfLeavesWidget->getMinRange();
}

void qSlicerIhepPairOfLeavesControlDialog::getSide2Range(int& min, int& max)
{
  Q_D(qSlicerIhepPairOfLeavesControlDialog);
  min = 0;
  max = d->PairOfLeavesWidget->getMaxRange();
}

void qSlicerIhepPairOfLeavesControlDialog::onSide1LeafRangeChanged(int range)
{
  Q_D(qSlicerIhepPairOfLeavesControlDialog);
  qDebug() << Q_FUNC_INFO << "Side1 range: " << range * DISTANCE_PER_STEP;
  d->DoubleSpinBox_Side1Required->setRange(0, range * DISTANCE_PER_STEP);
}

void qSlicerIhepPairOfLeavesControlDialog::onSide2LeafRangeChanged(int range)
{
  Q_D(qSlicerIhepPairOfLeavesControlDialog);
  qDebug() << Q_FUNC_INFO << "Side2 range: " << range * DISTANCE_PER_STEP;
  d->DoubleSpinBox_Side2Required->setRange(0, range * DISTANCE_PER_STEP);
}

void qSlicerIhepPairOfLeavesControlDialog::onLeafPairPositionChanged(int min, int max)
{
  Q_D(qSlicerIhepPairOfLeavesControlDialog);
  qDebug() << Q_FUNC_INFO << "Side1 value: " << min << " side2 value: " << max;
  d->DoubleSpinBox_Side1Required->setValue(min * DISTANCE_PER_STEP);
  d->DoubleSpinBox_Side2Required->setValue(max * DISTANCE_PER_STEP);
}

void qSlicerIhepPairOfLeavesControlDialog::onRequiredPositionChanged(QAbstractButton* button)
{
  Q_D(qSlicerIhepPairOfLeavesControlDialog);
  QRadioButton* rButton = qobject_cast<QRadioButton*>(button);
  if (!rButton)
  {
    return;
  }
  if (rButton == d->RadioButton_Initial)
  {
    d->PairOfLeavesWidget->setMinRequiredValue(d->LeavesInitialPosition.first);
    d->PairOfLeavesWidget->setMaxRequiredValue(d->LeavesInitialPosition.second);
    d->DoubleSpinBox_Side1Required->setValue(d->LeavesInitialPosition.first * DISTANCE_PER_STEP);
    d->DoubleSpinBox_Side2Required->setValue(d->LeavesInitialPosition.second * DISTANCE_PER_STEP);
  }
  else if (rButton == d->RadioButton_Open)
  {
    d->PairOfLeavesWidget->setMinRequiredValue(0);
    d->PairOfLeavesWidget->setMaxRequiredValue(0);
    d->DoubleSpinBox_Side1Required->setValue(0.);
    d->DoubleSpinBox_Side2Required->setValue(0.);
  }
  else if (rButton == d->RadioButton_Close)
  {
    int max = d->PairOfLeavesWidget->maximum();
    d->PairOfLeavesWidget->setMinRequiredValue(max / 2);
    d->PairOfLeavesWidget->setMaxRequiredValue(max / 2);
    d->DoubleSpinBox_Side1Required->setValue((max * DISTANCE_PER_STEP) / 2.);
    d->DoubleSpinBox_Side2Required->setValue((max * DISTANCE_PER_STEP) / 2.);
  }

}

void qSlicerIhepPairOfLeavesControlDialog::onSide1RequiredValueChanged(double value)
{
  Q_D(qSlicerIhepPairOfLeavesControlDialog);
  d->PairOfLeavesWidget->setMinRequiredValue(value / DISTANCE_PER_STEP);
}

void qSlicerIhepPairOfLeavesControlDialog::onSide2RequiredValueChanged(double value)
{
  Q_D(qSlicerIhepPairOfLeavesControlDialog);
  d->PairOfLeavesWidget->setMaxRequiredValue(value / DISTANCE_PER_STEP);
}

void qSlicerIhepPairOfLeavesControlDialog::onSide1CurrentValueChanged(int value)
{
  Q_D(qSlicerIhepPairOfLeavesControlDialog);
  d->PairOfLeavesWidget->setMinCurrentValue(value);
  d->Label_Side1Current->setText(tr("%1 mm").arg(value * DISTANCE_PER_STEP));
  int gap = d->PairOfLeavesWidget->getMinPosition() - value;
  this->onSide1GapChanged(gap);
}

void qSlicerIhepPairOfLeavesControlDialog::onSide2CurrentValueChanged(int value)
{
  Q_D(qSlicerIhepPairOfLeavesControlDialog);
  d->PairOfLeavesWidget->setMaxCurrentValue(value);
  d->Label_Side2Current->setText(tr("%1 mm").arg(value * DISTANCE_PER_STEP));
  int gap = d->PairOfLeavesWidget->getMaxPosition() - value;
  this->onSide2GapChanged(gap);
}

void qSlicerIhepPairOfLeavesControlDialog::onSide1GapChanged(int value)
{
  Q_D(qSlicerIhepPairOfLeavesControlDialog);
  float gap = value * DISTANCE_PER_STEP;
  gap *= 100.f;
  gap = std::roundf(gap);
  gap /= 100.f;
  d->Label_Side1Gap->setText(tr("%1 mm").arg(gap));
}

void qSlicerIhepPairOfLeavesControlDialog::onSide2GapChanged(int value)
{
  Q_D(qSlicerIhepPairOfLeavesControlDialog);
  float gap = value * DISTANCE_PER_STEP;
  gap *= 100.f;
  gap = std::roundf(gap);
  gap /= 100.f;
  d->Label_Side2Gap->setText(tr("%1 mm").arg(gap));
}

void qSlicerIhepPairOfLeavesControlDialog::onLeafPositionInPairChanged(bool side1, bool side2)
{
  Q_D(qSlicerIhepPairOfLeavesControlDialog);
  if (side1 && !side2)
  {
    int maxRange = d->PairOfLeavesWidget->getMaxRange();
    d->DoubleSpinBox_Side1Required->setRange(0, maxRange * DISTANCE_PER_STEP);
    qDebug() << Q_FUNC_INFO << "Max range: " << maxRange * DISTANCE_PER_STEP;
  }
  else if (!side1 && side2)
  {
    int minRange = d->PairOfLeavesWidget->getMinRange();
    d->DoubleSpinBox_Side2Required->setRange(0, minRange * DISTANCE_PER_STEP);
    qDebug() << Q_FUNC_INFO << "Min range: " << minRange * DISTANCE_PER_STEP;
  }
  qDebug() << Q_FUNC_INFO << "Leaf in pair changed side1: " << side1 << ", side2: " << side2;
}
