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
#include <vtkMRMLRTCabin26AIonBeamNode.h>
#include <vtkMRMLRTFixedBeamNode.h>
#include <vtkMRMLPatientPositioningNode.h>
#include <vtkMRMLCabin26AGeometryNode.h>

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
#include <vtkSlicerCabin26ARobotsTransformLogic.h>

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
  vtkSlicerCabin26ARobotsTransformLogic* cabin26ARobotsLogic() const;
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
vtkSlicerCabin26ARobotsTransformLogic* qSlicerPatientPositioningModuleWidgetPrivate::cabin26ARobotsLogic() const
{
  Q_Q(const qSlicerPatientPositioningModuleWidget);
  vtkSlicerPatientPositioningLogic* logic = vtkSlicerPatientPositioningLogic::SafeDownCast(q->logic());
  return (logic) ? logic->GetCabin26ARobotsTransformLogic() : nullptr;
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
  d->ComboBox_TreatmentMachine->addItem("26A", "Cabin26AGeometry");
  d->ComboBox_TreatmentMachine->addItem("27A", "Cabin27AGeometry");
  d->ComboBox_TreatmentMachine->addItem("27B", "Cabin27BGeometry");
  d->ComboBox_TreatmentMachine->addItem("27C", "Cabin27CGeometry");
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
  connect( d->MRMLNodeComboBox_ExternalXrayBeam, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onExternalXrayBeamNodeChanged(vtkMRMLNode*)));
  connect( d->SegmentSelectorWidget_PatientBody, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onPatientBodySegmentationNodeChanged(vtkMRMLNode*)));
  connect( d->SegmentSelectorWidget_PatientBody, SIGNAL(currentSegmentChanged(QString)), 
    this, SLOT(onPatientBodySegmentChanged(QString)));

  // Buttons
  connect( d->PushButton_LoadTreatmentMachine, SIGNAL(clicked()), 
    this, SLOT(onLoadTreatmentMachineButtonClicked()));
  connect( d->CheckBox_RotatePatientHeadFeet, SIGNAL(toggled(bool)), 
    this, SLOT(onRotatePatientHeadFeetToggled(bool)));
  connect( d->CheckBox_ForceCollisionDetectionUpdate, SIGNAL(toggled(bool)), 
    this, SLOT(onCollisionDetectionToggled(bool)));
  connect( d->CheckBox_FixedReferenceCamera, SIGNAL(toggled(bool)), 
    this, SLOT(onFixedReferenceCameraToggled(bool)));
  connect( d->PushButton_AlignBeams, SIGNAL(clicked()), 
    this, SLOT(onAlignBeamsButtonClicked()));

  // Widgets
  connect( d->SliderWidget_TableRobotA1, SIGNAL(valueChanged(double)), 
    this, SLOT(onTableTopRobotA1Changed(double)));
  connect( d->SliderWidget_TableRobotA2, SIGNAL(valueChanged(double)), 
    this, SLOT(onTableTopRobotA2Changed(double)));
  connect( d->SliderWidget_TableRobotA3, SIGNAL(valueChanged(double)), 
    this, SLOT(onTableTopRobotA3Changed(double)));
  connect( d->SliderWidget_TableRobotA4, SIGNAL(valueChanged(double)), 
    this, SLOT(onTableTopRobotA4Changed(double)));
  connect( d->SliderWidget_TableRobotA5, SIGNAL(valueChanged(double)), 
    this, SLOT(onTableTopRobotA5Changed(double)));
  connect( d->SliderWidget_TableRobotA6, SIGNAL(valueChanged(double)), 
    this, SLOT(onTableTopRobotA6Changed(double)));
  connect( d->SliderWidget_CarmRobotA1, SIGNAL(valueChanged(double)), 
    this, SLOT(onCArmRobotA1Changed(double)));
  connect( d->SliderWidget_CarmRobotA2, SIGNAL(valueChanged(double)), 
    this, SLOT(onCArmRobotA2Changed(double)));
  connect( d->SliderWidget_CarmRobotA3, SIGNAL(valueChanged(double)), 
    this, SLOT(onCArmRobotA3Changed(double)));
  connect( d->SliderWidget_CarmRobotA4, SIGNAL(valueChanged(double)), 
    this, SLOT(onCArmRobotA4Changed(double)));
  connect( d->SliderWidget_CarmRobotA5, SIGNAL(valueChanged(double)), 
    this, SLOT(onCArmRobotA5Changed(double)));

  connect( d->CoordinatesWidget_PatientTableTopTranslation, SIGNAL(coordinatesChanged(double*)),
    this, SLOT(onPatientTableTopTranslationChanged(double*)));
  connect( d->CoordinatesWidget_BaseFixedTranslation, SIGNAL(coordinatesChanged(double*)),
    this, SLOT(onBaseFixedToFixedReferenceTranslationChanged(double*)));
  // Children widgets
  connect( d->FixedBeamAxisWidget, SIGNAL(bevOrientationChanged(const std::array< double, 3 >&)),
    this, SLOT(onBeamsEyeViewOrientationChanged(const std::array< double, 3 >&)));
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

  // Set parameter node to FixedBeamAxis widgets
  d->FixedBeamAxisWidget->setParameterNode(d->ParameterNode);

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
  vtkMRMLRTCabin26AIonBeamNode* beamNode = vtkMRMLRTCabin26AIonBeamNode::SafeDownCast(node);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  d->ParameterNode->SetAndObserveFixedReferenceBeamNode(beamNode);
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onExternalXrayBeamNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerPatientPositioningModuleWidget);
  vtkMRMLRTFixedBeamNode* beamNode = vtkMRMLRTFixedBeamNode::SafeDownCast(node);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  d->ParameterNode->SetAndObserveExternalXrayBeamNode(beamNode);
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
void qSlicerPatientPositioningModuleWidget::onAlignBeamsButtonClicked()
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  vtkMRMLScene* scene = this->mrmlScene();
  if (!scene)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid scene";
    return;
  }
  vtkMRMLPatientPositioningNode* parameterNode = vtkMRMLPatientPositioningNode::SafeDownCast(d->MRMLNodeComboBox_ParameterSet->currentNode());
  if (!parameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  vtkMRMLRTBeamNode* patientBeamNode = vtkMRMLRTBeamNode::SafeDownCast(d->MRMLNodeComboBox_Beam->currentNode());
  if (!patientBeamNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid patient node";
    return;
  }

  vtkMRMLRTCabin26AIonBeamNode* fixedReferenceNode = vtkMRMLRTCabin26AIonBeamNode::SafeDownCast(d->MRMLNodeComboBox_FixedReferenceBeam->currentNode());
  if (!fixedReferenceNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid fixed reference node";
    return;
  }
  // Get RAS->FixedReference Transform
  vtkMRMLLinearTransformNode* rasToFixedReferenceTransformNode = d->cabin26ARobotsLogic()->GetFixedReferenceTransform();

  if (!rasToFixedReferenceTransformNode)
  {
    qDebug() << Q_FUNC_INFO << "GetTableTopToPatientBeamTransform: Invalid TableTop->RAS transform node";
    return;
  }
  // Patient beam->RAS node transform
  vtkMRMLTransformNode* patientBeamTransformNode = patientBeamNode->GetParentTransformNode();

  vtkNew< vtkMatrix4x4 > transformMatrix;
  // From FixedReference->RAS to RAS->PatientBeam transform
  // rasToFixedReferenceTransformNode (FixedReference->RAS) - Source
  // patientBeamTransformNode (RAS->PatientBeam) - Target
  // Source -> Target transform
  if (!vtkMRMLTransformNode::GetMatrixTransformBetweenNodes( rasToFixedReferenceTransformNode, patientBeamTransformNode, transformMatrix))
  {
    qDebug() << Q_FUNC_INFO << "Unable calculate transform between patient beam and fixed reference nodes";
    return;
  }

  // TableTop -> PatientBeam
  double tableTopUnityX[4] = { 1., 0., 0., 0. };
  double tableTopUnityY[4] = { 0., 1., 0., 0. };
  double tableTopUnityZ[4] = { 0., 0., 1., 0. };
  double tableTopUnityXInPatientBeam[4] = {};
  double tableTopUnityYInPatientBeam[4] = {};
  double tableTopUnityZInPatientBeam[4] = {};
  transformMatrix->MultiplyPoint( tableTopUnityX, tableTopUnityXInPatientBeam);
  transformMatrix->MultiplyPoint( tableTopUnityY, tableTopUnityYInPatientBeam);
  transformMatrix->MultiplyPoint( tableTopUnityZ, tableTopUnityZInPatientBeam);

  double longitudinalAngle = vtkMath::DegreesFromRadians(acos(tableTopUnityXInPatientBeam[0])) - 90.;
  double lateralAngle = vtkMath::DegreesFromRadians(acos(tableTopUnityZInPatientBeam[0])) - 90.;
  double verticalAngle = vtkMath::DegreesFromRadians(acos(tableTopUnityZInPatientBeam[2])) - 90;

  qDebug() << Q_FUNC_INFO << "Lateral: " << lateralAngle  << ", Longitudinal: " << longitudinalAngle << ", Vertical: " << verticalAngle;

  d->FixedBeamAxisWidget->setTableTopAngles(lateralAngle, longitudinalAngle, verticalAngle);
  double a[6] = {};
  parameterNode->GetCabin26AGeometryNode()->GetTableTopRobotAngles(a);

  parameterNode->GetCabin26AGeometryNode()->DisableModifiedEventOn();
  if (!parameterNode->GetCabin26AGeometryNode()->GetPatientHeadFeetRotation() && patientBeamNode->GetGantryAngle() <= 180. && patientBeamNode->GetCouchAngle() <= 90.)
  {
    qDebug() << Q_FUNC_INFO << "1";
    double v = d->SliderWidget_TableRobotA1->value();
    v -= std::abs(verticalAngle);
    d->SliderWidget_TableRobotA1->setValue(v);
    a[0] = v;

    double longitudinal = d->SliderWidget_TableRobotA5->value();
    if (longitudinalAngle < 0.)
    {
      longitudinal -= longitudinalAngle;
    }
    else
    {
      longitudinal += longitudinalAngle;
    }
    d->SliderWidget_TableRobotA5->setValue(longitudinal);
    a[4] = 90. + longitudinal;

    double lateral = d->SliderWidget_TableRobotA4->value();
    if (lateralAngle > 0.)
    {
      lateral += lateralAngle;
    }
    else
    {
      lateral -= lateralAngle;
    }
    d->SliderWidget_TableRobotA4->setValue(lateral);
    a[3] = lateral;
  }
  else if (parameterNode->GetCabin26AGeometryNode()->GetPatientHeadFeetRotation() && patientBeamNode->GetGantryAngle() <= 180. && patientBeamNode->GetCouchAngle() <= 90.)
  {
    qDebug() << Q_FUNC_INFO << "2";
    double vertical = d->SliderWidget_TableRobotA1->value();
    if (verticalAngle < 0.)
    {
      vertical += 180. - std::abs(verticalAngle);
    }
    else
    {
      vertical -= std::abs(verticalAngle);
    }
    d->SliderWidget_TableRobotA1->setValue(vertical);
    a[0] = vertical;

    double longitudinal = d->SliderWidget_TableRobotA5->value();
    if (longitudinalAngle < 0.)
    {
      longitudinal += longitudinalAngle;
    }
    else
    {
      longitudinal -= longitudinalAngle;
    }
    d->SliderWidget_TableRobotA5->setValue(longitudinal);
    a[4] = 90. + longitudinal;

    double lateral = d->SliderWidget_TableRobotA4->value();
    if (lateralAngle > 0.)
    {
      lateral -= lateralAngle;
    }
    else
    {
      lateral += lateralAngle;
    }
    d->SliderWidget_TableRobotA4->setValue(lateral);
    a[3] = lateral;
  }
  else if (!parameterNode->GetCabin26AGeometryNode()->GetPatientHeadFeetRotation() && patientBeamNode->GetGantryAngle() > 180. && patientBeamNode->GetCouchAngle() <= 90.)
  {
    qDebug() << Q_FUNC_INFO << "3";
    double v = d->SliderWidget_TableRobotA1->value();
    v += 180. - std::abs(verticalAngle);
    d->SliderWidget_TableRobotA1->setValue(v);
    a[0] = v;

    double longitudinal = d->SliderWidget_TableRobotA5->value();
    if (longitudinalAngle < 0.)
    {
      longitudinal -= longitudinalAngle;
    }
    else
    {
      longitudinal += longitudinalAngle;
    }
    d->SliderWidget_TableRobotA5->setValue(longitudinal);
    a[4] = 90. + longitudinal;

    double lateral = d->SliderWidget_TableRobotA4->value();
    if (lateralAngle >= 0.)
    {
      lateral += lateralAngle;
    }
    else
    {
      lateral -= lateralAngle;
    }
    d->SliderWidget_TableRobotA4->setValue(lateral);
    a[3] = lateral;
  }
  else if (parameterNode->GetCabin26AGeometryNode()->GetPatientHeadFeetRotation() && patientBeamNode->GetGantryAngle() > 180. && patientBeamNode->GetCouchAngle() <= 90.)
  {
    qDebug() << Q_FUNC_INFO << "4";
    double v = d->SliderWidget_TableRobotA1->value();
    v -= std::abs(verticalAngle);
    d->SliderWidget_TableRobotA1->setValue(v);
    a[0] = v;
    double longitudinal = d->SliderWidget_TableRobotA5->value();
    if (longitudinalAngle < 0.)
    {
      longitudinal += longitudinalAngle;
    }
    else
    {
      longitudinal -= longitudinalAngle;
    }
    d->SliderWidget_TableRobotA5->setValue(longitudinal);
    a[4] = 90. + longitudinal;

    double lateral = d->SliderWidget_TableRobotA4->value();
    if (lateralAngle >= 0.)
    {
      lateral -= lateralAngle;
    }
    else
    {
      lateral += lateralAngle;
    }
    d->SliderWidget_TableRobotA4->setValue(lateral);
    a[3] = lateral;
  }
  else if (!parameterNode->GetCabin26AGeometryNode()->GetPatientHeadFeetRotation() && patientBeamNode->GetGantryAngle() <= 180. && patientBeamNode->GetCouchAngle() > 90.)
  {
    qDebug() << Q_FUNC_INFO << "5";
    double vertical = d->SliderWidget_TableRobotA1->value();
    if (vertical >= -90. && vertical <= 90.)
    {
      vertical += -180. + std::abs(verticalAngle);
    }
    else
    {
      vertical -= std::abs(verticalAngle);
    }
    d->SliderWidget_TableRobotA1->setValue(vertical);
    a[0] = vertical;

    double longitudinal = d->SliderWidget_TableRobotA5->value();
    if (longitudinalAngle < 0.)
    {
      longitudinal += longitudinalAngle;
    }
    else
    {
      longitudinal -= longitudinalAngle;
    }
    d->SliderWidget_TableRobotA5->setValue(longitudinal);
    a[4] = 90. + longitudinal;

    double lateral = d->SliderWidget_TableRobotA4->value();
    if (lateralAngle >= 0.)
    {
      lateral += lateralAngle;
    }
    else
    {
      lateral -= lateralAngle;
    }
    d->SliderWidget_TableRobotA4->setValue(lateral);
    a[3] = lateral;
  }
  else if (parameterNode->GetCabin26AGeometryNode()->GetPatientHeadFeetRotation() && patientBeamNode->GetGantryAngle() <= 180. && patientBeamNode->GetCouchAngle() > 90.)
  {
    qDebug() << Q_FUNC_INFO << "6";
    double vertical = d->SliderWidget_TableRobotA1->value();
    if (vertical > 0.)
    {
      vertical -= std::abs(verticalAngle);
    }
    else
    {
      vertical += std::abs(verticalAngle);
    }
    d->SliderWidget_TableRobotA1->setValue(vertical);
    a[0] = vertical;

    double longitudinal = d->SliderWidget_TableRobotA5->value();
    if (longitudinalAngle < 0.)
    {
      longitudinal -= longitudinalAngle;
    }
    else
    {
      longitudinal += longitudinalAngle;
    }
    d->SliderWidget_TableRobotA5->setValue(longitudinal);
    a[4] = 90. + longitudinal;

    double lateral = d->SliderWidget_TableRobotA4->value();
    if (lateralAngle >= 0.)
    {
      lateral -= lateralAngle;
    }
    else
    {
      lateral += lateralAngle;
    }
    d->SliderWidget_TableRobotA4->setValue(lateral);
    a[3] = lateral;
  }
  else if (!parameterNode->GetCabin26AGeometryNode()->GetPatientHeadFeetRotation() && patientBeamNode->GetGantryAngle() > 180. && patientBeamNode->GetCouchAngle() > 90.)
  {
    qDebug() << Q_FUNC_INFO << "7";
    double v = d->SliderWidget_TableRobotA1->value();
    if (verticalAngle > 0.)
    {
      v -= std::abs(verticalAngle);
    }
    else
    {
      v += std::abs(verticalAngle);
    }
    d->SliderWidget_TableRobotA1->setValue(v);
    a[0] = v;

    double longitudinal = d->SliderWidget_TableRobotA5->value();
    if (longitudinalAngle < 0.)
    {
      longitudinal += longitudinalAngle;
    }
    else
    {
      longitudinal -= longitudinalAngle;
    }
    d->SliderWidget_TableRobotA5->setValue(longitudinal);
    a[4] = 90. + longitudinal;

    double lateral = d->SliderWidget_TableRobotA4->value();
    if (lateralAngle >= 0.)
    {
      lateral += lateralAngle;
    }
    else
    {
      lateral -= lateralAngle;
    }
    d->SliderWidget_TableRobotA4->setValue(lateral);
    a[3] = lateral;
  }
  else if (parameterNode->GetCabin26AGeometryNode()->GetPatientHeadFeetRotation() && patientBeamNode->GetGantryAngle() > 180. && patientBeamNode->GetCouchAngle() > 90.)
  {
    qDebug() << Q_FUNC_INFO << "8";
    double vertical = d->SliderWidget_TableRobotA1->value();
    if (vertical > 0.)
    {
      vertical += -180. - std::abs(verticalAngle);
    }
    else
    {
      vertical += -180. + std::abs(verticalAngle);
    }
    d->SliderWidget_TableRobotA1->setValue(vertical);
    a[0] = vertical;

    double longitudinal = d->SliderWidget_TableRobotA5->value();
    if (longitudinalAngle < 0.)
    {
      longitudinal -= longitudinalAngle;
    }
    else
    {
      longitudinal += longitudinalAngle;
    }
    d->SliderWidget_TableRobotA5->setValue(longitudinal);
    a[4] = 90. + longitudinal;

    double lateral = d->SliderWidget_TableRobotA4->value();
    if (lateralAngle >= 0.)
    {
      lateral -= lateralAngle;
    }
    else
    {
      lateral += lateralAngle;
    }
    d->SliderWidget_TableRobotA4->setValue(lateral);
    a[3] = lateral;
  }
  parameterNode->GetCabin26AGeometryNode()->SetTableTopRobotAngles(a);
  parameterNode->DisableModifiedEventOff();
  parameterNode->Modified();

  d->SliderWidget_Zt->setValue(-1. * d->SliderWidget_TableRobotA1->value());
  d->SliderWidget_Xt->setValue(-1. * d->SliderWidget_TableRobotA4->value());
  d->SliderWidget_Yt->setValue(90. + d->SliderWidget_TableRobotA5->value());
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
  d->FixedBeamAxisWidget->setPatientPositioningLogic(d->logic());

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
//  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  qDebug() << Q_FUNC_INFO << "Update";
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

  using SysCoord = vtkSlicerCabin26ARobotsTransformLogic::CoordinateSystemIdentifier;
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
  if (!treatmentMachineType.compare("Cabin26AGeometry"))
  {
    qDebug() << Q_FUNC_INFO << "Cabin26A";
    d->ParameterNode->SetTreatmentMachineType("Cabin26AGeometry");
  }
  else if (!treatmentMachineType.compare("Cabin27A"))
  {
    qDebug() << Q_FUNC_INFO << "Cabin27A";
    d->ParameterNode->SetTreatmentMachineType("Cabin27AGeometry");
  }
  else if (!treatmentMachineType.compare("Cabin27B"))
  {
    qDebug() << Q_FUNC_INFO << "Cabin27B";
    d->ParameterNode->SetTreatmentMachineType("Cabin27BGeometry");
  }
  else if (!treatmentMachineType.compare("Cabin27C"))
  {
    qDebug() << Q_FUNC_INFO << "Cabin27C";
    d->ParameterNode->SetTreatmentMachineType("Cabin27CGeometry");
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
  // Create a new vtkMRMLCabin26AGeometryNode;
  // Set and observe created node in PatientPositioningNode
  if (!scene->GetFirstNodeByClass("vtkMRMLCabin26AGeometryNode"))
  {
    vtkNew<vtkMRMLCabin26AGeometryNode> cabin26AGeometryNode;
    cabin26AGeometryNode->SetName("Cabin26AGeometry");
    cabin26AGeometryNode->SetHideFromEditors(0);
    cabin26AGeometryNode->SetSingletonTag("Cabin26AGeo");
    scene->AddNode(cabin26AGeometryNode);
    d->ParameterNode->SetAndObserveCabin26AGeometryNode(cabin26AGeometryNode.GetPointer());
  }
  else
  {
    vtkMRMLCabin26AGeometryNode* cabin26AGeometryNode = vtkMRMLCabin26AGeometryNode::SafeDownCast(
      scene->GetFirstNodeByClass("vtkMRMLCabin26AGeometryNode"));
    d->ParameterNode->SetAndObserveCabin26AGeometryNode(cabin26AGeometryNode);
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
  // Load and setup models
  std::vector< SysCoord > loadedParts;
  vtkMRMLCabin26AGeometryNode* cabin26AGeometryNode = d->ParameterNode->GetCabin26AGeometryNode();
  if (cabin26AGeometryNode)
  {
    d->getLayoutManager()->pauseRender();
    loadedParts = d->logic()->LoadTreatmentMachineComponents(d->ParameterNode);
    d->getLayoutManager()->resumeRender();

    // Fixed and external beam
    vtkMRMLRTFixedBeamNode* xrayNode = d->logic()->CreateExternalXrayPlanAndNode(d->ParameterNode);
    vtkMRMLRTCabin26AIonBeamNode* fixedBeamNode = d->logic()->CreateFixedBeamPlanAndNode(d->ParameterNode);

    d->MRMLNodeComboBox_ExternalXrayBeam->setCurrentNode(xrayNode);
    d->MRMLNodeComboBox_FixedReferenceBeam->setCurrentNode(fixedBeamNode);
  }

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

  // Enable treatment machine geometry controls
  d->CollapsibleButton_CollisionDetection->setEnabled(true);
  d->CollapsibleButton_PatientTableTopControl->setEnabled(true);
  d->CollapsibleButton_BaseFixedControl->setEnabled(true);
  d->CollapsibleButton_TableTopControl->setEnabled(true);
  d->CollapsibleButton_TableTopRobotControl->setEnabled(true);
  d->CollapsibleButton_XrayCarmRobotControl->setEnabled(true);
  d->FixedBeamAxisWidget->setEnabled(true);
  d->PushButton_AlignBeams->setEnabled(true);

  // Reset camera
/*
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  qMRMLThreeDView* threeDView = layoutManager->threeDWidget(0)->threeDView();
  threeDView->resetCamera();
*/

  vtkSlicerCabin26ARobotsTransformLogic* cabin26ARobotsLogic = d->cabin26ARobotsLogic();
  if (cabin26ARobotsLogic && cabin26AGeometryNode)
  {
    d->getLayoutManager()->pauseRender();
    cabin26ARobotsLogic->ResetToInitialPositions();
    cabin26ARobotsLogic->UpdatePatientToTableTopTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateTableTopToFlangeTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateFlangeToWristTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateWristToElbowTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateElbowToShoulderTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateShoulderToBaseRotationTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateBaseRotationToBaseFixedTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateBaseFixedToFixedReferenceTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmBaseFixedToFixedReferenceTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmBaseRotationToCArmBaseFixedTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmShoulderToCArmBaseRotationTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmElbowToCArmShoulderTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmWristToCArmElbowTransform(cabin26AGeometryNode);

    d->getLayoutManager()->resumeRender();
  }
  // Update table top robot geometry
  cabin26AGeometryNode->Modified();
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
  vtkMRMLCabin26AGeometryNode* cabin26AGeometryNode = d->ParameterNode->GetCabin26AGeometryNode();

  // Transform RAS translation to LPS
  double positionTmp[3] = { -1. * position[0], -1. * position[1], position[2] };

  d->getLayoutManager()->pauseRender();
  cabin26AGeometryNode->DisableModifiedEventOn();
  cabin26AGeometryNode->SetPatientToTableTopTranslation(positionTmp);

  vtkSlicerCabin26ARobotsTransformLogic* cabin26ARobotsLogic = d->cabin26ARobotsLogic();
  if (cabin26ARobotsLogic && cabin26AGeometryNode)
  {
    cabin26ARobotsLogic->UpdatePatientToTableTopTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateTableTopToFlangeTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateFlangeToWristTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateWristToElbowTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateElbowToShoulderTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateShoulderToBaseRotationTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateBaseRotationToBaseFixedTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateBaseFixedToFixedReferenceTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmBaseFixedToFixedReferenceTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmBaseRotationToCArmBaseFixedTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmShoulderToCArmBaseRotationTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmElbowToCArmShoulderTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmWristToCArmElbowTransform(cabin26AGeometryNode);
  }
  cabin26AGeometryNode->DisableModifiedEventOff();
  cabin26AGeometryNode->Modified();
  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onTableTopRobotA1Changed(double a1)
{
  Q_D(qSlicerPatientPositioningModuleWidget);
//  int currentMachineIndex = d->ComboBox_TreatmentMachine->currentIndex();

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  vtkMRMLCabin26AGeometryNode* cabin26AGeometryNode = d->ParameterNode->GetCabin26AGeometryNode();

  d->getLayoutManager()->pauseRender();
//  qCritical() << Q_FUNC_INFO << ": Angle A1 " << a1;
  double a[6] = {};
  cabin26AGeometryNode->GetTableTopRobotAngles(a);
  cabin26AGeometryNode->DisableModifiedEventOn();
//  qDebug() << "Angles before: " << a[0] << ' ' << a[1] << ' ' << a[2] << ' ' << a[3] << ' ' << a[4] << ' ' << a[5];
  a[0] = a1;
  cabin26AGeometryNode->SetTableTopRobotAngles(a);
  cabin26AGeometryNode->DisableModifiedEventOff();

//  qDebug() << "Angles after: " << a[0] << ' ' << a[1] << ' ' << a[2] << ' ' << a[3] << ' ' << a[4] << ' ' << a[5];

  // Update IEC transform
  vtkSlicerCabin26ARobotsTransformLogic* cabin26ARobotsLogic = d->cabin26ARobotsLogic();
  if (cabin26ARobotsLogic && cabin26AGeometryNode)
  {
    cabin26ARobotsLogic->UpdateBaseRotationToBaseFixedTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateBaseFixedToFixedReferenceTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmBaseFixedToFixedReferenceTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmBaseRotationToCArmBaseFixedTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmShoulderToCArmBaseRotationTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmElbowToCArmShoulderTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmWristToCArmElbowTransform(cabin26AGeometryNode);
  }
  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
  cabin26AGeometryNode->Modified();
  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onCArmRobotA1Changed(double a1)
{
  Q_D(qSlicerPatientPositioningModuleWidget);
//  int currentMachineIndex = d->ComboBox_TreatmentMachine->currentIndex();

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  vtkMRMLCabin26AGeometryNode* cabin26AGeometryNode = d->ParameterNode->GetCabin26AGeometryNode();

  d->getLayoutManager()->pauseRender();
  double a[6] = {};
  cabin26AGeometryNode->GetCArmRobotAngles(a);
  cabin26AGeometryNode->DisableModifiedEventOn();
  a[0] = a1;
  cabin26AGeometryNode->SetCArmRobotAngles(a);
  cabin26AGeometryNode->DisableModifiedEventOff();

  // Update IEC transform
  vtkSlicerCabin26ARobotsTransformLogic* cabin26ARobotsLogic = d->cabin26ARobotsLogic();
  if (cabin26ARobotsLogic && cabin26AGeometryNode)
  {
    cabin26ARobotsLogic->UpdateCArmBaseRotationToCArmBaseFixedTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmShoulderToCArmBaseRotationTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmElbowToCArmShoulderTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmWristToCArmElbowTransform(cabin26AGeometryNode);
  }
//  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
  cabin26AGeometryNode->Modified();
  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onCArmRobotA2Changed(double a2)
{
  Q_D(qSlicerPatientPositioningModuleWidget);
//  int currentMachineIndex = d->ComboBox_TreatmentMachine->currentIndex();

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  vtkMRMLCabin26AGeometryNode* cabin26AGeometryNode = d->ParameterNode->GetCabin26AGeometryNode();

  d->getLayoutManager()->pauseRender();
  double a[6] = {};
  cabin26AGeometryNode->GetCArmRobotAngles(a);
  cabin26AGeometryNode->DisableModifiedEventOn();
  a[1] = 90. + a2;
  cabin26AGeometryNode->SetCArmRobotAngles(a);
  cabin26AGeometryNode->DisableModifiedEventOff();

  // Update IEC transform
  vtkSlicerCabin26ARobotsTransformLogic* cabin26ARobotsLogic = d->cabin26ARobotsLogic();
  if (cabin26ARobotsLogic && cabin26AGeometryNode)
  {
    cabin26ARobotsLogic->UpdateCArmShoulderToCArmBaseRotationTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmElbowToCArmShoulderTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmWristToCArmElbowTransform(cabin26AGeometryNode);
  }
//  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
  cabin26AGeometryNode->Modified();
  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onCArmRobotA3Changed(double a3)
{
  Q_D(qSlicerPatientPositioningModuleWidget);
//  int currentMachineIndex = d->ComboBox_TreatmentMachine->currentIndex();

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  vtkMRMLCabin26AGeometryNode* cabin26AGeometryNode = d->ParameterNode->GetCabin26AGeometryNode();

  d->getLayoutManager()->pauseRender();
  double a[6] = {};
  cabin26AGeometryNode->GetCArmRobotAngles(a);
  cabin26AGeometryNode->DisableModifiedEventOn();
  a[2] = a3 - 90.;
  cabin26AGeometryNode->SetCArmRobotAngles(a);
  cabin26AGeometryNode->DisableModifiedEventOff();

  // Update IEC transform
  vtkSlicerCabin26ARobotsTransformLogic* cabin26ARobotsLogic = d->cabin26ARobotsLogic();
  if (cabin26ARobotsLogic && cabin26AGeometryNode)
  {
    cabin26ARobotsLogic->UpdateCArmElbowToCArmShoulderTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmWristToCArmElbowTransform(cabin26AGeometryNode);
  }
//  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
  cabin26AGeometryNode->Modified();
  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onCArmRobotA4Changed(double a4)
{
  Q_D(qSlicerPatientPositioningModuleWidget);
//  int currentMachineIndex = d->ComboBox_TreatmentMachine->currentIndex();

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  vtkMRMLCabin26AGeometryNode* cabin26AGeometryNode = d->ParameterNode->GetCabin26AGeometryNode();

  d->getLayoutManager()->pauseRender();
  double a[6] = {};
  cabin26AGeometryNode->GetCArmRobotAngles(a);
  cabin26AGeometryNode->DisableModifiedEventOn();
  a[3] = a4;
  cabin26AGeometryNode->SetCArmRobotAngles(a);
  cabin26AGeometryNode->DisableModifiedEventOff();

  // Update IEC transform
  vtkSlicerCabin26ARobotsTransformLogic* cabin26ARobotsLogic = d->cabin26ARobotsLogic();
  if (cabin26ARobotsLogic && cabin26AGeometryNode)
  {
    cabin26ARobotsLogic->UpdateCArmWristToCArmElbowTransform(cabin26AGeometryNode);
  }
//  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
  cabin26AGeometryNode->Modified();
  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onCArmRobotA5Changed(double a5)
{
  Q_D(qSlicerPatientPositioningModuleWidget);
//  int currentMachineIndex = d->ComboBox_TreatmentMachine->currentIndex();

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  vtkMRMLCabin26AGeometryNode* cabin26AGeometryNode = d->ParameterNode->GetCabin26AGeometryNode();

  d->getLayoutManager()->pauseRender();
  double a[6] = {};
  cabin26AGeometryNode->GetCArmRobotAngles(a);
  cabin26AGeometryNode->DisableModifiedEventOn();
  a[4] = a5;
  cabin26AGeometryNode->SetCArmRobotAngles(a);
  cabin26AGeometryNode->DisableModifiedEventOff();

  // Update IEC transform
  vtkSlicerCabin26ARobotsTransformLogic* cabin26ARobotsLogic = d->cabin26ARobotsLogic();
  if (cabin26ARobotsLogic && cabin26AGeometryNode)
  {
    cabin26ARobotsLogic->UpdateCArmWristToCArmElbowTransform(cabin26AGeometryNode);
  }
//  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
  cabin26AGeometryNode->Modified();
  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onTableTopRobotA2Changed(double a2)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter node is invalid!";
    return;
  }
  vtkMRMLCabin26AGeometryNode* cabin26AGeometryNode = d->ParameterNode->GetCabin26AGeometryNode();

  d->getLayoutManager()->pauseRender();
  double a[6] = {};
  cabin26AGeometryNode->GetTableTopRobotAngles(a);
  cabin26AGeometryNode->DisableModifiedEventOn();
  a[1] = 90. + a2;
  cabin26AGeometryNode->SetTableTopRobotAngles(a);
  cabin26AGeometryNode->DisableModifiedEventOff();

  // Update IEC transform
  vtkSlicerCabin26ARobotsTransformLogic* cabin26ARobotsLogic = d->cabin26ARobotsLogic();
  if (cabin26ARobotsLogic && cabin26AGeometryNode)
  {
    cabin26ARobotsLogic->UpdateShoulderToBaseRotationTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateBaseRotationToBaseFixedTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateBaseFixedToFixedReferenceTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmBaseFixedToFixedReferenceTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmBaseRotationToCArmBaseFixedTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmShoulderToCArmBaseRotationTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmElbowToCArmShoulderTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmWristToCArmElbowTransform(cabin26AGeometryNode);
  }
  cabin26AGeometryNode->Modified();
  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onTableTopRobotA3Changed(double a3)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter node is invalid!";
    return;
  }
  vtkMRMLCabin26AGeometryNode* cabin26AGeometryNode = d->ParameterNode->GetCabin26AGeometryNode();

  d->getLayoutManager()->pauseRender();
  double a[6] = {};
  cabin26AGeometryNode->GetTableTopRobotAngles(a);
  cabin26AGeometryNode->DisableModifiedEventOn();
  a[2] = a3 - 90.;
  cabin26AGeometryNode->SetTableTopRobotAngles(a);
  cabin26AGeometryNode->DisableModifiedEventOff();

  // Update IEC transform
  vtkSlicerCabin26ARobotsTransformLogic* cabin26ARobotsLogic = d->cabin26ARobotsLogic();
  if (cabin26ARobotsLogic && cabin26AGeometryNode)
  {
    cabin26ARobotsLogic->UpdateElbowToShoulderTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateShoulderToBaseRotationTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateBaseRotationToBaseFixedTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateBaseFixedToFixedReferenceTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmBaseFixedToFixedReferenceTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmBaseRotationToCArmBaseFixedTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmShoulderToCArmBaseRotationTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmElbowToCArmShoulderTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmWristToCArmElbowTransform(cabin26AGeometryNode);
  }
  cabin26AGeometryNode->Modified();
  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onTableTopRobotA4Changed(double a4)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter node is invalid!";
    return;
  }
  vtkMRMLCabin26AGeometryNode* cabin26AGeometryNode = d->ParameterNode->GetCabin26AGeometryNode();

  d->getLayoutManager()->pauseRender();
  double a[6] = {};
  cabin26AGeometryNode->GetTableTopRobotAngles(a);
  cabin26AGeometryNode->DisableModifiedEventOn();
  a[3] = a4;
  cabin26AGeometryNode->SetTableTopRobotAngles(a);
  cabin26AGeometryNode->DisableModifiedEventOff();

  // Update IEC transform
  vtkSlicerCabin26ARobotsTransformLogic* cabin26ARobotsLogic = d->cabin26ARobotsLogic();
  if (cabin26ARobotsLogic && cabin26AGeometryNode)
  {
    cabin26ARobotsLogic->UpdateWristToElbowTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateElbowToShoulderTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateShoulderToBaseRotationTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateBaseRotationToBaseFixedTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateBaseFixedToFixedReferenceTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmBaseFixedToFixedReferenceTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmBaseRotationToCArmBaseFixedTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmShoulderToCArmBaseRotationTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmElbowToCArmShoulderTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmWristToCArmElbowTransform(cabin26AGeometryNode);
  }
  cabin26AGeometryNode->Modified();
  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onTableTopRobotA5Changed(double a5)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter node is invalid!";
    return;
  }
  vtkMRMLCabin26AGeometryNode* cabin26AGeometryNode = d->ParameterNode->GetCabin26AGeometryNode();

  d->getLayoutManager()->pauseRender();
  double a[6] = {};
  cabin26AGeometryNode->GetTableTopRobotAngles(a);
  cabin26AGeometryNode->DisableModifiedEventOn();
  a[4] = 90. + a5; // - a5;
  cabin26AGeometryNode->SetTableTopRobotAngles(a);
  cabin26AGeometryNode->DisableModifiedEventOff();

  // Update IEC transform
  vtkSlicerCabin26ARobotsTransformLogic* cabin26ARobotsLogic = d->cabin26ARobotsLogic();
  if (cabin26ARobotsLogic && cabin26AGeometryNode)
  {
    cabin26ARobotsLogic->UpdateWristToElbowTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateElbowToShoulderTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateShoulderToBaseRotationTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateBaseRotationToBaseFixedTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateBaseFixedToFixedReferenceTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmBaseFixedToFixedReferenceTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmBaseRotationToCArmBaseFixedTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmShoulderToCArmBaseRotationTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmElbowToCArmShoulderTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmWristToCArmElbowTransform(cabin26AGeometryNode);
  }
  cabin26AGeometryNode->Modified();
  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onTableTopRobotA6Changed(double a6)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter node is invalid!";
    return;
  }
  vtkMRMLCabin26AGeometryNode* cabin26AGeometryNode = d->ParameterNode->GetCabin26AGeometryNode();

  d->getLayoutManager()->pauseRender();
  double a[6] = {};
  cabin26AGeometryNode->GetTableTopRobotAngles(a);
  cabin26AGeometryNode->DisableModifiedEventOn();
  a[5] = a6;
  cabin26AGeometryNode->SetTableTopRobotAngles(a);
  cabin26AGeometryNode->DisableModifiedEventOff();

  // Update IEC transform
  vtkSlicerCabin26ARobotsTransformLogic* cabin26ARobotsLogic = d->cabin26ARobotsLogic();
  if (cabin26ARobotsLogic && cabin26AGeometryNode)
  {
    cabin26ARobotsLogic->UpdateFlangeToWristTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateWristToElbowTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateElbowToShoulderTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateShoulderToBaseRotationTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateBaseRotationToBaseFixedTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateBaseFixedToFixedReferenceTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmBaseFixedToFixedReferenceTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmBaseRotationToCArmBaseFixedTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmShoulderToCArmBaseRotationTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmElbowToCArmShoulderTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmWristToCArmElbowTransform(cabin26AGeometryNode);
  }
  cabin26AGeometryNode->Modified();
  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onTableTopRobotAnglesChanged(double* a)
{
  Q_UNUSED(a);
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onPatientSupportRotationAngleChanged(double angle)
{
  Q_UNUSED(angle);
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onBaseFixedToFixedReferenceTranslationChanged(double* position)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter node is invalid!";
    return;
  }
  vtkMRMLCabin26AGeometryNode* cabin26AGeometryNode = d->ParameterNode->GetCabin26AGeometryNode();

  d->getLayoutManager()->pauseRender();
  cabin26AGeometryNode->DisableModifiedEventOn();
  cabin26AGeometryNode->SetBaseFixedToFixedReferenceTranslation(position);
  cabin26AGeometryNode->DisableModifiedEventOff();
  vtkSlicerCabin26ARobotsTransformLogic* cabin26ARobotsLogic = d->cabin26ARobotsLogic();
  if (cabin26ARobotsLogic && cabin26AGeometryNode)
  {
    cabin26ARobotsLogic->UpdateBaseFixedToFixedReferenceTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmBaseFixedToFixedReferenceTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmBaseRotationToCArmBaseFixedTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmShoulderToCArmBaseRotationTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmElbowToCArmShoulderTransform(cabin26AGeometryNode);
    cabin26ARobotsLogic->UpdateCArmWristToCArmElbowTransform(cabin26AGeometryNode);
  }
  cabin26AGeometryNode->Modified();
  this->checkForCollisions();
  d->getLayoutManager()->resumeRender();
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
  vtkMRMLCabin26AGeometryNode* cabin26AGeometryNode = d->ParameterNode->GetCabin26AGeometryNode();
  d->getLayoutManager()->pauseRender();
  cabin26AGeometryNode->SetPatientHeadFeetRotation(toggled);
  d->getLayoutManager()->resumeRender();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onFixedReferenceCameraToggled(bool toggled)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  vtkMRMLCameraNode* cameraNode = d->get3DViewCameraNode();

  // Get RAS -> FixedReference transform node
  vtkMRMLLinearTransformNode* node = d->logic()->GetCabin26ARobotsTransformLogic()->GetFixedReferenceTransform();
  if (toggled)
  {
    vtkNew<vtkMatrix4x4> rasToFixedReferenceToRasTransform;
    if (node)
    {
      node->GetMatrixTransformToParent(rasToFixedReferenceToRasTransform);
    }
    cameraNode->SetAppliedTransform(rasToFixedReferenceToRasTransform);
    cameraNode->SetAndObserveTransformNodeID(node ? node->GetID() : nullptr);
    return;
  }
  cameraNode->SetAndObserveTransformNodeID(nullptr);
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onCollisionDetectionToggled(bool toggled)
{
  Q_D(qSlicerPatientPositioningModuleWidget);
  Q_UNUSED(toggled);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter node is invalid!";
    return;
  }

  this->checkForCollisions();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::checkForCollisions()
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  if (!d->CheckBox_ForceCollisionDetectionUpdate->isChecked())
  {
    d->CollisionDetectionStatusLabel->setText(tr("Collision detection is disabled"));
    d->CollisionDetectionStatusLabel->setStyleSheet("color: gray");
    return;
  }

  if (!d->ParameterNode || !d->ParameterNode->GetCabin26AGeometryNode() || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter nodes are invalid!";
    return;
  }
  vtkMRMLCabin26AGeometryNode* cabin26AGeometryNode = d->ParameterNode->GetCabin26AGeometryNode();

  d->CollisionDetectionStatusLabel->setText(QString::fromStdString("Calculating collisions..."));
  d->CollisionDetectionStatusLabel->setStyleSheet("color: black");
  QApplication::processEvents();

  std::string collisionString = d->logic()->CheckForCollisions(d->ParameterNode, cabin26AGeometryNode->GetCollisionDetectionEnabled());

  if (collisionString.length() > 0)
  {
    d->CollisionDetectionStatusLabel->setText(QString::fromStdString(collisionString));
    d->CollisionDetectionStatusLabel->setStyleSheet("color: red");
  }
  else
  {
    QString noCollisionsMessage(tr("No collisions detected"));
    if (d->logic()->GetTableTopElbowCollisionDetection()->GetInputData(0) == nullptr
     || d->logic()->GetTableTopShoulderCollisionDetection()->GetInputData(0) == nullptr
     || d->logic()->GetTableTopBaseRotationCollisionDetection()->GetInputData(0) == nullptr
     || d->logic()->GetTableTopBaseFixedCollisionDetection()->GetInputData(0) == nullptr
     || d->logic()->GetTableTopFixedReferenceCollisionDetection()->GetInputData(0) == nullptr)
    {
      noCollisionsMessage.append(tr(" (excluding certain parts)"));
    }
    d->CollisionDetectionStatusLabel->setText(noCollisionsMessage);
    d->CollisionDetectionStatusLabel->setStyleSheet("color: green");
  }
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onBeamsEyeViewOrientationChanged(const std::array< double, 3 >& viewUpVector)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  vtkMRMLCameraNode* cameraNode = d->get3DViewCameraNode();
  if (!cameraNode)
  {
    return;
  }

  vtkMRMLRTCabin26AIonBeamNode* fixedReferenceNode = vtkMRMLRTCabin26AIonBeamNode::SafeDownCast(d->MRMLNodeComboBox_FixedReferenceBeam->currentNode());
  vtkMRMLRTIonBeamNode* beamNode = vtkMRMLRTIonBeamNode::SafeDownCast(d->MRMLNodeComboBox_Beam->currentNode());
  double sourcePosition[3] = {};
  double isocenter[3] = {};
  qDebug() << Q_FUNC_INFO << viewUpVector[0] << ' ' << viewUpVector[1] << ' ' << viewUpVector[2];

  if (fixedReferenceNode && fixedReferenceNode->GetSourcePosition(sourcePosition))
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
      return;
    }

    double vupOrig[4] = { viewUpVector[0], viewUpVector[1], viewUpVector[2], 0. };
    double vup[4];
    if (beamTransform)
    {
      mat->MultiplyPoint( vupOrig, vup);
      qDebug() << Q_FUNC_INFO << "VUP in FixedReference: " << vup[0] << ' ' << vup[1] << ' ' << vup[2];
    }
    else
    {
      return;
    }

    cameraNode->GetCamera()->SetPosition(sourcePosition);
    if (fixedReferenceNode->GetPlanIsocenterPositionWorld(isocenter))
    {
      cameraNode->GetCamera()->SetFocalPoint(isocenter);
    }
    cameraNode->SetViewUp(vup);
  }
  cameraNode->GetCamera()->Elevation(0.);
}
