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

  QStringList rtBeamNodes;
  rtBeamNodes.push_back("vtkMRMLRTBeamNode");
  d->MRMLNodeComboBox_RtBeam->setNodeTypes(rtBeamNodes);

  connect( d->MRMLNodeComboBox_RtBeam, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onRTBeamNodeChanged(vtkMRMLNode*)));

  QStringList volumeNodes;
  volumeNodes.push_back("vtkMRMLScalarVolumeNode");
  d->MRMLNodeComboBox_ReferenceVolume->setNodeTypes(volumeNodes);

  connect( d->MRMLNodeComboBox_RtBeam, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onRTBeamNodeChanged(vtkMRMLNode*)));
  connect( d->MRMLNodeComboBox_ReferenceVolume, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onReferenceVolumeNodeChanged(vtkMRMLNode*)));
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
    if (d->MRMLNodeComboBox_ParameterSet->currentNode())
    {
      this->setParameterNode(d->MRMLNodeComboBox_ParameterSet->currentNode());
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
    }
  }
}

/// RTBeam Node (RTBeam or RTIonBeam) changed
void qSlicerPlmDrrModuleWidget::onRTBeamNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerPlmDrrModuleWidget);
  vtkMRMLRTBeamNode* beamNode = vtkMRMLRTBeamNode::SafeDownCast(node);
  vtkMRMLRTIonBeamNode* ionBeamNode = vtkMRMLRTIonBeamNode::SafeDownCast(node);
  d->RtBeamNode = beamNode;
  Q_UNUSED(ionBeamNode);
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
/*
  vtkMRMLRoomsEyeViewNode* paramNode = vtkMRMLRoomsEyeViewNode::SafeDownCast(d->MRMLNodeComboBox_ParameterSet->currentNode());

  if (paramNode && this->mrmlScene())
  {
    if (paramNode->GetBeamNode())
    {
      d->MRMLNodeComboBox_Beam->setCurrentNode(paramNode->GetBeamNode());
    }
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
  }
*/
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
  this->onEnter();
  this->Superclass::enter();
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

  d->ModuleWindowInitialized = true;
}

//-----------------------------------------------------------------------------
void qSlicerPlmDrrModuleWidget::setParameterNode(vtkMRMLNode *node)
{
  Q_D(qSlicerPlmDrrModuleWidget);

  vtkMRMLPlmDrrNode* paramNode = vtkMRMLPlmDrrNode::SafeDownCast(node);

  // Make sure the parameter set node is selected (in case the function was not called by the selector combobox signal)
  d->MRMLNodeComboBox_ParameterSet->setCurrentNode(paramNode);

  // Each time the node is modified, the UI widgets are updated
  qvtkReconnect( paramNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));
  
  // Set selected MRML nodes in comboboxes in the parameter set if it was nullptr there
  // (then in the meantime the comboboxes selected the first one from the scene and we have to set that)
  if (paramNode)
  {
    if (!paramNode->GetBeamNode())
    {
      paramNode->SetAndObserveBeamNode(vtkMRMLRTBeamNode::SafeDownCast(d->MRMLNodeComboBox_Beam->currentNode()));
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
void qSlicerPlmDrrModuleWidget::onIsocenterToImageSliderValueChanged(double value)
{
  Q_D(qSlicerPlmDrrModuleWidget);

  vtkMRMLPlmDrrNode* paramNode = vtkMRMLPlmDrrNode::SafeDownCast(d->MRMLNodeComboBox_ParameterSet->currentNode());
  if (!paramNode || !d->ModuleWindowInitialized)
  {
    return;
  }

  paramNode->DisableModifiedEventOn();
  paramNode->SetIsocenterToDetectorDistance(value);
  paramNode->DisableModifiedEventOff();
  
  // Update IEC transform
  d->logic()->UpdateIsocenterToDetectorDistance(paramNode);
}
