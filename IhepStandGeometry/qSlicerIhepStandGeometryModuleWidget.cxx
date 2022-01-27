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
#include <vtkMRMLRTFixedIonBeamNode.h>
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
  vtkSmartPointer< vtkMRMLIhepStandGeometryNode > ParameterNode;
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
  connect( d->MRMLNodeComboBox_FixedBeam, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onRTFixedIonBeamNodeChanged(vtkMRMLNode*)));
  connect( d->MRMLNodeComboBox_ReferenceVolume, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onReferenceVolumeNodeChanged(vtkMRMLNode*)));

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

  connect( d->CheckBox_FixedReferenceCamera, SIGNAL(toggled(bool)), this, SLOT(updateFixedReferenceCamera(bool)));
  connect( d->CheckBox_RotatePatientHeadFeet, SIGNAL(toggled(bool)), this, SLOT(onRotatePatientHeadFeetToggled(bool)));

  // Buttons
//  connect( d->PushButton_LoadStandModels, SIGNAL(clicked()), this, SLOT(onLoadModelsButtonClicked()));
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

  if (!d->ParameterNode)
  {
    return;
  }

  // Load and setup models
  d->ParameterNode->DisableModifiedEventOn();
  d->ParameterNode->SetTreatmentMachineType("IHEPStand");
  d->logic()->LoadTreatmentMachineModels(d->ParameterNode);
  d->logic()->ResetModelsToInitialPosition(d->ParameterNode);

  d->ParameterNode->DisableModifiedEventOff();
  d->ParameterNode->Modified();
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

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    return;
  }

  d->logic()->ResetModelsToInitialPosition(d->ParameterNode);

  d->ParameterNode->Modified();
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

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    return;
  }
  d->ParameterNode->DisableModifiedEventOn();
  d->ParameterNode->SetPatientToTableTopTranslation(position);

  d->logic()->UpdatePatientToTableTopTransform(d->ParameterNode);
  d->logic()->UpdateTableTopToTableOriginTransform(d->ParameterNode);
  d->logic()->UpdateTableOriginToTableSupportTransform(d->ParameterNode);
  d->logic()->UpdateTableMirrorToTableSupportTransform(d->ParameterNode);
  d->logic()->UpdateTableMiddleToTableSupportTransform(d->ParameterNode);
  d->logic()->UpdateTableSupportToTablePlatformTransform(d->ParameterNode);
  d->logic()->UpdateTablePlatformToPatientSupportTransform(d->ParameterNode);
  d->logic()->UpdatePatientSupportToFixedReferenceTransform(d->ParameterNode);

  d->ParameterNode->DisableModifiedEventOff();
  
  d->ParameterNode->Modified();
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

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    return;
  }

  d->ParameterNode->DisableModifiedEventOn();
  d->ParameterNode->SetTableTopVerticalPositionMiddle(position);

//  d->logic()->UpdateTableOriginToTableSupportTransform(parameterNode);
//  d->logic()->UpdateTableMirrorToTableSupportTransform(parameterNode);
  d->logic()->UpdateTableMiddleToTableSupportTransform(d->ParameterNode);
//  d->logic()->UpdateTableSupportToTablePlatformTransform(parameterNode);
//  d->logic()->UpdateTablePlatformToPatientSupportTransform(parameterNode);
//  d->logic()->UpdatePatientSupportToFixedReferenceTransform(parameterNode);

  d->ParameterNode->DisableModifiedEventOff();

  d->ParameterNode->Modified();
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

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    return;
  }

  d->ParameterNode->DisableModifiedEventOn();
  d->ParameterNode->SetTableTopVerticalPositionMirror(position);

//  d->logic()->UpdateTableOriginToTableSupportTransform(parameterNode);
  d->logic()->UpdateTableMirrorToTableSupportTransform(d->ParameterNode);
//  d->logic()->UpdateTableMiddleToTableSupportTransform(parameterNode);
//  d->logic()->UpdateTableSupportToTablePlatformTransform(parameterNode);
//  d->logic()->UpdateTablePlatformToPatientSupportTransform(parameterNode);
//  d->logic()->UpdatePatientSupportToFixedReferenceTransform(parameterNode);

  d->ParameterNode->DisableModifiedEventOff();
  d->ParameterNode->Modified();
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

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    return;
  }

  d->ParameterNode->DisableModifiedEventOn();
  d->ParameterNode->SetTableTopVerticalPositionOrigin(position);

  d->logic()->UpdateTableOriginToTableSupportTransform(d->ParameterNode);
//  d->logic()->UpdateTableMirrorToTableSupportTransform(parameterNode);
//  d->logic()->UpdateTableMiddleToTableSupportTransform(parameterNode);
//  d->logic()->UpdateTableSupportToTablePlatformTransform(parameterNode);
//  d->logic()->UpdateTablePlatformToPatientSupportTransform(parameterNode);
//  d->logic()->UpdatePatientSupportToFixedReferenceTransform(parameterNode);

  d->ParameterNode->DisableModifiedEventOff();
  d->ParameterNode->Modified();
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

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    return;
  }

  d->ParameterNode->DisableModifiedEventOn();
  d->ParameterNode->SetTableTopLongitudinalPosition(position);  // Inferior-Superior movement along Y-axis

  d->logic()->UpdateTablePlatformToPatientSupportTransform(d->ParameterNode);
  d->logic()->UpdatePatientSupportToFixedReferenceTransform(d->ParameterNode);

  d->ParameterNode->DisableModifiedEventOff();
  d->ParameterNode->Modified();
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

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    return;
  }

  d->ParameterNode->DisableModifiedEventOn();
  d->ParameterNode->SetTableTopLateralPosition(position); // Left-Right movement along X-axis

  // update table top stand to patient support rotation transform
  d->logic()->UpdateTableSupportToTablePlatformTransform(d->ParameterNode);
  d->logic()->UpdateTablePlatformToPatientSupportTransform(d->ParameterNode);
  d->logic()->UpdatePatientSupportToFixedReferenceTransform(d->ParameterNode);

  d->ParameterNode->DisableModifiedEventOff();
  d->ParameterNode->Modified();
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

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    return;
  }

  d->ParameterNode->DisableModifiedEventOn();
  double patientSupportRotationAngle = d->ParameterNode->GetPatientSupportRotationAngle(); // backup
  d->ParameterNode->SetPatientSupportRotationAngle(0.0);

  d->ParameterNode->SetTableTopLongitudinalAngle(angle);

  // update table top to table origin vertical movement
  d->logic()->UpdateTableTopToTableOriginTransform(d->ParameterNode);
  d->logic()->UpdateTableOriginToTableSupportTransform(d->ParameterNode);
  d->logic()->UpdateTableMirrorToTableSupportTransform(d->ParameterNode);
  d->logic()->UpdateTableMiddleToTableSupportTransform(d->ParameterNode);
  d->logic()->UpdateTableSupportToTablePlatformTransform(d->ParameterNode);
  d->logic()->UpdateTablePlatformToPatientSupportTransform(d->ParameterNode);
  d->logic()->UpdatePatientSupportToFixedReferenceTransform(d->ParameterNode);

  double mirrorPos, middlePos;
  d->logic()->CalculateTableTopPositionsFromPlaneNode( d->ParameterNode, mirrorPos, middlePos);
  
  d->ParameterNode->SetTableTopVerticalPositionMirror(mirrorPos + d->ParameterNode->GetTableTopVerticalPositionOrigin());
  d->ParameterNode->SetTableTopVerticalPositionMiddle(middlePos + d->ParameterNode->GetTableTopVerticalPositionOrigin());
  
  d->ParameterNode->DisableModifiedEventOff();

  double originPos = d->SliderWidget_TableTopVerticalPositionOrigin->value();
  
  d->SliderWidget_TableTopVerticalPositionMiddle->setValue(originPos + middlePos);
  d->SliderWidget_TableTopVerticalPositionMirror->setValue(originPos + mirrorPos);
  
  d->ParameterNode->SetPatientSupportRotationAngle(patientSupportRotationAngle); // restore
  d->ParameterNode->Modified();
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

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    return;
  }

  d->ParameterNode->DisableModifiedEventOn();
  double patientSupportRotationAngle = d->ParameterNode->GetPatientSupportRotationAngle(); // backup
  d->ParameterNode->SetPatientSupportRotationAngle(0.0);

  d->ParameterNode->SetTableTopLateralAngle(angle);

  // update table top to table origin vertical movement
  d->logic()->UpdateTableTopToTableOriginTransform(d->ParameterNode);
  d->logic()->UpdateTableOriginToTableSupportTransform(d->ParameterNode);
  d->logic()->UpdateTableMirrorToTableSupportTransform(d->ParameterNode);
  d->logic()->UpdateTableMiddleToTableSupportTransform(d->ParameterNode);
  d->logic()->UpdateTableSupportToTablePlatformTransform(d->ParameterNode);
  d->logic()->UpdateTablePlatformToPatientSupportTransform(d->ParameterNode);
  d->logic()->UpdatePatientSupportToFixedReferenceTransform(d->ParameterNode);

  double mirrorPos, middlePos;
  d->logic()->CalculateTableTopPositionsFromPlaneNode( d->ParameterNode, mirrorPos, middlePos);
  
  d->ParameterNode->SetTableTopVerticalPositionMirror(mirrorPos + d->ParameterNode->GetTableTopVerticalPositionOrigin());
  d->ParameterNode->SetTableTopVerticalPositionMiddle(middlePos + d->ParameterNode->GetTableTopVerticalPositionOrigin());

  d->ParameterNode->DisableModifiedEventOff();

  double originPos = d->SliderWidget_TableTopVerticalPositionOrigin->value();

  d->SliderWidget_TableTopVerticalPositionMiddle->setValue(originPos + middlePos);
  d->SliderWidget_TableTopVerticalPositionMirror->setValue(originPos + mirrorPos);

  d->ParameterNode->SetPatientSupportRotationAngle(patientSupportRotationAngle); // restore
  d->ParameterNode->Modified();
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

  // Try to find one in the scene
  if (vtkMRMLNode* node = this->mrmlScene()->GetFirstNodeByClass("vtkMRMLIhepStandGeometryNode"))
  {
    d->ParameterNode = vtkMRMLIhepStandGeometryNode::SafeDownCast(node);
  }

  if (d->ParameterNode)
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

  if (!this->mrmlScene())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid scene";
    return;
  }

  // Enable widgets
  
  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node, or module window isn't initialized";
    return;
  }

  if (d->ParameterNode->GetBeamNode())
  {
    d->MRMLNodeComboBox_RtBeam->setCurrentNode(d->ParameterNode->GetBeamNode());
  }

//  vtkMRMLScalarVolumeNode* ctVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(d->MRMLNodeComboBox_CtVolume->currentNode());
  if (d->ParameterNode->GetReferenceVolumeNode())
  {
    d->MRMLNodeComboBox_ReferenceVolume->setCurrentNode(d->ParameterNode->GetReferenceVolumeNode());
  }

  if (d->ParameterNode->GetPatientBodySegmentationNode())
  {
    d->SegmentSelectorWidget_TargetVolume->setCurrentNode(d->ParameterNode->GetPatientBodySegmentationNode());
  }

  if (d->ParameterNode->GetFixedBeamNode())
  {
    d->MRMLNodeComboBox_FixedBeam->setCurrentNode(d->ParameterNode->GetFixedBeamNode());
  }

  if (d->ParameterNode->GetPatientBodySegmentationNode())
  {
    d->SegmentSelectorWidget_TargetVolume->setCurrentNode(d->ParameterNode->GetPatientBodySegmentationNode());
  }

  if (d->ParameterNode->GetPatientBodySegmentID())
  {
    QString id(static_cast<char *>(d->ParameterNode->GetPatientBodySegmentID()));
    d->SegmentSelectorWidget_TargetVolume->setCurrentSegmentID(id);
  }
  d->SliderWidget_TableTopVerticalPosition->setValue(d->ParameterNode->GetTableTopVerticalPosition());
  d->SliderWidget_TableTopVerticalPositionMiddle->setValue(d->ParameterNode->GetTableTopVerticalPositionMiddle());
  d->SliderWidget_TableTopVerticalPositionOrigin->setValue(d->ParameterNode->GetTableTopVerticalPositionOrigin());
  d->SliderWidget_TableTopVerticalPositionMirror->setValue(d->ParameterNode->GetTableTopVerticalPositionMirror());
  d->SliderWidget_TableTopStandLongitudinalPosition->setValue(d->ParameterNode->GetTableTopLongitudinalPosition());
  d->SliderWidget_TableTopStandLateralPosition->setValue(d->ParameterNode->GetTableTopLateralPosition());
  d->SliderWidget_PatientSupportRotationAngle->setValue(d->ParameterNode->GetPatientSupportRotationAngle());
  
  double patientToTableTopTranslation[3] = {};
  d->ParameterNode->GetPatientToTableTopTranslation(patientToTableTopTranslation);

  d->CoordinatesWidget_PatientTableTopTranslation->setCoordinates(patientToTableTopTranslation);
  d->CheckBox_RotatePatientHeadFeet->setChecked(d->ParameterNode->GetPatientHeadFeetRotation());
  d->CheckBox_FixedReferenceCamera->setChecked(d->ParameterNode->GetUseStandCoordinateSystem());
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onLogicModified()
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::setParameterNode(vtkMRMLNode* node)
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);

  vtkMRMLIhepStandGeometryNode* parameterNode = vtkMRMLIhepStandGeometryNode::SafeDownCast(node);
  if (!parameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter node is invalid!";
    return;
  }

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

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  
  d->ParameterNode->DisableModifiedEventOn();
  d->ParameterNode->SetAndObserveBeamNode(beamNode);
  d->ParameterNode->DisableModifiedEventOff();

  d->ParameterNode->Modified();
}

/// RTBeam Node (RTBeam or RTIonBeam) changed
void qSlicerIhepStandGeometryModuleWidget::onRTFixedIonBeamNodeChanged(vtkMRMLNode* node)
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

  vtkMRMLRTFixedIonBeamNode* beamNode = vtkMRMLRTFixedIonBeamNode::SafeDownCast(node);
  if (!beamNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid beam node";
    return;
  }

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  
  d->ParameterNode->DisableModifiedEventOn();
  d->ParameterNode->SetAndObserveFixedBeamNode(beamNode);
  d->ParameterNode->DisableModifiedEventOff();

  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onReferenceVolumeNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
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
  
  d->ParameterNode->DisableModifiedEventOn();
  d->ParameterNode->SetAndObserveReferenceVolumeNode(volumeNode);
  d->ParameterNode->DisableModifiedEventOff();

  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onPatientBodySegmentationNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
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

  d->ParameterNode->DisableModifiedEventOn();
  d->ParameterNode->SetAndObservePatientBodySegmentationNode(segmentationNode);
  d->ParameterNode->DisableModifiedEventOff();

  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onPatientBodySegmentNameChanged(const QString& bodySegmentName)
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
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

  d->ParameterNode->DisableModifiedEventOn();
  d->ParameterNode->SetPatientBodySegmentID(name);
  d->ParameterNode->DisableModifiedEventOff();

  d->ParameterNode->Modified();
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
    if (d->ParameterNode)
    {
      this->setParameterNode(d->ParameterNode);
    }
    else if (vtkMRMLNode* node = scene->GetFirstNodeByClass("vtkMRMLIhepStandGeometryNode"))
    {
      d->ParameterNode = vtkMRMLIhepStandGeometryNode::SafeDownCast(node);
      this->setParameterNode(node);
    }
    else
    {
      vtkMRMLNode* newNode = scene->AddNewNodeByClass( "vtkMRMLIhepStandGeometryNode", "Channel25");
      d->ParameterNode = vtkMRMLIhepStandGeometryNode::SafeDownCast(newNode);
      if (d->ParameterNode)
      {
        std::string singletonTag = std::string("IhepStand_") + d->ParameterNode->GetName();
        d->ParameterNode->SetSingletonTag(singletonTag.c_str());
        this->setParameterNode(d->ParameterNode);
        this->onLoadModelsButtonClicked();
        qDebug() << Q_FUNC_INFO << ": Load models";
      }
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

  d->ParameterNode->DisableModifiedEventOn();
  d->ParameterNode->SetUseStandCoordinateSystem(update);
  d->ParameterNode->DisableModifiedEventOff();

  d->ParameterNode->Modified();
  
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
void qSlicerIhepStandGeometryModuleWidget::onRotatePatientHeadFeetToggled(bool toggled)
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);

  if (d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter node is invalid!";
    return;
  }

  d->ParameterNode->SetPatientHeadFeetRotation(toggled);
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
