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
#include <QDir>

// SlicerQt includes
#include <qSlicerCoreApplication.h>
#include <qSlicerCoreIOManager.h>
#include <qSlicerIO.h>

#include "qSlicerPlmDrrModuleWidget.h"
#include "ui_qSlicerPlmDrrModuleWidget.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLSubjectHierarchyNode.h>

#include "vtkMRMLPlmDrrNode.h"

// RTBeam includes
#include <vtkMRMLRTBeamNode.h>
#include <vtkMRMLRTIonBeamNode.h>

// Logic includes
#include "vtkSlicerPlmDrrLogic.h"

// Plastimatch includes
#include <drr_options.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerPlmDrrModuleWidgetPrivate: public Ui_qSlicerPlmDrrModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerPlmDrrModuleWidget);
protected:
  qSlicerPlmDrrModuleWidget* const q_ptr;
public:
  qSlicerPlmDrrModuleWidgetPrivate(qSlicerPlmDrrModuleWidget& object);
  virtual ~qSlicerPlmDrrModuleWidgetPrivate();
  vtkSlicerPlmDrrLogic* logic() const;

public:
  vtkMRMLRTBeamNode* RtBeamNode;
  vtkMRMLScalarVolumeNode* ReferenceVolumeNode;
  Drr_options DrrOptions;
  bool ModuleWindowInitialized;
};

//-----------------------------------------------------------------------------
// qSlicerLoadableModuleTemplateModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerPlmDrrModuleWidgetPrivate::qSlicerPlmDrrModuleWidgetPrivate(qSlicerPlmDrrModuleWidget &object)
  :
  q_ptr(&object),
  RtBeamNode(nullptr),
  ReferenceVolumeNode(nullptr),
  ModuleWindowInitialized(false)
{
}

//-----------------------------------------------------------------------------
qSlicerPlmDrrModuleWidgetPrivate::~qSlicerPlmDrrModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
vtkSlicerPlmDrrLogic*
qSlicerPlmDrrModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerPlmDrrModuleWidget);
  return vtkSlicerPlmDrrLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
// qSlicerPlmDrrModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerPlmDrrModuleWidget::qSlicerPlmDrrModuleWidget(QWidget* _parent)
  :
  Superclass(_parent),
  d_ptr(new qSlicerPlmDrrModuleWidgetPrivate(*this))
{
  // Q_D(qSlicerPlmDrrModuleWidget);
}

//-----------------------------------------------------------------------------
qSlicerPlmDrrModuleWidget::~qSlicerPlmDrrModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerPlmDrrModuleWidget::setup()
{
  Q_D(qSlicerPlmDrrModuleWidget);

  d->setupUi(this);

  this->Superclass::setup();

  QStringList plmDrrNodes;
  plmDrrNodes.push_back("vtkMRMLPlmDrrNode");
  d->MRMLNodeComboBox_ParameterNode->setNodeTypes(plmDrrNodes);

  QStringList rtBeamNodes;
  rtBeamNodes.push_back("vtkMRMLRTBeamNode");
  d->MRMLNodeComboBox_RtBeam->setNodeTypes(rtBeamNodes);

  QStringList volumeNodes;
  volumeNodes.push_back("vtkMRMLScalarVolumeNode");
  d->MRMLNodeComboBox_ReferenceVolume->setNodeTypes(volumeNodes);

  // Nodes
  connect( d->MRMLNodeComboBox_RtBeam, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onRTBeamNodeChanged(vtkMRMLNode*)));
  connect( d->MRMLNodeComboBox_ReferenceVolume, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onReferenceVolumeNodeChanged(vtkMRMLNode*)));

  // Sliders
  connect( d->SliderWidget_IsocenterImagerDistance, SIGNAL(valueChanged(double)), 
    this, SLOT(onIsocenterImagerDistanceValueChanged(double)));

  // Coordinates widgets
  connect( d->CoordinatesWidget_ImagerCenterOffset, SIGNAL(coordinatesChanged(double*)), 
    this, SLOT(onImagerCenterOffsetCoordinatesChanged(double*)));
  connect( d->CoordinatesWidget_ImagePixelDimention, SIGNAL(coordinatesChanged(double*)), 
    this, SLOT(onImageDimentionChanged(double*)));
  connect( d->CoordinatesWidget_ImagePixelSpacing, SIGNAL(coordinatesChanged(double*)), 
    this, SLOT(onImageSpacingChanged(double*)));
  connect( d->CoordinatesWidget_ImageWindow, SIGNAL(coordinatesChanged(double*)), 
    this, SLOT(onImageWindowCoordinatesChanged(double*)));

  // Buttons
  connect( d->PushButton_SaveVolume, SIGNAL(clicked()), this, SLOT(onSaveVolumeClicked()));
  connect( d->PushButton_ComputeDrr, SIGNAL(clicked()), this, SLOT(onComputeDrrClicked()));
  connect( d->PushButton_LoadDrr, SIGNAL(clicked()), this, SLOT(onLoadDrrClicked()));

  // Handle scene change event if occurs
  qvtkConnect( d->logic(), vtkCommand::ModifiedEvent, this, SLOT(onLogicModified()));
}

//-----------------------------------------------------------------------------
void qSlicerPlmDrrModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerPlmDrrModuleWidget);
  this->Superclass::setMRMLScene(scene);

  qvtkReconnect( d->logic(), scene, vtkMRMLScene::EndImportEvent, this, SLOT(onSceneImportedEvent()));
  qvtkReconnect( d->logic(), scene, vtkMRMLScene::EndCloseEvent, this, SLOT(onSceneClosedEvent()));

  // Find parameters node or create it if there is none in the scene
  if (scene)
  {
    vtkMRMLNode* node = scene->GetNthNodeByClass( 0, "vtkMRMLPlmDrrNode");
    if (d->MRMLNodeComboBox_ParameterNode->currentNode())
    {
      this->setParameterNode(d->MRMLNodeComboBox_ParameterNode->currentNode());
    }
    else if (node)
    {
      this->setParameterNode(node);
    }
    else
    {
      auto newNode = vtkSmartPointer<vtkMRMLPlmDrrNode>::New();
      this->mrmlScene()->AddNode(newNode);
      this->setParameterNode(newNode);
//      d->logic()->CreateDefaultMarkupsNodes(newNode);
    }
  }
}

/// RTBeam Node (RTBeam or RTIonBeam) changed
void qSlicerPlmDrrModuleWidget::onRTBeamNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerPlmDrrModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid scene";
    return;
  }

  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(this->mrmlScene());
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy";
    return;
  }

  vtkMRMLRTBeamNode* beamNode = vtkMRMLRTBeamNode::SafeDownCast(node);
  vtkMRMLRTIonBeamNode* ionBeamNode = vtkMRMLRTIonBeamNode::SafeDownCast(node);
  d->RtBeamNode = beamNode;
  Q_UNUSED(ionBeamNode);

  vtkMRMLPlmDrrNode* paramNode = vtkMRMLPlmDrrNode::SafeDownCast(d->MRMLNodeComboBox_ParameterNode->currentNode());
  if (!paramNode || !d->ModuleWindowInitialized)
  {
    return;
  }

  qDebug() << Q_FUNC_INFO << ": Beam node is set to parameter node";

  paramNode->DisableModifiedEventOn();
  paramNode->SetAndObserveBeamNode(beamNode);
  paramNode->DisableModifiedEventOff();

  d->logic()->UpdateMarkupsNodes(paramNode);
/*
  // Trigger update of transforms based on selected beam
  beamNode->InvokeCustomModifiedEvent(vtkMRMLRTBeamNode::BeamTransformModified);

  // Show only selected beam, hide others
  std::vector<vtkMRMLNode*> beamNodes;
  this->mrmlScene()->GetNodesByClass("vtkMRMLRTBeamNode", beamNodes);
  for (std::vector<vtkMRMLNode*>::iterator beamIt=beamNodes.begin(); beamIt!=beamNodes.end(); ++beamIt)
  {
    vtkMRMLRTBeamNode* currentBeamNode = vtkMRMLRTBeamNode::SafeDownCast(*beamIt);
    shNode->SetDisplayVisibilityForBranch(
      shNode->GetItemByDataNode(currentBeamNode), (currentBeamNode==beamNode ? 1 : 0) );
  }

  // Select patient segmentation
  vtkMRMLRTPlanNode* planNode = beamNode->GetParentPlanNode();
  if (!planNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access parent plan of beam " << beamNode->GetName();
    return;
  }
  d->SegmentSelectorWidget_PatientBody->setCurrentNode(planNode->GetSegmentationNode());
*/
}

//-----------------------------------------------------------------------------
void qSlicerPlmDrrModuleWidget::onReferenceVolumeNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerPlmDrrModuleWidget);
  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);
  d->ReferenceVolumeNode = volumeNode;
}

//-----------------------------------------------------------------------------
void qSlicerPlmDrrModuleWidget::onSaveVolumeClicked()
{
  Q_D(qSlicerPlmDrrModuleWidget);

  if (d->ReferenceVolumeNode && d->ReferenceVolumeNode->AddDefaultStorageNode())
  {
    qDebug() << Q_FUNC_INFO << "Reference volume node OK!";

    qSlicerCoreIOManager* coreIOManager = qSlicerCoreApplication::application()->coreIOManager();
    qSlicerIO::IOProperties fileParameters;
    QDir dir(qSlicerCoreApplication::application()->temporaryPath());
    QString drrVolumeFileName = dir.absoluteFilePath("inputDrrVolume.mha");

    fileParameters["nodeID"] = d->ReferenceVolumeNode->GetID();
    fileParameters["fileName"] = drrVolumeFileName;
    if (coreIOManager->saveNodes( "VolumeFile", fileParameters))
    {
      d->DrrOptions.input_file = drrVolumeFileName.toStdString();
      qDebug() << Q_FUNC_INFO << "Reference volume node written into temporary mha file";
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerPlmDrrModuleWidget::onComputeDrrClicked()
{
  Q_D(qSlicerPlmDrrModuleWidget);
}

//-----------------------------------------------------------------------------
void qSlicerPlmDrrModuleWidget::onLoadDrrClicked()
{
  Q_D(qSlicerPlmDrrModuleWidget);
}

//-----------------------------------------------------------------------------
void qSlicerPlmDrrModuleWidget::onLogicModified()
{
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerPlmDrrModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerPlmDrrModuleWidget);

  vtkMRMLPlmDrrNode* paramNode = vtkMRMLPlmDrrNode::SafeDownCast(d->MRMLNodeComboBox_ParameterNode->currentNode());

  if (paramNode && this->mrmlScene())
  {
    if (paramNode->GetBeamNode())
    {
      qDebug() << Q_FUNC_INFO << "Set beam node into GUI";
      d->MRMLNodeComboBox_RtBeam->setCurrentNode(paramNode->GetBeamNode());
      // apply beam transform to detector closed curve markups
    }
/*    
    if (paramNode->GetPatientBodySegmentationNode())
    {
      d->SegmentSelectorWidget_PatientBody->setCurrentNode(paramNode->GetPatientBodySegmentationNode());
    }
    if (paramNode->GetPatientBodySegmentID())
    {
      d->SegmentSelectorWidget_PatientBody->setCurrentSegmentID(paramNode->GetPatientBodySegmentID());
    }

    d->GantryRotationSlider->setValue(paramNode->GetGantryRotationAngle());
    d->CollimatorRotationSlider->setValue(paramNode->GetCollimatorRotationAngle());
    d->PatientSupportRotationSlider->setValue(paramNode->GetPatientSupportRotationAngle());
    d->ImagingPanelMovementSlider->setValue(paramNode->GetImagingPanelMovement());
    d->VerticalTableTopDisplacementSlider->setValue(paramNode->GetVerticalTableTopDisplacement());
    d->LateralTableTopDisplacementSlider->setValue(paramNode->GetLateralTableTopDisplacement());
    d->LongitudinalTableTopDisplacementSlider->setValue(paramNode->GetLongitudinalTableTopDisplacement());
    d->VerticalTranslationSliderWidget->setValue(paramNode->GetAdditionalModelVerticalDisplacement());
    d->LongitudinalTranslationSliderWidget->setValue(paramNode->GetAdditionalModelLongitudinalDisplacement());
    d->LateralTranslationSliderWidget->setValue(paramNode->GetAdditionalModelLateralDisplacement());
    d->ApplicatorHolderCheckBox->setChecked(paramNode->GetApplicatorHolderVisibility());
    d->ElectronApplicatorCheckBox->setChecked(paramNode->GetElectronApplicatorVisibility());
*/  
  }
}

//-----------------------------------------------------------------------------
void qSlicerPlmDrrModuleWidget::onSceneImportedEvent()
{
  this->onEnter();
}

//-----------------------------------------------------------------------------
void qSlicerPlmDrrModuleWidget::onSceneClosedEvent()
{
  this->onEnter();
}

//-----------------------------------------------------------------------------
void qSlicerPlmDrrModuleWidget::enter()
{
  this->Superclass::enter();
  this->onEnter();
}

//-----------------------------------------------------------------------------
void qSlicerPlmDrrModuleWidget::onEnter()
{
  Q_D(qSlicerPlmDrrModuleWidget);

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
  this->setMRMLScene(this->mrmlScene());


  vtkMRMLPlmDrrNode* paramNode = vtkMRMLPlmDrrNode::SafeDownCast(d->MRMLNodeComboBox_ParameterNode->currentNode());
  if (!paramNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  d->logic()->CreateMarkupsNodes(paramNode);

  d->ModuleWindowInitialized = true;
}

//-----------------------------------------------------------------------------
void qSlicerPlmDrrModuleWidget::setParameterNode(vtkMRMLNode *node)
{
  Q_D(qSlicerPlmDrrModuleWidget);

  vtkMRMLPlmDrrNode* paramNode = vtkMRMLPlmDrrNode::SafeDownCast(node);

  // Make sure the parameter set node is selected (in case the function was not called by the selector combobox signal)
  d->MRMLNodeComboBox_ParameterNode->setCurrentNode(paramNode);

  // Each time the node is modified, the UI widgets are updated
  qvtkReconnect( paramNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));
  
  // Set selected MRML nodes in comboboxes in the parameter set if it was nullptr there
  // (then in the meantime the comboboxes selected the first one from the scene and we have to set that)
  if (paramNode)
  {
    if (!paramNode->GetBeamNode())
    {
      vtkMRMLRTBeamNode* beamNode = vtkMRMLRTBeamNode::SafeDownCast(d->MRMLNodeComboBox_RtBeam->currentNode());
      qvtkConnect( beamNode, vtkMRMLRTBeamNode::BeamGeometryModified, this, SLOT(onUpdateImageWindowFromBeamJaws()));
      paramNode->SetAndObserveBeamNode(beamNode);
      paramNode->Modified();
    }
/*    if (!paramNode->GetPatientBodySegmentationNode())
    {
      paramNode->SetAndObservePatientBodySegmentationNode(vtkMRMLSegmentationNode::SafeDownCast(d->SegmentSelectorWidget_PatientBody->currentNode()));
    }
    if (!paramNode->GetPatientBodySegmentID() && !d->SegmentSelectorWidget_PatientBody->currentSegmentID().isEmpty())
    {
      paramNode->SetPatientBodySegmentID(d->SegmentSelectorWidget_PatientBody->currentSegmentID().toUtf8().constData());
    }

    paramNode->SetApplicatorHolderVisibility(0);
    paramNode->SetElectronApplicatorVisibility(0);
*/
  }

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerPlmDrrModuleWidget::onIsocenterImagerDistanceValueChanged(double value)
{
  Q_D(qSlicerPlmDrrModuleWidget);

  vtkMRMLPlmDrrNode* paramNode = vtkMRMLPlmDrrNode::SafeDownCast(d->MRMLNodeComboBox_ParameterNode->currentNode());
  if (!paramNode || !d->ModuleWindowInitialized)
  {
    return;
  }

  paramNode->DisableModifiedEventOn();
  paramNode->SetIsocenterImagerDistance(value);
  paramNode->DisableModifiedEventOff();
  
  // Update imager and image markups
  d->logic()->UpdateMarkupsNodes(paramNode);
//  d->logic()->UpdateIsocenterDetectorDistance(paramNode);
}

//-----------------------------------------------------------------------------
void qSlicerPlmDrrModuleWidget::onImagerCenterOffsetCoordinatesChanged(double* detectorOffset)
{
  Q_D(qSlicerPlmDrrModuleWidget);

  vtkMRMLPlmDrrNode* paramNode = vtkMRMLPlmDrrNode::SafeDownCast(d->MRMLNodeComboBox_ParameterNode->currentNode());
  if (!paramNode || !d->ModuleWindowInitialized)
  {
    return;
  }

  double offset[2] = { detectorOffset[0], detectorOffset[1] };
  paramNode->DisableModifiedEventOn();
  paramNode->SetImagerCenterOffset(offset);
  paramNode->DisableModifiedEventOff();

  // Update imager and image markups
  d->logic()->UpdateMarkupsNodes(paramNode);
}
//-----------------------------------------------------------------------------
void qSlicerPlmDrrModuleWidget::onImageSpacingChanged(double* spacing)
{
  Q_D(qSlicerPlmDrrModuleWidget);

  vtkMRMLPlmDrrNode* paramNode = vtkMRMLPlmDrrNode::SafeDownCast(d->MRMLNodeComboBox_ParameterNode->currentNode());
  if (!paramNode || !d->ModuleWindowInitialized)
  {
    return;
  }

  double s[2] = { spacing[0], spacing[1] };
  paramNode->DisableModifiedEventOn();
  paramNode->SetImageSpacing(s);
  paramNode->DisableModifiedEventOff();

  // Update imager and image markups
  d->logic()->UpdateMarkupsNodes(paramNode);
//  d->logic()->UpdateImageSpacing(paramNode);
}

//-----------------------------------------------------------------------------
void qSlicerPlmDrrModuleWidget::onImageDimentionChanged(double* dimention)
{
  Q_D(qSlicerPlmDrrModuleWidget);

  vtkMRMLPlmDrrNode* paramNode = vtkMRMLPlmDrrNode::SafeDownCast(d->MRMLNodeComboBox_ParameterNode->currentNode());
  if (!paramNode || !d->ModuleWindowInitialized)
  {
    return;
  }

  int dim[2] = { static_cast<int>(dimention[0]), static_cast<int>(dimention[1]) }; // x, y
  paramNode->DisableModifiedEventOn();
  paramNode->SetImageDimention(dim);
  paramNode->DisableModifiedEventOff();
  
  // Update imager and image markups
  d->logic()->UpdateMarkupsNodes(paramNode);
//  d->logic()->UpdateImageDimention(paramNode);
}

//-----------------------------------------------------------------------------
void qSlicerPlmDrrModuleWidget::onImageWindowCoordinatesChanged(double* window)
{
  Q_D(qSlicerPlmDrrModuleWidget);

  vtkMRMLPlmDrrNode* paramNode = vtkMRMLPlmDrrNode::SafeDownCast(d->MRMLNodeComboBox_ParameterNode->currentNode());
  if (!paramNode || !d->ModuleWindowInitialized)
  {
    return;
  }

  int imageWindow[4] = { 
    static_cast<int>(window[0]), // c1 = x1
    static_cast<int>(window[1]), // r1 = y1
    static_cast<int>(window[2]), // c2 = x2
    static_cast<int>(window[3]) }; // r2 = y2

  paramNode->DisableModifiedEventOn();
  paramNode->SetImageWindow(imageWindow);
  paramNode->DisableModifiedEventOff();
  
  // Update imager and image markups
  d->logic()->UpdateMarkupsNodes(paramNode);
}

//-----------------------------------------------------------------------------
void qSlicerPlmDrrModuleWidget::onUpdateImageWindowFromBeamJaws()
{
  Q_D(qSlicerPlmDrrModuleWidget);

  qDebug() << Q_FUNC_INFO << "update image window from beam";
}
