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

// Slicer includes
#include "qSlicerPatientPositioningModuleWidget.h"
#include "ui_qSlicerPatientPositioningModuleWidget.h"

#include <qSlicerLayoutManager.h>
#include <qSlicerApplication.h>
#include <qMRMLSliceWidget.h>
#include <qSlicerSubjectHierarchyFolderPlugin.h>
#include <qSlicerSubjectHierarchyPluginHandler.h>

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

#include <vtkMRMLPatientPositioningNode.h>
#include <vtkMRMLChannel25GeometryNode.h>

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
#include <vtkSlicerTableTopRobotTransformLogic.h>

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
  vtkSlicerTableTopRobotTransformLogic* tableTopRobotLogic() const;

  /// IhepStandGeometry MRML node containing shown parameters
  vtkSmartPointer<vtkMRMLChannel25GeometryNode> Channel25GeometryNode;
  vtkSmartPointer<vtkMRMLPatientPositioningNode> ParameterNode;
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
vtkSlicerTableTopRobotTransformLogic* qSlicerPatientPositioningModuleWidgetPrivate::tableTopRobotLogic() const
{
  Q_Q(const qSlicerPatientPositioningModuleWidget);
  vtkSlicerPatientPositioningLogic* logic = vtkSlicerPatientPositioningLogic::SafeDownCast(q->logic());
  return (logic) ? logic->GetTableTopRobotTransformLogic() : nullptr;
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
  d->ComboBox_TreatmentMachine->addItem("Channel25Geometry", "Channel25Geometry");
  d->ComboBox_TreatmentMachine->addItem("From file...", "FromFile");

  // Nodes
  connect( d->MRMLNodeComboBox_ParameterSet, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onParameterNodeChanged(vtkMRMLNode*)));
  // Buttons
  connect( d->PushButton_LoadTreatmentMachine, SIGNAL(clicked()), 
    this, SLOT(onLoadTreatmentMachineButtonClicked()));
  connect( d->CheckBox_RotatePatientHeadFeet, SIGNAL(toggled(bool)), 
    this, SLOT(onRotatePatientHeadFeetToggled(bool)));

  // Widgets
  connect( d->SliderWidget_TableRobotA1, SIGNAL(valueChanged(double)), 
    this, SLOT(onTableTopRobotA1Changed(double)));
  connect( d->CoordinatesWidget_PatientTableTopTranslation, SIGNAL(coordinatesChanged(double*)),
    this, SLOT(onPatientTableTopTranslationChanged(double*)));
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

  // Set parameter node to children widgets
//  d->FooBar->setParameterNode(node);
 
  // Each time the node is modified, the UI widgets are updated
  qvtkReconnect( d->ParameterNode, parameterNode, vtkCommand::ModifiedEvent, 
    this, SLOT( updateWidgetFromMRML() ) );

  d->ParameterNode = parameterNode;

  // Set selected MRML nodes in comboboxes in the parameter set if it was nullptr there
  // (then in the meantime the comboboxes selected the first one from the scene and we have to set that)
  if (d->ParameterNode)
  {
  }
  this->updateWidgetFromMRML();
}

/*
//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onSetImagesToSliceViewClicked()
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  vtkMRMLPatientPositioningNode* parameterNode = vtkMRMLPatientPositioningNode::SafeDownCast(d->MRMLNodeComboBox_ParameterSet->currentNode());

  if (!parameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();

  qMRMLSliceWidget* sliceWidget = nullptr;
  vtkMRMLPatientPositioningNode::XrayProjectionType projType = vtkMRMLPatientPositioningNode::XrayProjectionType_Last;

  if (d->RadioButton_Vertical->isChecked())
  {
    qDebug() << Q_FUNC_INFO << ": Vertical";
//    layoutManager->setLayout(vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView);
    sliceWidget = layoutManager->sliceWidget("Red");
    projType = vtkMRMLPatientPositioningNode::Vertical;
  }
  else if (d->RadioButton_Horizontal->isChecked())
  {
    qDebug() << Q_FUNC_INFO << ": Horizontal";
//    layoutManager->setLayout(vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView);
    sliceWidget = layoutManager->sliceWidget("Yellow");
    projType = vtkMRMLPatientPositioningNode::Horizontal;
  }
  else if (d->RadioButton_Angle->isChecked())
  {
    qDebug() << Q_FUNC_INFO << ": Angle";
//    layoutManager->setLayout(vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView);
    sliceWidget = layoutManager->sliceWidget("Green");
    projType = vtkMRMLPatientPositioningNode::Angle;
  }
  if (sliceWidget)
  {
    vtkMRMLSliceNode* sliceNode = sliceWidget->mrmlSliceNode();
    vtkMRMLSliceCompositeNode* sliceCompositeNode = sliceWidget->mrmlSliceCompositeNode();
    d->logic()->SetXrayImagesProjection( parameterNode, projType, sliceCompositeNode, sliceNode);
    layoutManager->resetSliceViews();
    sliceNode->RotateToVolumePlane(parameterNode->GetXrayImageNode(projType));
  }
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onDrrNodeChanged(vtkMRMLNode* drrNode)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  vtkMRMLPatientPositioningNode* parameterNode = vtkMRMLPatientPositioningNode::SafeDownCast(d->MRMLNodeComboBox_ParameterSet->currentNode());

  if (!parameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  vtkMRMLPatientPositioningNode::XrayProjectionType projType = vtkMRMLPatientPositioningNode::XrayProjectionType_Last;

  if (d->RadioButton_Vertical->isChecked())
  {
    projType = vtkMRMLPatientPositioningNode::Vertical;
  }
  else if (d->RadioButton_Horizontal->isChecked())
  {
    projType = vtkMRMLPatientPositioningNode::Horizontal;
  }
  else if (d->RadioButton_Angle->isChecked())
  {
    projType = vtkMRMLPatientPositioningNode::Angle;
  }
  parameterNode->SetDrrNode(vtkMRMLScalarVolumeNode::SafeDownCast(drrNode), projType);
  parameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onXrayImageNodeChanged(vtkMRMLNode* xrayImageNode)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  vtkMRMLPatientPositioningNode* parameterNode = vtkMRMLPatientPositioningNode::SafeDownCast(d->MRMLNodeComboBox_ParameterSet->currentNode());

  if (!parameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  vtkMRMLPatientPositioningNode::XrayProjectionType projType = vtkMRMLPatientPositioningNode::XrayProjectionType_Last;

  if (d->RadioButton_Vertical->isChecked())
  {
    projType = vtkMRMLPatientPositioningNode::Vertical;
  }
  else if (d->RadioButton_Horizontal->isChecked())
  {
    projType = vtkMRMLPatientPositioningNode::Horizontal;
  }
  else if (d->RadioButton_Angle->isChecked())
  {
    projType = vtkMRMLPatientPositioningNode::Angle;
  }
  parameterNode->SetXrayImageNode(vtkMRMLScalarVolumeNode::SafeDownCast(xrayImageNode), projType);
  parameterNode->Modified();
}
*/
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

  vtkMRMLPatientPositioningNode* parameterNode = nullptr; 
  // Try to find one in the scene
  if (vtkMRMLNode* node = this->mrmlScene()->GetNthNodeByClass( 0, "vtkMRMLPatientPositioningNode"))
  {
    parameterNode = vtkMRMLPatientPositioningNode::SafeDownCast(node);
  }

  if (parameterNode)
  {
    // First thing first: update normal and vup vectors for parameter node
    // in case observed beam node transformation has been modified
//    d->logic()->UpdateNormalAndVupVectors(parameterNode);
  }

  // Create DRR markups nodes
//  d->logic()->CreateMarkupsNodes(parameterNode);

  // All required data for GUI is initiated
  this->updateWidgetFromMRML();
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

  vtkMRMLScene* scene = this->mrmlScene();
  if (!scene)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid scene";
    return;
  }

  vtkMRMLPatientPositioningNode* paramNode = vtkMRMLPatientPositioningNode::SafeDownCast(d->MRMLNodeComboBox_ParameterSet->currentNode());
  if (!paramNode)
  {
    return;
  }

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
    QString relativeFilePath = QString("%1/%2.json").arg(treatmentMachineType).arg(treatmentMachineType);
    descriptorFilePath = QDir(d->logic()->GetModuleShareDirectory().c_str()).filePath(relativeFilePath);
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
  if (!scene->GetFirstNodeByClass("vtkMRMLChannel25GeometryNode"))
  {
    d->Channel25GeometryNode = vtkSmartPointer<vtkMRMLChannel25GeometryNode>::New();
    d->Channel25GeometryNode->SetName("Channel25Geometry");
    d->Channel25GeometryNode->SetHideFromEditors(0);
    d->Channel25GeometryNode->SetSingletonTag("ChannelGeo");
    scene->AddNode(d->Channel25GeometryNode);
  }
  else
  {
    d->Channel25GeometryNode = vtkMRMLChannel25GeometryNode::SafeDownCast(
      scene->GetFirstNodeByClass("vtkMRMLChannel25GeometryNode"));
  }
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
  // Load and setup models
  if (d->Channel25GeometryNode)
  {
    d->Channel25GeometryNode->SetTableTopVerticalPosition(0.0);
    d->Channel25GeometryNode->SetTreatmentMachineDescriptorFilePath(descriptorFilePath.toUtf8().constData());
    std::vector<vtkSlicerTableTopRobotTransformLogic::CoordinateSystemIdentifier> loadedParts =
      d->logic()->LoadTreatmentMachine(d->Channel25GeometryNode);
    d->tableTopRobotLogic()->ResetRasToPatientIsocenterTranslate();
  }
/*
  // Warn the user if collision detection is disabled for certain part pairs
  QString disabledCollisionDetectionMessage(
    tr("Collision detection for the following part pairs may take very long due to high triangle numbers:\n\n"));
  bool collisionDetectionDisabled = false;
  if (d->logic()->GetGantryTableTopCollisionDetection()->GetInputData(0) == nullptr)
  {
    disabledCollisionDetectionMessage.append("Gantry-TableTop\n");
    collisionDetectionDisabled = true;
  }
  if (d->logic()->GetGantryPatientSupportCollisionDetection()->GetInputData(0) == nullptr)
  {
    disabledCollisionDetectionMessage.append("Gantry-PatientSupport\n");
    collisionDetectionDisabled = true;
  }
  if (d->logic()->GetCollimatorTableTopCollisionDetection()->GetInputData(0) == nullptr)
  {
    disabledCollisionDetectionMessage.append("Collimator-TableTop\n");
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
      d->logic()->SetupTreatmentMachineModels(paramNode, true);
    }
  }

  // Set treatment machine dependent properties  //TODO: Use degrees of freedom from JSON
  if (!treatmentMachineType.compare("VarianTrueBeamSTx"))
  {
    d->LateralTableTopDisplacementSlider->setMinimum(-230.0);
    d->LateralTableTopDisplacementSlider->setMaximum(230.0);
  }
  else if (!treatmentMachineType.compare("SiemensArtiste"))
  {
    d->LateralTableTopDisplacementSlider->setMinimum(-250.0);
    d->LateralTableTopDisplacementSlider->setMaximum(250.0);
  }

  // Reset camera
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  qMRMLThreeDView* threeDView = layoutManager->threeDWidget(0)->threeDView();
  threeDView->resetCamera();

  // Enable treatment machine geometry controls
  d->GantryRotationSlider->setEnabled(true);
  d->CollimatorRotationSlider->setEnabled(true);
  d->PatientSupportRotationSlider->setEnabled(true);
  d->VerticalTableTopDisplacementSlider->setEnabled(true);
  d->LongitudinalTableTopDisplacementSlider->setEnabled(true);
  d->LateralTableTopDisplacementSlider->setEnabled(true);
  d->ImagingPanelMovementSlider->setEnabled(true);

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
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onTableTopRobotA1Changed(double a1)
{
  Q_D(qSlicerPatientPositioningModuleWidget);
  int currentMachineIndex = d->ComboBox_TreatmentMachine->currentIndex();

  vtkMRMLScene* scene = this->mrmlScene();
  if (!scene)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid scene";
    return;
  }

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  if (d->Channel25GeometryNode)
  {
    d->Channel25GeometryNode->DisableModifiedEventOn();
    d->Channel25GeometryNode->SetPatientSupportRotationAngle(a1);
    d->Channel25GeometryNode->DisableModifiedEventOff();
  }

  // Update IEC transform
  vtkSlicerTableTopRobotTransformLogic* tableTopRobotLogic = d->tableTopRobotLogic();
//  tableTopRobotLogic->UpdateFixedReferenceToRASTransform(channelNode);
//  tableTopRobotLogic->ResetRasToPatientIsocenterTranslate();
  tableTopRobotLogic->UpdateBaseRotationToBaseFixedTransform(d->Channel25GeometryNode);
//  d->logic()->GetIECLogic()->UpdateFixedReferenceToRASTransform(d->currentPlanNode(paramNode));
  d->Channel25GeometryNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onTableTopRobotA2Changed(double a2)
{
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onTableTopRobotA3Changed(double a3)
{
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onTableTopRobotA4Changed(double a4)
{
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onTableTopRobotA5Changed(double a5)
{
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onTableTopRobotA6Changed(double a6)
{
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onTableTopRobotAnglesChanged(double* a)
{
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onPatientSupportRotationAngleChanged(double angle)
{
  Q_D(qSlicerPatientPositioningModuleWidget);
/*
  if (!this->mrmlScene())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid scene";
    return;
  }

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    return;
  }

  d->ParameterNode->DisableModifiedEventOn();
  d->ParameterNode->SetPatientSupportRotationAngle(angle);
  d->ParameterNode->DisableModifiedEventOff();

  // update table top stand to patient support rotation transform
  d->logic()->UpdatePatientSupportToFixedReferenceTransform(d->ParameterNode);

  d->ParameterNode->Modified();
*/
}


//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onPatientTableTopTranslationChanged(double* position)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid scene";
    return;
  }

  if (!d->Channel25GeometryNode)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter node is invalid!";
    return;
  }
  d->Channel25GeometryNode->DisableModifiedEventOn();
  d->Channel25GeometryNode->SetPatientToTableTopTranslation(position);
  d->Channel25GeometryNode->DisableModifiedEventOff();
  d->tableTopRobotLogic()->UpdatePatientToTableTopTransform(d->Channel25GeometryNode);
  d->Channel25GeometryNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onRotatePatientHeadFeetToggled(bool toggled)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  if (!d->Channel25GeometryNode)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter node is invalid!";
    return;
  }

  if (!d->Channel25GeometryNode)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter node is invalid!";
    return;
  }
  d->Channel25GeometryNode->SetPatientHeadFeetRotation(toggled);
}
