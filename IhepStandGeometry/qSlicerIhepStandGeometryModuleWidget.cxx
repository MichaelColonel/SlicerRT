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
#include "qSlicerIhepStandGeometryModuleWidget.h"
#include "ui_qSlicerIhepStandGeometryModuleWidget.h"

// Logic includes
#include "vtkSlicerIhepStandGeometryLogic.h"

// SlicerRT MRML IhepStandGeometry includes
#include "vtkMRMLIhepStandGeometryNode.h"

// Beams includes
//#include "vtkSlicerIECTransformLogic.h"
#include <vtkMRMLRTBeamNode.h>
#include <vtkMRMLRTIonBeamNode.h>
//#include "vtkMRMLRTPlanNode.h"

// Slicer includes
#include <qSlicerApplication.h>
#include <qSlicerLayoutManager.h>
#include <qSlicerIOManager.h>
#include <qSlicerDataDialog.h>
#include <qSlicerSaveDataDialog.h>
#include <qMRMLSliceWidget.h>
#include <qMRMLThreeDWidget.h>
#include <qMRMLThreeDView.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLSegmentationNode.h>
#include <vtkMRMLCameraNode.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLSubjectHierarchyNode.h>

// VTK includes
#include <vtkCamera.h>
#include <vtkPolyData.h>
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_IhepStandGeometry
class qSlicerIhepStandGeometryModuleWidgetPrivate: public Ui_qSlicerIhepStandGeometryModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerIhepStandGeometryModuleWidget);
protected:
  qSlicerIhepStandGeometryModuleWidget* const q_ptr;
public:
  qSlicerIhepStandGeometryModuleWidgetPrivate(qSlicerIhepStandGeometryModuleWidget &object);
  virtual ~qSlicerIhepStandGeometryModuleWidgetPrivate();
  vtkSlicerIhepStandGeometryLogic* logic() const;

  bool ModuleWindowInitialized;
};

//-----------------------------------------------------------------------------
// qSlicerIhepStandGeometryModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerIhepStandGeometryModuleWidgetPrivate::qSlicerIhepStandGeometryModuleWidgetPrivate(qSlicerIhepStandGeometryModuleWidget &object)
  :
  q_ptr(&object),
  ModuleWindowInitialized(false)
{
}

//-----------------------------------------------------------------------------
qSlicerIhepStandGeometryModuleWidgetPrivate::~qSlicerIhepStandGeometryModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
vtkSlicerIhepStandGeometryLogic* qSlicerIhepStandGeometryModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerIhepStandGeometryModuleWidget);
  return vtkSlicerIhepStandGeometryLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
// qSlicerIhepStandGeometryModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerIhepStandGeometryModuleWidget::qSlicerIhepStandGeometryModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerIhepStandGeometryModuleWidgetPrivate(*this) )
{
}

//-----------------------------------------------------------------------------
qSlicerIhepStandGeometryModuleWidget::~qSlicerIhepStandGeometryModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::setup()
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();


  // Nodes
  connect( d->MRMLNodeComboBox_RtBeam, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onRTBeamNodeChanged(vtkMRMLNode*)));
  connect( d->MRMLNodeComboBox_ReferenceVolume, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onReferenceVolumeNodeChanged(vtkMRMLNode*)));
  connect( d->MRMLNodeComboBox_ParameterSet, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onParameterNodeChanged(vtkMRMLNode*)));

  // Segmentation
  connect( d->SegmentSelectorWidget_TargetVolume, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onPatientBodySegmentationNodeChanged(vtkMRMLNode*)));
  connect( d->SegmentSelectorWidget_TargetVolume, SIGNAL(currentSegmentChanged(QString)), 
    this, SLOT(onPatientBodySegmentNameChanged(QString)));

  // Sliders, Coordinates widgets
  connect( d->SliderWidget_TableTopVerticalPosition, SIGNAL(valueChanged(double)), 
    this, SLOT(onTableTopVerticalPositionChanged(double)));
  connect( d->SliderWidget_TableTopVerticalPositionMiddle, SIGNAL(valueChanged(double)), 
    this, SLOT(onTableTopVerticalMiddlePositionChanged(double)));
  connect( d->SliderWidget_TableTopVerticalPositionMirror, SIGNAL(valueChanged(double)), 
    this, SLOT(onTableTopVerticalMirrorPositionChanged(double)));
  connect( d->SliderWidget_TableTopVerticalPositionOrigin, SIGNAL(valueChanged(double)), 
    this, SLOT(onTableTopVerticalOriginPositionChanged(double)));
  connect( d->CoordinatesWidget_PatientTableTopTranslation, SIGNAL(coordinatesChanged(double*)), 
    this, SLOT(onPatientTableTopTranslationChanged(double*)));

  // Buttons
  connect( d->PushButton_LoadStandModels, SIGNAL(clicked()), this, SLOT(onLoadStandModelsButtonClicked()));
  connect( d->PushButton_ResetModelsInitialPosition, SIGNAL(clicked()), this, SLOT(onResetToInitialPositionButtonClicked()));
  connect( d->PushButton_BevPlusX, SIGNAL(clicked()), this, SLOT(onBeamsEyeViewPlusXButtonClicked()));
  connect( d->PushButton_BevPlusY, SIGNAL(clicked()), this, SLOT(onBeamsEyeViewPlusYButtonClicked()));
  connect( d->PushButton_BevMinusX, SIGNAL(clicked()), this, SLOT(onBeamsEyeViewMinusXButtonClicked()));
  connect( d->PushButton_BevMinusY, SIGNAL(clicked()), this, SLOT(onBeamsEyeViewMinusYButtonClicked()));

  // Handle scene change event if occurs
  qvtkConnect( d->logic(), vtkCommand::ModifiedEvent, this, SLOT(onLogicModified()));
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onLoadStandModelsButtonClicked()
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid scene";
    return;
  }

  vtkMRMLIhepStandGeometryNode* parameterNode = vtkMRMLIhepStandGeometryNode::SafeDownCast(d->MRMLNodeComboBox_ParameterSet->currentNode());
  if (!parameterNode || !d->ModuleWindowInitialized)
  {
    return;
  }

  // Load and setup models
  parameterNode->SetTreatmentMachineType("IHEPStand");
  d->logic()->LoadTreatmentMachineModels(parameterNode);
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onResetToInitialPositionButtonClicked()
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid scene";
    return;
  }

  vtkMRMLIhepStandGeometryNode* parameterNode = vtkMRMLIhepStandGeometryNode::SafeDownCast(d->MRMLNodeComboBox_ParameterSet->currentNode());
  if (!parameterNode || !d->ModuleWindowInitialized)
  {
    return;
  }

  d->logic()->ResetModelsToInitialPosition(parameterNode);
 // qDebug() << Q_FUNC_INFO << ": finished";
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onPatientTableTopTranslationChanged(double* position)
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid scene";
    return;
  }

  vtkMRMLIhepStandGeometryNode* parameterNode = vtkMRMLIhepStandGeometryNode::SafeDownCast(d->MRMLNodeComboBox_ParameterSet->currentNode());
  if (!parameterNode || !d->ModuleWindowInitialized)
  {
    return;
  }

  parameterNode->SetPatientToTableTopTranslation(position);

  d->logic()->UpdatePatientToTableTopTransform(parameterNode);
  d->logic()->SetupTreatmentMachineModels(parameterNode);
//  qDebug() << Q_FUNC_INFO << ": finished";
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onTableTopVerticalPositionChanged(double position)
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid scene";
    return;
  }

  vtkMRMLIhepStandGeometryNode* parameterNode = vtkMRMLIhepStandGeometryNode::SafeDownCast(d->MRMLNodeComboBox_ParameterSet->currentNode());
  if (!parameterNode || !d->ModuleWindowInitialized)
  {
    return;
  }

  parameterNode->SetTableTopVerticalPosition(position);

  d->logic()->UpdateTableTopVerticalToTableTopStandTransform(parameterNode);
  d->logic()->SetupTreatmentMachineModels(parameterNode);
//  qDebug() << Q_FUNC_INFO << ": finished";
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onTableTopVerticalMiddlePositionChanged(double position)
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid scene";
    return;
  }

  vtkMRMLIhepStandGeometryNode* parameterNode = vtkMRMLIhepStandGeometryNode::SafeDownCast(d->MRMLNodeComboBox_ParameterSet->currentNode());
  if (!parameterNode || !d->ModuleWindowInitialized)
  {
    return;
  }

  parameterNode->SetTableTopVerticalPositionMiddle(position);

  d->logic()->UpdateMarkupsNodes(parameterNode);
  d->logic()->SetupTreatmentMachineModels(parameterNode);
//  qDebug() << Q_FUNC_INFO << ": finished";
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onTableTopVerticalMirrorPositionChanged(double position)
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid scene";
    return;
  }

  vtkMRMLIhepStandGeometryNode* parameterNode = vtkMRMLIhepStandGeometryNode::SafeDownCast(d->MRMLNodeComboBox_ParameterSet->currentNode());
  if (!parameterNode || !d->ModuleWindowInitialized)
  {
    return;
  }

  parameterNode->SetTableTopVerticalPositionMirror(position);
  d->logic()->UpdateMarkupsNodes(parameterNode);
  d->logic()->SetupTreatmentMachineModels(parameterNode);
//  qDebug() << Q_FUNC_INFO << ": finished";
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onTableTopVerticalOriginPositionChanged(double position)
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid scene";
    return;
  }

  vtkMRMLIhepStandGeometryNode* parameterNode = vtkMRMLIhepStandGeometryNode::SafeDownCast(d->MRMLNodeComboBox_ParameterSet->currentNode());
  if (!parameterNode || !d->ModuleWindowInitialized)
  {
    return;
  }

  parameterNode->SetTableTopVerticalPositionOrigin(position);
  d->logic()->UpdateMarkupsNodes(parameterNode);
  d->logic()->SetupTreatmentMachineModels(parameterNode);
//  qDebug() << Q_FUNC_INFO << ": finished";
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onBeamsEyeViewPlusXButtonClicked()
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);
  double viewUpVector[4] = { 1., 0., 0., 0. };
  this->onBeamsEyeViewButtonClicked(viewUpVector);
  d->Label_BevOrientation->setText(tr("+X"));
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onBeamsEyeViewMinusXButtonClicked()
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);
  double viewUpVector[4] = { -1., 0., 0., 0. };
  this->onBeamsEyeViewButtonClicked(viewUpVector);
  d->Label_BevOrientation->setText(tr("-X"));
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onBeamsEyeViewPlusYButtonClicked()
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);
  double viewUpVector[4] = { 0., 1., 0., 0. };
  this->onBeamsEyeViewButtonClicked(viewUpVector);
  d->Label_BevOrientation->setText(tr("+Y"));
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onBeamsEyeViewMinusYButtonClicked()
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);
  double viewUpVector[4] = { 0., -1., 0., 0. };
  this->onBeamsEyeViewButtonClicked(viewUpVector);
  d->Label_BevOrientation->setText(tr("-Y"));
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::enter()
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);
  this->Superclass::enter();
  this->onEnter();
  qDebug() << Q_FUNC_INFO << ": method finished";
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::exit()
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);
  this->Superclass::exit();
//  d->logic()->RestoreOriginalGeometryTransformHierarchy();
  d->Label_BevOrientation->setText("");
  qDebug() << Q_FUNC_INFO << ": method exit";
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onEnter()
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);

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

  vtkMRMLIhepStandGeometryNode* parameterNode = nullptr; 
  // Try to find one in the scene
  if (vtkMRMLNode* node = this->mrmlScene()->GetNthNodeByClass( 0, "vtkMRMLIhepStandGeometryNode"))
  {
    parameterNode = vtkMRMLIhepStandGeometryNode::SafeDownCast(node);
  }

  this->updateWidgetFromMRML();

  // All required data for GUI is initiated
  d->ModuleWindowInitialized = true;
  qDebug() << Q_FUNC_INFO << ": module window initiated ";
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);

  vtkMRMLIhepStandGeometryNode* parameterNode = vtkMRMLIhepStandGeometryNode::SafeDownCast(d->MRMLNodeComboBox_ParameterSet->currentNode());

  if (!this->mrmlScene())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid scene";
    return;
  }

  // Enable widgets
  
  if (!parameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node, or module window isn't initialized";
    return;
  }

  if (parameterNode->GetBeamNode())
  {
    d->MRMLNodeComboBox_RtBeam->setCurrentNode(parameterNode->GetBeamNode());
  }

//  vtkMRMLScalarVolumeNode* ctVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(d->MRMLNodeComboBox_CtVolume->currentNode());
  if (parameterNode->GetReferenceVolumeNode())
  {
    d->MRMLNodeComboBox_ReferenceVolume->setCurrentNode(parameterNode->GetReferenceVolumeNode());
  }

  if (parameterNode->GetPatientBodySegmentationNode())
  {
    d->SegmentSelectorWidget_TargetVolume->setCurrentNode(parameterNode->GetPatientBodySegmentationNode());
  }

  if (parameterNode->GetPatientBodySegmentID())
  {
    QString id(static_cast<char *>(parameterNode->GetPatientBodySegmentID()));
    d->SegmentSelectorWidget_TargetVolume->setCurrentSegmentID(id);
  }
  d->SliderWidget_TableTopVerticalPosition->setValue(parameterNode->GetTableTopVerticalPosition());
  
  double patientToTableTopTranslation[3] = {};
  parameterNode->GetPatientToTableTopTranslation(patientToTableTopTranslation);

  d->CoordinatesWidget_PatientTableTopTranslation->setCoordinates(patientToTableTopTranslation);

  qDebug() << Q_FUNC_INFO << ": finished";
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onLogicModified()
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onParameterNodeChanged(vtkMRMLNode*)
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::setParameterNode(vtkMRMLNode* node)
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);

  vtkMRMLIhepStandGeometryNode* parameterNode = vtkMRMLIhepStandGeometryNode::SafeDownCast(node);

  // Make sure the parameter set node is selected (in case the function was not called by the selector combobox signal)
  d->MRMLNodeComboBox_ParameterSet->setCurrentNode(node);

  // Set parameter node to children widgets (PlastimatchParameters)
//  d->PlastimatchParametersWidget->setParameterNode(node);
 
  // Each time the node is modified, the UI widgets are updated
  qvtkReconnect( parameterNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));

  // Set selected MRML nodes in comboboxes in the parameter set if it was nullptr there
  // (then in the meantime the comboboxes selected the first one from the scene and we have to set that)
  if (parameterNode)
  {
    if (!parameterNode->GetBeamNode())
    {
      vtkMRMLRTBeamNode* beamNode = vtkMRMLRTBeamNode::SafeDownCast(d->MRMLNodeComboBox_RtBeam->currentNode());
//      qvtkConnect( beamNode, vtkMRMLRTBeamNode::BeamTransformModified, this, SLOT(updateNormalAndVupVectors()));
      parameterNode->SetAndObserveBeamNode(beamNode);
      parameterNode->Modified();
    }
  }
  this->updateWidgetFromMRML();
  qDebug() << Q_FUNC_INFO << ": finished";
}

/// RTBeam Node (RTBeam or RTIonBeam) changed
void qSlicerIhepStandGeometryModuleWidget::onRTBeamNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid scene";
    return;
  }

//  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(this->mrmlScene());
//  if (!shNode)
//  {
//    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy";
//    return;
//  }

  vtkMRMLRTBeamNode* beamNode = vtkMRMLRTBeamNode::SafeDownCast(node);
  if (!beamNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid beam node";
    return;
  }

  vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();
  vtkTransform* beamTransform = nullptr;
  vtkNew<vtkMatrix4x4> mat;

  if (beamTransformNode)
  {
    beamTransform = vtkTransform::SafeDownCast(beamTransformNode->GetTransformToParent());
    beamTransform->GetMatrix(mat);
  }
  else
  {
    qCritical() << Q_FUNC_INFO << "Beam transform node is invalid";
    return;
  }
/*
  double xP[4] = { 1., 0., 0., 0. }; // beam negative X-axis
  double xM[4] = { -1., 0., 0., 0. }; // beam negative X-axis
  double yP[4] = { 0., 1., 0., 0. }; // beam negative X-axis
  double yM[4] = { 0., -1., 0., 0. }; // beam negative X-axis
  double zP[4] = { 0., 0., 1., 0. }; // beam negative X-axis
  double zM[4] = { 0., 0., -1., 0. }; // beam negative X-axis
  
  double vup[4];
  mat->MultiplyPoint( xP, vup);
  qDebug() << "xP " << vup[0] << " " << vup[1] << " " << vup[2];
  mat->MultiplyPoint( xM, vup);
  qDebug() << "xM " << vup[0] << " " << vup[1] << " " << vup[2]; 
  mat->MultiplyPoint( yP, vup);
  qDebug() << "yP " << vup[0] << " " << vup[1] << " " << vup[2]; 
  mat->MultiplyPoint( yM, vup);
  qDebug() << "yM " << vup[0] << " " << vup[1] << " " << vup[2]; 
  mat->MultiplyPoint( zP, vup);
  qDebug() << "zP " << vup[0] << " " << vup[1] << " " << vup[2]; 
  mat->MultiplyPoint( zM, vup);
  qDebug() << "zM " << vup[0] << " " << vup[1] << " " << vup[2];
*/
  vtkMRMLRTIonBeamNode* ionBeamNode = vtkMRMLRTIonBeamNode::SafeDownCast(node);
  Q_UNUSED(ionBeamNode);

  vtkMRMLIhepStandGeometryNode* parameterNode = vtkMRMLIhepStandGeometryNode::SafeDownCast(d->MRMLNodeComboBox_ParameterSet->currentNode());
  if (!parameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  
  parameterNode->SetAndObserveBeamNode(beamNode);
  parameterNode->Modified();
  qDebug() << Q_FUNC_INFO << beamNode->GetName() << ": finished";

//  d->logic()->CalculateAngles(parameterNode);
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onReferenceVolumeNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);

  vtkMRMLIhepStandGeometryNode* parameterNode = vtkMRMLIhepStandGeometryNode::SafeDownCast(d->MRMLNodeComboBox_ParameterSet->currentNode());

  if (!parameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);
  if (!volumeNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid reference CT volume node";
    return;
  }
  
  parameterNode->SetAndObserveReferenceVolumeNode(volumeNode);
  parameterNode->Modified();
  qDebug() << Q_FUNC_INFO << ": finished";
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onPatientBodySegmentationNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);

  vtkMRMLIhepStandGeometryNode* parameterNode = vtkMRMLIhepStandGeometryNode::SafeDownCast(d->MRMLNodeComboBox_ParameterSet->currentNode());

  if (!parameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(node);
  if (!segmentationNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid body segmentation node";
    return;
  }

  parameterNode->SetAndObservePatientBodySegmentationNode(segmentationNode);
  parameterNode->Modified();
  qDebug() << Q_FUNC_INFO << ": finished";
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onPatientBodySegmentNameChanged(const QString& bodySegmentName)
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);

  vtkMRMLIhepStandGeometryNode* parameterNode = vtkMRMLIhepStandGeometryNode::SafeDownCast(d->MRMLNodeComboBox_ParameterSet->currentNode());

  if (!parameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  if (bodySegmentName.isEmpty())
  {
    qCritical() << Q_FUNC_INFO << ": Body segment name is empty";
    return;
  }

  QByteArray byteString = bodySegmentName.toLatin1();
  const char* name = byteString.constData();
  parameterNode->SetPatientBodySegmentID(name);
  parameterNode->Modified();
  qDebug() << Q_FUNC_INFO << ": finished";
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onSceneClosedEvent()
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);
  this->updateWidgetFromMRML();
  qDebug() << Q_FUNC_INFO << ": finished";
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onSceneImportedEvent()
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);
  this->onEnter();
  qDebug() << Q_FUNC_INFO << ": finished";
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);
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
    else if (vtkMRMLNode* node = scene->GetNthNodeByClass( 0, "vtkMRMLIhepStandGeometryNode"))
    {
      this->setParameterNode(node);
    }
    else
    {
      vtkNew<vtkMRMLIhepStandGeometryNode> newNode;
      this->mrmlScene()->AddNode(newNode);
      this->setParameterNode(newNode);
    }
  }
  qDebug() << Q_FUNC_INFO << ": finished";
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onBeamsEyeViewButtonClicked(const double viewUpVector[4])
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);

  // Get 3D view node
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  qMRMLThreeDView* threeDView = layoutManager->threeDWidget(0)->threeDView();
  vtkMRMLViewNode* viewNode = threeDView->mrmlViewNode();

  // Get camera node for view
  vtkCollection* cameras = this->mrmlScene()->GetNodesByClass("vtkMRMLCameraNode");
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
    cameras->Delete();
    return;
  }

  vtkMRMLRTBeamNode* beamNode = vtkMRMLRTBeamNode::SafeDownCast(d->MRMLNodeComboBox_RtBeam->currentNode());
  double sourcePosition[3] = {0.0, 0.0, 0.0};
  double isocenter[3] = {0.0, 0.0, 0.0};

  if (beamNode && beamNode->GetSourcePosition(sourcePosition))
  {
    vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();
    vtkTransform* beamTransform = nullptr;
    vtkNew<vtkMatrix4x4> mat;
    mat->Identity();

    if (beamTransformNode)
    {
      beamTransform = vtkTransform::SafeDownCast(beamTransformNode->GetTransformToParent());
      beamTransform->GetMatrix(mat);
    }
    else
    {
      qCritical() << Q_FUNC_INFO << "Beam transform node is invalid";
      cameras->Delete();
      return;
    }

//    double viewUpVector[4] = { -1., 0., 0., 0. }; // beam negative X-axis
    double vup[4];
  
    mat->MultiplyPoint( viewUpVector, vup);

    // Translation to origin for in-place rotation

    qDebug() << "Beam view-up Vector " << viewUpVector[0] << " " \
      << viewUpVector[1] << " " << viewUpVector[2]; 

    qDebug() << "Beam view-up Vector in RAS " << vup[0] << " " << vup[1] << " " << vup[2]; 

    //vtkMRMLModelNode* collimatorModel = vtkMRMLModelNode::SafeDownCast(this->mrmlScene()->GetFirstNodeByName("CollimatorModel"));
    //vtkPolyData* collimatorModelPolyData = collimatorModel->GetPolyData();

    //double collimatorCenterOfRotation[3] = {0.0, 0.0, 0.0};
    //double collimatorModelBounds[6] = { 0, 0, 0, 0, 0, 0 };

    //collimatorModelPolyData->GetBounds(collimatorModelBounds);
    //collimatorCenterOfRotation[0] = (collimatorModelBounds[0] + collimatorModelBounds[1]) / 2;
    //collimatorCenterOfRotation[1] = (collimatorModelBounds[2] + collimatorModelBounds[3]) / 2;
    //collimatorCenterOfRotation[2] = collimatorModelBounds[4];

    //cameraNode->GetCamera()->SetPosition(collimatorCenterOfRotation);
    cameraNode->GetCamera()->SetPosition(sourcePosition);
    if (beamNode->GetPlanIsocenterPosition(isocenter))
    {
      cameraNode->GetCamera()->SetFocalPoint(isocenter);
    }
    cameraNode->SetViewUp(vup);
  }
  
  cameraNode->GetCamera()->Elevation(0.);
  cameras->Delete();
}
