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

  qSlicerIhepMlcControlLayoutWidget* MlcControlWidget{ nullptr };
  int PreviousLayoutId{ 0 };
  int MlcCustomLayoutId{ 507 };
  vtkWeakPointer<vtkMRMLIhepMlcControlNode> ParameterNode;
};

//-----------------------------------------------------------------------------
// qSlicerIhepMlcControlModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerIhepMlcControlModuleWidgetPrivate::qSlicerIhepMlcControlModuleWidgetPrivate(qSlicerIhepMlcControlModuleWidget &object)
  :
  q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
qSlicerIhepMlcControlModuleWidgetPrivate::~qSlicerIhepMlcControlModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
vtkSlicerIhepMlcControlLogic* qSlicerIhepMlcControlModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerIhepMlcControlModuleWidget);
  return vtkSlicerIhepMlcControlLogic::SafeDownCast(q->logic());
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

  // Buttons
  QObject::connect( d->PushButton_SwitchLayout, SIGNAL(toggled(bool)),
    this, SLOT(onSwitchToMlcControlLayoutToggled(bool)));
  QObject::connect( d->CheckBox_ParallelBeam, SIGNAL(toggled(bool)),
    this, SLOT(onParallelBeamToggled(bool)));
  QObject::connect( d->PushButton_GenerateMlcBoundary, SIGNAL(clicked()),
    this, SLOT(onGenerateMlcBoundaryClicked()));
  QObject::connect( d->PushButton_UpdateMlcBoundary, SIGNAL(clicked()),
    this, SLOT(onUpdateMlcBoundaryClicked()));

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

  // GroupBox
  QObject::connect( d->ButtonGroup_MlcLayers, SIGNAL(buttonClicked(QAbstractButton*)),
    this, SLOT(onMlcLayersButtonClicked(QAbstractButton*)));
  QObject::connect( d->ButtonGroup_MlcOrientation, SIGNAL(buttonClicked(QAbstractButton*)),
    this, SLOT(onMlcOrientationButtonClicked(QAbstractButton*)));
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
  if (tableNode)
  {
    d->ParameterNode->GetBeamNode()->SetAndObserveMultiLeafCollimatorTableNode(tableNode);
    d->PushButton_GenerateMlcBoundary->setEnabled(false);
    d->PushButton_UpdateMlcBoundary->setEnabled(true);
  }
  else
  {
    d->ParameterNode->GetBeamNode()->SetAndObserveMultiLeafCollimatorTableNode(nullptr);
    d->PushButton_GenerateMlcBoundary->setEnabled(true);
    d->PushButton_UpdateMlcBoundary->setEnabled(false);
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
  if (vtkMRMLTableNode* mlcTable = parameterNode->GetBeamNode()->GetMultiLeafCollimatorTableNode())
  {
    d->MRMLNodeComboBox_MlcTable->setCurrentNode(parameterNode->GetBeamNode()->GetMultiLeafCollimatorTableNode());
    d->PushButton_GenerateMlcBoundary->setEnabled(false);
    d->PushButton_UpdateMlcBoundary->setEnabled(true);
  }
  else
  {
    d->PushButton_GenerateMlcBoundary->setEnabled(true);
    d->PushButton_UpdateMlcBoundary->setEnabled(false);
  }
  
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
  qDebug() << Q_FUNC_INFO << ": MLC layers " << d->ParameterNode->GetLayers();
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
