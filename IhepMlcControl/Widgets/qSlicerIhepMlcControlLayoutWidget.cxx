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

// IhepMlcControl Widgets includes
#include "qSlicerIhepMlcControlLayoutWidget.h"
#include "ui_qSlicerIhepMlcControlLayoutWidget.h"

#include "qSlicerPairOfLeavesWidget.h"

namespace {

struct ContainerWidgets {
  QLabel* Side1Address{ nullptr };
  QLabel* Side1State{ nullptr };
  qSlicerAbstractPairOfLeavesWidget* PairOfLeavesWidget{ nullptr };
  QLabel* Side2Address{ nullptr };
  QLabel* Side2State{ nullptr };
};

}

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
  void init();
  void removeLeavesWidgetsFromLayout();
  bool isNumberOfLeafPairsChanged() const;

  /// IhepMlcControl MRML node containing shown parameters
  vtkWeakPointer<vtkMRMLIhepMlcControlNode> ParameterNode;

  std::vector< ContainerWidgets > ContainerWidgetsVector;
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

// --------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidgetPrivate::init()
{
  Q_Q(qSlicerIhepMlcControlLayoutWidget);

  // MLC layer button group
  QObject::connect( this->ButtonGroup_MlcLayout, SIGNAL(buttonClicked(QAbstractButton*)), 
    q, SLOT(onMlcLayerChanged(QAbstractButton*)));

}

// --------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidgetPrivate::removeLeavesWidgetsFromLayout()
{
  Q_Q(qSlicerIhepMlcControlLayoutWidget);
  for(ContainerWidgets& widgets : this->ContainerWidgetsVector)
  {
    if (widgets.Side1Address)
    {
      this->GridLayout_Leaves->removeWidget(widgets.Side1Address);

      delete widgets.Side1Address;
      widgets.Side1Address = nullptr;
    }
    if (widgets.Side1State)
    {
      this->GridLayout_Leaves->removeWidget(widgets.Side1State);

      delete widgets.Side1State;
      widgets.Side1State = nullptr;
    }
    if (widgets.Side2Address)
    {
      this->GridLayout_Leaves->removeWidget(widgets.Side2Address);

      delete widgets.Side2Address;
      widgets.Side2Address = nullptr;
    }
    if (widgets.Side2State)
    {
      this->GridLayout_Leaves->removeWidget(widgets.Side2State);

      delete widgets.Side2State;
      widgets.Side2State = nullptr;
    }
    if (widgets.PairOfLeavesWidget)
    {
      QObject::disconnect( widgets.PairOfLeavesWidget, SIGNAL(pairOfLeavesDoubleClicked()),
        q, SLOT(onPairOfLeavesDoubleClicked()));
      QObject::disconnect( widgets.PairOfLeavesWidget, SIGNAL(maxPositionChanged(int)),
        q, SLOT(onPairOfLeavesSize2ValueChanged(int)));
      QObject::disconnect( widgets.PairOfLeavesWidget, SIGNAL(minPositionChanged(int)),
        q, SLOT(onPairOfLeavesSize1ValueChanged(int)));

      this->GridLayout_Leaves->removeWidget(widgets.PairOfLeavesWidget);

      delete widgets.PairOfLeavesWidget;
      widgets.PairOfLeavesWidget = nullptr;
    }
  }
  this->ContainerWidgetsVector.clear();
}

// --------------------------------------------------------------------------
bool qSlicerIhepMlcControlLayoutWidgetPrivate::isNumberOfLeafPairsChanged() const
{
  return (this->ParameterNode->GetNumberOfLeafPairs() != static_cast<int>(this->ContainerWidgetsVector.size()));
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
  d->init();
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
  if (!d->ParameterNode)
  {
    return;
  }

  vtkMRMLIhepMlcControlNode::PairOfLeavesData pairOfLeaves;
  if (d->ParameterNode->GetPairOfLeavesData(pairOfLeaves, pairOfLeavesIndex))
  {
    const vtkMRMLIhepMlcControlNode::LeafData& side1 = pairOfLeaves.first;
    const vtkMRMLIhepMlcControlNode::LeafData& side2 = pairOfLeaves.second;

    ContainerWidgets widgets;

    widgets.Side1Address = new QLabel(tr("%1").arg(side1.Address), this);
    widgets.Side1Address->setAlignment(Qt::AlignHCenter);
    widgets.Side1Address->setMinimumSize(widgets.Side1Address->sizeHint());

    widgets.Side1State = new QLabel(this);
    widgets.Side1State->setAlignment(Qt::AlignHCenter);
    widgets.Side1State->setPixmap(QPixmap(":/indicators/Icons/gray.png"));

    widgets.PairOfLeavesWidget = new qSlicerVerticalPairOfLeavesWidget(
      side1.Range + side2.Range, side1.Steps, side2.Steps,
      side1.EncoderCounts, side2.EncoderCounts, this);
    widgets.PairOfLeavesWidget->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Expanding);

    widgets.Side2State = new QLabel(this);
    widgets.Side2State->setAlignment(Qt::AlignHCenter);
    widgets.Side2State->setPixmap(QPixmap(":/indicators/Icons/gray.png"));

    widgets.Side2Address = new QLabel(tr("%1").arg(side2.Address), this);
    widgets.Side2Address->setAlignment(Qt::AlignHCenter);
    widgets.Side2Address->setMinimumSize(widgets.Side2Address->sizeHint());

    d->GridLayout_Leaves->addWidget(widgets.Side2Address, 0, pairOfLeavesIndex + 1);
    d->GridLayout_Leaves->addWidget(widgets.Side2State, 1, pairOfLeavesIndex + 1);
    d->GridLayout_Leaves->addWidget(widgets.PairOfLeavesWidget, 2, pairOfLeavesIndex + 1);
    d->GridLayout_Leaves->addWidget(widgets.Side1State, 3, pairOfLeavesIndex + 1);
    d->GridLayout_Leaves->addWidget(widgets.Side1Address, 4, pairOfLeavesIndex + 1);

    widgets.PairOfLeavesWidget->setLeavesNumbers(side1.Address, side2.Address);
    widgets.PairOfLeavesWidget->setControlEnabled(false);

    connect( widgets.PairOfLeavesWidget, SIGNAL(pairOfLeavesDoubleClicked()), this, SLOT(onPairOfLeavesDoubleClicked()));
    connect( widgets.PairOfLeavesWidget, SIGNAL(maxPositionChanged(int)), this, SLOT(onPairOfLeavesSize2ValueChanged(int)));
    connect( widgets.PairOfLeavesWidget, SIGNAL(minPositionChanged(int)), this, SLOT(onPairOfLeavesSize1ValueChanged(int)));
    
    d->ContainerWidgetsVector.push_back(widgets);
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::setParameterNode(vtkMRMLNode* node)
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);
  vtkMRMLIhepMlcControlNode* parameterNode = vtkMRMLIhepMlcControlNode::SafeDownCast(node);
  // Each time the node is modified, the UI widgets are updated
  qvtkReconnect( d->ParameterNode, parameterNode, vtkCommand::ModifiedEvent, 
    this, SLOT( updateWidgetFromMRML() ) );

  d->ParameterNode = parameterNode;
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);
  if (!d->ParameterNode)
  {
    return;
  }

  if (d->isNumberOfLeafPairsChanged())
  {
    // Clear leaves container
    d->removeLeavesWidgetsFromLayout();

    // Fill leaves container with a new number of leaf pairs
    for (int i = 0; i < d->ParameterNode->GetNumberOfLeafPairs(); ++i)
    {
      this->fillLeavesControlContainer(i);
    }
  }
  else
  {
    // Set new leaf parameters
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::onMlcLayerChanged(QAbstractButton* button)
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);
  QRadioButton* radioButton = qobject_cast<QRadioButton*>(button);
  if (radioButton == d->RadioButton_MlcLayer1)
  {
    ;
  }
  else if (radioButton == d->RadioButton_MlcLayer2)
  {
    ;
  }
  else
  {
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::onPairOfLeavesDoubleClicked()
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::onPairOfLeavesSideValuesChanged(int,int)
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::onPairOfLeavesSide1RangeChanged(int,int)
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::onPairOfLeavesSide2RangeChanged(int,int)
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::onPairOfLeavesAddressChanged(bool,bool)
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::onPairOfLeavesSize2ValueChanged(int)
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::onPairOfLeavesSize1ValueChanged(int)
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);
}
