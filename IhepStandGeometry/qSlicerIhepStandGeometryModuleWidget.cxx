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
//  connect( d->MRMLNodeComboBox_RtBeam, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
//    this, SLOT(onRTBeamNodeChanged(vtkMRMLNode*)));
//  connect( d->MRMLNodeComboBox_CtVolume, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
//    this, SLOT(onCtVolumeNodeChanged(vtkMRMLNode*)));
  connect( d->MRMLNodeComboBox_ParameterSet, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onParameterNodeChanged(vtkMRMLNode*)));

  // Coordinates widgets

  // Buttons
  connect( d->pushButton_LoadStandModels, SIGNAL(clicked()), this, SLOT(onLoadStandModelsButtonClicked()));

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
void qSlicerIhepStandGeometryModuleWidget::enter()
{
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::exit()
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onEnter()
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);
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
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onSceneClosedEvent()
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModuleWidget::onSceneImportedEvent()
{
  Q_D(qSlicerIhepStandGeometryModuleWidget);
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
