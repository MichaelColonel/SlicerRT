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

// SlicerRT MRML IEC and IhepStandGeometry includes
#include "vtkSlicerIECTransformLogic.h"
#include "vtkMRMLIhepStandGeometryNode.h"

#include <vtkMRMLRTBeamNode.h>
#include <vtkMRMLRTIonBeamNode.h>
#include "vtkMRMLRTPlanNode.h"

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
  vtkMRMLCameraNode* ThreeDViewCameraNode;
};

//-----------------------------------------------------------------------------
// qSlicerIhepStandGeometryModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerIhepStandGeometryModuleWidgetPrivate::qSlicerIhepStandGeometryModuleWidgetPrivate(qSlicerIhepStandGeometryModuleWidget &object)
  :
  q_ptr(&object),
  ModuleWindowInitialized(false),
  ThreeDViewCameraNode(nullptr)
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
  connect( d->SliderWidget_TableTopVerticalPositionMiddle, SIGNAL(valueChanged(double)), 
    this, SLOT(onTableMiddleVerticalPositionChanged(double)));
  connect( d->SliderWidget_TableTopVerticalPositionMirror, SIGNAL(valueChanged(double)), 
    this, SLOT(onTableMirrorVerticalPositionChanged(double)));
  connect( d->SliderWidget_TableTopVerticalPositionOrigin, SIGNAL(valueChanged(double)), 
    this, SLOT(onTableOriginVerticalPositionChanged(double)));
  connect( d->SliderWidget_TableTopStandLongitudinalPosition, SIGNAL(valueChanged(double)), 
    this, SLOT(onTableLongitudinalPositionChanged(double)));
  connect( d->SliderWidget_TableTopStandLateralPosition, SIGNAL(valueChanged(double)), 
    this, SLOT(onTableLateralPositionChanged(double)));
  connect( d->SliderWidget_PatientSupportRotationAngle, SIGNAL(valueChanged(double)), 
    this, SLOT(onPatientSupportRotationAngleChanged(double)));

  connect( d->SliderWidget_TableTopLongitudinalAngle, SIGNAL(valueChanged(double)), 
    this, SLOT(onTableTopLongitudinalAngleChanged(double)));
  connect( d->SliderWidget_TableTopLateralAngle, SIGNAL(valueChanged(double)), 
    this, SLOT(onTableTopLateralAngleChanged(double)));

  connect( d->CoordinatesWidget_PatientTableTopTranslation, SIGNAL(coordinatesChanged(double*)), 
    this, SLOT(onPatientTableTopTranslationChanged(double*)));

  connect( d->checkBox_FixedReferenceCamera, SIGNAL(toggled(bool)), this, SLOT(updateFixedReferenceCamera(bool)));

  // Buttons
  connect( d->PushButton_LoadStandModels, SIGNAL(clicked()), this, SLOT(onLoadModelsButtonClicked()));
  connect( d->PushButton_ResetModelsInitialPosition, SIGNAL(clicked()), this, SLOT(onResetToInitialPositionButtonClicked()));
  connect( d->PushButton_BevPlusX, SIGNAL(clicked()), this, SLOT(onBeamsEyeViewPlusXButtonClicked()));
  connect( d->PushButton_BevPlusY, SIGNAL(clicked()), this, SLOT(onBeamsEyeViewPlusYButtonClicked()));
  connect( d->PushButton_BevMinusX, SIGNAL(clicked()), this, SLOT(onBeamsEyeViewMinusXButtonClicked()));
  connect( d->PushButton_BevMinusY, SIGNAL(clicked()), this, SLOT(onBeamsEyeViewMinusYButtonClicked()));
  // Handle scene change event if occurs
  qvtkConnect( d->logic(), vtkCommand::ModifiedEvent, this, SLOT(onLogicModified()));
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onLoadModelsButtonClicked()
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
  parameterNode->DisableModifiedEventOn();
  parameterNode->SetTreatmentMachineType("IHEPStand");
  d->logic()->LoadTreatmentMachineModels(parameterNode);
  d->logic()->ResetModelsToInitialPosition(parameterNode);
  parameterNode->DisableModifiedEventOff();
  

  parameterNode->Modified();
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

  parameterNode->Modified();
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
  parameterNode->DisableModifiedEventOn();
  parameterNode->SetPatientToTableTopTranslation(position);

  d->logic()->UpdatePatientToTableTopTransform(parameterNode);
//  d->logic()->UpdateTableTopToTableOriginTransform(parameterNode);
//  d->logic()->UpdateTableOriginToTableSupportTransform(parameterNode);
//  d->logic()->UpdateTableMirrorToTableSupportTransform(parameterNode);
//  d->logic()->UpdateTableMiddleToTableSupportTransform(parameterNode);
//  d->logic()->UpdateTableSupportToTablePlatformTransform(parameterNode);
//  d->logic()->UpdateTablePlatformToPatientSupportTransform(parameterNode);
//  d->logic()->UpdatePatientSupportToFixedReferenceTransform(parameterNode);

  parameterNode->DisableModifiedEventOff();
  
  parameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onTableMiddleVerticalPositionChanged(double position)
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

  parameterNode->DisableModifiedEventOn();
  parameterNode->SetTableTopVerticalPositionMiddle(position);

//  d->logic()->UpdateTableOriginToTableSupportTransform(parameterNode);
//  d->logic()->UpdateTableMirrorToTableSupportTransform(parameterNode);
  d->logic()->UpdateTableMiddleToTableSupportTransform(parameterNode);
//  d->logic()->UpdateTableSupportToTablePlatformTransform(parameterNode);
//  d->logic()->UpdateTablePlatformToPatientSupportTransform(parameterNode);
//  d->logic()->UpdatePatientSupportToFixedReferenceTransform(parameterNode);

  parameterNode->DisableModifiedEventOff();

  parameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onTableMirrorVerticalPositionChanged(double position)
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

  parameterNode->DisableModifiedEventOn();
  parameterNode->SetTableTopVerticalPositionMirror(position);

//  d->logic()->UpdateTableOriginToTableSupportTransform(parameterNode);
  d->logic()->UpdateTableMirrorToTableSupportTransform(parameterNode);
//  d->logic()->UpdateTableMiddleToTableSupportTransform(parameterNode);
//  d->logic()->UpdateTableSupportToTablePlatformTransform(parameterNode);
//  d->logic()->UpdateTablePlatformToPatientSupportTransform(parameterNode);
//  d->logic()->UpdatePatientSupportToFixedReferenceTransform(parameterNode);

  parameterNode->DisableModifiedEventOff();

  parameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onTableOriginVerticalPositionChanged(double position)
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

  parameterNode->DisableModifiedEventOn();
  parameterNode->SetTableTopVerticalPositionOrigin(position);

  d->logic()->UpdateTableOriginToTableSupportTransform(parameterNode);
//  d->logic()->UpdateTableMirrorToTableSupportTransform(parameterNode);
//  d->logic()->UpdateTableMiddleToTableSupportTransform(parameterNode);
//  d->logic()->UpdateTableSupportToTablePlatformTransform(parameterNode);
//  d->logic()->UpdateTablePlatformToPatientSupportTransform(parameterNode);
//  d->logic()->UpdatePatientSupportToFixedReferenceTransform(parameterNode);

  parameterNode->DisableModifiedEventOff();

  parameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onTableLongitudinalPositionChanged(double position)
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

  parameterNode->DisableModifiedEventOn();
  parameterNode->SetTableTopLongitudinalPosition(position);  // Inferior-Superior movement along Y-axis

  d->logic()->UpdateTablePlatformToPatientSupportTransform(parameterNode);
  d->logic()->UpdatePatientSupportToFixedReferenceTransform(parameterNode);

  parameterNode->DisableModifiedEventOff();

  parameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onTableLateralPositionChanged(double position)
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

  parameterNode->DisableModifiedEventOn();
  parameterNode->SetTableTopLateralPosition(position); // Left-Right movement along X-axis

  // update table top stand to patient support rotation transform
  d->logic()->UpdateTableSupportToTablePlatformTransform(parameterNode);
  d->logic()->UpdateTablePlatformToPatientSupportTransform(parameterNode);
  d->logic()->UpdatePatientSupportToFixedReferenceTransform(parameterNode);

  parameterNode->DisableModifiedEventOff();

  parameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onTableTopLongitudinalAngleChanged(double angle)
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

  parameterNode->DisableModifiedEventOn();
  parameterNode->SetTableTopLongitudinalAngle(angle);

  // update table top to table origin vertical movement
  d->logic()->UpdateTableTopToTableOriginTransform(parameterNode);
  d->logic()->UpdateTableOriginToTableSupportTransform(parameterNode);
  d->logic()->UpdateTableMirrorToTableSupportTransform(parameterNode);
  d->logic()->UpdateTableMiddleToTableSupportTransform(parameterNode);
  d->logic()->UpdateTableSupportToTablePlatformTransform(parameterNode);
  d->logic()->UpdateTablePlatformToPatientSupportTransform(parameterNode);
  d->logic()->UpdatePatientSupportToFixedReferenceTransform(parameterNode);

  double mirrorPos, middlePos;
  d->logic()->CalculateTableTopPositionsFromPlaneNode( parameterNode, mirrorPos, middlePos);
  
  parameterNode->SetTableTopVerticalPositionMirror(mirrorPos + parameterNode->GetTableTopVerticalPositionOrigin());
  parameterNode->SetTableTopVerticalPositionMiddle(middlePos + parameterNode->GetTableTopVerticalPositionOrigin());
  
  parameterNode->DisableModifiedEventOff();

  double originPos = d->SliderWidget_TableTopVerticalPositionOrigin->value();
  
  d->SliderWidget_TableTopVerticalPositionMiddle->setValue(originPos + middlePos);
  d->SliderWidget_TableTopVerticalPositionMirror->setValue(originPos + mirrorPos);

  parameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onTableTopLateralAngleChanged(double angle)
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

  parameterNode->DisableModifiedEventOn();
  parameterNode->SetTableTopLateralAngle(angle);

  // update table top to table origin vertical movement
  d->logic()->UpdateTableTopToTableOriginTransform(parameterNode);
  d->logic()->UpdateTableOriginToTableSupportTransform(parameterNode);
  d->logic()->UpdateTableMirrorToTableSupportTransform(parameterNode);
  d->logic()->UpdateTableMiddleToTableSupportTransform(parameterNode);
  d->logic()->UpdateTableSupportToTablePlatformTransform(parameterNode);
  d->logic()->UpdateTablePlatformToPatientSupportTransform(parameterNode);
  d->logic()->UpdatePatientSupportToFixedReferenceTransform(parameterNode);

  double mirrorPos, middlePos;
  d->logic()->CalculateTableTopPositionsFromPlaneNode( parameterNode, mirrorPos, middlePos);
  
  parameterNode->SetTableTopVerticalPositionMirror(mirrorPos + parameterNode->GetTableTopVerticalPositionOrigin());
  parameterNode->SetTableTopVerticalPositionMiddle(middlePos + parameterNode->GetTableTopVerticalPositionOrigin());
  parameterNode->DisableModifiedEventOff();

  double originPos = d->SliderWidget_TableTopVerticalPositionOrigin->value();

  d->SliderWidget_TableTopVerticalPositionMiddle->setValue(originPos + middlePos);
  d->SliderWidget_TableTopVerticalPositionMirror->setValue(originPos + mirrorPos);

  parameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onPatientSupportRotationAngleChanged(double angle)
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

  parameterNode->DisableModifiedEventOn();
  parameterNode->SetPatientSupportRotationAngle(angle);
  parameterNode->DisableModifiedEventOff();

  // update table top stand to patient support rotation transform
  d->logic()->UpdatePatientSupportToFixedReferenceTransform(parameterNode);

  parameterNode->Modified();
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

  d->Label_BevOrientation->setText("");
  qDebug() << Q_FUNC_INFO << ": method finished";
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

  if (parameterNode)
  {
    ;
  }

  this->updateWidgetFromMRML();

  // All required data for GUI is initiated
  d->ModuleWindowInitialized = true;
  d->ThreeDViewCameraNode = this->Get3DViewCameraNode();
  d->logic()->SetFixedReferenceCamera(d->ThreeDViewCameraNode);
  qDebug() << Q_FUNC_INFO << ": module window initiated";
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
  d->SliderWidget_TableTopVerticalPositionMiddle->setValue(parameterNode->GetTableTopVerticalPositionMiddle());
  d->SliderWidget_TableTopVerticalPositionOrigin->setValue(parameterNode->GetTableTopVerticalPositionOrigin());
  d->SliderWidget_TableTopVerticalPositionMirror->setValue(parameterNode->GetTableTopVerticalPositionMirror());
  d->SliderWidget_TableTopStandLongitudinalPosition->setValue(parameterNode->GetTableTopLongitudinalPosition());
  d->SliderWidget_TableTopStandLateralPosition->setValue(parameterNode->GetTableTopLateralPosition());
  d->SliderWidget_PatientSupportRotationAngle->setValue(parameterNode->GetPatientSupportRotationAngle());
  
  double patientToTableTopTranslation[3] = {};
  parameterNode->GetPatientToTableTopTranslation(patientToTableTopTranslation);

  d->CoordinatesWidget_PatientTableTopTranslation->setCoordinates(patientToTableTopTranslation);
  d->checkBox_FixedReferenceCamera->setChecked(parameterNode->GetUseStandCoordinateSystem());
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
  
  parameterNode->DisableModifiedEventOn();
  parameterNode->SetAndObserveBeamNode(beamNode);
  parameterNode->DisableModifiedEventOff();

  parameterNode->Modified();
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
  
  parameterNode->DisableModifiedEventOn();
  parameterNode->SetAndObserveReferenceVolumeNode(volumeNode);
  parameterNode->DisableModifiedEventOff();

  parameterNode->Modified();
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

  parameterNode->DisableModifiedEventOn();
  parameterNode->SetAndObservePatientBodySegmentationNode(segmentationNode);
  parameterNode->DisableModifiedEventOff();

  parameterNode->Modified();
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

  parameterNode->DisableModifiedEventOn();
  parameterNode->SetPatientBodySegmentID(name);
  parameterNode->DisableModifiedEventOff();

  parameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onSceneClosedEvent()
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onSceneImportedEvent()
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);
  this->onEnter();
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

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::updateFixedReferenceCamera(bool update/* = true */)
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);

  vtkMRMLIhepStandGeometryNode* parameterNode = vtkMRMLIhepStandGeometryNode::SafeDownCast(d->MRMLNodeComboBox_ParameterSet->currentNode());

  parameterNode->DisableModifiedEventOn();
  parameterNode->SetUseStandCoordinateSystem(update);
  parameterNode->DisableModifiedEventOff();

  parameterNode->Modified();
  
// Disable camera update in GUI, use camera update in logic
#if defined (commentout)

  if (!update)
  {
    return;
  }

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

  vtkMRMLTransformNode* fixedReferenceTransformNode = d->logic()->GetFixedReferenceMarkupsTransform();
  if (fixedReferenceTransformNode)
  {
    vtkTransform* fixedReferenceTransform = nullptr;
    vtkNew<vtkMatrix4x4> mat;
    mat->Identity();

    if (fixedReferenceTransformNode)
    {
      fixedReferenceTransform = vtkTransform::SafeDownCast(fixedReferenceTransformNode->GetTransformToParent());
      fixedReferenceTransform->GetMatrix(mat);
    }
    else
    {
      qCritical() << Q_FUNC_INFO << "FixedReference transform node is invalid";
      cameras->Delete();
      return;
    }

    double viewUpVector[4] = { 0., 1., 0., 0. }; // positive Y-axis
    double vup[4];
  
    mat->MultiplyPoint( viewUpVector, vup);

    double fixedIsocenter[4] = { 0., 0., 0., 1. }; // origin in FixedReference transform
    double isocenterWorld[4];

    double sourcePosition[4] = { 0., 0., -4000., 1. }; // origin in FixedReference transform
    double sourcePositionWorld[4];

    mat->MultiplyPoint( fixedIsocenter, isocenterWorld);
    mat->MultiplyPoint( sourcePosition, sourcePositionWorld);

    cameraNode->GetCamera()->SetPosition(sourcePositionWorld);
    cameraNode->GetCamera()->SetFocalPoint(isocenterWorld);

    cameraNode->SetViewUp(vup);
  }
  
  cameraNode->GetCamera()->Elevation(30.);
  cameraNode->GetCamera()->Azimuth(-45.);
  cameras->Delete();

#endif
}

//-----------------------------------------------------------------------------
vtkMRMLCameraNode* qSlicerIhepStandGeometryModuleWidget::Get3DViewCameraNode()
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
    return nullptr;
  }
  cameras->Delete();
  return cameraNode;
}
