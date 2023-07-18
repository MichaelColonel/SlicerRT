/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QTimer>
#include <QRadioButton>

// Slicer includes
#include <qSlicerSingletonViewFactory.h>
#include <qSlicerLayoutManager.h>
#include <qSlicerApplication.h>

#include "qSlicerIhepMlcControlModuleWidget.h"
#include "ui_qSlicerIhepMlcControlModuleWidget.h"

#include "qSlicerIhepMlcDeviceLogic.h"
#include "qSlicerIhepMlcControlLayoutWidget.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLLayoutLogic.h>
#include <vtkMRMLTableNode.h>

// SlicerRT MRML Beams includes
#include <vtkMRMLRTBeamNode.h>
#include <vtkMRMLRTIonBeamNode.h>

// SlicerRT MRML IhepMlcControl includes
#include "vtkMRMLIhepMlcControlNode.h"

// Logic includes
#include "vtkSlicerIhepMlcControlLogic.h"

// STD includes
#include <cstring>
#include <queue>
#include <bitset>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerIhepMlcControlModuleWidgetPrivate: public Ui_qSlicerIhepMlcControlModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerIhepMlcControlModuleWidget);
protected:
  qSlicerIhepMlcControlModuleWidget* const q_ptr;
public:
  qSlicerIhepMlcControlModuleWidgetPrivate(qSlicerIhepMlcControlModuleWidget &object);
  virtual ~qSlicerIhepMlcControlModuleWidgetPrivate();
  vtkSlicerIhepMlcControlLogic* logic() const;
  vtkMRMLIhepMlcControlNode::LayerType getSelectedMlcLayer() const;

  qSlicerIhepMlcControlLayoutWidget* MlcControlWidget{ nullptr };
  int PreviousLayoutId{ 0 };
  int MlcCustomLayoutId{ 507 };
  vtkWeakPointer<vtkMRMLIhepMlcControlNode> ParameterNode;

  QSerialPort* MlcLayer1SerialPort{ nullptr };
  QSerialPort* MlcLayer2SerialPort{ nullptr };

  qSlicerIhepMlcDeviceLogic* MlcLayer1Logic{ nullptr };
  qSlicerIhepMlcDeviceLogic* MlcLayer2Logic{ nullptr };
};

//-----------------------------------------------------------------------------
// qSlicerIhepMlcControlModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerIhepMlcControlModuleWidgetPrivate::qSlicerIhepMlcControlModuleWidgetPrivate(qSlicerIhepMlcControlModuleWidget &object)
  :
  q_ptr(&object)
{
  this->MlcLayer1Logic = new qSlicerIhepMlcDeviceLogic(&object);
  this->MlcLayer2Logic = new qSlicerIhepMlcDeviceLogic(&object);
}

//-----------------------------------------------------------------------------
qSlicerIhepMlcControlModuleWidgetPrivate::~qSlicerIhepMlcControlModuleWidgetPrivate()
{
  if (this->MlcLayer1SerialPort)
  {
    this->MlcLayer1Logic->closeDevice(this->MlcLayer1SerialPort);
  }
  if (this->MlcLayer2SerialPort)
  {
    this->MlcLayer2Logic->closeDevice(this->MlcLayer2SerialPort);
  }

  delete this->MlcLayer1Logic;
  delete this->MlcLayer2Logic;
  this->MlcLayer1Logic = nullptr;
  this->MlcLayer2Logic = nullptr;
  this->MlcLayer1SerialPort = nullptr;
  this->MlcLayer2SerialPort = nullptr;
}

//-----------------------------------------------------------------------------
vtkSlicerIhepMlcControlLogic* qSlicerIhepMlcControlModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerIhepMlcControlModuleWidget);
  return vtkSlicerIhepMlcControlLogic::SafeDownCast(q->logic());
}

// --------------------------------------------------------------------------
vtkMRMLIhepMlcControlNode::LayerType qSlicerIhepMlcControlModuleWidgetPrivate::getSelectedMlcLayer() const
{
  if (this->RadioButton_DeviceLayer1->isEnabled() && this->RadioButton_DeviceLayer1->isChecked())
  {
    return vtkMRMLIhepMlcControlNode::Layer1;
  }
  else if (this->RadioButton_DeviceLayer2->isEnabled() && this->RadioButton_DeviceLayer2->isChecked())
  {
    return vtkMRMLIhepMlcControlNode::Layer2;
  }
  else
  {
    qWarning() << Q_FUNC_INFO << ": Invalid MLC layer value";
    return vtkMRMLIhepMlcControlNode::Layer_Last;
  }
}

//-----------------------------------------------------------------------------
// qSlicerIhepMlcControlModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerIhepMlcControlModuleWidget::qSlicerIhepMlcControlModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerIhepMlcControlModuleWidgetPrivate(*this) )
{
}

//-----------------------------------------------------------------------------
qSlicerIhepMlcControlModuleWidget::~qSlicerIhepMlcControlModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::setup()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  for (int i = 0; i < d->TableWidget_LeafState->rowCount(); ++i)
  {
    QTableWidgetItem* item = new QTableWidgetItem();
    item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    d->TableWidget_LeafState->setItem( i, 0, item);
  }

  d->MlcControlWidget = new qSlicerIhepMlcControlLayoutWidget;

  qSlicerSingletonViewFactory* viewFactory = new qSlicerSingletonViewFactory();
  viewFactory->setWidget(d->MlcControlWidget);
  viewFactory->setTagName("MlcControlLayout");

  const char* layoutString = \
    "<layout type=\"vertical\">" \
    " <item>" \
    "  <MlcControlLayout></MlcControlLayout>" \
    " </item>" \
    "</layout>";

  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  layoutManager->registerViewFactory(viewFactory);
  // Save previous layout
  d->PreviousLayoutId = layoutManager->layout();

  vtkMRMLLayoutNode* layoutNode = layoutManager->layoutLogic()->GetLayoutNode();
  if (layoutNode)
  {
    layoutNode->AddLayoutDescription( d->MlcCustomLayoutId, layoutString);
  }

  // MRMLNodeComboBoxes
  QObject::connect( d->MRMLNodeComboBox_Beam, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    this, SLOT(onBeamNodeChanged(vtkMRMLNode*)));
  QObject::connect( d->MRMLNodeComboBox_MlcTable, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    this, SLOT(onMlcTableNodeChanged(vtkMRMLNode*)));

  // MLC layer button group
  QObject::connect( d->ButtonGroup_MlcDeviceLayer, SIGNAL(buttonClicked(QAbstractButton*)), 
    this, SLOT(onMlcLayerChanged(QAbstractButton*)));

  // Buttons
  QObject::connect( d->PushButton_SetMlcTable, SIGNAL(clicked()),
    this, SLOT(onSetMlcTableClicked()));
  QObject::connect( d->PushButton_SwitchLayout, SIGNAL(toggled(bool)),
    this, SLOT(onSwitchToMlcControlLayoutToggled(bool)));
  QObject::connect( d->CheckBox_ParallelBeam, SIGNAL(toggled(bool)),
    this, SLOT(onParallelBeamToggled(bool)));
  QObject::connect( d->PushButton_GenerateMlcBoundary, SIGNAL(clicked()),
    this, SLOT(onGenerateMlcBoundaryClicked()));
  QObject::connect( d->PushButton_UpdateMlcBoundary, SIGNAL(clicked()),
    this, SLOT(onUpdateMlcBoundaryClicked()));
  QObject::connect( d->PushButton_ConnectMlcDevice, SIGNAL(clicked()),
    this, SLOT(onConnectMlcLayerDevicesClicked()));
  QObject::connect( d->PushButton_SetLeafParameters, SIGNAL(clicked()),
    this, SLOT(onLeafSetParametersClicked()));
  QObject::connect( d->PushButton_SetLeafRelative, SIGNAL(clicked()),
    this, SLOT(onLeafSetRelativeClicked()));
  QObject::connect( d->PushButton_GetLeafState, SIGNAL(clicked()),
    this, SLOT(onLeafGetStateClicked()));
  QObject::connect( d->PushButton_StartLeaf, SIGNAL(clicked()),
    this, SLOT(onLeafStartClicked()));
  QObject::connect( d->PushButton_StopLeaf, SIGNAL(clicked()),
    this, SLOT(onLeafStopClicked()));
  QObject::connect( d->PushButton_StartLeaves, SIGNAL(clicked()),
    this, SLOT(onLeavesStartBroadcastClicked()));
  QObject::connect( d->PushButton_StopLeaves, SIGNAL(clicked()),
    this, SLOT(onLeavesStopBroadcastClicked()));
  QObject::connect( d->PushButton_OpenLeaves, SIGNAL(clicked()),
    this, SLOT(onLeavesOpenBroadcastClicked()));
  QObject::connect( d->PushButton_SetLeavesParameters, SIGNAL(clicked()),
    this, SLOT(onLeavesSetParametersClicked()));
  QObject::connect( d->PushButton_SetLeavesRelativeParameters, SIGNAL(clicked()),
    this, SLOT(onLeavesSetRelativeParametesClicked()));
  QObject::connect( d->PushButton_GetLeavesState, SIGNAL(clicked()),
    this, SLOT(onLeavesGetStateClicked()));

  // Combo boxes
  QObject::connect( d->ComboBox_MotorFrequency, SIGNAL(currentIndexChanged(int)),
    this, SLOT(onLeafMotorFrequencyIndexChanged(int)));

  // Sliders
  QObject::connect( d->SliderWidget_NumberOfLeavesPairs, SIGNAL(valueChanged(double)),
    this, SLOT(onNumberOfLeafPairsChanged(double)));
  QObject::connect( d->SliderWidget_PairOfLeavesBoundarySize, SIGNAL(valueChanged(double)),
    this, SLOT(onPairOfLeavesSizeChanged(double)));
  QObject::connect( d->SliderWidget_IsocenterOffset, SIGNAL(valueChanged(double)),
    this, SLOT(onIsocenterOffsetChanged(double)));
  QObject::connect( d->SliderWidget_DistanceBetweenLayers, SIGNAL(valueChanged(double)),
    this, SLOT(onDistanceBetweenTwoLayersChanged(double)));
  QObject::connect( d->SliderWidget_LayersOffset, SIGNAL(valueChanged(double)),
    this, SLOT(onOffsetBetweenTwoLayersChanged(double)));

  QObject::connect( d->HorizontalSlider_LeafAddress, SIGNAL(valueChanged(int)),
    this, SLOT(onLeafAddressChanged(int)));
  QObject::connect( d->HorizontalSlider_LeafSteps, SIGNAL(valueChanged(int)),
    this, SLOT(onLeafStepsChanged(int)));

  // GroupBox
  QObject::connect( d->ButtonGroup_MlcLayers, SIGNAL(buttonClicked(QAbstractButton*)),
    this, SLOT(onMlcLayersButtonClicked(QAbstractButton*)));
  QObject::connect( d->ButtonGroup_MlcOrientation, SIGNAL(buttonClicked(QAbstractButton*)),
    this, SLOT(onMlcOrientationButtonClicked(QAbstractButton*)));
  QObject::connect( d->ButtonGroup_LeafDirection, SIGNAL(buttonClicked(QAbstractButton*)),
    this, SLOT(onLeafDirectionButtonClicked(QAbstractButton*)));
  QObject::connect( d->ButtonGroup_LeafStepMode, SIGNAL(buttonClicked(QAbstractButton*)),
    this, SLOT(onLeafStepModeButtonClicked(QAbstractButton*)));

  // CheckBox
  QObject::connect( d->CheckBox_MotorReset, SIGNAL(toggled(bool)),
    this, SLOT(onLeafResetToggled(bool)));
  QObject::connect( d->CheckBox_MotorEnable, SIGNAL(toggled(bool)),
    this, SLOT(onLeafEnabledToggled(bool)));
  QObject::connect( d->CheckBox_ContinuousStateMonitoring, SIGNAL(toggled(bool)),
    this, SLOT(onContinuousStateMonitoringToggled(bool)));

  // MLC Control widget signals: layer, position
  QObject::connect( this, SIGNAL(mlcLayerChanged(vtkMRMLIhepMlcControlNode::LayerType)),
    d->MlcControlWidget, SLOT(onMlcLayerChanged(vtkMRMLIhepMlcControlNode::LayerType)));
  QObject::connect( d->MlcControlWidget, SIGNAL(mlcLayerChanged(vtkMRMLIhepMlcControlNode::LayerType)),
    this, SLOT(onMlcLayerChanged(vtkMRMLIhepMlcControlNode::LayerType)));
  QObject::connect( d->MlcControlWidget, SIGNAL(leafDataStepsChanged(int, int)), this,
    SLOT(onLeafDataStepsChanged(int,int)));

  // MLC Control widget and MLC device logic signals
/*
  QObject::connect( d->MlcLayer1Logic, SIGNAL(leafPositionChanged(int, cn::LayerType, cn::SideType,int, int)),
    d->MlcControlWidget, SLOT(onLeafPositionChanged(int, cn::LayerType, cn::SideType,int, int)));
  QObject::connect( d->MlcLayer1Logic, SIGNAL(leafSwitchChanged(int, cn::LayerType, cn::SideType,bool)),
    d->MlcControlWidget, SLOT(onLeafSwitchChanged(int, cn::LayerType, cn::SideType,bool)));
  QObject::connect( d->MlcLayer2Logic, SIGNAL(leafPositionChanged(int, cn::LayerType, cn::SideType,int, int)),
    d->MlcControlWidget, SLOT(onLeafPositionChanged(int, cn::LayerType, cn::SideType,int, int)));
  QObject::connect( d->MlcLayer2Logic, SIGNAL(leafSwitchChanged(int, cn::LayerType, cn::SideType,bool)),
    d->MlcControlWidget, SLOT(onLeafSwitchChanged(int, cn::LayerType, cn::SideType,bool)));
*/
  QObject::connect( d->MlcLayer1Logic, SIGNAL(leafStateCommandBufferChanged(const vtkMRMLIhepMlcControlNode::CommandBufferType&, vtkMRMLIhepMlcControlNode::LayerType, vtkMRMLIhepMlcControlNode::SideType)),
    this, SLOT(onLeafStateCommandBufferChanged(const vtkMRMLIhepMlcControlNode::CommandBufferType&, vtkMRMLIhepMlcControlNode::LayerType, vtkMRMLIhepMlcControlNode::SideType)));
  QObject::connect( d->MlcLayer2Logic, SIGNAL(leafStateCommandBufferChanged(const vtkMRMLIhepMlcControlNode::CommandBufferType&, vtkMRMLIhepMlcControlNode::LayerType, vtkMRMLIhepMlcControlNode::SideType)),
    this, SLOT(onLeafStateCommandBufferChanged(const vtkMRMLIhepMlcControlNode::CommandBufferType&, vtkMRMLIhepMlcControlNode::LayerType, vtkMRMLIhepMlcControlNode::SideType)));

  QObject::connect( d->MlcLayer1Logic, SIGNAL(leafStateCommandBufferChanged(const vtkMRMLIhepMlcControlNode::CommandBufferType&, vtkMRMLIhepMlcControlNode::LayerType, vtkMRMLIhepMlcControlNode::SideType)),
    d->MlcControlWidget, SLOT(onLeafStateCommandBufferChanged(const vtkMRMLIhepMlcControlNode::CommandBufferType&, vtkMRMLIhepMlcControlNode::LayerType, vtkMRMLIhepMlcControlNode::SideType)));
  QObject::connect( d->MlcLayer2Logic, SIGNAL(leafStateCommandBufferChanged(const vtkMRMLIhepMlcControlNode::CommandBufferType&, vtkMRMLIhepMlcControlNode::LayerType, vtkMRMLIhepMlcControlNode::SideType)),
    d->MlcControlWidget, SLOT(onLeafStateCommandBufferChanged(const vtkMRMLIhepMlcControlNode::CommandBufferType&, vtkMRMLIhepMlcControlNode::LayerType, vtkMRMLIhepMlcControlNode::SideType)));

  QObject::connect( d->CollapsibleGroupBox_LeafControl, SIGNAL(toggled(bool)), d->MlcControlWidget, SLOT(onDebugModeEnabled(bool)));

  // Select predefined shape as square
  QTimer::singleShot(0, d->MlcControlWidget, [=](){ d->MlcControlWidget->onMlcPredefinedIndexChanged(3); } );
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::exit()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  this->Superclass::exit();

  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  layoutManager->setLayout(d->PreviousLayoutId);
}


//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::enter()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  this->Superclass::enter();

  this->onEnter();

  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  d->PreviousLayoutId = layoutManager->layout();
  layoutManager->setLayout(d->MlcCustomLayoutId);
  QTimer::singleShot(100, this, SLOT(onSetMlcControlLayout()));
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onSwitchToMlcControlLayoutToggled(bool toggled)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  layoutManager->setLayout(toggled ? d->MlcCustomLayoutId : d->PreviousLayoutId);
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onSetMlcControlLayout()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  layoutManager->setLayout(d->MlcCustomLayoutId);
  QSignalBlocker blocker1(d->PushButton_SwitchLayout);
  d->PushButton_SwitchLayout->setChecked(true);
}


//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  this->Superclass::setMRMLScene(scene);

  qvtkReconnect( d->logic(), scene, vtkMRMLScene::EndImportEvent, this, SLOT(onSceneImportedEvent()));
  qvtkReconnect( d->logic(), scene, vtkMRMLScene::EndCloseEvent, this, SLOT(onSceneClosedEvent()));

  // Set scene to dose engine logic
  d->MlcLayer1Logic->setMRMLScene(scene);
  d->MlcLayer2Logic->setMRMLScene(scene);

  // Find parameters node or create it if there is none in the scene
  if (scene)
  {
    if (d->MRMLNodeComboBox_ParameterSet->currentNode())
    {
      this->setParameterNode(d->MRMLNodeComboBox_ParameterSet->currentNode());
    }
    else if (vtkMRMLNode* node = scene->GetFirstNodeByClass("vtkMRMLIhepMlcControlNode"))
    {
      this->setParameterNode(node);
    }
    else
    {
      vtkNew<vtkMRMLIhepMlcControlNode> newNode;
      std::string nodeName = this->mrmlScene()->GenerateUniqueName("IHEP_MLC_VRBS");
      newNode->SetName(nodeName.c_str());
      newNode->SetSingletonTag("IHEP_MLC");
      this->mrmlScene()->AddNode(newNode);
      this->setParameterNode(newNode);
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::setParameterNode(vtkMRMLNode *node)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);

  vtkMRMLIhepMlcControlNode* parameterNode = vtkMRMLIhepMlcControlNode::SafeDownCast(node);

  // Make sure the parameter set node is selected (in case the function was not called by the selector combobox signal)
  d->MRMLNodeComboBox_ParameterSet->setCurrentNode(node);

  // Set parameter node to children widgets (MlcControlWidget)
  d->MlcControlWidget->setParameterNode(node);
  d->MlcLayer1Logic->setParameterNode(node);
  d->MlcLayer2Logic->setParameterNode(node);

  // Each time the node is modified, the UI widgets are updated
  qvtkReconnect( parameterNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));
  d->ParameterNode = parameterNode;

  // Set selected MRML nodes in comboboxes in the parameter set if it was nullptr there
  // (then in the meantime the comboboxes selected the first one from the scene and we have to set that)
  if (parameterNode)
  {
    if (!parameterNode->GetBeamNode())
    {
      vtkMRMLRTBeamNode* beamNode = vtkMRMLRTBeamNode::SafeDownCast(d->MRMLNodeComboBox_Beam->currentNode());
//      qvtkConnect( beamNode, vtkMRMLRTBeamNode::BeamTransformModified, this, SLOT(updateNormalAndVupVectors())); // update beam transform and geo
      parameterNode->SetAndObserveBeamNode(beamNode);
    }
  }
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onBeamNodeChanged(vtkMRMLNode *node)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }
  vtkMRMLRTBeamNode* beamNode = vtkMRMLRTBeamNode::SafeDownCast(node);
  if (beamNode)
  {
    d->ParameterNode->SetAndObserveBeamNode(beamNode);
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onMlcTableNodeChanged(vtkMRMLNode *node)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }
  vtkMRMLTableNode* tableNode = vtkMRMLTableNode::SafeDownCast(node);
  d->ParameterNode->GetBeamNode()->SetAndObserveMultiLeafCollimatorTableNode(tableNode);
  d->logic()->UpdateLeavesDataFromMlcPositionTableNode(d->ParameterNode);
  d->PushButton_GenerateMlcBoundary->setEnabled(true);
  d->PushButton_UpdateMlcBoundary->setEnabled(tableNode);
  d->PushButton_SetMlcTable->setEnabled(tableNode);

  // Fill leaves container with a new number of leaf pairs
  vtkMRMLIhepMlcControlNode::LayerType layer = d->getSelectedMlcLayer();
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
    qDebug() << Q_FUNC_INFO << ": Leaves steps: Side1 - " << side1.Steps << ", Side2 - " << side2.Steps;
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);

  vtkMRMLIhepMlcControlNode* parameterNode = vtkMRMLIhepMlcControlNode::SafeDownCast(d->MRMLNodeComboBox_ParameterSet->currentNode());

  if (!this->mrmlScene())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid scene";
    return;
  }

  // Enable widgets
//  d->CheckBox_ShowDrrMarkups->setEnabled(parameterNode);
//  d->CollapsibleButton_ReferenceInput->setEnabled(parameterNode);
//  d->CollapsibleButton_GeometryBasicParameters->setEnabled(parameterNode);
//  d->PlastimatchParametersWidget->setEnabled(parameterNode);
//  d->PushButton_ComputeDrr->setEnabled(parameterNode);

  if (!parameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  if (!parameterNode->GetBeamNode())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid referenced parameter's beam node";
    d->PushButton_GenerateMlcBoundary->setEnabled(false);
    return;
  }

  // Update widgets from parameter node
  d->CheckBox_ParallelBeam->setChecked(parameterNode->GetParallelBeam());
  d->MRMLNodeComboBox_Beam->setCurrentNode(parameterNode->GetBeamNode());

  d->PushButton_GenerateMlcBoundary->setEnabled(true);

  vtkMRMLTableNode* mlcTableNode = parameterNode->GetBeamNode()->GetMultiLeafCollimatorTableNode();
  d->MRMLNodeComboBox_MlcTable->setCurrentNode(mlcTableNode);
//  d->MlcControlWidget->setMlcTableNode(mlcTableNode);
  d->PushButton_UpdateMlcBoundary->setEnabled(mlcTableNode);
  d->PushButton_SetMlcTable->setEnabled(mlcTableNode);

  switch (parameterNode->GetOrientation())
  {
  case vtkMRMLIhepMlcControlNode::X:
    d->RadioButton_MLCX->setChecked(true);
    d->RadioButton_MLCY->setChecked(false);
    break;
  case vtkMRMLIhepMlcControlNode::Y:
    d->RadioButton_MLCX->setChecked(false);
    d->RadioButton_MLCY->setChecked(true);
    break;
  default:
    break;
  }
  switch (parameterNode->GetLayers())
  {
  case vtkMRMLIhepMlcControlNode::OneLayer:
    d->RadioButton_OneLayer->setChecked(true);
    d->RadioButton_TwoLayers->setChecked(false);
    break;
  case vtkMRMLIhepMlcControlNode::TwoLayers:
    d->RadioButton_OneLayer->setChecked(false);
    d->RadioButton_TwoLayers->setChecked(true);
    break;
  default:
    break;
  }
  d->SliderWidget_NumberOfLeavesPairs->setValue(parameterNode->GetNumberOfLeafPairs());
  d->SliderWidget_PairOfLeavesBoundarySize->setValue(parameterNode->GetPairOfLeavesSize());
  d->SliderWidget_IsocenterOffset->setValue(parameterNode->GetIsocenterOffset());
  d->SliderWidget_DistanceBetweenLayers->setValue(parameterNode->GetDistanceBetweenTwoLayers());
  d->SliderWidget_LayersOffset->setValue(parameterNode->GetOffsetBetweenTwoLayers());
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onSceneImportedEvent()
{
  this->onEnter();
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onSceneClosedEvent()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onEnter()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid scene";
    return;
  }

  // First check the logic if it has a parameter node
  if (!d->logic())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid logic";
    return;
  }

  vtkMRMLIhepMlcControlNode* parameterNode = nullptr; 
  // Try to find one in the scene
  if (vtkMRMLNode* node = this->mrmlScene()->GetFirstNodeByClass("vtkMRMLIhepMlcControlNode"))
  {
    parameterNode = vtkMRMLIhepMlcControlNode::SafeDownCast(node);
  }

  if (parameterNode && parameterNode->GetBeamNode())
  {
    // First thing first: update normal and vup vectors for parameter node
    // in case observed beam node transformation has been modified
///    d->logic()->UpdateNormalAndVupVectors(parameterNode);
  }

  // Create DRR markups nodes
///  d->logic()->CreateMarkupsNodes(parameterNode);

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onParameterNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  vtkMRMLIhepMlcControlNode* parameterNode = vtkMRMLIhepMlcControlNode::SafeDownCast(node);

  if (!parameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  this->setParameterNode(parameterNode);
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onParallelBeamToggled(bool toggled)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  d->ParameterNode->SetParallelBeam(toggled);
  qDebug() << Q_FUNC_INFO << ": MLC parallel beam " << toggled;
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onMlcLayersButtonClicked(QAbstractButton* button)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  QRadioButton* rButton = qobject_cast<QRadioButton*>(button);
  if (rButton && rButton == d->RadioButton_OneLayer)
  {
    d->ParameterNode->SetLayers(vtkMRMLIhepMlcControlNode::OneLayer);
  }
  else if (rButton && rButton == d->RadioButton_TwoLayers)
  {
    d->ParameterNode->SetLayers(vtkMRMLIhepMlcControlNode::TwoLayers);
  }
  else
  {
  }
  qDebug() << Q_FUNC_INFO << ": Number of MLC layers " << d->ParameterNode->GetLayers();
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onMlcOrientationButtonClicked(QAbstractButton* button)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  QRadioButton* rButton = qobject_cast<QRadioButton*>(button);
  if (rButton && rButton == d->RadioButton_MLCX)
  {
    d->ParameterNode->SetOrientation(vtkMRMLIhepMlcControlNode::X);
  }
  else if (rButton && rButton == d->RadioButton_MLCY)
  {
    d->ParameterNode->SetOrientation(vtkMRMLIhepMlcControlNode::Y);
  }
  else
  {
  }
  qDebug() << Q_FUNC_INFO << ": MLC orientation " << d->ParameterNode->GetOrientation();
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onNumberOfLeafPairsChanged(double numberOfLeaves)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  d->ParameterNode->SetNumberOfLeafPairs(static_cast<int>(numberOfLeaves));
  qDebug() << Q_FUNC_INFO << ": number of leaves " << numberOfLeaves;
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onPairOfLeavesSizeChanged(double size)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  d->ParameterNode->SetPairOfLeavesSize(size);
  qDebug() << Q_FUNC_INFO << ": pair of leaves size " << size;
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onIsocenterOffsetChanged(double offset)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  d->ParameterNode->SetIsocenterOffset(offset);
  qDebug() << Q_FUNC_INFO << ": isocenter offset distance " << offset;
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onDistanceBetweenTwoLayersChanged(double distance)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  d->ParameterNode->SetDistanceBetweenTwoLayers(distance);
  qDebug() << Q_FUNC_INFO << ": distance between layers " << distance;
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onOffsetBetweenTwoLayersChanged(double distance)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  d->ParameterNode->SetOffsetBetweenTwoLayers(distance);
  qDebug() << Q_FUNC_INFO << ": layer offset distance " << distance;
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onGenerateMlcBoundaryClicked()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  if (vtkMRMLTableNode* tableNode = d->logic()->CreateMlcTableNodeBoundaryData(d->ParameterNode))
  {
    vtkMRMLRTBeamNode* beamNode = d->ParameterNode->GetBeamNode();
    if (d->logic()->SetBeamParentForMlcTableNode(beamNode, tableNode))
    {
      qDebug() << Q_FUNC_INFO << ": Table created";
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onUpdateMlcBoundaryClicked()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  if (!d->ParameterNode->GetBeamNode())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid RTBeam node";
    return;
  }

  if (vtkMRMLTableNode* tableNode = d->ParameterNode->GetBeamNode()->GetMultiLeafCollimatorTableNode())
  {
    tableNode->RemoveAllColumns();
    if (d->logic()->UpdateMlcTableNodeBoundaryData(d->ParameterNode, tableNode))
    {
      qDebug() << Q_FUNC_INFO << ": Table updated";
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onSetMlcTableClicked()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  if (!d->ParameterNode->GetBeamNode())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid RTBeam node";
    return;
  }

  if (d->logic()->UpdateLeavesDataFromMlcPositionTableNode(d->ParameterNode))
  {
    qDebug() << Q_FUNC_INFO << ":Leaves steps were updated";
  }
  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onConnectMlcLayerDevicesClicked()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  // if devices are connected disconnect them
  if (d->PushButton_ConnectMlcDevice->text() == tr("Disconnect"))
  {
    if (d->MlcLayer1SerialPort && d->MlcLayer1SerialPort->isOpen())
    {
      d->MlcLayer1Logic->closeDevice(d->MlcLayer1SerialPort);
    }
    if (d->MlcLayer2SerialPort && d->MlcLayer2SerialPort->isOpen())
    {
      d->MlcLayer2Logic->closeDevice(d->MlcLayer2SerialPort);
    }
    d->PushButton_ConnectMlcDevice->setText(tr("Connect"));
    qDebug() << Q_FUNC_INFO << ": Disconnected";
    return;
  }

  // connect devices
  if (d->ParameterNode->GetLayers() == vtkMRMLIhepMlcControlNode::OneLayer)
  {
    QString portName = d->LineEdit_DeviceLayer1->text();
    d->MlcLayer1SerialPort = d->MlcLayer1Logic->openDevice(portName, vtkMRMLIhepMlcControlNode::Layer1);
    if (d->MlcLayer1SerialPort)
    {
      qDebug() << Q_FUNC_INFO << ": Layer-1 Connected";
    }
    if (d->MlcLayer1SerialPort && d->MlcLayer1SerialPort->isOpen())
    {
      d->PushButton_ConnectMlcDevice->setText(tr("Disconnect"));
    }
  }
  if (d->ParameterNode->GetLayers() == vtkMRMLIhepMlcControlNode::TwoLayers)
  {
    QString portName = d->LineEdit_DeviceLayer1->text();
    d->MlcLayer1SerialPort = d->MlcLayer1Logic->openDevice(portName, vtkMRMLIhepMlcControlNode::Layer1);
    if (d->MlcLayer1SerialPort)
    {
      qDebug() << Q_FUNC_INFO << ": Layer-1 Connected";
    }
    portName = d->LineEdit_DeviceLayer2->text();
    d->MlcLayer2SerialPort = d->MlcLayer2Logic->openDevice(portName, vtkMRMLIhepMlcControlNode::Layer2);
    if (d->MlcLayer2SerialPort)
    {
      qDebug() << Q_FUNC_INFO << ": Layer-2 Connected";
    }

    if ((d->MlcLayer1SerialPort && d->MlcLayer1SerialPort->isOpen()) ||
      (d->MlcLayer2SerialPort && d->MlcLayer2SerialPort->isOpen()))
    {
      d->PushButton_ConnectMlcDevice->setText(tr("Disconnect"));
    }
  }
  d->RadioButton_DeviceLayer1->setEnabled(d->MlcLayer1SerialPort);
  d->RadioButton_DeviceLayer2->setEnabled(d->MlcLayer2SerialPort);
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeafAddressChanged(int address)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }

  int pos, range;
  if (d->MlcControlWidget && d->MlcControlWidget->getLeafDataByAddress(address, range, pos))
  {
    d->HorizontalSlider_LeafSteps->setRange(0, range);
    d->HorizontalSlider_LeafSteps->setValue(pos);
    d->SpinBox_LeafSteps->setRange(0, range);
    d->SpinBox_LeafSteps->setValue(pos);
  }
  vtkMRMLIhepMlcControlNode::LayerType layer = d->getSelectedMlcLayer();
  vtkMRMLIhepMlcControlNode::LeafData leafData;
  if (d->ParameterNode->GetLeafDataByAddressInLayer(leafData, address, layer))
  {
    (leafData.Direction) ? d->RadioButton_Clockwise->setChecked(true) : d->RadioButton_CounterClockwise->setChecked(true);
    d->ComboBox_MotorFrequency->setCurrentIndex(leafData.Frequency);
    (leafData.Mode) ? d->RadioButton_HalfStep->setChecked(true) : d->RadioButton_FullStep->setChecked(true);
    d->CheckBox_MotorReset->setChecked(leafData.Reset);
    d->CheckBox_MotorEnable->setChecked(leafData.Enabled);
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeafStepModeButtonClicked(QAbstractButton* button)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }
  vtkMRMLIhepMlcControlNode::LayerType layer = d->getSelectedMlcLayer();
  vtkMRMLIhepMlcControlNode::LeafData leafData;
  int address = d->HorizontalSlider_LeafAddress->value();
  if (d->ParameterNode->GetLeafDataByAddressInLayer(leafData, address, layer))
  {
    QRadioButton* rButton = qobject_cast<QRadioButton*>(button);
    if (rButton && rButton == d->RadioButton_FullStep)
    {
      leafData.Mode = false;
    }
    else if (rButton && rButton == d->RadioButton_HalfStep)
    {
      leafData.Mode = true;
    }
    else
    {
      return;
    }
    if (d->ParameterNode->SetLeafDataByAddressInLayer(leafData, address, layer))
    {
      qDebug() << Q_FUNC_INFO << ": Leaf step mode is set";
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeafDirectionButtonClicked(QAbstractButton* button)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }
  vtkMRMLIhepMlcControlNode::LayerType layer = d->getSelectedMlcLayer();
  vtkMRMLIhepMlcControlNode::LeafData leafData;
  int address = d->HorizontalSlider_LeafAddress->value();
  if (d->ParameterNode->GetLeafDataByAddressInLayer(leafData, address, layer))
  {
    QRadioButton* rButton = qobject_cast<QRadioButton*>(button);
    if (rButton && rButton == d->RadioButton_Clockwise)
    {
      leafData.Direction = true;
    }
    else if (rButton && rButton == d->RadioButton_CounterClockwise)
    {
      leafData.Direction = false;
    }
    else
    {
      return;
    }
    if (d->ParameterNode->SetLeafDataByAddressInLayer(leafData, address, layer))
    {
      qDebug() << Q_FUNC_INFO << ": Leaf direction is set";
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeafStepsChanged(int steps)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }
  vtkMRMLIhepMlcControlNode::LayerType layer = d->getSelectedMlcLayer();
  vtkMRMLIhepMlcControlNode::LeafData leafData;
  int address = d->HorizontalSlider_LeafAddress->value();
  if (d->ParameterNode->GetLeafDataByAddressInLayer(leafData, address, layer))
  {
    leafData.Steps = steps;
    leafData.RequiredPosition = steps;
    if (d->ParameterNode->SetLeafDataByAddressInLayer(leafData, address, layer))
    {
      qDebug() << Q_FUNC_INFO << ": Leaf steps is set";
      if (d->logic()->UpdateMlcPositionTableFromLeafData(d->ParameterNode, leafData))
      {
        qDebug() << Q_FUNC_INFO << ": MLC leaf position is changed";
      }
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeafResetToggled(bool reset)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }
  vtkMRMLIhepMlcControlNode::LayerType layer = d->getSelectedMlcLayer();
  vtkMRMLIhepMlcControlNode::LeafData leafData;
  int address = d->HorizontalSlider_LeafAddress->value();
  if (d->ParameterNode->GetLeafDataByAddressInLayer(leafData, address, layer))
  {
    leafData.Reset = reset;
    if (d->ParameterNode->SetLeafDataByAddressInLayer(leafData, address, layer))
    {
      qDebug() << Q_FUNC_INFO << ": MLC leaf reset is set";
//      d->MlcControlWidget->setLeafData(leafData);
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeafEnabledToggled(bool enabled)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }
  vtkMRMLIhepMlcControlNode::LayerType layer = d->getSelectedMlcLayer();
  vtkMRMLIhepMlcControlNode::LeafData leafData;
  int address = d->HorizontalSlider_LeafAddress->value();
  if (d->ParameterNode->GetLeafDataByAddressInLayer(leafData, address, layer))
  {
    leafData.Enabled = enabled;
    if (d->ParameterNode->SetLeafDataByAddressInLayer(leafData, address, layer))
    {
      qDebug() << Q_FUNC_INFO << ": MLC leaf enable is set";
//      d->MlcControlWidget->setLeafData(leafData);
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeafMotorFrequencyIndexChanged(int freq)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }
  vtkMRMLIhepMlcControlNode::LayerType layer = d->getSelectedMlcLayer();
  vtkMRMLIhepMlcControlNode::LeafData leafData;
  int address = d->HorizontalSlider_LeafAddress->value();
  if (d->ParameterNode->GetLeafDataByAddressInLayer(leafData, address, layer))
  {
    leafData.Frequency = freq;
    if (d->ParameterNode->SetLeafDataByAddressInLayer(leafData, address, layer))
    {
      qDebug() << Q_FUNC_INFO << ": MLC leaf frequency is set";
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeafSetParametersClicked()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }
  
  vtkMRMLIhepMlcControlNode::LayerType layer = d->getSelectedMlcLayer();
  vtkMRMLIhepMlcControlNode::LeafData leafData;
  int address = d->HorizontalSlider_LeafAddress->value();
  QByteArray com;
  if (d->ParameterNode->GetLeafDataByAddressInLayer(leafData, address, layer))
  {
    leafData.Frequency = d->ComboBox_MotorFrequency->currentIndex();
    switch (layer)
    {
    case vtkMRMLIhepMlcControlNode::Layer1:
      com = d->MlcLayer1Logic->getParametersCommandByLeafData(leafData);
      break;
    case vtkMRMLIhepMlcControlNode::Layer2:
      com = d->MlcLayer2Logic->getParametersCommandByLeafData(leafData);
      break;
    default:
      break;
    }
  }
  if (com.size())
  {
    switch (layer)
    {
    case vtkMRMLIhepMlcControlNode::Layer1:
      d->MlcLayer1Logic->addCommandToQueue(com);
      break;
    case vtkMRMLIhepMlcControlNode::Layer2:
      d->MlcLayer2Logic->addCommandToQueue(com);
      break;
    default:
      break;
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeafSetRelativeClicked()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }
  vtkMRMLIhepMlcControlNode::LayerType layer = d->getSelectedMlcLayer();
  vtkMRMLIhepMlcControlNode::LeafData leafData;
  int address = d->HorizontalSlider_LeafAddress->value();
  QByteArray com;
  if (d->ParameterNode->GetLeafDataByAddressInLayer(leafData, address, layer))
  {
    leafData.Frequency = d->ComboBox_MotorFrequency->currentIndex();
    switch (layer)
    {
    case vtkMRMLIhepMlcControlNode::Layer1:
      com = d->MlcLayer1Logic->getRelativeParametersCommandByLeafData(leafData);
      break;
    case vtkMRMLIhepMlcControlNode::Layer2:
      com = d->MlcLayer2Logic->getRelativeParametersCommandByLeafData(leafData);
      break;
    default:
      break;
    }
  }
  if (com.size())
  {
    switch (layer)
    {
    case vtkMRMLIhepMlcControlNode::Layer1:
      d->MlcLayer1Logic->addCommandToQueue(com);
      break;
    case vtkMRMLIhepMlcControlNode::Layer2:
      d->MlcLayer2Logic->addCommandToQueue(com);
      break;
    default:
      break;
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeafGetStateClicked()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }
  vtkMRMLIhepMlcControlNode::LayerType layer = d->getSelectedMlcLayer();
  vtkMRMLIhepMlcControlNode::LeafData leafData;
  int address = d->HorizontalSlider_LeafAddress->value();
  QByteArray com;
  if (d->ParameterNode->GetLeafDataByAddressInLayer(leafData, address, layer))
  {
    switch (layer)
    {
    case vtkMRMLIhepMlcControlNode::Layer1:
      com = d->MlcLayer1Logic->getStateCommandByLeafData(leafData);
      break;
    case vtkMRMLIhepMlcControlNode::Layer2:
      com = d->MlcLayer2Logic->getStateCommandByLeafData(leafData);
      break;
    default:
      break;
    }
  }
  if (com.size())
  {
    switch (layer)
    {
    case vtkMRMLIhepMlcControlNode::Layer1:
      d->MlcLayer1Logic->addCommandToQueue(com);
      break;
    case vtkMRMLIhepMlcControlNode::Layer2:
      d->MlcLayer2Logic->addCommandToQueue(com);
      break;
    default:
      break;
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeafStartClicked()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }
  vtkMRMLIhepMlcControlNode::LayerType layer = d->getSelectedMlcLayer();
  vtkMRMLIhepMlcControlNode::LeafData leafData;
  int address = d->HorizontalSlider_LeafAddress->value();
  QByteArray com;
  if (d->ParameterNode->GetLeafDataByAddressInLayer(leafData, address, layer))
  {
    switch (layer)
    {
    case vtkMRMLIhepMlcControlNode::Layer1:
      com = d->MlcLayer1Logic->getStartCommandByLeafData(leafData);
      break;
    case vtkMRMLIhepMlcControlNode::Layer2:
      com = d->MlcLayer2Logic->getStartCommandByLeafData(leafData);
      break;
    default:
      break;
    }
  }
  if (com.size())
  {
    switch (layer)
    {
    case vtkMRMLIhepMlcControlNode::Layer1:
      d->MlcLayer1Logic->addCommandToQueue(com);
      break;
    case vtkMRMLIhepMlcControlNode::Layer2:
      d->MlcLayer2Logic->addCommandToQueue(com);
      break;
    default:
      break;
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeafStopClicked()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }
  vtkMRMLIhepMlcControlNode::LayerType layer = d->getSelectedMlcLayer();
  vtkMRMLIhepMlcControlNode::LeafData leafData;
  int address = d->HorizontalSlider_LeafAddress->value();
  QByteArray com;
  if (d->ParameterNode->GetLeafDataByAddressInLayer(leafData, address, layer))
  {
    switch (layer)
    {
    case vtkMRMLIhepMlcControlNode::Layer1:
      com = d->MlcLayer1Logic->getStopCommandByLeafData(leafData);
      break;
    case vtkMRMLIhepMlcControlNode::Layer2:
      com = d->MlcLayer2Logic->getStopCommandByLeafData(leafData);
      break;
    default:
      break;
    }
  }
  if (com.size())
  {
    switch (layer)
    {
    case vtkMRMLIhepMlcControlNode::Layer1:
      d->MlcLayer1Logic->addCommandToQueue(com);
      break;
    case vtkMRMLIhepMlcControlNode::Layer2:
      d->MlcLayer2Logic->addCommandToQueue(com);
      break;
    default:
      break;
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeafDataStepsChanged(int address, int leafDataSteps)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }
  qDebug() << Q_FUNC_INFO << "Address: " << address << " movement steps: " << leafDataSteps;
  if (d->logic()->UpdateMlcTableNodePositionData(d->ParameterNode, address, leafDataSteps))
  {
    qDebug() << Q_FUNC_INFO << "MLC table modified";
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeavesSetParametersClicked()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  vtkMRMLIhepMlcControlNode::LayerType selectedLayer = d->getSelectedMlcLayer();
  QList< QByteArray > mlcLayerParametesCommands;

  std::vector< int > addresses; // empty vector == all addresses
  switch (selectedLayer)
  {
  case vtkMRMLIhepMlcControlNode::Layer1:
    mlcLayerParametesCommands = d->MlcLayer1Logic->getParametersCommands(addresses);
    break;
  case vtkMRMLIhepMlcControlNode::Layer2:
    mlcLayerParametesCommands = d->MlcLayer2Logic->getParametersCommands(addresses);
    break;
  default:
    break;
  }
  if (mlcLayerParametesCommands.size())
  {
    switch (selectedLayer)
    {
    case vtkMRMLIhepMlcControlNode::Layer1:
      d->MlcLayer1Logic->addCommandsToQueue(mlcLayerParametesCommands);
      break;
    case vtkMRMLIhepMlcControlNode::Layer2:
      d->MlcLayer2Logic->addCommandsToQueue(mlcLayerParametesCommands);
      break;
    default:
      break;
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeavesSetRelativeParametesClicked()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  vtkMRMLIhepMlcControlNode::LayerType selectedLayer = d->getSelectedMlcLayer();
  QList< QByteArray > mlcLayerRelativeParametesCommands;

  std::vector< int > addresses; // empty vector == all addresses
  switch (selectedLayer)
  {
  case vtkMRMLIhepMlcControlNode::Layer1:
    mlcLayerRelativeParametesCommands = d->MlcLayer1Logic->getRelativeParametersCommands(addresses);
    break;
  case vtkMRMLIhepMlcControlNode::Layer2:
    mlcLayerRelativeParametesCommands = d->MlcLayer2Logic->getRelativeParametersCommands(addresses);
    break;
  default:
    break;
  }
  if (mlcLayerRelativeParametesCommands.size())
  {
    QByteArray startBroadcastCommand;
    switch (selectedLayer)
    {
    case vtkMRMLIhepMlcControlNode::Layer1:
      d->MlcLayer1Logic->addCommandsToQueue(mlcLayerRelativeParametesCommands);
      {
        startBroadcastCommand = d->MlcLayer1Logic->getStartBroadcastCommand();
        if (startBroadcastCommand.size())
        {
          d->MlcLayer1Logic->addCommandToQueue(startBroadcastCommand);
        }
      }
      break;
    case vtkMRMLIhepMlcControlNode::Layer2:
      d->MlcLayer2Logic->addCommandsToQueue(mlcLayerRelativeParametesCommands);
      {
        startBroadcastCommand = d->MlcLayer2Logic->getStartBroadcastCommand();
        if (startBroadcastCommand.size())
        {
          d->MlcLayer2Logic->addCommandToQueue(startBroadcastCommand);
        }
      }
      break;
    default:
      break;
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeavesGetStateClicked()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  vtkMRMLIhepMlcControlNode::LayerType selectedLayer = d->getSelectedMlcLayer();
  QList< QByteArray > mlcLayerStateCommands;

  std::vector< int > addresses; // empty vector == all addresses
  switch (selectedLayer)
  {
  case vtkMRMLIhepMlcControlNode::Layer1:
    mlcLayerStateCommands = d->MlcLayer1Logic->getStateCommands(addresses);
    break;
  case vtkMRMLIhepMlcControlNode::Layer2:
    mlcLayerStateCommands = d->MlcLayer2Logic->getStateCommands(addresses);
    break;
  default:
    break;
  }
  if (mlcLayerStateCommands.size())
  {
    switch (selectedLayer)
    {
    case vtkMRMLIhepMlcControlNode::Layer1:
      d->MlcLayer1Logic->addCommandsToQueue(mlcLayerStateCommands);
      break;
    case vtkMRMLIhepMlcControlNode::Layer2:
      d->MlcLayer2Logic->addCommandsToQueue(mlcLayerStateCommands);
      break;
    default:
      break;
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeavesStartBroadcastClicked()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  vtkMRMLIhepMlcControlNode::LayerType selectedLayer = d->getSelectedMlcLayer();
  QByteArray com;

  switch (selectedLayer)
  {
  case vtkMRMLIhepMlcControlNode::Layer1:
    com = d->MlcLayer1Logic->getStartBroadcastCommand();
    break;
  case vtkMRMLIhepMlcControlNode::Layer2:
    com = d->MlcLayer2Logic->getStartBroadcastCommand();
    break;
  default:
    break;
  }
  if (com.size())
  {
    switch (selectedLayer)
    {
    case vtkMRMLIhepMlcControlNode::Layer1:
      d->MlcLayer1Logic->addCommandToQueue(com);
      break;
    case vtkMRMLIhepMlcControlNode::Layer2:
      d->MlcLayer2Logic->addCommandToQueue(com);
      break;
    default:
      break;
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeavesStopBroadcastClicked()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  vtkMRMLIhepMlcControlNode::LayerType selectedLayer = d->getSelectedMlcLayer();
  QByteArray com;

  switch (selectedLayer)
  {
  case vtkMRMLIhepMlcControlNode::Layer1:
    com = d->MlcLayer1Logic->getStopBroadcastCommand();
    break;
  case vtkMRMLIhepMlcControlNode::Layer2:
    com = d->MlcLayer2Logic->getStopBroadcastCommand();
    break;
  default:
    break;
  }
  if (com.size())
  {
    switch (selectedLayer)
    {
    case vtkMRMLIhepMlcControlNode::Layer1:
      d->MlcLayer1Logic->addCommandToQueue(com);
      break;
    case vtkMRMLIhepMlcControlNode::Layer2:
      d->MlcLayer2Logic->addCommandToQueue(com);
      break;
    default:
      break;
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeavesOpenBroadcastClicked()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  vtkMRMLIhepMlcControlNode::LayerType selectedLayer = d->getSelectedMlcLayer();
  QByteArray com;

  switch (selectedLayer)
  {
  case vtkMRMLIhepMlcControlNode::Layer1:
    com = d->MlcLayer1Logic->getOpenBroadcastCommand();
    break;
  case vtkMRMLIhepMlcControlNode::Layer2:
    com = d->MlcLayer2Logic->getOpenBroadcastCommand();
    break;
  default:
    break;
  }
  if (com.size())
  {
    switch (selectedLayer)
    {
    case vtkMRMLIhepMlcControlNode::Layer1:
      d->MlcLayer1Logic->addCommandToQueue(com);
      break;
    case vtkMRMLIhepMlcControlNode::Layer2:
      d->MlcLayer2Logic->addCommandToQueue(com);
      break;
    default:
      break;
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onMlcLayersPredefinedPositionChanged(vtkMRMLIhepMlcControlNode::PredefinedPositionType)
{
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onMlcLayerChanged(vtkMRMLIhepMlcControlNode::LayerType selectedLayer)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  switch (selectedLayer)
  {
  case vtkMRMLIhepMlcControlNode::Layer1:
    d->RadioButton_DeviceLayer1->setChecked(true);
    break;
  case vtkMRMLIhepMlcControlNode::Layer2:
    d->RadioButton_DeviceLayer2->setChecked(true);
    break;
  default:
    break;
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onMlcLayerChanged(QAbstractButton* button)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  QRadioButton* radioButton = qobject_cast<QRadioButton*>(button);
  vtkMRMLIhepMlcControlNode::LayerType selectedMlcLayer = vtkMRMLIhepMlcControlNode::Layer_Last;
  if (radioButton == d->RadioButton_DeviceLayer1)
  {
    selectedMlcLayer = vtkMRMLIhepMlcControlNode::Layer1;
  }
  else if (radioButton == d->RadioButton_DeviceLayer2)
  {
    selectedMlcLayer = vtkMRMLIhepMlcControlNode::Layer2;
  }
  if (selectedMlcLayer != vtkMRMLIhepMlcControlNode::Layer_Last)
  {
    emit mlcLayerChanged(selectedMlcLayer);
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onContinuousStateMonitoringToggled(bool toggled)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  qDebug() << Q_FUNC_INFO << ": state " << toggled;
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeafStateCommandBufferChanged(const vtkMRMLIhepMlcControlNode::CommandBufferType& stateBuffer,
  vtkMRMLIhepMlcControlNode::LayerType layer,
  vtkMRMLIhepMlcControlNode::SideType side)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);

  vtkMRMLIhepMlcControlNode::LeafData leafData;
  vtkMRMLIhepMlcControlNode::LayerType selectedLayer = d->getSelectedMlcLayer();
  vtkMRMLIhepMlcControlNode::ProcessCommandBufferToLeafData(stateBuffer, leafData);
  if (leafData.Address == d->HorizontalSlider_LeafAddress->value() && layer == selectedLayer)
  {
    leafData.Side = side;
    leafData.Layer = layer;
    this->setLeafData(leafData);
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::setLeafData(const vtkMRMLIhepMlcControlNode::LeafData& data)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }
  d->ParameterNode->SetLeafDataState(data);

///  d->MlcControlWidget->setLeafData(data);

  QTableWidgetItem* item = d->TableWidget_LeafState->item( 0, 0);
  item->setText(QString::number(data.Address));

  item = d->TableWidget_LeafState->item( 1, 0);
  item->setText(QString::number(data.State));

  item = d->TableWidget_LeafState->item( 2, 0);
  if (data.Mode)
  {
    item->setText(tr("Half step"));
  }
  else
  {
    item->setText(tr("Full step"));
  }

  item = d->TableWidget_LeafState->item( 3, 0);
  item->setText(QString::number(data.StateReset));
  if (data.StateReset)
  {
    item->setText(tr("On"));
  }
  else
  {
    item->setText(tr("Off"));
  }

  item = d->TableWidget_LeafState->item( 4, 0);
  if (!data.StateDirection)
  {
    item->setText(tr("To the switch"));
  }
  else
  {
    item->setText(tr("Away from the switch"));
  }

  item = d->TableWidget_LeafState->item( 5, 0);
  if (data.StateEnabled)
  {
    item->setText(tr("Enabled"));
  }
  else
  {
    item->setText(tr("Disabled"));
  }
  item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

  item = d->TableWidget_LeafState->item( 6, 0);
  item->setText(QString::number(data.StepsLeft));

  item = d->TableWidget_LeafState->item( 7, 0);
  item->setText(QString::number(data.EncoderCounts));

  item = d->TableWidget_LeafState->item( 8, 0);
  if (data.EncoderDirection)
  {
    item->setText(tr("To the switch"));
  }
  else
  {
    item->setText(tr("Away from the switch"));
  }

  item = d->TableWidget_LeafState->item( 9, 0);
  if (data.SwitchState)
  {
    item->setText(tr("Pressed"));
  }
  else
  {
    item->setText(tr("Released"));
  }
  
  item = d->TableWidget_LeafState->item( 10, 0);
  item->setText(QString::number(data.CurrentPosition));
}
