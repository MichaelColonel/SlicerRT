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

// FooBar Widgets includes
#include "qSlicerIhepMlcControlLayoutWidget.h"
#include "ui_qSlicerIhepMlcControlLayoutWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_IhepMlcControl
class qSlicerIhepMlcControlLayoutWidgetPrivate
  : public Ui_qSlicerIhepMlcControlLayoutWidget
{
  Q_DECLARE_PUBLIC(qSlicerIhepMlcControlLayoutWidget);
protected:
  qSlicerIhepMlcControlLayoutWidget* const q_ptr;

public:
  qSlicerIhepMlcControlLayoutWidgetPrivate(
    qSlicerIhepMlcControlLayoutWidget& object);
  virtual void setupUi(qSlicerIhepMlcControlLayoutWidget*);
};

// --------------------------------------------------------------------------
qSlicerIhepMlcControlLayoutWidgetPrivate::qSlicerIhepMlcControlLayoutWidgetPrivate(
  qSlicerIhepMlcControlLayoutWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidgetPrivate::setupUi(qSlicerIhepMlcControlLayoutWidget* widget)
{
  this->Ui_qSlicerIhepMlcControlLayoutWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerIhepMlcControlLayoutWidget methods

//-----------------------------------------------------------------------------
qSlicerIhepMlcControlLayoutWidget::qSlicerIhepMlcControlLayoutWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerIhepMlcControlLayoutWidgetPrivate(*this) )
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerIhepMlcControlLayoutWidget
::~qSlicerIhepMlcControlLayoutWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::fillLeavesControlContainer(int pairOfLeavesIndex)
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);
  Q_UNUSED(pairOfLeavesIndex);
/*
  int side1Address = this->m_CurrentPairOfLeavesLayerPositions[value].side1.leafParameters->address;
  unsigned int* side1Range = this->m_CurrentPairOfLeavesLayerPositions[value].side1.leafParameters->range;
  int side1Required = this->m_CurrentPairOfLeavesLayerPositions[value].side1.leafParameters->steps;
  int side1Current = this->m_CurrentPairOfLeavesLayerPositions[value].side1.currentSteps;

  int side2Address = this->m_CurrentPairOfLeavesLayerPositions[value].side2.leafParameters->address;
  unsigned int* side2Range = this->m_CurrentPairOfLeavesLayerPositions[value].side2.leafParameters->range;
  int side2Required = this->m_CurrentPairOfLeavesLayerPositions[value].side2.leafParameters->steps;
  int side2Current = this->m_CurrentPairOfLeavesLayerPositions[value].side2.currentSteps;

  QLabel* side1 = new QLabel(tr("%1").arg(side1Address));
  side1->setAlignment(Qt::AlignHCenter);
  side1->setMinimumSize(side1->sizeHint());

  QLabel* side1Label = new QLabel(this);
  side1Label->setAlignment(Qt::AlignHCenter);
  side1Label->setPixmap(QPixmap(":/indicators/Icons/gray.png"));

  AbstractLeavesWidget* leavesPair = new VerticalLeavesWidget(side1Range[1] + side2Range[1], side1Required, side2Required, side1Current, side2Current, this);
  leavesPair->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Expanding);

  QLabel* side2Label = new QLabel(this);
  side2Label->setAlignment(Qt::AlignHCenter);
  side2Label->setPixmap(QPixmap(":/indicators/Icons/gray.png"));

  QLabel* side2 = new QLabel(tr("%1").arg(side2Address));
  side2->setAlignment(Qt::AlignHCenter);
  side2->setMinimumSize(side2->sizeHint());

  this->ui->leavesGridLayout->addWidget(side2, 0, value + 1);
  this->ui->leavesGridLayout->addWidget(side2Label, 1, value + 1);
  this->ui->leavesGridLayout->addWidget(leavesPair, 2, value + 1);
  this->ui->leavesGridLayout->addWidget(side1Label, 3, value + 1);
  this->ui->leavesGridLayout->addWidget(side1, 4, value + 1);

  leavesPair->setLeavesNumbers(side1Address, side2Address);
  leavesPair->setControlEnabled(false);

  leaverPairsWidgets[value].leavesPair = leavesPair;
  connect( leavesPair, SIGNAL(pairOfLeavesDoubleClicked()), this, SLOT(onLeavesPairDoubleClicked()));
  connect( leavesPair, SIGNAL(maxPositionChanged(int)), this, SLOT(onSize2ValueChanged(int)));
  connect( leavesPair, SIGNAL(minPositionChanged(int)), this, SLOT(onSize1ValueChanged(int)));
*/
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::setParameterNode(vtkMRMLNode* node)
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);
  Q_UNUSED(node);
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);
}
