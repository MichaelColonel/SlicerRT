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

// SlicerQt includes
#include <qSlicerCoreApplication.h>
#include <qSlicerAbstractCoreModule.h>
#include <qSlicerModuleManager.h>

// Qt includes
#include <QDebug>
#include <QRadioButton>
// CTK includes
#include <ctkCheckBox.h>

// Logic includes
#include "vtkSlicerIhepMlcControlLogic.h"

// IhepMlcControl Widgets includes
#include "qSlicerIhepMlcControlLayoutWidget.h"
#include "ui_qSlicerIhepMlcControlLayoutWidget.h"

#include "qSlicerIhepPairOfLeavesControlDialog.h"
#include "qSlicerPairOfLeavesWidget.h"

namespace {

struct ContainerWidgets {
  ctkCheckBox* Side1EnabledCheckBox{ nullptr };
  QDoubleSpinBox* Side1Required{ nullptr };
  QDoubleSpinBox* Side1Current{ nullptr };
  QDoubleSpinBox* Side1Gap{ nullptr };
  QLabel* Side1AddressLabel{ nullptr };
  QLabel* Side1StateLabel{ nullptr };
  qSlicerAbstractPairOfLeavesWidget* PairOfLeavesWidget{ nullptr };
  QLabel* Side2StateLabel{ nullptr };
  QLabel* Side2AddressLabel{ nullptr };
  QDoubleSpinBox* Side2Gap{ nullptr };
  QDoubleSpinBox* Side2Current{ nullptr };
  QDoubleSpinBox* Side2Required{ nullptr };
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
  void updateMlcPositionsFromLeavesData();
  vtkSlicerIhepMlcControlLogic* logic() const;

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
vtkSlicerIhepMlcControlLogic* qSlicerIhepMlcControlLayoutWidgetPrivate::logic() const
{
  Q_Q(const qSlicerIhepMlcControlLayoutWidget);
  qSlicerAbstractCoreModule* aModule = qSlicerCoreApplication::application()->moduleManager()->module("IhepMlcControl");
  if (aModule)
  {
    vtkSlicerIhepMlcControlLogic* moduleLogic = vtkSlicerIhepMlcControlLogic::SafeDownCast(aModule->logic());
    return moduleLogic;
  }
  return nullptr;
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

  // Select predefined shape as square
  this->ComboBox_MlcPositions->setCurrentIndex(3);
}

// --------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidgetPrivate::removeLeavesWidgetsFromLayout()
{
  Q_Q(qSlicerIhepMlcControlLayoutWidget);
  for(ContainerWidgets& widgets : this->ContainerWidgetsVector)
  {
    if (widgets.Side1EnabledCheckBox)
    {
      this->GridLayout_Leaves->removeWidget(widgets.Side1EnabledCheckBox);

      delete widgets.Side1EnabledCheckBox;
      widgets.Side1EnabledCheckBox = nullptr;
    }
    if (widgets.Side1Required)
    {
      this->GridLayout_Leaves->removeWidget(widgets.Side1Required);

      delete widgets.Side1Required;
      widgets.Side1Required = nullptr;
    }
    if (widgets.Side1Current)
    {
      this->GridLayout_Leaves->removeWidget(widgets.Side1Current);

      delete widgets.Side1Current;
      widgets.Side1Current = nullptr;
    }
    if (widgets.Side1Gap)
    {
      this->GridLayout_Leaves->removeWidget(widgets.Side1Gap);

      delete widgets.Side1Gap;
      widgets.Side1Gap = nullptr;
    }
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
    if (widgets.Side2Gap)
    {
      this->GridLayout_Leaves->removeWidget(widgets.Side2Gap);

      delete widgets.Side2Gap;
      widgets.Side2Gap = nullptr;
    }
    if (widgets.Side2Current)
    {
      this->GridLayout_Leaves->removeWidget(widgets.Side2Current);

      delete widgets.Side2Current;
      widgets.Side2Current = nullptr;
    }
    if (widgets.Side2Required)
    {
      this->GridLayout_Leaves->removeWidget(widgets.Side2Required);

      delete widgets.Side2Required;
      widgets.Side2Required = nullptr;
    }
    if (widgets.Side2EnabledCheckBox)
    {
      this->GridLayout_Leaves->removeWidget(widgets.Side2EnabledCheckBox);

      delete widgets.Side2EnabledCheckBox;
      widgets.Side2EnabledCheckBox = nullptr;
    }
    if (widgets.PairOfLeavesWidget)
    {
      QObject::disconnect( widgets.PairOfLeavesWidget, SIGNAL(pairOfLeavesDoubleClicked()),
        q, SLOT(onPairOfLeavesDoubleClicked()));

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
      side = vtkMRMLIhepMlcControlNode::Side1;
      return &widgets;
    }
    if (addressStr == leafSide2AddressStr)
    {
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

  if (!this->logic())
  {
    qWarning() << Q_FUNC_INFO << ": Mlc control logic is invalid";
    return;
  }

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

    ContainerWidgets& pairOfLeavesWidgets = this->ContainerWidgetsVector[i];

    int minRange = this->ParameterNode->GetCalibrationRangeInLayer(layer);

    pairOfLeavesWidgets.Side1AddressLabel->setText(QString::number(side1.Address));
    pairOfLeavesWidgets.Side2AddressLabel->setText(QString::number(side2.Address));
    pairOfLeavesWidgets.PairOfLeavesWidget->setMaximum(minRange);
    pairOfLeavesWidgets.PairOfLeavesWidget->setLeavesNumbers(side1.Address, side2.Address);
    pairOfLeavesWidgets.PairOfLeavesWidget->setMinCurrentValue(side1.GetActualCurrentPosition());
    pairOfLeavesWidgets.PairOfLeavesWidget->setMaxCurrentValue(side2.GetActualCurrentPosition());
    pairOfLeavesWidgets.PairOfLeavesWidget->setMinRequiredValue(side1.RequiredPosition);
    pairOfLeavesWidgets.PairOfLeavesWidget->setMaxRequiredValue(side2.RequiredPosition);
    double side1ReqPosition = -1.;
    bool res = this->logic()->LeafStepsToMlcPosition(this->ParameterNode, side1.RequiredPosition, side1.Side, side1.Layer, side1ReqPosition);
    if (res)
    {
      pairOfLeavesWidgets.Side1Required->setValue(side1ReqPosition);
    }
    double side2ReqPosition = -1.;
    res = this->logic()->LeafStepsToMlcPosition(this->ParameterNode, side2.RequiredPosition, side2.Side, side2.Layer, side2ReqPosition);
    if (res)
    {
      pairOfLeavesWidgets.Side2Required->setValue(side2ReqPosition);
    }
    double side1CurPosition = -1.;
    if (!side1.isPositionUnknown())
    {
      res = this->logic()->LeafStepsToMlcPosition(this->ParameterNode, side1.GetActualCurrentPosition(), side1.Side, side1.Layer, side1CurPosition);
      if (res)
      {
        pairOfLeavesWidgets.Side1Current->setValue(side1CurPosition);
      }
    }
    else
    {
      pairOfLeavesWidgets.Side1Current->setSpecialValueText(QObject::tr("Unknown"));
    }
    double side2CurPosition = -1.;
    if (!side2.isPositionUnknown())
    {
      res = this->logic()->LeafStepsToMlcPosition(this->ParameterNode, side2.GetActualCurrentPosition(), side2.Side, side2.Layer, side2CurPosition);
      if (res)
      {
        pairOfLeavesWidgets.Side2Current->setValue(side2CurPosition);
      }
    }
    else
    {
      pairOfLeavesWidgets.Side2Current->setSpecialValueText(QObject::tr("Unknown"));
    }
    if (pairOfLeavesWidgets.Side1Current->specialValueText() == QObject::tr("Unknown") || pairOfLeavesWidgets.Side2Current->specialValueText() == QObject::tr("Unknown"))
    {
      pairOfLeavesWidgets.Side1Gap->setSpecialValueText(QObject::tr("Unknown"));
      pairOfLeavesWidgets.Side2Gap->setSpecialValueText(QObject::tr("Unknown"));
    }
    else
    {
      pairOfLeavesWidgets.Side1Gap->setValue(pairOfLeavesWidgets.Side1Required->value() - pairOfLeavesWidgets.Side1Current->value());
      pairOfLeavesWidgets.Side2Gap->setValue(pairOfLeavesWidgets.Side2Required->value() - pairOfLeavesWidgets.Side2Current->value());
    }

    q->setLeafData(side1);
    q->setLeafData(side2);
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

  vtkMRMLIhepMlcControlNode::LayerType selectedMlcLayer = d->getSelectedMlcLayer();
  vtkMRMLIhepMlcControlNode::PairOfLeavesData pairOfLeaves;
  if (d->ParameterNode->GetPairOfLeavesData(pairOfLeaves, pairOfLeavesIndex))
  {
    const vtkMRMLIhepMlcControlNode::LeafData& side1 = pairOfLeaves.first;
    const vtkMRMLIhepMlcControlNode::LeafData& side2 = pairOfLeaves.second;

    ContainerWidgets widgets;

    widgets.Side1EnabledCheckBox = new ctkCheckBox(this);
    widgets.Side1EnabledCheckBox->setChecked(true);
    widgets.Side1EnabledCheckBox->setEnabled(false);
    widgets.Side1EnabledCheckBox->hide();

    int range = d->ParameterNode->GetCalibrationRangeInLayer(selectedMlcLayer);
    double rangeDistance = vtkMRMLIhepMlcControlNode::InternalCounterValueToDistance(range);

    widgets.Side1AddressLabel = new QLabel(QString::number(side1.Address), this);
    widgets.Side1AddressLabel->setAlignment(Qt::AlignHCenter);
    widgets.Side1AddressLabel->setMinimumSize(widgets.Side1AddressLabel->sizeHint());
    widgets.Side1Required = new QDoubleSpinBox(this);
    widgets.Side1Required->setButtonSymbols(QAbstractSpinBox::NoButtons);
    widgets.Side1Required->setRange(-1. * rangeDistance, rangeDistance);
    widgets.Side1Current = new QDoubleSpinBox(this);
    widgets.Side1Current->setButtonSymbols(QAbstractSpinBox::NoButtons);
    widgets.Side1Current->setRange(-1. * rangeDistance, rangeDistance);
    widgets.Side1Current->setSpecialValueText(tr("Unknown"));
    widgets.Side1Gap = new QDoubleSpinBox(this);
    widgets.Side1Gap->setButtonSymbols(QAbstractSpinBox::NoButtons);
    widgets.Side1Gap->setRange(-1. * rangeDistance, rangeDistance);
    widgets.Side1Gap->setSpecialValueText(tr("Unknown"));

    widgets.Side1StateLabel = new QLabel(this);
    widgets.Side1StateLabel->setAlignment(Qt::AlignHCenter);
    widgets.Side1StateLabel->setPixmap(QPixmap(":/indicators/Icons/gray.png"));

    int minRangeSide1 = d->ParameterNode->GetMinCalibrationStepsBySideInLayer(vtkMRMLIhepMlcControlNode::Side1, selectedMlcLayer);
    int minRangeSide2 = d->ParameterNode->GetMinCalibrationStepsBySideInLayer(vtkMRMLIhepMlcControlNode::Side2, selectedMlcLayer);

    widgets.PairOfLeavesWidget = new qSlicerVerticalPairOfLeavesWidget(
      minRangeSide1 + minRangeSide2, side1.Steps, side2.Steps,
      side1.GetActualCurrentPosition(), side2.GetActualCurrentPosition(), this);
    widgets.PairOfLeavesWidget->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Expanding);

    widgets.Side2StateLabel = new QLabel(this);
    widgets.Side2StateLabel->setAlignment(Qt::AlignHCenter);
    widgets.Side2StateLabel->setPixmap(QPixmap(":/indicators/Icons/gray.png"));

    widgets.Side2AddressLabel = new QLabel(QString::number(side2.Address), this);
    widgets.Side2AddressLabel->setAlignment(Qt::AlignHCenter);
    widgets.Side2AddressLabel->setMinimumSize(widgets.Side2AddressLabel->sizeHint());
    widgets.Side2Gap = new QDoubleSpinBox(this);
    widgets.Side2Gap->setButtonSymbols(QAbstractSpinBox::NoButtons);
    widgets.Side2Gap->setRange(-1. * rangeDistance, rangeDistance);
    widgets.Side2Gap->setSpecialValueText(tr("Unknown"));
    widgets.Side2Current = new QDoubleSpinBox(this);
    widgets.Side2Current->setButtonSymbols(QAbstractSpinBox::NoButtons);
    widgets.Side2Current->setRange(-1. * rangeDistance, rangeDistance);
    widgets.Side2Current->setSpecialValueText(tr("Unknown"));
    widgets.Side2Required = new QDoubleSpinBox(this);
    widgets.Side2Required->setButtonSymbols(QAbstractSpinBox::NoButtons);
    widgets.Side2Required->setRange(-1. * rangeDistance, rangeDistance);

    widgets.Side2EnabledCheckBox = new ctkCheckBox(this);
    widgets.Side2EnabledCheckBox->setChecked(true);
    widgets.Side2EnabledCheckBox->setEnabled(false);
    widgets.Side2EnabledCheckBox->hide();

    d->GridLayout_Leaves->addWidget(widgets.Side2EnabledCheckBox, 0, pairOfLeavesIndex + 1);
    d->GridLayout_Leaves->addWidget(widgets.Side2Required, 1, pairOfLeavesIndex + 1);
    d->GridLayout_Leaves->addWidget(widgets.Side2Current, 2, pairOfLeavesIndex + 1);
    d->GridLayout_Leaves->addWidget(widgets.Side2Gap, 3, pairOfLeavesIndex + 1);
    d->GridLayout_Leaves->addWidget(widgets.Side2AddressLabel, 4, pairOfLeavesIndex + 1);
    d->GridLayout_Leaves->addWidget(widgets.Side2StateLabel, 5, pairOfLeavesIndex + 1);
    d->GridLayout_Leaves->addWidget(widgets.PairOfLeavesWidget, 6, pairOfLeavesIndex + 1);
    d->GridLayout_Leaves->addWidget(widgets.Side1StateLabel, 7, pairOfLeavesIndex + 1);
    d->GridLayout_Leaves->addWidget(widgets.Side1AddressLabel, 8, pairOfLeavesIndex + 1);
    d->GridLayout_Leaves->addWidget(widgets.Side1Gap, 9, pairOfLeavesIndex + 1);
    d->GridLayout_Leaves->addWidget(widgets.Side1Current, 10, pairOfLeavesIndex + 1);
    d->GridLayout_Leaves->addWidget(widgets.Side1Required, 11, pairOfLeavesIndex + 1);
    d->GridLayout_Leaves->addWidget(widgets.Side1EnabledCheckBox, 12, pairOfLeavesIndex + 1);

    widgets.PairOfLeavesWidget->setLeavesNumbers(side1.Address, side2.Address);
    widgets.PairOfLeavesWidget->setControlEnabled(false);

    connect( widgets.PairOfLeavesWidget, SIGNAL(pairOfLeavesDoubleClicked()),
      this, SLOT(onPairOfLeavesDoubleClicked()));

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
    break;
  case vtkMRMLIhepMlcControlNode::Layer2:
    d->RadioButton_MlcLayer2->setChecked(true);
    d->updateMlcPositionsFromLeavesData();
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

    int side1CurrentPosition = (side1.CurrentPosition != USHRT_MAX) ? side1.GetActualCurrentPosition() : 0;
    int side2CurrentPosition = (side2.CurrentPosition != USHRT_MAX) ? side2.GetActualCurrentPosition() : 0;
    int side1Range = d->ParameterNode->GetLeafRangeByAddressInLayer(side1.Address, selectedMlcLayer);
    int side2Range = d->ParameterNode->GetLeafRangeByAddressInLayer(side2.Address, selectedMlcLayer);
    qSlicerIhepPairOfLeavesControlDialog controlDialog(
      side1.Address, side2.Address,
      side1Range, side1CurrentPosition, side1.Steps,
      side2Range, side2CurrentPosition, side2.Steps, this);
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
      side1Range = controlDialog.getSide1Range();
      side2Range = controlDialog.getSide2Range();

      pairOfLeavesWidgets.PairOfLeavesWidget->setMinRequiredValue(side1Steps);
      pairOfLeavesWidgets.PairOfLeavesWidget->setMaxRequiredValue(side2Steps);

      int current1Steps, current2Steps;
      controlDialog.getSideCurrentPositions( current1Steps, current2Steps);
      side1.CurrentPosition = current1Steps;
      side2.CurrentPosition = current2Steps;
      side1.Range = side1Range;
      side2.Range = side2Range;
      d->ParameterNode->SetLeafDataByAddressInLayer( side1, side1.Address, selectedMlcLayer);
      d->ParameterNode->SetLeafDataByAddressInLayer( side2, side2.Address, selectedMlcLayer);
      emit leafDataStepsChanged(side1.Address, side1Steps, vtkMRMLIhepMlcControlNode::Side1, selectedMlcLayer);
      emit leafDataStepsChanged(side2.Address, side2Steps, vtkMRMLIhepMlcControlNode::Side2, selectedMlcLayer);
    }
  }
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

    if (d->logic()->UpdateLeavesDataFromMlcPositionTableNode(d->ParameterNode))
    {
      qDebug() << Q_FUNC_INFO << ":Leaves steps were updated";
    }
    d->ParameterNode->Modified();
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
void qSlicerIhepMlcControlLayoutWidget::onPairOfLeavesPositionsRangesChanged(int minValue, int minRange, int maxValue, int maxRange)
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
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

  if (!d->logic())
  {
    qWarning() << Q_FUNC_INFO << ": MlcControlLogic is invalid";
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

    if (data.isPositionUnknown())
    {
      widgets->Side1Current->setSpecialValueText(tr("Unknown"));
      widgets->Side2Current->setSpecialValueText(tr("Unknown"));
      widgets->Side1Gap->setSpecialValueText(tr("Unknown"));
      widgets->Side2Gap->setSpecialValueText(tr("Unknown"));
      leavesWidget->setMinCurrentValueFromLeafData(0);
      leavesWidget->setMaxCurrentValueFromLeafData(0);
      widgets->Side1StateLabel->setPixmap(QPixmap(":/indicators/Icons/gray.png"));
      widgets->Side2StateLabel->setPixmap(QPixmap(":/indicators/Icons/gray.png"));
      return;
    }
    if (data.SwitchState)
    {
      label->setPixmap(QPixmap(":/indicators/Icons/red.png"));
      if (side == vtkMRMLIhepMlcControlNode::Side1 && side == data.Side)
      {
        leavesWidget->setMinCurrentValueFromLeafData(0);
        if (data.PositionIsWithinTolerance(widgets->Side1Required->value(), widgets->Side1Current->value()))
        {
          widgets->Side1Current->setStyleSheet("background-color: green");
          widgets->Side1Gap->setStyleSheet("background-color: green");
        }
        else
        {
          widgets->Side1Current->setStyleSheet("background-color: red");
          widgets->Side1Gap->setStyleSheet("background-color: red");
        }
      }
      else if (side == vtkMRMLIhepMlcControlNode::Side2 && side == data.Side)
      {
        leavesWidget->setMaxCurrentValueFromLeafData(0);
        if (data.PositionIsWithinTolerance(widgets->Side2Required->value(), widgets->Side2Current->value()))
        {
          widgets->Side2Current->setStyleSheet("background-color: green");
          widgets->Side2Gap->setStyleSheet("background-color: green");
        }
        else
        {
          widgets->Side2Current->setStyleSheet("background-color: red");
          widgets->Side2Gap->setStyleSheet("background-color: red");
        }
      }
    }
    else
    {
      if (side == vtkMRMLIhepMlcControlNode::Side1 && side == data.Side)
      {
        double side1CurPosition = -1.;
        bool res = d->logic()->LeafStepsToMlcPosition(d->ParameterNode, data.CurrentPosition, data.Side, data.Layer, side1CurPosition);
        if (res)
        {
          widgets->Side1Current->setValue(side1CurPosition);
          widgets->Side1Gap->setValue(widgets->Side1Required->value() - widgets->Side1Current->value());
        }

        leavesWidget->setMinCurrentValueFromLeafData(data.CurrentPosition);
        if (data.isMovingToTheSwitch())
        {
          label->setPixmap(QPixmap(":/indicators/Icons/side1-to-the-switch.png"));
        }
        else if (data.isMovingFromTheSwitch())
        {
          label->setPixmap(QPixmap(":/indicators/Icons/side1-away-from-switch.png"));
        }
        else if (data.isStopped())
        {
          label->setPixmap(QPixmap(":/indicators/Icons/green.png"));
        }
        else
        {
          ;
        }
        if (data.PositionIsWithinTolerance(widgets->Side1Required->value(), widgets->Side1Current->value()))
        {
          widgets->Side1Current->setStyleSheet("background-color: green");
          widgets->Side1Gap->setStyleSheet("background-color: green");
        }
        else
        {
          widgets->Side1Current->setStyleSheet("background-color: red");
          widgets->Side1Gap->setStyleSheet("background-color: red");
        }
      }
      else if (side == vtkMRMLIhepMlcControlNode::Side2 && side == data.Side)
      {
        double side2CurPosition = -1.;
        bool res = d->logic()->LeafStepsToMlcPosition(d->ParameterNode, data.CurrentPosition, data.Side, data.Layer, side2CurPosition);
        if (res)
        {
          widgets->Side2Current->setValue(side2CurPosition);
          widgets->Side2Gap->setValue(widgets->Side2Required->value() - widgets->Side2Current->value());
        }
        leavesWidget->setMaxCurrentValueFromLeafData(data.CurrentPosition);        
        if (data.isMovingToTheSwitch())
        {
          label->setPixmap(QPixmap(":/indicators/Icons/side2-to-the-switch.png"));
        }
        else if (data.isMovingFromTheSwitch())
        {
          label->setPixmap(QPixmap(":/indicators/Icons/side2-away-from-switch.png"));
        }
        else if (data.isStopped())
        {
          label->setPixmap(QPixmap(":/indicators/Icons/green.png"));
        }
        else
        {
          ;
        }
        if (data.PositionIsWithinTolerance(widgets->Side2Required->value(), widgets->Side2Current->value()))
        {
          widgets->Side2Current->setStyleSheet("background-color: green");
          widgets->Side2Gap->setStyleSheet("background-color: green");
        }
        else
        {
          widgets->Side2Current->setStyleSheet("background-color: red");
          widgets->Side2Gap->setStyleSheet("background-color: red");
        }
      }
    }
  }
  else
  {
    qWarning() << Q_FUNC_INFO << ": Wrong MLC layer widgets to display leaf data";
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
void qSlicerIhepMlcControlLayoutWidget::onLeafStateCommandBufferChanged(const vtkMRMLIhepMlcControlNode::CommandBufferType& stateBuffer,
  vtkMRMLIhepMlcControlNode::LayerType layer,
  vtkMRMLIhepMlcControlNode::SideType side)
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);

  vtkMRMLIhepMlcControlNode::LeafData leafData;
  vtkMRMLIhepMlcControlNode::LayerType selectedLayer = d->getSelectedMlcLayer();
  vtkMRMLIhepMlcControlNode::ProcessCommandBufferToLeafData(stateBuffer, leafData);
  if (layer == selectedLayer)
  {
    leafData.Side = side;
    leafData.Layer = layer;
    this->setLeafData(leafData);
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::onMlcLayerConnected(vtkMRMLIhepMlcControlNode::LayerType layer)
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);

  vtkMRMLIhepMlcControlNode::LayerType selectedLayer = d->getSelectedMlcLayer();
  if (selectedLayer == layer)
  {
    qDebug() << Q_FUNC_INFO << "MLC Layer-" << static_cast<int>(layer) << " connected, update widget";
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlLayoutWidget::onMlcLayerDisconnected(vtkMRMLIhepMlcControlNode::LayerType layer)
{
  Q_D(qSlicerIhepMlcControlLayoutWidget);

  vtkMRMLIhepMlcControlNode::LayerType selectedLayer = d->getSelectedMlcLayer();
  if (selectedLayer == layer)
  {
    qDebug() << Q_FUNC_INFO << "MLC Layer-" << static_cast<int>(layer) << " disconnected, update widget";
  }
}
