
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

  this->fillLeavesControlWidgetContainer( side1Address, side2Address,
    side1Max, side1CurrentPosition, side1RequiredPosition,
    side2Max, side2CurrentPosition, side2RequiredPosition);
/*
  connect( ui->PositionButtonGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(onRequiredPositionChanged(QAbstractButton*)));
  connect( ui->Side1DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onSide1RequiredValueChanged(double)));
  connect( ui->Side2DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onSide2RequiredValueChanged(double)));

  connect( ui->LeavesPairWidget, SIGNAL(minRangeChanged(int)), this, SLOT(onSide1LeafRangeChanged(int)));
  connect( ui->LeavesPairWidget, SIGNAL(maxRangeChanged(int)), this, SLOT(onSide2LeafRangeChanged(int)));
  connect( ui->LeavesPairWidget, SIGNAL(minRequiredCurrentGapChanged(int)), this, SLOT(onSide1GapChanged(int)));
  connect( ui->LeavesPairWidget, SIGNAL(maxRequiredCurrentGapChanged(int)), this, SLOT(onSide2GapChanged(int)));

  connect( ui->LeavesPairWidget, SIGNAL(positionsChanged(int,int)), this, SLOT(onLeafPairPositionChanged(int,int)));
  connect( ui->LeavesPairWidget, SIGNAL(valuesPositionChanged(bool,bool)), this, SLOT(onLeafPositionInPairChanged(bool,bool)));
*/
}

qSlicerIhepPairOfLeavesControlDialog::~qSlicerIhepPairOfLeavesControlDialog()
{
}

void qSlicerIhepPairOfLeavesControlDialog::fillLeavesControlWidgetContainer(int side1Address, int side2Address,
  int side1Max, int side1CurrentPosition, int side1RequiredPosition,
  int side2Max, int side2CurrentPosition, int side2RequiredPosition)
{
  Q_D(qSlicerIhepPairOfLeavesControlDialog);
/*
//  qDebug() << Q_FUNC_INFO << side1Max << " " << side2Max << " " << side1RequiredPosition << " " << side2RequiredPosition;
  d->labelSide1LeafNumber->setText(QObject::tr("Leaf\n#%1").arg(side1Address));
  d->labelSide2LeafNumber->setText(QObject::tr("Leaf\n#%1").arg(side2Address));

  d->Side1DoubleSpinBox->setRange(0, side1Max * DISTANCE_PER_STEP);
  d->Side2DoubleSpinBox->setRange(0, side2Max * DISTANCE_PER_STEP);

  d->Side1DoubleSpinBox->setValue(side1RequiredPosition * DISTANCE_PER_STEP);
  d->Side2DoubleSpinBox->setValue(side2RequiredPosition * DISTANCE_PER_STEP);

  d->LeavesPairWidget->setRange(0, side1Max + side2Max);
  d->LeavesPairWidget->setLeavesNumbers(side1Address, side2Address);
  d->LeavesPairWidget->setMinRequiredValue(side1RequiredPosition);
  d->LeavesPairWidget->setMaxRequiredValue(side2RequiredPosition);
  this->ui->LeavesPairWidget->setMinCurrentValue(side1CurrentPosition);
  this->ui->LeavesPairWidget->setMaxCurrentValue(side2CurrentPosition);

  this->onSide1CurrentValueChanged(side1CurrentPosition);
  this->onSide2CurrentValueChanged(side2CurrentPosition);

  this->ui->LeavesPairWidget->setControlEnabled(true);

  this->onLeafPositionInPairChanged(true, false);
  this->onLeafPositionInPairChanged(false, true);
*/
}

void qSlicerIhepPairOfLeavesControlDialog::getSidePositions(int& side1, int& side2)
{
//  this->ui->LeavesPairWidget->getMinMaxPositions(side1, side2);
}

void qSlicerIhepPairOfLeavesControlDialog::getSide1Range(int& min, int& max)
{
//  min = 0;
//  max = this->ui->LeavesPairWidget->getMinRange();
}

void qSlicerIhepPairOfLeavesControlDialog::getSide2Range(int& min, int& max)
{
//  min = 0;
//  max = this->ui->LeavesPairWidget->getMaxRange();
}

void qSlicerIhepPairOfLeavesControlDialog::onSide1LeafRangeChanged(int range)
{
//  qDebug() << Q_FUNC_INFO << "Side1 range: " << range * DISTANCE_PER_STEP;
//  this->ui->Side1DoubleSpinBox->setRange(0, range * DISTANCE_PER_STEP);
}

void qSlicerIhepPairOfLeavesControlDialog::onSide2LeafRangeChanged(int range)
{
//  qDebug() << Q_FUNC_INFO << "Side2 range: " << range * DISTANCE_PER_STEP;
//  this->ui->Side2DoubleSpinBox->setRange(0, range * DISTANCE_PER_STEP);
}

void qSlicerIhepPairOfLeavesControlDialog::onLeafPairPositionChanged(int min, int max)
{
//  qDebug() << Q_FUNC_INFO << "Side1 value: " << min << " side2 value: " << max;
//  this->ui->Side1DoubleSpinBox->setValue(min * DISTANCE_PER_STEP);
//  this->ui->Side2DoubleSpinBox->setValue(max * DISTANCE_PER_STEP);
}

void qSlicerIhepPairOfLeavesControlDialog::onRequiredPositionChanged(QAbstractButton* button)
{
/*
  QRadioButton* rButton = qobject_cast<QRadioButton*>(button);
  if (!rButton)
  {
    return;
  }
  if (rButton == this->ui->InitialRadioButton)
  {
    this->ui->LeavesPairWidget->setMinRequiredValue(initialPosition.first);
    this->ui->LeavesPairWidget->setMaxRequiredValue(initialPosition.second);
    this->ui->Side1DoubleSpinBox->setValue(initialPosition.first * DISTANCE_PER_STEP);
    this->ui->Side2DoubleSpinBox->setValue(initialPosition.second * DISTANCE_PER_STEP);
  }
  else if (rButton == this->ui->OpenRadioButton)
  {
    this->ui->LeavesPairWidget->setMinRequiredValue(0);
    this->ui->LeavesPairWidget->setMaxRequiredValue(0);
    this->ui->Side1DoubleSpinBox->setValue(0.);
    this->ui->Side2DoubleSpinBox->setValue(0.);
  }
  else if (rButton == this->ui->CloseRadioButton)
  {
    int max = this->ui->LeavesPairWidget->maximum();
    this->ui->LeavesPairWidget->setMinRequiredValue(max / 2);
    this->ui->LeavesPairWidget->setMaxRequiredValue(max / 2);
    this->ui->Side1DoubleSpinBox->setValue((max * DISTANCE_PER_STEP) / 2.);
    this->ui->Side2DoubleSpinBox->setValue((max * DISTANCE_PER_STEP) / 2.);
  }
*/
}

void qSlicerIhepPairOfLeavesControlDialog::onSide1RequiredValueChanged(double value)
{
//  this->ui->LeavesPairWidget->setMinRequiredValue(value / DISTANCE_PER_STEP);
}

void qSlicerIhepPairOfLeavesControlDialog::onSide2RequiredValueChanged(double value)
{
//  this->ui->LeavesPairWidget->setMaxRequiredValue(value / DISTANCE_PER_STEP);
}

void qSlicerIhepPairOfLeavesControlDialog::onSide1CurrentValueChanged(int value)
{
//  this->ui->LeavesPairWidget->setMinCurrentValue(value);
//  this->ui->Side1CurrentLabel->setText(tr("%1 mm").arg(value * DISTANCE_PER_STEP));
//  int gap = this->ui->LeavesPairWidget->getMinPosition() - value;
//  this->onSide1GapChanged(gap);
}

void qSlicerIhepPairOfLeavesControlDialog::onSide2CurrentValueChanged(int value)
{
//  this->ui->LeavesPairWidget->setMaxCurrentValue(value);
//  this->ui->Side2CurrentLabel->setText(tr("%1 mm").arg(value * DISTANCE_PER_STEP));
//  int gap = this->ui->LeavesPairWidget->getMaxPosition() - value;
//  this->onSide2GapChanged(gap);
}

void qSlicerIhepPairOfLeavesControlDialog::onSide1GapChanged(int value)
{
//  float gap = value * DISTANCE_PER_STEP;
//  gap *= 100.f;
//  gap = std::roundf(gap);
//  gap /= 100.f;
//  this->ui->Side1GapLabel->setText(tr("%1 mm").arg(gap));
}

void qSlicerIhepPairOfLeavesControlDialog::onSide2GapChanged(int value)
{
//  float gap = value * DISTANCE_PER_STEP;
//  gap *= 100.f;
//  gap = std::roundf(gap);
//  gap /= 100.f;
//  this->ui->Side2GapLabel->setText(tr("%1 mm").arg(gap));
}

void qSlicerIhepPairOfLeavesControlDialog::onLeafPositionInPairChanged(bool side1, bool side2)
{
  if (side1 && !side2)
  {
//    int maxRange = this->ui->LeavesPairWidget->getMaxRange();
///    this->ui->Side2DoubleSpinBox->setRange(0, maxRange * DISTANCE_PER_STEP);
//    qDebug() << Q_FUNC_INFO << "Max range: " << maxRange * DISTANCE_PER_STEP;
  }
  else if (!side1 && side2)
  {
//    int minRange = this->ui->LeavesPairWidget->getMinRange();
///    this->ui->Side1DoubleSpinBox->setRange(0, minRange * DISTANCE_PER_STEP);
//    qDebug() << Q_FUNC_INFO << "Min range: " << minRange * DISTANCE_PER_STEP;
  }
//  qDebug() << Q_FUNC_INFO << "Leaf in pair changed side1: " << side1 << ", side2: " << side2;
}
