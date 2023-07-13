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

// Qt includes
#include <QDebug>
#include <QRadioButton>
// CTK includes
#include <ctkCheckBox.h>

// IhepMlcControl Widgets includes
#include "qSlicerIhepMlcControlLayoutWidget.h"
#include "ui_qSlicerIhepMlcControlLayoutWidget.h"

#include "qSlicerIhepPairOfLeavesControlDialog.h"
#include "qSlicerPairOfLeavesWidget.h"

namespace {

struct ContainerWidgets {
  ctkCheckBox* Side1EnabledCheckBox{ nullptr };
  QLabel* Side1AddressLabel{ nullptr };
  QLabel* Side1StateLabel{ nullptr };
  qSlicerAbstractPairOfLeavesWidget* PairOfLeavesWidget{ nullptr };
  QLabel* Side2AddressLabel{ nullptr };
  QLabel* Side2StateLabel{ nullptr };
  ctkCheckBox* Side2EnabledCheckBox{ nullptr };
};

const struct PredefinedPositionDescription {
  vtkMRMLIhepMlcControlNode::PredefinedPositionType PredefinedPosition;
  const char* ComboBoxText;
  const char* DetailedDescriptionText;
} predefinedPosition[8] = {
  { vtkMRMLIhepMlcControlNode::Side1Edge, "Side-1 edge", "Edge or ramp on side-1" },
  { vtkMRMLIhepMlcControlNode::Side2Edge, "Side-2 edge", "Edge or ramp on side-2" },
  { vtkMRMLIhepMlcControlNode::DoubleSidedEdge, "Double sided edge", "Edge or ramp from both sides" },
  { vtkMRMLIhepMlcControlNode::Square, "Square", "Square shape" },
  { vtkMRMLIhepMlcControlNode::Circle, "Circle", "Circle shape" },
  { vtkMRMLIhepMlcControlNode::Open, "Open", "Open both sides" },
  { vtkMRMLIhepMlcControlNode::Close, "Close", "Close both sides" },
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
  vtkMRMLIhepMlcControlNode::LayerType getSelectedMlcLayer() const;
  ContainerWidgets* getPairOfLeavesContainerByLeafAddress(int leafAddress, vtkMRMLIhepMlcControlNode::SideType& side);
  ContainerWidgets* getPairOfLeavesContainerByIndex(int index);
  void updateMlcPositionsFromLeavesData();

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

  QObject::connect( this->CheckBox_Side1Enabled, SIGNAL(stateChanged(int)),
    q, SLOT(onSide1StateChanged(int)));
  QObject::connect( this->CheckBox_Side2Enabled, SIGNAL(stateChanged(int)),
    q, SLOT(onSide2StateChanged(int)));
  QObject::connect( this->PushButton_ApplyPredefinedMlcPositions, SIGNAL(clicked()),
    q, SLOT(onApplyPredefinedMlcPositionsClicked()));
  QObject::connect( this->PushButton_OpenCurrentMlc, SIGNAL(clicked()),
    q, SLOT(onOpenMlcClicked()));
  QObject::connect( this->PushButton_CloseCurrentMlc, SIGNAL(clicked()),
    q, SLOT(onCloseMlcClicked()));
  QObject::connect( this->PushButton_SetCurrentMlc, SIGNAL(clicked()),
    q, SLOT(onSetCurrentLeafParametersClicked()));

  // Select predefined shape as square
  this->ComboBox_MlcPositions->setCurrentIndex(3);
  qDebug() << Q_FUNC_INFO << ": Selected layer: " << this->getSelectedMlcLayer();
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

// --------------------------------------------------------------------------
ContainerWidgets* qSlicerIhepMlcControlLayoutWidgetPrivate::getPairOfLeavesContainerByIndex(int index)
{
  Q_Q(qSlicerIhepMlcControlLayoutWidget);
  size_t ind = index;
  if (ind >= this->ContainerWidgetsVector.size())
  {
    return nullptr;
  }
  return &this->ContainerWidgetsVector.at(index);
}

// --------------------------------------------------------------------------
ContainerWidgets* qSlicerIhepMlcControlLayoutWidgetPrivate::getPairOfLeavesContainerByLeafAddress(int address,
  vtkMRMLIhepMlcControlNode::SideType& side)
{
  Q_Q(qSlicerIhepMlcControlLayoutWidget);
  for(ContainerWidgets& widgets : this->ContainerWidgetsVector)
  {
    QString leafSide1AddressStr = widgets.Side1AddressLabel->text();
    QString leafSide2AddressStr = widgets.Side2AddressLabel->text();
    QString addressStr = QString::number(address);
    if (addressStr == leafSide1AddressStr)
    {
      qDebug() << Q_FUNC_INFO << ": Leaf side1: " << leafSide1AddressStr;
      side = vtkMRMLIhepMlcControlNode::Side1;
      return &widgets;
    }
    if (addressStr == leafSide2AddressStr)
    {
      qDebug() << Q_FUNC_INFO << ": Leaf side2: " << leafSide2AddressStr;
      side = vtkMRMLIhepMlcControlNode::Side2;
      return &widgets;
    }
  }
  qWarning() << Q_FUNC_INFO << ": Widgets container hasn't been found for address: " << address;
  side = vtkMRMLIhepMlcControlNode::Side_Last;
  return nullptr;
}

// --------------------------------------------------------------------------
vtkMRMLIhepMlcControlNode::LayerType qSlicerIhepMlcControlLayoutWidgetPrivate::getSelectedMlcLayer() const
{
  if (this->RadioButton_MlcLayer1->isEnabled() && this->RadioButton_MlcLayer1->isChecked())
  {
    return vtkMRMLIhepMlcControlNode::Layer1;
  }
  else if (this->RadioButton_MlcLayer2->isEnabled() && this->RadioButton_MlcLayer2->isChecked())
  {
    return vtkMRMLIhepMlcControlNode::Layer2;
  }
  else
  {
    qWarning() << Q_FUNC_INFO << ": Invalid MLC layer value";
    return vtkMRMLIhepMlcControlNode::Layer_Last;
  }
}

void qSlicerIhepMlcControlLayoutWidgetPrivate::updateMlcPositionsFromLeavesData()
{
  Q_Q(qSlicerIhepMlcControlLayoutWidget);

  vtkMRMLIhepMlcControlNode::LayerType layer = this->getSelectedMlcLayer();
  if (!this->ParameterNode || (layer == vtkMRMLIhepMlcControlNode::Layer_Last))
  {
    return;
  }

  qWarning() << Q_FUNC_INFO << ": MLC layer value " << layer;

  // Fill leaves container with a new number of leaf pairs 
  for (int i = 0; i < this->ParameterNode->GetNumberOfLeafPairs(); ++i)
  {
    // Set new leaf parameters
    vtkMRMLIhepMlcControlNode::PairOfLeavesData leavesData;
    if (!this->ParameterNode->GetPairOfLeavesData( leavesData, i, layer))
    {
      qWarning() << Q_FUNC_INFO << ": Unable to get pair of leaves data";
      break;
    }
    const vtkMRMLIhepMlcControlNode::LeafData& side1 = leavesData.first;
    const vtkMRMLIhepMlcControlNode::LeafData& side2 = leavesData.second;
    if (side1.Layer != layer || side2.Layer != layer)
    {
      continue;
    }
    qWarning() << Q_FUNC_INFO << ": side1 steps " << side1.Steps << " side2 steps " << side2.Steps;

    ContainerWidgets& pairOfLeavesWidgets = this->ContainerWidgetsVector[i];

    pairOfLeavesWidgets.Side1AddressLabel->setText(QString::number(side1.Address));
    pairOfLeavesWidgets.Side2AddressLabel->setText(QString::number(side2.Address));
    pairOfLeavesWidgets.PairOfLeavesWidget->setLeavesNumbers(side1.Address, side2.Address);
    pairOfLeavesWidgets.PairOfLeavesWidget->setMinCurrentValue(side1.GetActualCurrentPosition());
    pairOfLeavesWidgets.PairOfLeavesWidget->setMaxCurrentValue(side2.GetActualCurrentPosition());
    pairOfLeavesWidgets.PairOfLeavesWidget->setMinRequiredValue(side1.RequiredPosition);
    pairOfLeavesWidgets.PairOfLeavesWidget->setMaxRequiredValue(side2.RequiredPosition);
    q->setLeafData(side1);
    q->setLeafData(side2);
/*
    (side1.SwitchState) ?
      pairOfLeavesWidgets.Side1StateLabel->setPixmap(QPixmap(":/indicators/Icons/green.png")) :
      pairOfLeavesWidgets.Side1StateLabel->setPixmap(QPixmap(":/indicators/Icons/gray.png"));
    (side2.SwitchState) ?
      pairOfLeavesWidgets.Side2StateLabel->setPixmap(QPixmap(":/indicators/Icons/green.png")) :
      pairOfLeavesWidgets.Side2StateLabel->setPixmap(QPixmap(":/indicators/Icons/gray.png"));
*/
  }
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

    widgets.Side1EnabledCheckBox = new ctkCheckBox(this);
    widgets.Side1EnabledCheckBox->setChecked(true);
    widgets.Side1EnabledCheckBox->hide();

    widgets.Side1AddressLabel = new QLabel(QString::number(side1.Address), this);
    widgets.Side1AddressLabel->setAlignment(Qt::AlignHCenter);
    widgets.Side1AddressLabel->setMinimumSize(widgets.Side1AddressLabel->sizeHint());

    widgets.Side1StateLabel = new QLabel(this);
    widgets.Side1StateLabel->setAlignment(Qt::AlignHCenter);
    widgets.Side1StateLabel->setPixmap(QPixmap(":/indicators/Icons/gray.png"));

    widgets.PairOfLeavesWidget = new qSlicerVerticalPairOfLeavesWidget(
      side1.Range + side2.Range, side1.Steps, side2.Steps,
      side1.GetActualCurrentPosition(), side2.GetActualCurrentPosition(), this);
    widgets.PairOfLeavesWidget->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Expanding);

    widgets.Side2StateLabel = new QLabel(this);
    widgets.Side2StateLabel->setAlignment(Qt::AlignHCenter);
    widgets.Side2StateLabel->setPixmap(QPixmap(":/indicators/Icons/gray.png"));

    widgets.Side2AddressLabel = new QLabel(QString::number(side2.Address), this);
    widgets.Side2AddressLabel->setAlignment(Qt::AlignHCenter);
    widgets.Side2AddressLabel->setMinimumSize(widgets.Side2AddressLabel->sizeHint());

    widgets.Side2EnabledCheckBox = new ctkCheckBox(this);
    widgets.Side2EnabledCheckBox->setChecked(true);
    widgets.Side2EnabledCheckBox->hide();

    d->GridLayout_Leaves->addWidget(widgets.Side2EnabledCheckBox, 0, pairOfLeavesIndex + 1);
    d->GridLayout_Leaves->addWidget(widgets.Side2AddressLabel, 1, pairOfLeavesIndex + 1);
    d->GridLayout_Leaves->addWidget(widgets.Side2StateLabel, 2, pairOfLeavesIndex + 1);
    d->GridLayout_Leaves->addWidget(widgets.PairOfLeavesWidget, 3, pairOfLeavesIndex + 1);
    d->GridLayout_Leaves->addWidget(widgets.Side1StateLabel, 4, pairOfLeavesIndex + 1);
    d->GridLayout_Leaves->addWidget(widgets.Side1AddressLabel, 5, pairOfLeavesIndex + 1);
    d->GridLayout_Leaves->addWidget(widgets.Side1EnabledCheckBox, 6, pairOfLeavesIndex + 1);

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
  qDebug() << Q_FUNC_INFO << ": Parameter node is changed, update widget from MRML";
  if (!d->ParameterNode)
  {
    return;
  }
  qDebug() << Q_FUNC_INFO << ": Parameter node is changed, node is valid";

//  vtkMRMLIhepMlcControlNode::LayerType layer = d->getSelectedMlcLayer();

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
    d->updateMlcPositionsFromLeavesData();
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
    emit mlcLayerChanged(selectedMlcLayer);
    d->updateMlcPositionsFromLeavesData();
    qDebug() << Q_FUNC_INFO << ": Layer number emitted: " << selectedMlcLayer;
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::onMlcLayerChanged(vtkMRMLIhepMlcControlNode::LayerType selectedLayer)
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);
  switch (selectedLayer)
  {
  case vtkMRMLIhepMlcControlNode::Layer1:
    d->RadioButton_MlcLayer1->setChecked(true);
    d->updateMlcPositionsFromLeavesData();
    qDebug() << Q_FUNC_INFO << ": Layer1 selected";
    break;
  case vtkMRMLIhepMlcControlNode::Layer2:
    d->RadioButton_MlcLayer2->setChecked(true);
    d->updateMlcPositionsFromLeavesData();
    qDebug() << Q_FUNC_INFO << ": Layer2 selected";
    break;
  default:
    break;
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
    vtkMRMLIhepMlcControlNode::LayerType selectedMlcLayer = d->getSelectedMlcLayer();

    vtkMRMLIhepMlcControlNode::PairOfLeavesData leavesData;
    if (!d->ParameterNode->GetPairOfLeavesData( leavesData, pairIndex, selectedMlcLayer))
    {
      qWarning() << Q_FUNC_INFO << ": Unable to get pair of leaves data";
      return;
    }
    vtkMRMLIhepMlcControlNode::LeafData& side1 = leavesData.first;
    vtkMRMLIhepMlcControlNode::LeafData& side2 = leavesData.second;

    int side1CurrentPosition = side1.GetActualCurrentPosition();//d->ParameterNode->GetCurrentPositionByAddress(side1.Address);
    int side2CurrentPosition = side2.GetActualCurrentPosition();//d->ParameterNode->GetCurrentPositionByAddress(side2.Address);
    qSlicerIhepPairOfLeavesControlDialog controlDialog(
      side1.Address, side2.Address,
      side1.Range, side1CurrentPosition, side1.Steps,
      side2.Range, side2CurrentPosition, side2.Steps, this);
    int res = controlDialog.exec();
    if (res == QDialog::Accepted)
    {
      int side1Steps, side2Steps;
      controlDialog.getSideRequiredPositions( side1Steps, side2Steps);
      side1.Steps = side1Steps;
      side2.Steps = side2Steps;
      side1.RequiredPosition = side1Steps;
      side2.RequiredPosition = side2Steps;

      ContainerWidgets& pairOfLeavesWidgets = d->ContainerWidgetsVector[pairIndex];
      int side1Range = controlDialog.getSide1Range();
      int side2Range = controlDialog.getSide2Range();

      pairOfLeavesWidgets.PairOfLeavesWidget->setMinRequiredValue(side1Steps);
      pairOfLeavesWidgets.PairOfLeavesWidget->setMaxRequiredValue(side2Steps);

      int current1Steps, current2Steps;
      controlDialog.getSideCurrentPositions( current1Steps, current2Steps);
      side1.CurrentPosition = current1Steps;
      side2.CurrentPosition = current2Steps;
      d->ParameterNode->SetLeafDataByAddress( side1, side1.Address);
      d->ParameterNode->SetLeafDataByAddress( side2, side2.Address);
      emit leafDataStepsChanged(side1.Address, side1Steps);
      emit leafDataStepsChanged(side2.Address, side2Steps);

      qDebug() << Q_FUNC_INFO << ": Side 1 steps " << side1Steps << " distance " << d->ParameterNode->InternalCounterValueToDistance(side1Steps);
      qDebug() << Q_FUNC_INFO << ": Side 2 steps " << side2Steps << " distance " << d->ParameterNode->InternalCounterValueToDistance(side2Steps);
      qDebug() << Q_FUNC_INFO << ": Side 1 range 0 ... " << side1Range;
      qDebug() << Q_FUNC_INFO << ": Side 2 range 0 ... " << side2Range;
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

  vtkMRMLIhepMlcControlNode::LayerType selectedMlcLayer = d->getSelectedMlcLayer();

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
  qDebug() << Q_FUNC_INFO << ": Side 1 adjustment " << v << " steps " << d->ParameterNode->DistanceToInternalCounterValue(v);
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
  qDebug() << Q_FUNC_INFO << ": Side 2 adjustment " << v << " steps " << d->ParameterNode->DistanceToInternalCounterValue(v);
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

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::onLeafAddressPositionChanged(int address, double requiredPosition, double currentPosition)
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  vtkMRMLIhepMlcControlNode::SideType side = vtkMRMLIhepMlcControlNode::Side_Last;
  ContainerWidgets* widgets = d->getPairOfLeavesContainerByLeafAddress(address, side);
  if (widgets && side == vtkMRMLIhepMlcControlNode::Side1)
  {
    int minCurrentValue = d->ParameterNode->DistanceToInternalCounterValue(currentPosition);
    int minRequiredValue = d->ParameterNode->DistanceToInternalCounterValue(requiredPosition);
    widgets->PairOfLeavesWidget->setMinCurrentValue(minCurrentValue);
    widgets->PairOfLeavesWidget->setMinRequiredValue(minRequiredValue);
  }
  else if (widgets && side == vtkMRMLIhepMlcControlNode::Side2)
  {
    int maxCurrentValue = d->ParameterNode->DistanceToInternalCounterValue(currentPosition);
    int maxRequiredValue = d->ParameterNode->DistanceToInternalCounterValue(requiredPosition);
    widgets->PairOfLeavesWidget->setMaxCurrentValue(maxCurrentValue);
    widgets->PairOfLeavesWidget->setMaxRequiredValue(maxRequiredValue);
  }
}

//-----------------------------------------------------------------------------
bool qSlicerIhepMlcControlLayoutWidget::getLeafDataByAddress(int address, int& range, int& position)
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return false;
  }
  vtkMRMLIhepMlcControlNode::SideType side;
  ContainerWidgets* widgets = d->getPairOfLeavesContainerByLeafAddress(address, side);
  bool res = false;
  if (widgets)
  {
    switch (side)
    {
    case vtkMRMLIhepMlcControlNode::Side1:
      range = widgets->PairOfLeavesWidget->getMinRange();
      position = widgets->PairOfLeavesWidget->getMinRequiredPosition();
      res = true;
      break;
    case vtkMRMLIhepMlcControlNode::Side2:
      range = widgets->PairOfLeavesWidget->getMaxRange();
      position = widgets->PairOfLeavesWidget->getMaxRequiredPosition();
      res = true;
      break;
    default:
      range = -1;
      position = -1;
      break;
    }
  }
  return res;
}

//-----------------------------------------------------------------------------
vtkMRMLIhepMlcControlNode::LayerType qSlicerIhepMlcControlLayoutWidget::getSelectedMlcLayer() const
{
  Q_D(const qSlicerIhepMlcControlLayoutWidget);
  return d->getSelectedMlcLayer();
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::setLeafData(const vtkMRMLIhepMlcControlNode::LeafData& data)
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  vtkMRMLIhepMlcControlNode::SideType side;
  ContainerWidgets* widgets = d->getPairOfLeavesContainerByLeafAddress(data.Address, side);
  if (data.Layer != d->getSelectedMlcLayer())
  {
    qWarning() << Q_FUNC_INFO << ": Wrong MLC layer to display leaf data";
    return;
  }

  if (widgets)
  {
    QLabel* label = (side == vtkMRMLIhepMlcControlNode::Side1) ? widgets->Side1StateLabel : widgets->Side2StateLabel;
    qSlicerAbstractPairOfLeavesWidget* leavesWidget = widgets->PairOfLeavesWidget;
    if (data.SwitchState)
    {
      label->setPixmap(QPixmap(":/indicators/Icons/red.png"));
      if (side == vtkMRMLIhepMlcControlNode::Side1 && side == data.Side)
      {
        leavesWidget->setMinCurrentValueFromLeafData(0);
      }
      else if (side == vtkMRMLIhepMlcControlNode::Side2 && side == data.Side)
      {
        leavesWidget->setMaxCurrentValueFromLeafData(0);
      }
    }
    else
    {
      if (side == vtkMRMLIhepMlcControlNode::Side1 && side == data.Side)
      {
        if (data.CurrentPosition > 0)
        {
          leavesWidget->setMinCurrentValueFromLeafData(data.CurrentPosition);
        }
        else
        {
          leavesWidget->setMinCurrentValueFromLeafData(0);
        }
        if (data.isMovingToTheSwitch())
        {
          leavesWidget->setMinCurrentValueFromLeafData(data.CurrentPosition - 2 * data.EncoderCounts);
          label->setPixmap(QPixmap(":/indicators/Icons/side1-to-the-switch.png"));
        }
        else if (data.isMovingFromTheSwitch())
        {
          leavesWidget->setMinCurrentValueFromLeafData(data.CurrentPosition + 2 * data.EncoderCounts);
          label->setPixmap(QPixmap(":/indicators/Icons/side1-away-from-switch.png"));
        }
        else if (data.isStopped())
        {
          leavesWidget->setMinCurrentValueFromLeafData(data.CurrentPosition);
          label->setPixmap(QPixmap(":/indicators/Icons/green.png"));
        }
      }
      else if (side == vtkMRMLIhepMlcControlNode::Side2 && side == data.Side)
      {
        if (data.CurrentPosition > 0)
        {
          leavesWidget->setMaxCurrentValueFromLeafData(data.CurrentPosition);
        }
        else
        {
          leavesWidget->setMaxCurrentValueFromLeafData(0);
        }
        
        if (data.isMovingToTheSwitch())
        {
          leavesWidget->setMaxCurrentValueFromLeafData(data.CurrentPosition - 2 * data.EncoderCounts);
          label->setPixmap(QPixmap(":/indicators/Icons/side2-to-the-switch.png"));
        }
        else if (data.isMovingFromTheSwitch())
        {
          leavesWidget->setMaxCurrentValueFromLeafData(data.CurrentPosition + 2 * data.EncoderCounts);
          label->setPixmap(QPixmap(":/indicators/Icons/side2-away-from-switch.png"));
        }
        else if (data.isStopped())
        {
          leavesWidget->setMaxCurrentValueFromLeafData(data.CurrentPosition);
          label->setPixmap(QPixmap(":/indicators/Icons/green.png"));
        }
      }
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::onSetCurrentLeafParametersClicked()
{
  Q_D(const qSlicerIhepMlcControlLayoutWidget);

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  vtkMRMLIhepMlcControlNode::LayerType layer = d->getSelectedMlcLayer();
  if (layer != vtkMRMLIhepMlcControlNode::Layer_Last)
  {
    // Fill leaves container with a new number of leaf pairs
    for (int i = 0; i < d->ParameterNode->GetNumberOfLeafPairs(); ++i)
    {
      vtkMRMLIhepMlcControlNode::PairOfLeavesData leavesData;
      if (!d->ParameterNode->GetPairOfLeavesData( leavesData, i, layer))
      {
        qWarning() << Q_FUNC_INFO << ": Unable to get pair of leaves data";
        break;
      }
      const vtkMRMLIhepMlcControlNode::LeafData& side1 = leavesData.first;
      const vtkMRMLIhepMlcControlNode::LeafData& side2 = leavesData.second;
      int side1Movement = d->ParameterNode->GetRelativeMovementByAddress(side1.Address);
      int side2Movement = d->ParameterNode->GetRelativeMovementByAddress(side2.Address);
      emit leafDataStepsChanged(side1.Address, side1Movement);
      emit leafDataStepsChanged(side2.Address, side2Movement);
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::onSetOpenLeafParametersClicked()
{
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::onSetCloseLeafParametersClicked()
{
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::onLeafPositionChanged(int address,
  vtkMRMLIhepMlcControlNode::LayerType layer,
  vtkMRMLIhepMlcControlNode::SideType side,
  int requiredPosition, int currentPosition)
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  vtkMRMLIhepMlcControlNode::SideType side_;
  ContainerWidgets* widgets = d->getPairOfLeavesContainerByLeafAddress(address, side_);
  if (layer != d->getSelectedMlcLayer())
  {
    qWarning() << Q_FUNC_INFO << ": Wrong MLC layer to display leaf data";
    return;
  }

  if (widgets)
  {
    qSlicerAbstractPairOfLeavesWidget* leavesWidget = widgets->PairOfLeavesWidget;
    if (side == vtkMRMLIhepMlcControlNode::Side1 && side == side_)
    {
      leavesWidget->setMinCurrentValueFromLeafData(currentPosition);
      leavesWidget->setMinRequiredValueFromLeafData(requiredPosition);
    }
    else if (side == vtkMRMLIhepMlcControlNode::Side2 && side == side_)
    {
      leavesWidget->setMaxCurrentValueFromLeafData(currentPosition);
      leavesWidget->setMaxRequiredValueFromLeafData(requiredPosition);
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::onLeafSwitchChanged(int address,
  vtkMRMLIhepMlcControlNode::LayerType layer,
  vtkMRMLIhepMlcControlNode::SideType side,
  bool switchIsPressed)
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  vtkMRMLIhepMlcControlNode::SideType side_;
  ContainerWidgets* widgets = d->getPairOfLeavesContainerByLeafAddress(address, side_);
  if (layer != d->getSelectedMlcLayer())
  {
    qWarning() << Q_FUNC_INFO << ": Wrong MLC layer to display leaf data";
    return;
  }

  if (widgets)
  {
    QLabel* label = (side == vtkMRMLIhepMlcControlNode::Side1) ? widgets->Side1StateLabel : widgets->Side2StateLabel;
    qSlicerAbstractPairOfLeavesWidget* leavesWidget = widgets->PairOfLeavesWidget;
    if (switchIsPressed)
    {
      label->setPixmap(QPixmap(":/indicators/Icons/red.png"));
      if (side == vtkMRMLIhepMlcControlNode::Side1 && side == side_)
      {
        leavesWidget->setMinCurrentValueFromLeafData(0);
      }
      else if (side == vtkMRMLIhepMlcControlNode::Side2 && side == side_)
      {
        leavesWidget->setMaxCurrentValueFromLeafData(0);
      }
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::onDebugModeEnabled(bool enabled)
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  d->CheckBox_Side1Enabled->setEnabled(enabled);
  d->CheckBox_Side2Enabled->setEnabled(enabled);

  for(ContainerWidgets& widgets : d->ContainerWidgetsVector)
  {
    if (enabled)
    {
      widgets.Side1EnabledCheckBox->show();
      widgets.Side2EnabledCheckBox->show();
    }
    else
    {
      widgets.Side1EnabledCheckBox->hide();
      widgets.Side2EnabledCheckBox->hide();
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::onSide1StateChanged(int state)
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  for(ContainerWidgets& widgets : d->ContainerWidgetsVector)
  {
    switch (state)
    {
    case 0: // false
      widgets.Side1EnabledCheckBox->setChecked(false);
      widgets.Side1EnabledCheckBox->setEnabled(false);
      break;
    case 1: // tri-state
      widgets.Side1EnabledCheckBox->setEnabled(true);
      break;
    case 2: // true
      widgets.Side1EnabledCheckBox->setChecked(true);
      widgets.Side1EnabledCheckBox->setEnabled(false);
      break;
    default:
      break;
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::onSide2StateChanged(int state)
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  for(ContainerWidgets& widgets : d->ContainerWidgetsVector)
  {
    switch (state)
    {
    case 0: // false
      widgets.Side2EnabledCheckBox->setChecked(false);
      widgets.Side2EnabledCheckBox->setEnabled(false);
      break;
    case 1: // tri-state
      widgets.Side2EnabledCheckBox->setEnabled(true);
      break;
    case 2: // true
      widgets.Side2EnabledCheckBox->setChecked(true);
      widgets.Side2EnabledCheckBox->setEnabled(false);
      break;
    default:
      break;
    }
  }
}

//-----------------------------------------------------------------------------
/*
void qSlicerIhepMlcControlLayoutWidget::setMlcTableNode(vtkMRMLTableNode* mlcTableNode)
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  vtkMRMLIhepMlcControlNode::PairOfLeavesData pairOfLeaves;
  // Set leaves position from mlc table
  for (int i = 0; i < d->ParameterNode->GetNumberOfLeafPairs(); ++i)
  {
    if (d->ParameterNode->GetPairOfLeavesData( pairOfLeaves, i, d->getSelectedMlcLayer()))
    {
      vtkMRMLIhepMlcControlNode::LeafData& side1 = pairOfLeaves.first;
      vtkMRMLIhepMlcControlNode::LeafData& side2 = pairOfLeaves.second;
      ContainerWidgets* widgets = d->getPairOfLeavesContainerByIndex(i);
    
      int side1Steps = d->ParameterNode->GetStepsFromMlcTableByAddress(mlcTableNode, side1.Address);
      int side2Steps = d->ParameterNode->GetStepsFromMlcTableByAddress(mlcTableNode, side2.Address);
      side1.Steps = side1Steps;
      side2.Steps = side2Steps;
      d->ParameterNode->SetLeafDataByAddress(side1, side1.Address);
      d->ParameterNode->SetLeafDataByAddress(side2, side2.Address);
      widgets->PairOfLeavesWidget->setMinRequiredValue(side1Steps);
      widgets->PairOfLeavesWidget->setMaxRequiredValue(side1Steps);
    }
  }
}
*/
