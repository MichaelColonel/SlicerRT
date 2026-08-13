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

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSubjectHierarchyNode.h>

// U70RoomGeo logic and nodes
#include <vtkMRMLU70RoomGeoNode.h>
#include <vtkMRMLChannel26GeometryNode.h>
#include <vtkSlicerU70RoomGeoLogic.h>

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
qMRMLLayoutManager* qSlicerU70RoomGeoModuleWidgetPrivate::getLayoutManager() const
{
  Q_Q(const qSlicerU70RoomGeoModuleWidget);

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

    d->getLayoutManager()->resumeRender();
  }
  // Update channel-26 geometry node
  channel26GeometryNode->Modified();

  QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::onUnloadTreatmentRoomButtonClicked()
{
  Q_D(qSlicerU70RoomGeoModuleWidget);
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
  qvtkReconnect( d->ParameterNode, parameterNode, vtkCommand::ModifiedEvent, 
    this, SLOT( updateWidgetFromMRML() ) );

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
