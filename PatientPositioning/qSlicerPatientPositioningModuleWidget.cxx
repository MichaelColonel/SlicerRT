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

// Slicer includes
#include "qSlicerPatientPositioningModuleWidget.h"
#include "ui_qSlicerPatientPositioningModuleWidget.h"

#include <qSlicerLayoutManager.h>
#include <qSlicerApplication.h>
#include <qMRMLSliceWidget.h>
#include <qSlicerSubjectHierarchyFolderPlugin.h>
#include <qSlicerSubjectHierarchyPluginHandler.h>
#include <qMRMLThreeDWidget.h>
#include <qMRMLThreeDView.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLSegmentationNode.h>
#include <vtkMRMLCameraNode.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLSubjectHierarchyNode.h>

#include <vtkMRMLRTPlanNode.h>
#include <vtkMRMLRTBeamNode.h>

// PatientPositioning MRML includes
#include <vtkMRMLRTChannel26Cabin3BeamNode.h>
#include <vtkMRMLRTCarmBeamNode.h>
#include <vtkMRMLPatientPositioningNode.h>
#include <vtkMRMLChannel26GeometryNode.h>

// Qt includes
#include <QDebug>
#include <QDir>
#include <QFileDialog>

// CTK includes
#include <ctkMessageBox.h>
#include <ctkSliderWidget.h>

// VTK includes
#include <vtkCamera.h>
#include "vtkCollisionDetectionFilter.h"
#include <vtkPolyData.h>
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>

// Logic includes
#include <vtkSlicerPatientPositioningLogic.h>
#include <vtkSlicerChannel26Cabin3RobotsTransformLogic.h>
#include <vtkSlicerDrrImageComputationLogic.h>
// DRR MRML includes
#include <vtkMRMLDrrImageComputationNode.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_PatientPositioning
class qSlicerPatientPositioningModuleWidgetPrivate: public Ui_qSlicerPatientPositioningModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerPatientPositioningModuleWidget);
protected:
  qSlicerPatientPositioningModuleWidget* const q_ptr;
public:
  qSlicerPatientPositioningModuleWidgetPrivate(qSlicerPatientPositioningModuleWidget &object);
  virtual ~qSlicerPatientPositioningModuleWidgetPrivate();
  vtkSlicerPatientPositioningLogic* logic() const;
  vtkSlicerChannel26Cabin3RobotsTransformLogic* channel26RobotsLogic() const;
  vtkSlicerDrrImageComputationLogic* drrImageComputationLogic() const;
  vtkMRMLCameraNode* get3DViewCameraNode() const;
  qMRMLLayoutManager* getLayoutManager() const;

  /// PatientPositioning and Geometry MRML nodes containing shown parameters
  vtkSmartPointer<vtkMRMLPatientPositioningNode> ParameterNode;
  bool ModuleWindowInitialized{ false };
};

//-----------------------------------------------------------------------------
// qSlicerPatientPositioningModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerPatientPositioningModuleWidgetPrivate::qSlicerPatientPositioningModuleWidgetPrivate(qSlicerPatientPositioningModuleWidget &object)
  :
  q_ptr(&object)
{
}

qSlicerPatientPositioningModuleWidgetPrivate::~qSlicerPatientPositioningModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
vtkSlicerPatientPositioningLogic* qSlicerPatientPositioningModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerPatientPositioningModuleWidget);
  return vtkSlicerPatientPositioningLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
vtkSlicerChannel26Cabin3RobotsTransformLogic* qSlicerPatientPositioningModuleWidgetPrivate::channel26RobotsLogic() const
{
  Q_Q(const qSlicerPatientPositioningModuleWidget);
  vtkSlicerPatientPositioningLogic* logic = vtkSlicerPatientPositioningLogic::SafeDownCast(q->logic());
  return (logic) ? logic->GetChannel26RobotsTransformLogic() : nullptr;
}

//-----------------------------------------------------------------------------
vtkMRMLCameraNode* qSlicerPatientPositioningModuleWidgetPrivate::get3DViewCameraNode() const
{
  Q_Q(const qSlicerPatientPositioningModuleWidget);

  // Get 3D view node
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  qMRMLThreeDView* threeDView = layoutManager->threeDWidget(0)->threeDView();
  vtkMRMLViewNode* viewNode = threeDView->mrmlViewNode();

  // Get camera node for view
  vtkCollection* cameras = q->mrmlScene()->GetNodesByClass("vtkMRMLCameraNode");
  vtkMRMLCameraNode* cameraNode = nullptr;
  for (int i = 0; i < cameras->GetNumberOfItems(); i++)
  {
    cameraNode = vtkMRMLCameraNode::SafeDownCast(cameras->GetItemAsObject(i));
    std::string viewUniqueName = std::string(viewNode->GetNodeTagName()) + cameraNode->GetLayoutName();
    if (viewUniqueName == viewNode->GetID())
    {
      break;
    }
  }
  if (!cameraNode)
  {
    qCritical() << Q_FUNC_INFO << "Failed to find camera for view " << (viewNode ? viewNode->GetID() : "(null)");
  }
  cameras->Delete();
  return cameraNode;
}

//-----------------------------------------------------------------------------
qMRMLLayoutManager* qSlicerPatientPositioningModuleWidgetPrivate::getLayoutManager() const
{
  Q_Q(const qSlicerPatientPositioningModuleWidget);

  // Get 3D view node
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  return slicerApplication->layoutManager();
}

//-----------------------------------------------------------------------------
// qSlicerPatientPositioningModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerPatientPositioningModuleWidget::qSlicerPatientPositioningModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerPatientPositioningModuleWidgetPrivate(*this) )
{
}

//-----------------------------------------------------------------------------
qSlicerPatientPositioningModuleWidget::~qSlicerPatientPositioningModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::setup()
{
  Q_D(qSlicerPatientPositioningModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  // Add treatment machine options
  d->ComboBox_TreatmentMachine->clear();
  d->ComboBox_TreatmentMachine->addItem("Channel-26 Cabin-3", "Channel26Cabin3Geometry");
  d->ComboBox_TreatmentMachine->addItem("Channel-26 Cabin-2", "Channel26Cabin2Geometry");
  d->ComboBox_TreatmentMachine->addItem("Channel-26 Cabin-1", "Channel26Cabin1Geometry");
  d->ComboBox_TreatmentMachine->addItem("From file...", "FromFile");

  // Nodes
  connect( d->MRMLNodeComboBox_ParameterSet, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onParameterNodeChanged(vtkMRMLNode*)));
  connect( d->MRMLNodeComboBox_Plan, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onPlanNodeChanged(vtkMRMLNode*)));
  connect( d->MRMLNodeComboBox_Beam, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onBeamNodeChanged(vtkMRMLNode*)));
  connect( d->MRMLNodeComboBox_FixedReferenceBeam, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onFixedReferenceBeamNodeChanged(vtkMRMLNode*)));
  connect( d->MRMLNodeComboBox_CarmXrayBeam, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onCarmXrayBeamNodeChanged(vtkMRMLNode*)));
  connect( d->SegmentSelectorWidget_PatientBody, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onPatientBodySegmentationNodeChanged(vtkMRMLNode*)));
  connect( d->SegmentSelectorWidget_PatientBody, SIGNAL(currentSegmentChanged(QString)), 
    this, SLOT(onPatientBodySegmentChanged(QString)));

  // Buttons
  connect( d->PushButton_LoadTreatmentMachine, SIGNAL(clicked()), 
    this, SLOT(onLoadTreatmentMachineButtonClicked()));
  connect( d->CheckBox_RotatePatientHeadFeet, SIGNAL(toggled(bool)), 
    this, SLOT(onRotatePatientHeadFeetToggled(bool)));

  // Buttons
  connect( d->PushButton_BevXPlus, SIGNAL(clicked()), this, SLOT(onBeamsEyeViewPlusXButtonClicked()));
  connect( d->PushButton_BevXMinus, SIGNAL(clicked()), this, SLOT(onBeamsEyeViewMinusXButtonClicked()));
  connect( d->PushButton_BevYMinus, SIGNAL(clicked()), this, SLOT(onBeamsEyeViewMinusYButtonClicked()));
  connect( d->PushButton_BevYPlus, SIGNAL(clicked()), this, SLOT(onBeamsEyeViewPlusYButtonClicked()));

  // Widgets
  // Table robot angles
  connect( d->SliderWidget_TableRobotA6, SIGNAL(valueChanged(double)), 
    this, SLOT(onTableRobotA6Changed(double)));
  connect( d->SliderWidget_TableRobotA5, SIGNAL(valueChanged(double)), 
    this, SLOT(onTableRobotA5Changed(double)));
  connect( d->SliderWidget_TableRobotA4, SIGNAL(valueChanged(double)), 
    this, SLOT(onTableRobotA4Changed(double)));
  connect( d->SliderWidget_TableRobotA3, SIGNAL(valueChanged(double)), 
    this, SLOT(onTableRobotA3Changed(double)));
  connect( d->SliderWidget_TableRobotA2, SIGNAL(valueChanged(double)), 
    this, SLOT(onTableRobotA2Changed(double)));
  connect( d->SliderWidget_TableRobotA1, SIGNAL(valueChanged(double)), 
    this, SLOT(onTableRobotA1Changed(double)));
  // C-arm robot angles
  connect( d->SliderWidget_CarmRobotA1, SIGNAL(valueChanged(double)), 
    this, SLOT(onCarmRobotA1Changed(double)));
  connect( d->SliderWidget_CarmRobotA2, SIGNAL(valueChanged(double)), 
    this, SLOT(onCarmRobotA2Changed(double)));
  connect( d->SliderWidget_CarmRobotA3, SIGNAL(valueChanged(double)), 
    this, SLOT(onCarmRobotA3Changed(double)));
  connect( d->SliderWidget_CarmRobotA4, SIGNAL(valueChanged(double)), 
    this, SLOT(onCarmRobotA4Changed(double)));
  connect( d->SliderWidget_CarmRobotA5, SIGNAL(valueChanged(double)), 
    this, SLOT(onCarmRobotA5Changed(double)));
  connect( d->SliderWidget_CarmRobotA6, SIGNAL(valueChanged(double)), 
    this, SLOT(onCarmRobotA6Changed(double)));

  connect( d->CoordinatesWidget_PatientTableTopTranslation, SIGNAL(coordinatesChanged(double*)),
    this, SLOT(onPatientTableTopTranslationChanged(double*)));
 
  // models, markups, camera checkboxes
  connect( d->CheckBox_ShowModels, SIGNAL(toggled(bool)), this, SLOT(onShowModelsToggled(bool)));
  connect( d->CheckBox_ShowMarkups, SIGNAL(toggled(bool)), this, SLOT(onShowMarkupsToggled(bool)));
  connect( d->CheckBox_FixedReferenceCamera, SIGNAL(toggled(bool)), 
    this, SLOT(onFixedReferenceCameraToggled(bool)));

  // Children custom widgets
  connect( this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), d->CarmXrayBeamWidget, SLOT(setMRMLScene(vtkMRMLScene*)));
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerPatientPositioningModuleWidget);
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
    else if (vtkMRMLNode* node = scene->GetNthNodeByClass( 0, "vtkMRMLPatientPositioningNode"))
    {
      this->setParameterNode(node);
    }
    else
    {
      vtkMRMLNode* newNode = scene->AddNewNodeByClass("vtkMRMLPatientPositioningNode");
      this->setParameterNode(newNode);
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::setParameterNode(vtkMRMLNode *node)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  vtkMRMLPatientPositioningNode* parameterNode = vtkMRMLPatientPositioningNode::SafeDownCast(node);

  // Make sure the parameter set node is selected (in case the function was not called by the selector combobox signal)
  d->MRMLNodeComboBox_ParameterSet->setCurrentNode(node);

  // Each time the node is modified, the UI widgets are updated
  qvtkReconnect( d->ParameterNode, parameterNode, vtkCommand::ModifiedEvent, 
    this, SLOT( updateWidgetFromMRML() ) );

  d->ParameterNode = parameterNode;

  // Set parameter node to children (FixedBeamAxis, CarmXrayBeamWidget) widgets
//  d->FixedBeamAxisWidget->setParameterNode(d->ParameterNode);
  d->CarmXrayBeamWidget->setParameterNode(d->ParameterNode);

  // Set selected MRML nodes in comboboxes in the parameter set if it was nullptr there
  // (then in the meantime the comboboxes selected the first one from the scene and we have to set that)
  if (d->ParameterNode)
  {
    vtkMRMLRTBeamNode* beamNode = vtkMRMLRTBeamNode::SafeDownCast(d->MRMLNodeComboBox_Beam->currentNode());
    d->ParameterNode->SetAndObserveBeamNode(beamNode);
    vtkMRMLRTBeamNode* planNode = vtkMRMLRTBeamNode::SafeDownCast(d->MRMLNodeComboBox_Plan->currentNode());
    Q_UNUSED(planNode);
  }
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::enter()
{
  Q_D(qSlicerPatientPositioningModuleWidget);
  this->Superclass::enter();
  this->onEnter();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::exit()
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  this->Superclass::exit();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onParameterNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerPatientPositioningModuleWidget);
  vtkMRMLPatientPositioningNode* parameterNode = vtkMRMLPatientPositioningNode::SafeDownCast(node);

  if (!parameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  this->setParameterNode(parameterNode);
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onPlanNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerPatientPositioningModuleWidget);
  vtkMRMLRTPlanNode* planNode = vtkMRMLRTPlanNode::SafeDownCast(node);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  Q_UNUSED(planNode);
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onBeamNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerPatientPositioningModuleWidget);
  vtkMRMLRTBeamNode* beamNode = vtkMRMLRTBeamNode::SafeDownCast(node);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  d->ParameterNode->SetAndObserveBeamNode(beamNode);
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onFixedReferenceBeamNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerPatientPositioningModuleWidget);
  vtkMRMLRTChannel26Cabin3BeamNode* beamNode = vtkMRMLRTChannel26Cabin3BeamNode::SafeDownCast(node);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  d->ParameterNode->SetAndObserveFixedReferenceBeamNode(beamNode);
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onCarmXrayBeamNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerPatientPositioningModuleWidget);
  vtkMRMLRTCarmBeamNode* beamNode = vtkMRMLRTCarmBeamNode::SafeDownCast(node);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  d->ParameterNode->SetAndObserveCarmXrayBeamNode(beamNode);
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onPatientBodySegmentationNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerPatientPositioningModuleWidget);
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(node);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  d->ParameterNode->SetAndObservePatientBodySegmentationNode(segmentationNode);
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onPatientBodySegmentChanged(QString segmentID)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  if (!d->ParameterNode || segmentID.isEmpty() || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  d->ParameterNode->SetPatientBodySegmentID(segmentID.toUtf8().constData());
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onEnter()
{
  Q_D(qSlicerPatientPositioningModuleWidget);

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
  // Select or create parameter node
  vtkMRMLPatientPositioningNode* parameterNode = nullptr; 
  // Try to find one in the scene
  if (vtkMRMLNode* node = this->mrmlScene()->GetFirstNodeByClass("vtkMRMLPatientPositioningNode"))
  {
    parameterNode = vtkMRMLPatientPositioningNode::SafeDownCast(node);
  }

  if (parameterNode)
  {
  }

  // Set logics to childred widgets
//  d->CarmXrayBeamWidget->setMRMLScene(this->mrmlScene());
  d->CarmXrayBeamWidget->setPatientPositioningLogic(d->logic());

  // All required data for GUI is initiated
  this->updateWidgetFromMRML();
  
  d->ModuleWindowInitialized = true;
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  vtkMRMLPatientPositioningNode* parameterNode = vtkMRMLPatientPositioningNode::SafeDownCast(d->MRMLNodeComboBox_ParameterSet->currentNode());

  if (!this->mrmlScene())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid scene";
    return;
  }

  if (!parameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onSceneImportedEvent()
{
  this->onEnter();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onSceneClosedEvent()
{
  Q_D(qSlicerPatientPositioningModuleWidget);
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onLoadTreatmentMachineButtonClicked()
{
  Q_D(qSlicerPatientPositioningModuleWidget);
  int currentMachineIndex = d->ComboBox_TreatmentMachine->currentIndex();
  Q_UNUSED(currentMachineIndex);

  vtkMRMLScene* scene = this->mrmlScene();
  if (!scene)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid scene";
    return;
  }

  if (!d->ParameterNode)
  {
    return;
  }

  using SysCoord = vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier;
  // Get treatment machine descriptor file path
  QString treatmentMachineType(d->ComboBox_TreatmentMachine->currentData().toString());
  QString descriptorFilePath;
  if (!treatmentMachineType.compare("FromFile"))
  {
    // Ask user for descriptor JSON file if load from file option is selected
    descriptorFilePath = QFileDialog::getOpenFileName( this, "Select treatment machine descriptor JSON file...",
      QString(), "Json files (*.json);; All files (*)" ); 
  }
  else //TODO: Currently support two default types in addition to loading file. Need to rethink the module
  {
    std::string treatMachType = treatmentMachineType.toStdString();
    d->ParameterNode->SetTreatmentMachineType(treatMachType.c_str());
    QString relativeFilePath = QString("%1/%2.json").arg(treatmentMachineType).arg(treatmentMachineType);
    descriptorFilePath = QDir(d->logic()->GetModuleShareDirectory().c_str()).filePath(relativeFilePath);
  }
  std::string descFilePath = descriptorFilePath.toStdString();
  d->ParameterNode->SetTreatmentMachineDescriptorFilePath(descFilePath.c_str());

  // Set treatment machine dependent properties  //TODO: Use degrees of freedom from JSON
  if (!treatmentMachineType.compare("Channel-26 Cabin-3"))
  {
    qDebug() << Q_FUNC_INFO << "Channel-26 Cabin-3";
    d->ParameterNode->SetTreatmentMachineType("Channel26Cabin3Geometry");
  }
  else if (!treatmentMachineType.compare("Channel-26 Cabin-2"))
  {
    qDebug() << Q_FUNC_INFO << "Channel-26 Cabin-2";
    d->ParameterNode->SetTreatmentMachineType("Channel26Cabin2Geometry");
  }
  else if (!treatmentMachineType.compare("Channel-26 Cabin-1"))
  {
    qDebug() << Q_FUNC_INFO << "Channel-26 Cabin-1";
    d->ParameterNode->SetTreatmentMachineType("Channel26Cabin1Geometry");
  }

  // Check if there is a machine already loaded and ask user what to do if so
  vtkMRMLSubjectHierarchyNode* shNode = this->mrmlScene()->GetSubjectHierarchyNode();
  std::vector<vtkIdType> allItemIDs;
  shNode->GetItemChildren(shNode->GetSceneItemID(), allItemIDs, true);
  std::vector<vtkIdType> machineFolderItemIDs;
  std::vector<vtkIdType>::iterator itemIt;
  for (itemIt=allItemIDs.begin(); itemIt!=allItemIDs.end(); ++itemIt)
  {
    std::string machineDescriptorFilePath = shNode->GetItemAttribute(*itemIt, "TreatmentMachineDescriptorFilePath");
    if (!machineDescriptorFilePath.compare(descriptorFilePath.toUtf8().constData()))
    {
      QMessageBox::warning(this, tr("Machine already loaded"), tr("This treatment machine is already loaded."));
      return;
    }
    if (!machineDescriptorFilePath.empty())
    {
      machineFolderItemIDs.push_back(*itemIt);
    }
  }
  // Create a new vtkMRMLChannel26GeometryNode;
  // Set and observe created node in PatientPositioningNode
  if (!scene->GetFirstNodeByClass("vtkMRMLChannel26GeometryNode"))
  {
    vtkNew<vtkMRMLChannel26GeometryNode> channel26GeometryNode;
    channel26GeometryNode->SetName("Channel26Geometry");
    channel26GeometryNode->SetHideFromEditors(0);
    scene->AddNode(channel26GeometryNode);
    d->ParameterNode->SetAndObserveChannel26GeometryNode(channel26GeometryNode.GetPointer());
  }
  else
  {
    vtkMRMLChannel26GeometryNode* channel26GeometryNode = vtkMRMLChannel26GeometryNode::SafeDownCast(
      scene->GetFirstNodeByClass("vtkMRMLChannel26GeometryNode"));
    d->ParameterNode->SetAndObserveChannel26GeometryNode(channel26GeometryNode);
  }
  d->ParameterNode->SetTreatmentMachineDescriptorFilePath(descriptorFilePath.toUtf8().constData());

  if (machineFolderItemIDs.size() > 0)
  {
    ctkMessageBox* existingMachineMsgBox = new ctkMessageBox(this);
    existingMachineMsgBox->setWindowTitle(tr("Other machines loaded"));
    existingMachineMsgBox->setText(tr("There is another treatment machine loaded in the scene. Would you like to hide or delete it?"));

    existingMachineMsgBox->addButton(tr("Hide"), QMessageBox::AcceptRole);
    existingMachineMsgBox->addButton(tr("Delete"), QMessageBox::DestructiveRole);
    existingMachineMsgBox->addButton(tr("No action"), QMessageBox::RejectRole);

    existingMachineMsgBox->setDontShowAgainVisible(true);
    existingMachineMsgBox->setDontShowAgainSettingsKey("SlicerRT/DontAskOnMultipleTreatmentMachines");
    existingMachineMsgBox->setIcon(QMessageBox::Question);
    existingMachineMsgBox->exec();
    int resultCode = existingMachineMsgBox->buttonRole(existingMachineMsgBox->clickedButton());
    if (resultCode == QMessageBox::AcceptRole)
    {
      qSlicerSubjectHierarchyFolderPlugin* folderPlugin = qobject_cast<qSlicerSubjectHierarchyFolderPlugin*>(
        qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName("Folder") );
      for (itemIt=machineFolderItemIDs.begin(); itemIt!=machineFolderItemIDs.end(); ++itemIt)
      {
        folderPlugin->setDisplayVisibility(*itemIt, false);
      }
    }
    else if (resultCode == QMessageBox::DestructiveRole)
    {
      for (itemIt=machineFolderItemIDs.begin(); itemIt!=machineFolderItemIDs.end(); ++itemIt)
      {
        shNode->RemoveItem(*itemIt);
      }
    }
  }
  QApplication::setOverrideCursor(Qt::WaitCursor);
  // Load and setup models
  std::vector< SysCoord > loadedParts;
  vtkMRMLChannel26GeometryNode* channel26GeometryNode = d->ParameterNode->GetChannel26GeometryNode();
  if (channel26GeometryNode)
  {
    d->getLayoutManager()->pauseRender();
    loadedParts = d->logic()->LoadTreatmentMachineComponents(d->ParameterNode);
    d->getLayoutManager()->resumeRender();
  }
  // TODO: Unable collision detection later when models will be OK
/*
  // Warn the user if collision detection is disabled for certain part pairs
  QString disabledCollisionDetectionMessage(
    tr("Collision detection for the following part pairs may take very long due to high triangle numbers:\n\n"));
  bool collisionDetectionDisabled = false;
  if (d->logic()->GetTableTopFixedReferenceCollisionDetection()->GetInputData(0) == nullptr)
  {
    disabledCollisionDetectionMessage.append("TableTop-FixedReference\n");
    collisionDetectionDisabled = true;
  }
  if (d->logic()->GetTableTopElbowCollisionDetection()->GetInputData(0) == nullptr)
  {
    disabledCollisionDetectionMessage.append("TableTop-Elbow\n");
    collisionDetectionDisabled = true;
  }
  if (d->logic()->GetTableTopShoulderCollisionDetection()->GetInputData(0) == nullptr)
  {
    disabledCollisionDetectionMessage.append("TableTop-Shoulder\n");
    collisionDetectionDisabled = true;
  }
  if (d->logic()->GetTableTopBaseRotationCollisionDetection()->GetInputData(0) == nullptr)
  {
    disabledCollisionDetectionMessage.append("TableTop-BaseRotation\n");
    collisionDetectionDisabled = true;
  }
  if (d->logic()->GetTableTopBaseFixedCollisionDetection()->GetInputData(0) == nullptr)
  {
    disabledCollisionDetectionMessage.append("TableTop-BaseFixed\n");
    collisionDetectionDisabled = true;
  }
  disabledCollisionDetectionMessage.append(tr("\nWhat would you like to do?"));
  if (collisionDetectionDisabled)
  {
    ctkMessageBox* existingMachineMsgBox = new ctkMessageBox(this);
    existingMachineMsgBox->setWindowTitle(tr("Collision detection might take too long"));
    existingMachineMsgBox->setText(disabledCollisionDetectionMessage);
    existingMachineMsgBox->addButton(tr("Disable on these part pairs"), QMessageBox::AcceptRole);
    existingMachineMsgBox->addButton(tr("Calculate anyway"), QMessageBox::RejectRole);
    existingMachineMsgBox->setIcon(QMessageBox::Warning);
    existingMachineMsgBox->exec();
    int resultCode = existingMachineMsgBox->buttonRole(existingMachineMsgBox->clickedButton());
    if (resultCode == QMessageBox::RejectRole)
    {
      // Set up treatment machine models again but make sure collision detection is not disabled between any parts
      d->logic()->SetupTreatmentMachineModels(d->ParameterNode, true);
    }
  }
*/

  // Enable treatment machine geometry controls
  d->CollapsibleButton_RobotsControl->setEnabled(true);
  d->CollapsibleButton_PatientTableTopControl->setEnabled(true);

  // Reset camera
/*
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  qMRMLThreeDView* threeDView = layoutManager->threeDWidget(0)->threeDView();
  threeDView->resetCamera();
*/

  vtkSlicerChannel26Cabin3RobotsTransformLogic* channel26Logic = d->channel26RobotsLogic();
  if (channel26Logic && channel26GeometryNode)
  {
    d->getLayoutManager()->pauseRender();
    channel26Logic->ResetToInitialPositions();
    channel26Logic->UpdateTransformsHierarchy(channel26GeometryNode, SysCoord::Patient);

    d->getLayoutManager()->resumeRender();
  }
  // Update channel-26 geometry node
  channel26GeometryNode->Modified();

  // Cabin-3 FixedReference beam
  vtkMRMLRTChannel26Cabin3BeamNode* cabin3BeamNode = d->logic()->CreateCabin3BeamPlanAndNode(d->ParameterNode);
  d->MRMLNodeComboBox_FixedReferenceBeam->setCurrentNode(cabin3BeamNode);

  /// Setup Markups fixed beam axis and fixed isocenter
  vtkMRMLMarkupsLineNode* beamAxisLineNode = d->logic()->CreateCabin3BeamAxisLineNode(d->ParameterNode);
  vtkMRMLMarkupsFiducialNode* fixedIsocenterNode = d->logic()->CreateCabin3IsocenterFiducialNode(d->ParameterNode);
  Q_UNUSED(beamAxisLineNode);
  Q_UNUSED(fixedIsocenterNode);
  
  // C-arm x-ray beam and DRR computation node
  vtkMRMLRTCarmBeamNode* xrayNode = d->logic()->CreateCarmXrayPlanAndNode(d->ParameterNode);
  d->MRMLNodeComboBox_CarmXrayBeam->setCurrentNode(xrayNode);
  vtkMRMLDrrImageComputationNode* drrNode = d->logic()->CreateCarmXrayDrrNode(d->ParameterNode);
  drrNode->SetAndObserveBeamNode(xrayNode);
  d->ParameterNode->SetAndObserveDrrComputationNode(drrNode);

  // set DRR node to children widgets
//  d->CarmXrayBeamWidget->setDrrImageComputationNode(drrNode);
  d->CollapsibleButton_CarmRtImageDrrRegistration->setEnabled(drrNode ? true : false);
/*
  // Hide controls that do not have corresponding parts loaded
  bool imagingPanelsLoaded = (std::find(loadedParts.begin(), loadedParts.end(), vtkSlicerRoomsEyeViewModuleLogic::ImagingPanelLeft) != loadedParts.end() ||
      std::find(loadedParts.begin(), loadedParts.end(), vtkSlicerRoomsEyeViewModuleLogic::ImagingPanelRight) != loadedParts.end());
  d->labelImagingPanel->setVisible(imagingPanelsLoaded);
  d->ImagingPanelMovementSlider->setVisible(imagingPanelsLoaded);

  // Set orientation marker
  //TODO: Add new option 'Treatment room' to orientation marker choices and merged model with actual colors (surface scalars?)
  //vtkMRMLViewNode* viewNode = threeDView->mrmlViewNode();
  //viewNode->SetOrientationMarkerHumanModelNodeID(this->mrmlScene()->GetFirstNodeByName("EBRTOrientationMarkerModel")->GetID());
*/
  d->ParameterNode->Modified();
  QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onPatientTableTopTranslationChanged(double* position)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter node is invalid!";
    return;
  }
  vtkMRMLChannel26GeometryNode* channel26GeometryNode = d->ParameterNode->GetChannel26GeometryNode();

  // Transform RAS translation to LPS
  double positionTmp[3] = { -1. * position[0], -1. * position[1], position[2] };

  d->getLayoutManager()->pauseRender();
  channel26GeometryNode->DisableModifiedEventOn();
  channel26GeometryNode->SetPatientToTableTopTranslation(positionTmp);

  vtkSlicerChannel26Cabin3RobotsTransformLogic* channel26Logic = d->channel26RobotsLogic();
  if (channel26Logic && channel26GeometryNode)
  {
    using SysCoord = vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier;
    channel26Logic->UpdateTransformsHierarchy(channel26GeometryNode, SysCoord::Patient);
  }
  channel26GeometryNode->DisableModifiedEventOff();
  channel26GeometryNode->Modified();
  d->getLayoutManager()->resumeRender();
  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onTableRobotA6Changed(double a6)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter node is invalid!";
    return;
  }
  vtkMRMLChannel26GeometryNode* channel26GeometryNode = d->ParameterNode->GetChannel26GeometryNode();

  d->getLayoutManager()->pauseRender();
  double a[6] = {};
  channel26GeometryNode->GetTableRobotAngles(a);
  channel26GeometryNode->DisableModifiedEventOn();
  a[5] = a6;
  channel26GeometryNode->SetTableRobotAngles(a);
  channel26GeometryNode->DisableModifiedEventOff();

  // Update Channel-26 transforms
  vtkSlicerChannel26Cabin3RobotsTransformLogic* channel26Logic = d->channel26RobotsLogic();
  if (channel26Logic && channel26GeometryNode)
  {
    using SysCoord = vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier;
    channel26Logic->UpdateTransformsHierarchy(channel26GeometryNode, SysCoord::TableRobotFlange);
  }
  channel26GeometryNode->Modified();
  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onTableRobotA5Changed(double a5)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter node is invalid!";
    return;
  }
  vtkMRMLChannel26GeometryNode* channel26GeometryNode = d->ParameterNode->GetChannel26GeometryNode();

  d->getLayoutManager()->pauseRender();
  double a[6] = {};
  channel26GeometryNode->GetTableRobotAngles(a);
  channel26GeometryNode->DisableModifiedEventOn();
  a[4] = 90 + a5; // - a5;
  channel26GeometryNode->SetTableRobotAngles(a);
  channel26GeometryNode->DisableModifiedEventOff();

  // Update Channel-26 transforms
  vtkSlicerChannel26Cabin3RobotsTransformLogic* channel26Logic = d->channel26RobotsLogic();
  if (channel26Logic && channel26GeometryNode)
  {
    using SysCoord = vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier;
    channel26Logic->UpdateTransformsHierarchy(channel26GeometryNode, SysCoord::TableRobotWrist);
  }
  channel26GeometryNode->Modified();
  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onTableRobotA4Changed(double a4)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter node is invalid!";
    return;
  }
  vtkMRMLChannel26GeometryNode* channel26GeometryNode = d->ParameterNode->GetChannel26GeometryNode();

  d->getLayoutManager()->pauseRender();
  double a[6] = {};
  channel26GeometryNode->GetTableRobotAngles(a);
  channel26GeometryNode->DisableModifiedEventOn();
  a[3] = a4;
  channel26GeometryNode->SetTableRobotAngles(a);
  channel26GeometryNode->DisableModifiedEventOff();

  // Update Channel-26 transforms
  vtkSlicerChannel26Cabin3RobotsTransformLogic* channel26Logic = d->channel26RobotsLogic();
  if (channel26Logic && channel26GeometryNode)
  {
    using SysCoord = vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier;
    channel26Logic->UpdateTransformsHierarchy(channel26GeometryNode, SysCoord::TableRobotElbowWrist);
  }
  channel26GeometryNode->Modified();
  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onTableRobotA3Changed(double a3)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter node is invalid!";
    return;
  }
  vtkMRMLChannel26GeometryNode* channel26GeometryNode = d->ParameterNode->GetChannel26GeometryNode();

  d->getLayoutManager()->pauseRender();
  double a[6] = {};
  channel26GeometryNode->GetTableRobotAngles(a);
  channel26GeometryNode->DisableModifiedEventOn();
  a[2] = 90. - a3;
  channel26GeometryNode->SetTableRobotAngles(a);
  channel26GeometryNode->DisableModifiedEventOff();

  // Update Channel-26 transforms
  vtkSlicerChannel26Cabin3RobotsTransformLogic* channel26Logic = d->channel26RobotsLogic();
  if (channel26Logic && channel26GeometryNode)
  {
    using SysCoord = vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier;
    channel26Logic->UpdateTransformsHierarchy(channel26GeometryNode, SysCoord::TableRobotElbowShoulder);
  }
  channel26GeometryNode->Modified();
  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onTableRobotA2Changed(double a2)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter node is invalid!";
    return;
  }
  vtkMRMLChannel26GeometryNode* channel26GeometryNode = d->ParameterNode->GetChannel26GeometryNode();

  d->getLayoutManager()->pauseRender();
  double a[6] = {};
  channel26GeometryNode->GetTableRobotAngles(a);
  channel26GeometryNode->DisableModifiedEventOn();
  a[1] = 90. + a2;
  channel26GeometryNode->SetTableRobotAngles(a);
  channel26GeometryNode->DisableModifiedEventOff();

  // Update Channel-26 transforms
  vtkSlicerChannel26Cabin3RobotsTransformLogic* channel26Logic = d->channel26RobotsLogic();
  if (channel26Logic && channel26GeometryNode)
  {
    using SysCoord = vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier;
    channel26Logic->UpdateTransformsHierarchy(channel26GeometryNode, SysCoord::TableRobotShoulder);
  }
  channel26GeometryNode->Modified();
  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onTableRobotA1Changed(double a1)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter node is invalid!";
    return;
  }
  vtkMRMLChannel26GeometryNode* channel26GeometryNode = d->ParameterNode->GetChannel26GeometryNode();

  d->getLayoutManager()->pauseRender();
  double a[6] = {};
  channel26GeometryNode->GetTableRobotAngles(a);
  channel26GeometryNode->DisableModifiedEventOn();
  a[0] = a1;
  channel26GeometryNode->SetTableRobotAngles(a);
  channel26GeometryNode->DisableModifiedEventOff();

  // Update Channel-26 transforms
  vtkSlicerChannel26Cabin3RobotsTransformLogic* channel26Logic = d->channel26RobotsLogic();
  if (channel26Logic && channel26GeometryNode)
  {
    using SysCoord = vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier;
    channel26Logic->UpdateTransformsHierarchy(channel26GeometryNode, SysCoord::TableRobotBaseRotation);
  }
  channel26GeometryNode->Modified();
  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onCarmRobotA1Changed(double a1)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter node is invalid!";
    return;
  }
  vtkMRMLChannel26GeometryNode* channel26GeometryNode = d->ParameterNode->GetChannel26GeometryNode();

  d->getLayoutManager()->pauseRender();
  double a[6] = {};
  channel26GeometryNode->GetCarmRobotAngles(a);
  channel26GeometryNode->DisableModifiedEventOn();
  a[0] = a1;
  channel26GeometryNode->SetCarmRobotAngles(a);
  channel26GeometryNode->DisableModifiedEventOff();

  // Update Channel-26 transforms
  vtkSlicerChannel26Cabin3RobotsTransformLogic* channel26Logic = d->channel26RobotsLogic();
  if (channel26Logic && channel26GeometryNode)
  {
    using SysCoord = vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier;
    channel26Logic->UpdateTransformsHierarchy(channel26GeometryNode, SysCoord::CarmRobotBaseRotation);
  }
  channel26GeometryNode->Modified();
  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onCarmRobotA2Changed(double a2)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter node is invalid!";
    return;
  }
  vtkMRMLChannel26GeometryNode* channel26GeometryNode = d->ParameterNode->GetChannel26GeometryNode();

  d->getLayoutManager()->pauseRender();
  double a[6] = {};
  channel26GeometryNode->GetCarmRobotAngles(a);
  channel26GeometryNode->DisableModifiedEventOn();
  a[1] = 90. + a2;
  channel26GeometryNode->SetCarmRobotAngles(a);
  channel26GeometryNode->DisableModifiedEventOff();

  // Update Channel-26 transforms
  vtkSlicerChannel26Cabin3RobotsTransformLogic* channel26Logic = d->channel26RobotsLogic();
  if (channel26Logic && channel26GeometryNode)
  {
    using SysCoord = vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier;
    channel26Logic->UpdateTransformsHierarchy(channel26GeometryNode, SysCoord::CarmRobotShoulder);
  }
  channel26GeometryNode->Modified();
  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onCarmRobotA3Changed(double a3)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter node is invalid!";
    return;
  }
  vtkMRMLChannel26GeometryNode* channel26GeometryNode = d->ParameterNode->GetChannel26GeometryNode();

  d->getLayoutManager()->pauseRender();
  double a[6] = {};
  channel26GeometryNode->GetCarmRobotAngles(a);
  channel26GeometryNode->DisableModifiedEventOn();
  a[2] = 90. - a3;
  channel26GeometryNode->SetCarmRobotAngles(a);
  channel26GeometryNode->DisableModifiedEventOff();

  // Update Channel-26 transforms
  vtkSlicerChannel26Cabin3RobotsTransformLogic* channel26Logic = d->channel26RobotsLogic();
  if (channel26Logic && channel26GeometryNode)
  {
    using SysCoord = vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier;
    channel26Logic->UpdateTransformsHierarchy(channel26GeometryNode, SysCoord::CarmRobotElbowShoulder);
  }
  channel26GeometryNode->Modified();
  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onCarmRobotA4Changed(double a4)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter node is invalid!";
    return;
  }
  vtkMRMLChannel26GeometryNode* channel26GeometryNode = d->ParameterNode->GetChannel26GeometryNode();

  d->getLayoutManager()->pauseRender();
  double a[6] = {};
  channel26GeometryNode->GetCarmRobotAngles(a);
  channel26GeometryNode->DisableModifiedEventOn();
  a[3] = a4;
  channel26GeometryNode->SetCarmRobotAngles(a);
  channel26GeometryNode->DisableModifiedEventOff();

  // Update Channel-26 transforms
  vtkSlicerChannel26Cabin3RobotsTransformLogic* channel26Logic = d->channel26RobotsLogic();
  if (channel26Logic && channel26GeometryNode)
  {
    using SysCoord = vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier;
    channel26Logic->UpdateTransformsHierarchy(channel26GeometryNode, SysCoord::CarmRobotElbowWrist);
  }
  channel26GeometryNode->Modified();
  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onCarmRobotA5Changed(double a5)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter node is invalid!";
    return;
  }
  vtkMRMLChannel26GeometryNode* channel26GeometryNode = d->ParameterNode->GetChannel26GeometryNode();

  d->getLayoutManager()->pauseRender();
  double a[6] = {};
  channel26GeometryNode->GetCarmRobotAngles(a);
  channel26GeometryNode->DisableModifiedEventOn();
  a[4] = a5;
  channel26GeometryNode->SetCarmRobotAngles(a);
  channel26GeometryNode->DisableModifiedEventOff();

  // Update Channel-26 transforms
  vtkSlicerChannel26Cabin3RobotsTransformLogic* channel26Logic = d->channel26RobotsLogic();
  if (channel26Logic && channel26GeometryNode)
  {
    using SysCoord = vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier;
    channel26Logic->UpdateTransformsHierarchy(channel26GeometryNode, SysCoord::CarmRobotWrist);
  }
  channel26GeometryNode->Modified();
  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onCarmRobotA6Changed(double a6)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter node is invalid!";
    return;
  }
  vtkMRMLChannel26GeometryNode* channel26GeometryNode = d->ParameterNode->GetChannel26GeometryNode();

  d->getLayoutManager()->pauseRender();
  double a[6] = {};
  channel26GeometryNode->GetCarmRobotAngles(a);
  channel26GeometryNode->DisableModifiedEventOn();
  a[5] = a6;
  channel26GeometryNode->SetCarmRobotAngles(a);
  channel26GeometryNode->DisableModifiedEventOff();

  // Update Channel-26 transforms
  vtkSlicerChannel26Cabin3RobotsTransformLogic* channel26Logic = d->channel26RobotsLogic();
  if (channel26Logic && channel26GeometryNode)
  {
    using SysCoord = vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier;
    channel26Logic->UpdateTransformsHierarchy(channel26GeometryNode, SysCoord::CarmRobotFlange);
  }
  channel26GeometryNode->Modified();
  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onShowMarkupsToggled(bool toggled)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter node is invalid!";
    return;
  }
  d->logic()->ShowMarkupsNodes(d->ParameterNode, toggled);
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onShowModelsToggled(bool toggled)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter node is invalid!";
    return;
  }
  d->logic()->ShowModelsNodes(d->ParameterNode, toggled);
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onRotatePatientHeadFeetToggled(bool toggled)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter node is invalid!";
    return;
  }
  vtkMRMLChannel26GeometryNode* channel26GeometryNode = d->ParameterNode->GetChannel26GeometryNode();
  d->getLayoutManager()->pauseRender();
  channel26GeometryNode->SetPatientHeadFeetRotation(toggled);
  d->getLayoutManager()->resumeRender();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onFixedReferenceCameraToggled(bool toggled)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  vtkMRMLCameraNode* cameraNode = d->get3DViewCameraNode();

  // Get FixedReference->RAS transform node
  vtkSlicerChannel26Cabin3RobotsTransformLogic* robotsLogic = d->logic()->GetChannel26RobotsTransformLogic();
  vtkMRMLLinearTransformNode* node = nullptr;
  if (robotsLogic)
  {
    node = robotsLogic->GetFixedReferenceTransform(); // FixedReference->RAS transform node
  }
  if (toggled)
  {
    vtkNew<vtkMatrix4x4> fixedReferenceToRasTransform;
    if (node)
    {
      node->GetMatrixTransformToParent(fixedReferenceToRasTransform);
    }
    cameraNode->SetAppliedTransform(fixedReferenceToRasTransform);
    cameraNode->SetAndObserveTransformNodeID(node ? node->GetID() : nullptr);
    return;
  }
  cameraNode->SetAndObserveTransformNodeID(nullptr);
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::bevOrientationChanged(const std::array< double, 3 >& viewUpVector)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  vtkMRMLCameraNode* cameraNode = d->get3DViewCameraNode();
  if (!cameraNode)
  {
    return;
  }

  vtkMRMLRTBeamNode* carmXrayBeamNode = vtkMRMLRTBeamNode::SafeDownCast(d->MRMLNodeComboBox_BevBeamNode->currentNode());

  double sourcePosition[4] = { 0.0, 0.0, 0.0, 1.0 };
  double isocenter[4] = { 0.0, 0.0, 0.0, 1.0 }; // isocenter in C-arm x-ray beam
  if (carmXrayBeamNode && carmXrayBeamNode->GetSourcePosition(sourcePosition))
  {
    vtkMRMLTransformNode* beamTransformNode = carmXrayBeamNode->GetParentTransformNode();

    vtkNew<vtkMatrix4x4> mat;
    mat->Identity();

    if (beamTransformNode)
    {
      beamTransformNode->GetMatrixTransformToWorld(mat);
    }
    else
    {
      qCritical() << Q_FUNC_INFO << "C-Arm x-ray beam node is invalid";
      return;
    }

    double vupBeam[4] = { viewUpVector[0], viewUpVector[1], viewUpVector[2], 0. };
    double vupCamera[4];

    mat->MultiplyPoint( vupBeam, vupCamera);
    cameraNode->GetCamera()->SetPosition(sourcePosition);
    double isocenterWorld[4] = {};
    mat->MultiplyPoint(isocenter, isocenterWorld);
    cameraNode->GetCamera()->SetFocalPoint(isocenterWorld);
    cameraNode->SetViewUp(vupCamera);
  }
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onBeamsEyeViewPlusXButtonClicked()
{
  Q_D(qSlicerPatientPositioningModuleWidget);
//  double viewUpVector[4] = { 1., 0., 0., 0. };
//  this->onBeamsEyeViewButtonClicked(viewUpVector);
  d->Label_BevOrientation->setText(tr("+X"));
  this->bevOrientationChanged(std::array< double, 3 >{ 1., 0., 0.});
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onBeamsEyeViewMinusXButtonClicked()
{
  Q_D(qSlicerPatientPositioningModuleWidget);
//  double viewUpVector[4] = { -1., 0., 0., 0. };
//  this->onBeamsEyeViewButtonClicked(viewUpVector);
  d->Label_BevOrientation->setText(tr("-X"));
  this->bevOrientationChanged(std::array< double, 3 >{ -1., 0., 0.});
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onBeamsEyeViewPlusYButtonClicked()
{
  Q_D(qSlicerPatientPositioningModuleWidget);
//  double viewUpVector[4] = { 0., 1., 0., 0. };
//  this->onBeamsEyeViewButtonClicked(viewUpVector);
  d->Label_BevOrientation->setText(tr("+Y"));
  this->bevOrientationChanged(std::array< double, 3 >{ 0., 1., 0.});
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onBeamsEyeViewMinusYButtonClicked()
{
  Q_D(qSlicerPatientPositioningModuleWidget);
//  double viewUpVector[4] = { 0., -1., 0., 0. };
//  this->onBeamsEyeViewButtonClicked(viewUpVector);
  d->Label_BevOrientation->setText(tr("-Y"));
  this->bevOrientationChanged(std::array< double, 3 >{ 0., -1., 0.});
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::checkForCollisions()
{
  Q_D(qSlicerPatientPositioningModuleWidget);
}
