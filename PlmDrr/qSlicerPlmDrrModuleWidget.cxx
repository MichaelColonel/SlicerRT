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
#include <vtkMRMLScalarVolumeNode.h>

// RTBeam includes
#include <vtkMRMLRTBeamNode.h>
#include <vtkMRMLRTIonBeamNode.h>

// Plastimatch includes
#include <drr_options.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerPlmDrrModuleWidgetPrivate: public Ui_qSlicerPlmDrrModuleWidget
{
public:
  qSlicerPlmDrrModuleWidgetPrivate();
  vtkMRMLRTBeamNode* RtBeam;
  vtkMRMLScalarVolumeNode* ReferenceVolume;
  Drr_options DrrOptions;
};

//-----------------------------------------------------------------------------
// qSlicerPlmDrrModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerPlmDrrModuleWidgetPrivate::qSlicerPlmDrrModuleWidgetPrivate()
  :
  RtBeam(nullptr),
  ReferenceVolume(nullptr)
{
}

//-----------------------------------------------------------------------------
// qSlicerPlmDrrModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerPlmDrrModuleWidget::qSlicerPlmDrrModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerPlmDrrModuleWidgetPrivate )
{
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

/// RTBeam Node (RTBeam or RTIonBeam) changed
void qSlicerPlmDrrModuleWidget::onRTBeamNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerPlmDrrModuleWidget);
  vtkMRMLRTBeamNode* beamNode = vtkMRMLRTBeamNode::SafeDownCast(node);
  vtkMRMLRTIonBeamNode* ionBeamNode = vtkMRMLRTIonBeamNode::SafeDownCast(node);
  d->RtBeam = beamNode;
  Q_UNUSED(ionBeamNode);
}

//-----------------------------------------------------------------------------
void qSlicerPlmDrrModuleWidget::onReferenceVolumeNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerPlmDrrModuleWidget);
  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);
  d->ReferenceVolume = volumeNode;
}

//-----------------------------------------------------------------------------
void qSlicerPlmDrrModuleWidget::onSaveVolumeClicked()
{
  Q_D(qSlicerPlmDrrModuleWidget);

  if (d->ReferenceVolume && d->ReferenceVolume->AddDefaultStorageNode())
  {
    qDebug() << Q_FUNC_INFO << "Reference volume node OK!";

    qSlicerCoreIOManager* coreIOManager = qSlicerCoreApplication::application()->coreIOManager();
    qSlicerIO::IOProperties fileParameters;
    QDir dir(qSlicerCoreApplication::application()->temporaryPath());
    QString drrVolumeFileName = dir.absoluteFilePath("inputDrrVolume.mha");

    fileParameters["nodeID"] = d->ReferenceVolume->GetID();
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
  Q_D(qSlicerRoomsEyeViewModuleWidget);
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
