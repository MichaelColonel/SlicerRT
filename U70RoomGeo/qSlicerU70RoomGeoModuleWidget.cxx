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
#include <QFileDialog>
#include <QMessageBox>

// CTK includes
#include <ctkMessageBox.h>

// Slicer includes
#include "qSlicerU70RoomGeoModuleWidget.h"
#include "ui_qSlicerU70RoomGeoModuleWidget.h"

#include <qSlicerLayoutManager.h>
#include <qSlicerApplication.h>
#include <qSlicerSubjectHierarchyFolderPlugin.h>
#include <qSlicerSubjectHierarchyPluginHandler.h>

#include <qMRMLSliceWidget.h>
#include <qMRMLThreeDWidget.h>
#include <qMRMLThreeDView.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSubjectHierarchyNode.h>
#include <vtkMRMLCameraNode.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLLinearTransformNode.h>

// U70RoomGeo logic and nodes
#include <vtkMRMLU70RoomGeoNode.h>
#include <vtkMRMLChannel26GeometryNode.h>
#include <vtkSlicerU70RoomGeoLogic.h>

// VTK includes
#include <vtkCamera.h>
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>

//-----------------------------------------------------------------------------
/// \ingroup SlicerRt_QtModules_U70RoomGeo
class qSlicerU70RoomGeoModuleWidgetPrivate : public Ui_qSlicerU70RoomGeoModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerU70RoomGeoModuleWidget);
protected:
  qSlicerU70RoomGeoModuleWidget* const q_ptr;
public:
  qSlicerU70RoomGeoModuleWidgetPrivate(qSlicerU70RoomGeoModuleWidget& object);
  ~qSlicerU70RoomGeoModuleWidgetPrivate() = default;

  vtkSmartPointer<vtkSlicerU70RoomGeoLogic> logic() const;
  vtkSlicerChannel26Cabin3RobotsTransformLogic* channel26RobotsLogic() const;

  qMRMLThreeDView* get3DView() const;
  vtkMRMLCameraNode* get3DViewCameraNode() const;

  QString getTreatmentRoomGeometryFile() const;
  qMRMLLayoutManager* getLayoutManager() const;

  /// U70 treatment room geometry MRML node containing shown parameters
  vtkSmartPointer<vtkMRMLU70RoomGeoNode> ParameterNode;

  bool ModuleWindowInitialized{ false };
};

//-----------------------------------------------------------------------------
// qSlicerU70RoomGeoModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerU70RoomGeoModuleWidgetPrivate::qSlicerU70RoomGeoModuleWidgetPrivate(qSlicerU70RoomGeoModuleWidget& object)
  : q_ptr(&object)
  , ModuleWindowInitialized(false)
{
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkSlicerU70RoomGeoLogic> qSlicerU70RoomGeoModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerU70RoomGeoModuleWidget);
  return vtkSlicerU70RoomGeoLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
vtkSlicerChannel26Cabin3RobotsTransformLogic* qSlicerU70RoomGeoModuleWidgetPrivate::channel26RobotsLogic() const
{
  Q_Q(const qSlicerU70RoomGeoModuleWidget);
  vtkSlicerU70RoomGeoLogic* logic = vtkSlicerU70RoomGeoLogic::SafeDownCast(q->logic());
  return (logic) ? logic->GetChannel26RobotsTransformLogic() : nullptr;
}

QString qSlicerU70RoomGeoModuleWidgetPrivate::getTreatmentRoomGeometryFile() const
{
  QRadioButton* rButton = qobject_cast< QRadioButton* >(this->ButtonGroup_TreatmentRoomType->checkedButton());
  if (rButton && rButton == this->RadioButton_Cabin1)
  {
    return "Channel26Cabin1Geometry";
  }
  else if (rButton && rButton == this->RadioButton_Cabin2)
  {
    return "Channel26Cabin2Geometry";
  }
  else if (rButton && rButton == this->RadioButton_Cabin3)
  {
    return "Channel26Cabin3Geometry";
  }
  return QString();
}

//-----------------------------------------------------------------------------
qMRMLThreeDView* qSlicerU70RoomGeoModuleWidgetPrivate::get3DView() const
{
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  if (!layoutManager->threeDViewCount())
  {
    return nullptr;
  }

  // Some extensions (for example ones adding virtual reality or hologram
  // display support) introduce their own kind of 3D-like view; Slicer's standard 3D
  // view recognizes those as views of its own type as well, and quietly creates an
  // additional, normally hidden, view for them.
  // To make sure the correct view is the one selected, this first asks the layout
  // manager which 3D view is currently active, and if that is not available, falls
  // back to the first view that went through the normal layout setup - the hidden
  // views added by other extensions never go through that setup, so this reliably
  // picks the right one.
  vtkMRMLViewNode* activeViewNode = layoutManager->activeMRMLThreeDViewNode();
  if (activeViewNode)
  {
    qMRMLThreeDWidget* widget = layoutManager->threeDWidget(QString(activeViewNode->GetLayoutName()));
    if (widget)
    {
      return widget->threeDView();
    }
  }
  for (int i = 0; i < layoutManager->threeDViewCount(); ++i)
  {
    qMRMLThreeDWidget* widget = layoutManager->threeDWidget(i);
    if (widget && widget->threeDView()->mrmlViewNode() && widget->threeDView()->mrmlViewNode()->IsMappedInLayout())
    {
      return widget->threeDView();
    }
  }
  return nullptr;
}

//-----------------------------------------------------------------------------
vtkMRMLCameraNode* qSlicerU70RoomGeoModuleWidgetPrivate::get3DViewCameraNode() const
{
  qMRMLThreeDView* threeDView = this->get3DView();
  if (!threeDView)
  {
    return nullptr;
  }

  vtkMRMLCameraNode* cameraNode = threeDView->cameraNode();
  if (!cameraNode)
  {
    qCritical() << Q_FUNC_INFO << "Failed to find camera for view "
                << (threeDView->mrmlViewNode() ? threeDView->mrmlViewNode()->GetID() : "(null)");
  }
  return cameraNode;
}

//-----------------------------------------------------------------------------
qMRMLLayoutManager* qSlicerU70RoomGeoModuleWidgetPrivate::getLayoutManager() const
{
  // Get 3D view node
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  return slicerApplication->layoutManager();
}

//-----------------------------------------------------------------------------
// qSlicerU70RoomGeoModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerU70RoomGeoModuleWidget::qSlicerU70RoomGeoModuleWidget(QWidget* _parent)
: Superclass(_parent)
, d_ptr(new qSlicerU70RoomGeoModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerU70RoomGeoModuleWidget::~qSlicerU70RoomGeoModuleWidget() = default;

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::setup()
{
  Q_D(qSlicerU70RoomGeoModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  // buttons
  QObject::connect(d->PushButton_LoadModels, SIGNAL(clicked()), this, SLOT(onLoadTreatmentRoomButtonClicked()));
  QObject::connect(d->PushButton_UnloadModels, SIGNAL(clicked()), this, SLOT(onUnloadTreatmentRoomButtonClicked()));

  // Widgets
  // Table robot angles
  QObject::connect(d->SliderWidget_TableRobotA6, SIGNAL(valueChanged(double)), 
    this, SLOT(onTableRobotA6Changed(double)));
  QObject::connect(d->SliderWidget_TableRobotA5, SIGNAL(valueChanged(double)), 
    this, SLOT(onTableRobotA5Changed(double)));
  QObject::connect(d->SliderWidget_TableRobotA4, SIGNAL(valueChanged(double)), 
    this, SLOT(onTableRobotA4Changed(double)));
  QObject::connect(d->SliderWidget_TableRobotA3, SIGNAL(valueChanged(double)), 
    this, SLOT(onTableRobotA3Changed(double)));
  QObject::connect(d->SliderWidget_TableRobotA2, SIGNAL(valueChanged(double)), 
    this, SLOT(onTableRobotA2Changed(double)));
  QObject::connect(d->SliderWidget_TableRobotA1, SIGNAL(valueChanged(double)), 
    this, SLOT(onTableRobotA1Changed(double)));
  // C-arm robot angles
  QObject::connect(d->SliderWidget_CarmRobotA1, SIGNAL(valueChanged(double)), 
    this, SLOT(onCarmRobotA1Changed(double)));
  QObject::connect(d->SliderWidget_CarmRobotA2, SIGNAL(valueChanged(double)), 
    this, SLOT(onCarmRobotA2Changed(double)));
  QObject::connect(d->SliderWidget_CarmRobotA3, SIGNAL(valueChanged(double)), 
    this, SLOT(onCarmRobotA3Changed(double)));
  QObject::connect(d->SliderWidget_CarmRobotA4, SIGNAL(valueChanged(double)), 
    this, SLOT(onCarmRobotA4Changed(double)));
  QObject::connect(d->SliderWidget_CarmRobotA5, SIGNAL(valueChanged(double)), 
    this, SLOT(onCarmRobotA5Changed(double)));
  QObject::connect(d->SliderWidget_CarmRobotA6, SIGNAL(valueChanged(double)), 
    this, SLOT(onCarmRobotA6Changed(double)));

  QObject::connect(d->CoordinatesWidget_PatientTableTopTranslation, SIGNAL(coordinatesChanged(double*)),
    this, SLOT(onPatientTableTopTranslationChanged(double*)));


  // Models, markups, camera checkboxes
  QObject::connect(d->CheckBox_ShowModels, SIGNAL(toggled(bool)),
    this, SLOT(onShowModelsToggled(bool)));
  QObject::connect(d->CheckBox_ShowMarkups, SIGNAL(toggled(bool)),
    this, SLOT(onShowMarkupsToggled(bool)));
  QObject::connect(d->CheckBox_FixedReferenceCamera, SIGNAL(toggled(bool)),
    this, SLOT(onFixedReferenceCameraToggled(bool)));
 // Patient head/feet orientation rotation checkboxes
  QObject::connect(d->CheckBox_RotatePatientHeadFeet, SIGNAL(toggled(bool)), 
    this, SLOT(onRotatePatientHeadFeetToggled(bool)));
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::onLoadTreatmentRoomButtonClicked()
{
  Q_D(qSlicerU70RoomGeoModuleWidget);

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
  QString treatmentMachineType = d->getTreatmentRoomGeometryFile();

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
  if (!treatmentMachineType.isEmpty())
  {
    d->ParameterNode->SetTreatmentMachineType("Channel26Cabin3Geometry");
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
  // Set and observe created node in U70RoomGeo node
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
    QScopedPointer< ctkMessageBox > existingMachineMsgBox(new ctkMessageBox(this));
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
  d->PushButton_UnloadModels->setEnabled(true);
  d->PushButton_LoadModels->setEnabled(false);

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

    /// Setup Markups fixed beam axis and fixed isocenter
    vtkMRMLMarkupsLineNode* beamAxisLineNode = d->channel26RobotsLogic()->CreateBeamAxisLineNode(channel26GeometryNode);
    vtkMRMLMarkupsFiducialNode* fixedIsocenterNode = d->channel26RobotsLogic()->CreateIsocenterFiducialNode(channel26GeometryNode);
    Q_UNUSED(beamAxisLineNode);
    Q_UNUSED(fixedIsocenterNode);

    // Update channel-26 geometry node
    channel26GeometryNode->Modified();

    d->logic()->ShowMarkupsNodes(d->ParameterNode, false);
    d->logic()->ShowModelsNodes(d->ParameterNode, false);
    
    d->getLayoutManager()->resumeRender();
  }

  QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::onUnloadTreatmentRoomButtonClicked()
{
  Q_D(qSlicerU70RoomGeoModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid scene";
    return;
  }

  if (!d->ParameterNode)
  {
    return;
  }

  QString treatmentMachineType = d->getTreatmentRoomGeometryFile();
  QString relativeFilePath = QString("%1/%2.json").arg(treatmentMachineType).arg(treatmentMachineType);
  QString descriptorFilePath = QDir(d->logic()->GetModuleShareDirectory().c_str()).filePath(relativeFilePath);

  // Check if there is a machine already loaded and ask user what to do if so
  vtkMRMLSubjectHierarchyNode* shNode = this->mrmlScene()->GetSubjectHierarchyNode();
  std::vector<vtkIdType> allItemIDs;
  shNode->GetItemChildren(shNode->GetSceneItemID(), allItemIDs, true);
  std::vector<vtkIdType> machineFolderItemIDs;
  std::vector<vtkIdType>::iterator itemIt;
  for (itemIt=allItemIDs.begin(); itemIt!=allItemIDs.end(); ++itemIt)
  {
    std::string machineDescriptorFilePath = shNode->GetItemAttribute(*itemIt,
      vtkSlicerU70RoomGeoLogic::TREATMENT_MACHINE_DESCRIPTOR_FILE_PATH_ATTRIBUTE_NAME);
    if (!machineDescriptorFilePath.compare(descriptorFilePath.toUtf8().constData()))
    {
      machineFolderItemIDs.push_back(*itemIt);
    }
  }

  // Ask user what do to if a machine is already loaded
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
      d->channel26RobotsLogic()->RemoveAllMarkups();
      d->channel26RobotsLogic()->RemoveAllTransforms();
      // Disable treatment machine geometry controls
      d->PushButton_UnloadModels->setEnabled(false);
      d->PushButton_LoadModels->setEnabled(true);
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerU70RoomGeoModuleWidget);

  this->Superclass::setMRMLScene(scene);
  qvtkReconnect(d->logic(), scene, vtkMRMLScene::EndImportEvent, this, SLOT(onSceneImportedEvent()));
  qvtkReconnect(d->logic(), scene, vtkMRMLScene::EndCloseEvent, this, SLOT(onSceneClosedEvent()));

  // Find parameters node or create it if there is none in the scene
  if (scene)
  {
    vtkMRMLNode* node = scene->GetFirstNodeByClass("vtkMRMLChannel26GeometryNode");
    if (node)
    {
      this->setParameterNode(node);
    }
    else
    {
      vtkSmartPointer<vtkMRMLU70RoomGeoNode> newNode = vtkSmartPointer<vtkMRMLU70RoomGeoNode>::New();
      this->mrmlScene()->AddNode(newNode);
      this->setParameterNode(newNode);
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::setParameterNode(vtkMRMLNode* node)
{
  Q_D(qSlicerU70RoomGeoModuleWidget);

  vtkMRMLU70RoomGeoNode* parameterNode = vtkMRMLU70RoomGeoNode::SafeDownCast(node);

  // Each time the node is modified, the UI widgets are updated
  qvtkReconnect(d->ParameterNode, parameterNode, vtkCommand::ModifiedEvent,
    this, SLOT(updateWidgetFromMRML()));

  d->ParameterNode = parameterNode;

  // Set parameter node to children (FixedBeamAxis, CarmXrayBeamWidget) widgets
//  d->FixedBeamAxisWidget->setParameterNode(d->ParameterNode);
//  d->CarmXrayBeamWidget->setParameterNode(d->ParameterNode);

  // Set selected MRML nodes in comboboxes in the parameter set if it was nullptr there
  // (then in the meantime the comboboxes selected the first one from the scene and we have to set that)
  if (d->ParameterNode)
  {
  }
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::onSceneImportedEvent()
{
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::onSceneClosedEvent()
{
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerU70RoomGeoModuleWidget);

  if (d->ParameterNode && this->mrmlScene())
  {
    qDebug() << Q_FUNC_INFO << "Parameter node and scene are OK!";
  }
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::enter()
{
  Q_D(qSlicerU70RoomGeoModuleWidget);
  this->Superclass::enter();
  this->onEnter();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::onEnter()
{
  Q_D(qSlicerU70RoomGeoModuleWidget);

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
  vtkMRMLU70RoomGeoNode* parameterNode = nullptr; 
  // Try to find one in the scene
  if (vtkMRMLNode* node = this->mrmlScene()->GetFirstNodeByClass("vtkMRMLU70RoomGeoNode"))
  {
    parameterNode = vtkMRMLU70RoomGeoNode::SafeDownCast(node);
  }

  if (parameterNode)
  {
    this->setParameterNode(parameterNode);
  }

  // Set logics to childred widgets
//  d->CarmXrayBeamWidget->setMRMLScene(this->mrmlScene());
//  d->CarmXrayBeamWidget->setPatientPositioningLogic(d->logic());

  // All required data for GUI is initiated
  this->updateWidgetFromMRML();
  
  d->ModuleWindowInitialized = true;
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::onShowMarkupsToggled(bool toggled)
{
  Q_D(qSlicerU70RoomGeoModuleWidget);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter node is invalid!";
    return;
  }
  d->logic()->ShowMarkupsNodes(d->ParameterNode, toggled);
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::onShowModelsToggled(bool toggled)
{
  Q_D(qSlicerU70RoomGeoModuleWidget);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter node is invalid!";
    return;
  }
  d->logic()->ShowModelsNodes(d->ParameterNode, toggled);
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::onFixedReferenceCameraToggled(bool toggled)
{
  Q_D(qSlicerU70RoomGeoModuleWidget);

  vtkMRMLCameraNode* cameraNode = d->get3DViewCameraNode();

  // Get FixedReference->RAS transform node
  vtkSlicerChannel26Cabin3RobotsTransformLogic* robotsLogic = d->logic()->GetChannel26RobotsTransformLogic();
  vtkMRMLLinearTransformNode* fixedReferenceToRasTransformNode = nullptr;
  if (robotsLogic)
  {
    fixedReferenceToRasTransformNode = robotsLogic->GetFixedReferenceTransform();
  }

  vtkNew<vtkMatrix4x4> fixedReferenceToRasTransformMatrix;
  fixedReferenceToRasTransformMatrix->Identity();
  if (toggled && fixedReferenceToRasTransformNode)
  {
    // Get FixedReference -> RAS transform matrix
    fixedReferenceToRasTransformNode->GetMatrixTransformToParent(fixedReferenceToRasTransformMatrix);
    // Apply FixedReference -> RAS transform matrix to the camera node
    cameraNode->SetAppliedTransform(fixedReferenceToRasTransformMatrix);
    // Observe FixedReference -> RAS transform node by the camera node
    cameraNode->SetAndObserveTransformNodeID(fixedReferenceToRasTransformNode->GetID());
    return;
  }
  cameraNode->SetAndObserveTransformNodeID(nullptr);
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::onPatientTableTopTranslationChanged(double* position)
{
  Q_D(qSlicerU70RoomGeoModuleWidget);

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
//  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::onTableRobotA6Changed(double a6)
{
  Q_D(qSlicerU70RoomGeoModuleWidget);

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
  a[5] = 90. + a6;
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
//  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
//  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::onTableRobotA5Changed(double a5)
{
  Q_D(qSlicerU70RoomGeoModuleWidget);

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
//  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
//  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::onTableRobotA4Changed(double a4)
{
  Q_D(qSlicerU70RoomGeoModuleWidget);

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
//  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
//  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::onTableRobotA3Changed(double a3)
{
  Q_D(qSlicerU70RoomGeoModuleWidget);

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
  a[2] = a3 - 90.;
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
//  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
//  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::onTableRobotA2Changed(double a2)
{
  Q_D(qSlicerU70RoomGeoModuleWidget);

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
//  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
//  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::onTableRobotA1Changed(double a1)
{
  Q_D(qSlicerU70RoomGeoModuleWidget);

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
//  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
//  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::onCarmRobotA1Changed(double a1)
{
  Q_D(qSlicerU70RoomGeoModuleWidget);

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
//  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
//  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::onCarmRobotA2Changed(double a2)
{
  Q_D(qSlicerU70RoomGeoModuleWidget);

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
//  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
//  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::onCarmRobotA3Changed(double a3)
{
  Q_D(qSlicerU70RoomGeoModuleWidget);

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
//  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
//  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::onCarmRobotA4Changed(double a4)
{
  Q_D(qSlicerU70RoomGeoModuleWidget);

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
//  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
//  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::onCarmRobotA5Changed(double a5)
{
  Q_D(qSlicerU70RoomGeoModuleWidget);

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
//  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
//  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::onCarmRobotA6Changed(double a6)
{
  Q_D(qSlicerU70RoomGeoModuleWidget);

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
//  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
//  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::onRotatePatientHeadFeetToggled(bool toggled)
{
  Q_D(qSlicerU70RoomGeoModuleWidget);

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
