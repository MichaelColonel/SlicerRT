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

#include "qSlicerIhepPairOfLeavesControlDialog.h"
#include "qSlicerPairOfLeavesWidget.h"

namespace {

struct ContainerWidgets {
  QLabel* Side1AddressLabel{ nullptr };
  QLabel* Side1StateLabel{ nullptr };
  qSlicerAbstractPairOfLeavesWidget* PairOfLeavesWidget{ nullptr };
  QLabel* Side2AddressLabel{ nullptr };
  QLabel* Side2StateLabel{ nullptr };
};

const struct PredefinedPositionDescription {
  vtkMRMLIhepMlcControlNode::PredefinedPositionType PredefinedPosition;
  const char* ComboBoxText;
  const char* DetailedDescriptionText;
} predefinedPosition[3] = {
  { vtkMRMLIhepMlcControlNode::Side1Edge, "Side-1 edge", "Edge or ramp on side-1" },
  { vtkMRMLIhepMlcControlNode::Side2Edge, "Side-2 edge", "Edge or ramp on side-2" },
  { vtkMRMLIhepMlcControlNode::PredefinedPosition_Last, nullptr, nullptr }
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

  int i = 0;
  while (predefinedPosition[i].PredefinedPosition != vtkMRMLIhepMlcControlNode::PredefinedPosition_Last)
  {
    this->ComboBox_MlcPositions->addItem(predefinedPosition[i].ComboBoxText);
    ++i;
  }

  // MLC layer button group
  QObject::connect( this->ButtonGroup_MlcLayout, SIGNAL(buttonClicked(QAbstractButton*)), 
    q, SLOT(onMlcLayerChanged(QAbstractButton*)));

  // Side1 and Side2 adjustment sliders
  QObject::connect( this->DoubleSlider_Side1Adjustment, SIGNAL(valueChanged(double)),
    q, SLOT(onSide1AdjustmentChanged(double)));
  QObject::connect( this->DoubleSlider_Side2Adjustment, SIGNAL(valueChanged(double)),
    q, SLOT(onSide2AdjustmentChanged(double)));
  QObject::connect( this->DoubleSlider_Side1Adjustment, SIGNAL(sliderReleased()),
    q, SLOT(onSide1AdjustmentSliderReleased()));
  QObject::connect( this->DoubleSlider_Side2Adjustment, SIGNAL(sliderReleased()),
    q, SLOT(onSide2AdjustmentSliderReleased()));

  // Layout widgets (buttons, combo box)
  QObject::connect( this->ComboBox_MlcPositions, SIGNAL(currentIndexChanged(int)),
    q, SLOT(onMlcPredefinedIndexChanged(int)));

  QObject::connect( this->PushButton_SetPredefinedMlcPositions, SIGNAL(clicked()),
    q, SLOT(onSetPredefinedMlcPositionsClicked()));
  QObject::connect( this->PushButton_ApplyPredefinedMlcPositions, SIGNAL(clicked()),
    q, SLOT(onApplyPredefinedMlcPositionsClicked()));
  QObject::connect( this->PushButton_OpenMlc, SIGNAL(clicked()),
    q, SLOT(onOpenMlcClicked()));
  QObject::connect( this->PushButton_CloseMlc, SIGNAL(clicked()),
    q, SLOT(onCloseMlcClicked()));
}

// --------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidgetPrivate::removeLeavesWidgetsFromLayout()
{
  Q_Q(qSlicerIhepMlcControlLayoutWidget);
  for(ContainerWidgets& widgets : this->ContainerWidgetsVector)
  {
    if (widgets.Side1AddressLabel)
    {
      this->GridLayout_Leaves->removeWidget(widgets.Side1AddressLabel);

      delete widgets.Side1AddressLabel;
      widgets.Side1AddressLabel = nullptr;
    }
    if (widgets.Side1StateLabel)
    {
      this->GridLayout_Leaves->removeWidget(widgets.Side1StateLabel);

      delete widgets.Side1StateLabel;
      widgets.Side1StateLabel = nullptr;
    }
    if (widgets.Side2AddressLabel)
    {
      this->GridLayout_Leaves->removeWidget(widgets.Side2AddressLabel);

      delete widgets.Side2AddressLabel;
      widgets.Side2AddressLabel = nullptr;
    }
    if (widgets.Side2StateLabel)
    {
      this->GridLayout_Leaves->removeWidget(widgets.Side2StateLabel);

      delete widgets.Side2StateLabel;
      widgets.Side2StateLabel = nullptr;
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

    widgets.Side1AddressLabel = new QLabel(tr("%1").arg(side1.Address), this);
    widgets.Side1AddressLabel->setAlignment(Qt::AlignHCenter);
    widgets.Side1AddressLabel->setMinimumSize(widgets.Side1AddressLabel->sizeHint());

    widgets.Side1StateLabel = new QLabel(this);
    widgets.Side1StateLabel->setAlignment(Qt::AlignHCenter);
    widgets.Side1StateLabel->setPixmap(QPixmap(":/indicators/Icons/gray.png"));

    widgets.PairOfLeavesWidget = new qSlicerVerticalPairOfLeavesWidget(
      side1.Range + side2.Range, side1.Steps, side2.Steps,
      side1.EncoderCounts, side2.EncoderCounts, this);
    widgets.PairOfLeavesWidget->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Expanding);

    widgets.Side2StateLabel = new QLabel(this);
    widgets.Side2StateLabel->setAlignment(Qt::AlignHCenter);
    widgets.Side2StateLabel->setPixmap(QPixmap(":/indicators/Icons/gray.png"));

    widgets.Side2AddressLabel = new QLabel(tr("%1").arg(side2.Address), this);
    widgets.Side2AddressLabel->setAlignment(Qt::AlignHCenter);
    widgets.Side2AddressLabel->setMinimumSize(widgets.Side2AddressLabel->sizeHint());

    d->GridLayout_Leaves->addWidget(widgets.Side2AddressLabel, 0, pairOfLeavesIndex + 1);
    d->GridLayout_Leaves->addWidget(widgets.Side2StateLabel, 1, pairOfLeavesIndex + 1);
    d->GridLayout_Leaves->addWidget(widgets.PairOfLeavesWidget, 2, pairOfLeavesIndex + 1);
    d->GridLayout_Leaves->addWidget(widgets.Side1StateLabel, 3, pairOfLeavesIndex + 1);
    d->GridLayout_Leaves->addWidget(widgets.Side1AddressLabel, 4, pairOfLeavesIndex + 1);

    widgets.PairOfLeavesWidget->setLeavesNumbers(side1.Address, side2.Address);
    widgets.PairOfLeavesWidget->setControlEnabled(false);

    connect( widgets.PairOfLeavesWidget, SIGNAL(pairOfLeavesDoubleClicked()),
      this, SLOT(onPairOfLeavesDoubleClicked()));
    connect( widgets.PairOfLeavesWidget, SIGNAL(maxPositionChanged(int)),
      this, SLOT(onPairOfLeavesSize2ValueChanged(int)));
    connect( widgets.PairOfLeavesWidget, SIGNAL(minPositionChanged(int)),
      this, SLOT(onPairOfLeavesSize1ValueChanged(int)));
    
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
  vtkMRMLIhepMlcControlNode::LayerType selectedMlcLayer = vtkMRMLIhepMlcControlNode::Layer_Last;
  if (radioButton == d->RadioButton_MlcLayer1)
  {
    selectedMlcLayer = vtkMRMLIhepMlcControlNode::Layer1;
  }
  else if (radioButton == d->RadioButton_MlcLayer2)
  {
    selectedMlcLayer = vtkMRMLIhepMlcControlNode::Layer2;
  }
  if (selectedMlcLayer != vtkMRMLIhepMlcControlNode::Layer_Last)
  {
    // Fill leaves container with a new number of leaf pairs
    for (int i = 0; i < d->ParameterNode->GetNumberOfLeafPairs(); ++i)
    {
      vtkMRMLIhepMlcControlNode::PairOfLeavesData leavesData;
      if (!d->ParameterNode->GetPairOfLeavesData( leavesData, i, selectedMlcLayer))
      {
        qWarning() << Q_FUNC_INFO << ": Unable to get pair of leaves data";
        break;
      }
      const vtkMRMLIhepMlcControlNode::LeafData& side1 = leavesData.first;
      const vtkMRMLIhepMlcControlNode::LeafData& side2 = leavesData.second;

      ContainerWidgets& pairOfLeavesWidgets = d->ContainerWidgetsVector[i];

      pairOfLeavesWidgets.Side1AddressLabel->setText(tr("%1").arg(side1.Address));
      pairOfLeavesWidgets.Side2AddressLabel->setText(tr("%1").arg(side2.Address));
      pairOfLeavesWidgets.PairOfLeavesWidget->setLeavesNumbers(side1.Address, side2.Address);
      pairOfLeavesWidgets.PairOfLeavesWidget->setMinCurrentValue(side1.EncoderCounts);
      pairOfLeavesWidgets.PairOfLeavesWidget->setMaxCurrentValue(side2.EncoderCounts);
      pairOfLeavesWidgets.PairOfLeavesWidget->setMinRequiredValue(side1.Steps);
      pairOfLeavesWidgets.PairOfLeavesWidget->setMaxRequiredValue(side2.Steps);
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::onPairOfLeavesDoubleClicked()
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);

  if (!d->ParameterNode)
  {
    return;
  }

  qSlicerAbstractPairOfLeavesWidget* widget = qobject_cast<qSlicerAbstractPairOfLeavesWidget*>(this->sender());
  if (widget && d->ContainerWidgetsVector.size())
  {
    int pairIndex = -1;

    for (size_t i = 0; i < d->ContainerWidgetsVector.size(); ++i)
    {
      ContainerWidgets& pairOfLeavesWidgets = d->ContainerWidgetsVector[i];
      if (pairOfLeavesWidgets.PairOfLeavesWidget == widget)
      {
        pairIndex = i;
        break;
      }
    }

    if (pairIndex == -1)
    {
      qWarning() << Q_FUNC_INFO << ": Invalid MLC widgets container index";
      return;
    }
    vtkMRMLIhepMlcControlNode::LayerType selectedMlcLayer = vtkMRMLIhepMlcControlNode::Layer_Last;
    if (d->RadioButton_MlcLayer1->isChecked())
    {
      selectedMlcLayer = vtkMRMLIhepMlcControlNode::Layer1;
    }
    else if (d->RadioButton_MlcLayer2->isChecked())
    {
      selectedMlcLayer = vtkMRMLIhepMlcControlNode::Layer2;
    }
    else
    {
      qWarning() << Q_FUNC_INFO << ": Invalid MLC layer value";
      return;
    }
    vtkMRMLIhepMlcControlNode::PairOfLeavesData leavesData;
    if (!d->ParameterNode->GetPairOfLeavesData( leavesData, pairIndex, selectedMlcLayer))
    {
      qWarning() << Q_FUNC_INFO << ": Unable to get pair of leaves data";
      return;
    }
    vtkMRMLIhepMlcControlNode::LeafData& side1 = leavesData.first;
    vtkMRMLIhepMlcControlNode::LeafData& side2 = leavesData.second;

    qSlicerIhepPairOfLeavesControlDialog controlDialog(
      side1.Address, side2.Address,
      side1.Range, side1.EncoderCounts, side1.Steps,
      side2.Range, side2.EncoderCounts, side2.Steps, this);
    int res = controlDialog.exec();
    if (res == QDialog::Accepted)
    {
      int side1Steps, side2Steps;
      controlDialog.getSidePositions( side1Steps, side2Steps);
      side1.Steps = side1Steps;
      side2.Steps = side2Steps;

      ContainerWidgets& pairOfLeavesWidgets = d->ContainerWidgetsVector[pairIndex];
      int side1Range = controlDialog.getSide1Range();
      Q_UNUSED(side1Range);

      int side2Range = controlDialog.getSide2Range();
      Q_UNUSED(side2Range);

      pairOfLeavesWidgets.PairOfLeavesWidget->setMinRequiredValue(side1Steps);
      pairOfLeavesWidgets.PairOfLeavesWidget->setMaxRequiredValue(side2Steps);
      d->ParameterNode->SetPairOfLeavesData( leavesData, pairIndex, selectedMlcLayer);
    }
  }
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

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::onSetPredefinedMlcPositionsClicked()
{
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::onApplyPredefinedMlcPositionsClicked()
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  vtkMRMLIhepMlcControlNode::LayerType selectedMlcLayer = vtkMRMLIhepMlcControlNode::Layer_Last;
  if (d->RadioButton_MlcLayer1->isChecked())
  {
    selectedMlcLayer = vtkMRMLIhepMlcControlNode::Layer1;
  }
  else if (d->RadioButton_MlcLayer2->isChecked())
  {
    selectedMlcLayer = vtkMRMLIhepMlcControlNode::Layer2;
  }

  int index = d->ComboBox_MlcPositions->currentIndex();
  vtkMRMLIhepMlcControlNode::PredefinedPositionType predefPosition = predefinedPosition[index].PredefinedPosition;
  if (predefPosition != vtkMRMLIhepMlcControlNode::PredefinedPosition_Last
    && selectedMlcLayer != vtkMRMLIhepMlcControlNode::Layer_Last)
  {
    d->ParameterNode->SetPredefinedPosition(selectedMlcLayer, predefPosition);

    // Fill leaves container with a new number of leaf pairs
    for (int i = 0; i < d->ParameterNode->GetNumberOfLeafPairs(); ++i)
    {
      vtkMRMLIhepMlcControlNode::PairOfLeavesData leavesData;
      if (!d->ParameterNode->GetPairOfLeavesData( leavesData, i, selectedMlcLayer))
      {
        qWarning() << Q_FUNC_INFO << ": Unable to get pair of leaves data";
        break;
      }
      ContainerWidgets& pairOfLeavesWidgets = d->ContainerWidgetsVector[i];

      const vtkMRMLIhepMlcControlNode::LeafData& side1 = leavesData.first;
      pairOfLeavesWidgets.PairOfLeavesWidget->setMinRequiredValue(side1.Steps);

      const vtkMRMLIhepMlcControlNode::LeafData& side2 = leavesData.second;
      pairOfLeavesWidgets.PairOfLeavesWidget->setMaxRequiredValue(side2.Steps);
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::onOpenMlcClicked()
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::onCloseMlcClicked()
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::onMlcPredefinedIndexChanged(int index)
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  qDebug() << Q_FUNC_INFO << ": Predefined MLC position index is changed";
  d->LineEdit_MlcDescription->setText(predefinedPosition[index].DetailedDescriptionText);
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::onSide1AdjustmentChanged(double v)
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  qDebug() << Q_FUNC_INFO << ": Side 1 adjustment " << v;
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::onSide2AdjustmentChanged(double v)
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  qDebug() << Q_FUNC_INFO << ": Side 2 adjustment " << v;
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::onSide1AdjustmentSliderReleased()
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  QSignalBlocker blocker1(d->DoubleSlider_Side1Adjustment);
  QSignalBlocker blocker2(d->DoubleSpinBox_Side1Adjustment);
  d->DoubleSlider_Side1Adjustment->setValue(0.);
  d->DoubleSpinBox_Side1Adjustment->setValue(0.);
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::onSide2AdjustmentSliderReleased()
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  QSignalBlocker blocker1(d->DoubleSlider_Side2Adjustment);
  QSignalBlocker blocker2(d->DoubleSpinBox_Side2Adjustment);
  d->DoubleSlider_Side2Adjustment->setValue(0.);
  d->DoubleSpinBox_Side2Adjustment->setValue(0.);
}