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
//  connect( d->MRMLNodeComboBox_CtVolume, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
//    this, SLOT(onCtVolumeNodeChanged(vtkMRMLNode*)));
  connect( d->MRMLNodeComboBox_ParameterSet, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onParameterNodeChanged(vtkMRMLNode*)));

  // Sliders, Coordinates widgets
  connect( d->SliderWidget_PatientSupportRotationAngle, SIGNAL(valueChanged(double)), 
    this, SLOT(onPatientSupportRotationAngleChanged(double)));
  connect( d->SliderWidget_TableTopMovementY, SIGNAL(valueChanged(double)), 
    this, SLOT(onLongitudinalTableTopDisplacementChanged(double)));
  connect( d->SliderWidget_TableTopMovementZ, SIGNAL(valueChanged(double)), 
    this, SLOT(onVerticalTableTopDisplacementChanged(double)));

  // Buttons
  connect( d->PushButton_LoadStandModels, SIGNAL(clicked()), this, SLOT(onLoadStandModelsButtonClicked()));
  connect( d->PushButton_ResetModelsInitialPosition, SIGNAL(clicked()), this, SLOT(onResetToInitialPositionButtonClicked()));
  connect( d->PushButton_MoveModelsToIsocenter, SIGNAL(clicked()), this, SLOT(onMoveModelsToIsocenter()));

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
//  QString treatmentMachineType(d->TreatmentMachineComboBox->currentData().toString());
//  paramNode->SetTreatmentMachineType(treatmentMachineType.toUtf8().constData());
  parameterNode->SetTreatmentMachineType("IHEPStand");
  d->logic()->LoadTreatmentMachineModels(parameterNode);

  // Reset camera
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  qMRMLThreeDView* threeDView = layoutManager->threeDWidget(0)->threeDView();
  threeDView->resetCamera();
/*
  // Set treatment machine dependent properties
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
*/
  // Set orientation marker
  //TODO: Add new option 'Treatment room' to orientation marker choices and merged model with actual colors (surface scalars?)
  //vtkMRMLViewNode* viewNode = threeDView->mrmlViewNode();
  //viewNode->SetOrientationMarkerHumanModelNodeID(this->mrmlScene()->GetFirstNodeByName("EBRTOrientationMarkerModel")->GetID());
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
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onPatientSupportRotationAngleChanged(double rotationAngle)
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
  parameterNode->SetPatientSupportRotationAngle(rotationAngle);
  parameterNode->DisableModifiedEventOff();

  d->logic()->UpdatePatientSupportRotationToFixedReferenceTransform( parameterNode, rotationAngle);
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onTableTopLongitudinalDisplacementChanged(double longitudinalDisplacement)
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
  parameterNode->SetTableTopLongitudinalDisplacement(longitudinalDisplacement);
  parameterNode->DisableModifiedEventOff();

  d->logic()->UpdateTableTopToTableTopEccentricRotationTransform(parameterNode);
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onTableTopVerticalDisplacementChanged(double verticalDisplacement)
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
  parameterNode->SetTableTopVerticalDisplacement(verticalDisplacement);
  parameterNode->DisableModifiedEventOff();

  d->logic()->UpdateTableTopToTableTopEccentricRotationTransform(parameterNode);
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onMoveModelsToIsocenter()
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

  double isocenter[3] = {};
  vtkMRMLRTBeamNode* beam = parameterNode->GetBeamNode();
  if (beam->GetPlanIsocenterPosition(isocenter))
  {
    d->logic()->MoveModelsToIsocenter( parameterNode, isocenter);
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::enter()
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);
  this->Superclass::enter();
  this->onEnter();
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::exit()
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);
  this->Superclass::exit();
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
  qDebug() << Q_FUNC_INFO << ": GUI initiated successfully";
/*
  if (!parameterNode->GetBeamNode())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid referenced parameter's beam node";
    return;
  }

  vtkMRMLScalarVolumeNode* ctVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(d->MRMLNodeComboBox_CtVolume->currentNode());
  if (!ctVolumeNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid referenced volume node";
    return;
  }

  // Update widgets info from parameter node
  d->MRMLNodeComboBox_RtBeam->setCurrentNode(parameterNode->GetBeamNode());
  d->SliderWidget_IsocenterImagerDistance->setValue(parameterNode->GetIsocenterImagerDistance());

  int imagerResolution[2] = {};
  double imagerRes[2] = {};
  parameterNode->GetImagerResolution(imagerResolution);
  imagerRes[0] = static_cast<double>(imagerResolution[0]);
  imagerRes[1] = static_cast<double>(imagerResolution[1]);
  d->CoordinatesWidget_ImagerResolution->setCoordinates(imagerRes);
  d->CoordinatesWidget_ImagerSpacing->setCoordinates(parameterNode->GetImagerSpacing());

  d->RangeWidget_ImageWindowColumns->setMinimum(0.);
  d->RangeWidget_ImageWindowColumns->setMaximum(double(imagerResolution[0] - 1));
  d->RangeWidget_ImageWindowRows->setMinimum(0.);
  d->RangeWidget_ImageWindowRows->setMaximum(double(imagerResolution[1] - 1));

  bool useImageWindow = parameterNode->GetImageWindowFlag();
  int imageWindow[4] = {};
  parameterNode->GetImageWindow(imageWindow);

  // TODO: Image window is disabled for now
  useImageWindow = false;
  d->CheckBox_UseImageWindow->setChecked(useImageWindow);
  if (!useImageWindow)
  {
    d->RangeWidget_ImageWindowColumns->setValues( 0., double(imagerResolution[0] - 1));
    d->RangeWidget_ImageWindowRows->setValues( 0., double(imagerResolution[1] - 1));
  }
  else
  {
    d->RangeWidget_ImageWindowColumns->setValues( 
      static_cast<double>(std::max<int>( 0, imageWindow[0])),
      static_cast<double>(std::min<int>( imagerResolution[0] - 1, imageWindow[2])));
    d->RangeWidget_ImageWindowRows->setValues( 
      static_cast<double>(std::max<int>( 0, imageWindow[1])), 
      static_cast<double>(std::min<int>( imagerResolution[1] - 1, imageWindow[3])));
  }
*/
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

  vtkMRMLRTIonBeamNode* ionBeamNode = vtkMRMLRTIonBeamNode::SafeDownCast(node);
  Q_UNUSED(ionBeamNode);

  vtkMRMLIhepStandGeometryNode* parameterNode = vtkMRMLIhepStandGeometryNode::SafeDownCast(d->MRMLNodeComboBox_ParameterSet->currentNode());
  if (!parameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  parameterNode->SetAndObserveBeamNode(beamNode);
  parameterNode->Modified(); // Update imager and image markups, DRR arguments in logic
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
