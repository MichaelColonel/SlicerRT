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
#include "qSlicerRtImageModuleWidget.h"
#include "ui_qSlicerRtImageModuleWidget.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLSubjectHierarchyNode.h>

// RTBeam includes
#include <vtkMRMLRTBeamNode.h>
#include <vtkMRMLRTIonBeamNode.h>

#include "vtkMRMLRTImageNode.h"

// Logic includes
#include "vtkSlicerRtImageLogic.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerRtImageModuleWidgetPrivate: public Ui_qSlicerRtImageModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerRtImageModuleWidget);
protected:
  qSlicerRtImageModuleWidget* const q_ptr;
public:
  qSlicerRtImageModuleWidgetPrivate(qSlicerRtImageModuleWidget &object);
  virtual ~qSlicerRtImageModuleWidgetPrivate();
  vtkSlicerRtImageLogic* logic() const;

  vtkMRMLRTImageNode* ParameterNode;
  vtkMRMLRTBeamNode* RtBeamNode;
  vtkMRMLScalarVolumeNode* CtVolumeNode;
  bool ModuleWindowInitialized;
};

//-----------------------------------------------------------------------------
// qSlicerRtImageModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerRtImageModuleWidgetPrivate::qSlicerRtImageModuleWidgetPrivate(qSlicerRtImageModuleWidget &object)
  :
  q_ptr(&object)
{
}

qSlicerRtImageModuleWidgetPrivate::~qSlicerRtImageModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
vtkSlicerRtImageLogic* qSlicerRtImageModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerRtImageModuleWidget);
  return vtkSlicerRtImageLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
// qSlicerRtImageModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerRtImageModuleWidget::qSlicerRtImageModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerRtImageModuleWidgetPrivate(*this) )
{
}

//-----------------------------------------------------------------------------
qSlicerRtImageModuleWidget::~qSlicerRtImageModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerRtImageModuleWidget::setup()
{
  Q_D(qSlicerRtImageModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  QStringList rtImageNodes;
  rtImageNodes.push_back("vtkMRMLRTImageNode");
  d->MRMLNodeComboBox_ParameterSet->setNodeTypes(rtImageNodes);

  QStringList rtBeamNodes;
  rtBeamNodes.push_back("vtkMRMLRTBeamNode");
  d->MRMLNodeComboBox_RtBeam->setNodeTypes(rtBeamNodes);

  QStringList volumeNodes;
  volumeNodes.push_back("vtkMRMLScalarVolumeNode");
  d->MRMLNodeComboBox_CtVolume->setNodeTypes(volumeNodes);
}

//-----------------------------------------------------------------------------
void qSlicerRtImageModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerRtImageModuleWidget);
  this->Superclass::setMRMLScene(scene);

  qvtkReconnect( d->logic(), scene, vtkMRMLScene::EndImportEvent, this, SLOT(onSceneImportedEvent()));

  // Find parameters node or create it if there is none in the scene
  if (scene)
  {
    vtkMRMLNode* node = scene->GetNthNodeByClass( 0, "vtkMRMLRTImageNode");
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
      vtkNew<vtkMRMLRTImageNode> newNode;
      this->mrmlScene()->AddNode(newNode);
      this->setParameterNode(newNode);
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerRtImageModuleWidget::setParameterNode(vtkMRMLNode *node)
{
  Q_D(qSlicerRtImageModuleWidget);

  d->ParameterNode = vtkMRMLRTImageNode::SafeDownCast(node);

  // Make sure the parameter set node is selected (in case the function was not called by the selector combobox signal)
  d->MRMLNodeComboBox_ParameterSet->setCurrentNode(d->ParameterNode);

  // Each time the node is modified, the UI widgets are updated
  qvtkReconnect( d->ParameterNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));
  
  // Set selected MRML nodes in comboboxes in the parameter set if it was nullptr there
  // (then in the meantime the comboboxes selected the first one from the scene and we have to set that)
  if (d->ParameterNode)
  {
    if (!d->ParameterNode->GetBeamNode())
    {
      vtkMRMLRTBeamNode* beamNode = vtkMRMLRTBeamNode::SafeDownCast(d->MRMLNodeComboBox_RtBeam->currentNode());
      qvtkConnect( beamNode, vtkMRMLRTBeamNode::BeamGeometryModified, this, SLOT(onUpdateImageWindowFromBeamJaws()));
      d->ParameterNode->SetAndObserveBeamNode(beamNode);
      d->ParameterNode->Modified();
    }
  }
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerRtImageModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerRtImageModuleWidget);

  d->ParameterNode = vtkMRMLRTImageNode::SafeDownCast(d->MRMLNodeComboBox_ParameterSet->currentNode());

  if (!this->mrmlScene())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid scene";
    return;
  }

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid RT Image node";
    return;
  }

  if (!d->ParameterNode->GetBeamNode())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid referenced parameter's beam node";
    return;
  }

  if (!d->CtVolumeNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid referenced volume node";
    return;
  }

  // Update widgets info from parameter node and update plastimatch drr command
  d->MRMLNodeComboBox_RtBeam->setCurrentNode(d->ParameterNode->GetBeamNode());
  d->SliderWidget_IsocenterImagerDistance->setValue(d->ParameterNode->GetIsocenterImagerDistance());
/*
  int imageDimInteger[2] = {};
  double imageDim[2] = {};
  d->ParameterNode->GetImageDimention(imageDimInteger);
  imageDim[0] = static_cast<double>(imageDimInteger[0]);
  imageDim[1] = static_cast<double>(imageDimInteger[1]);
  d->CoordinatesWidget_ImagePixelDimention->setCoordinates(imageDim);
  d->CoordinatesWidget_ImagePixelSpacing->setCoordinates(d->ParameterNode->GetImageSpacing());
    
  int imageWindowInteger[4] = {};
  double imageWindow[4] = {};
  d->ParameterNode->GetImageWindow(imageWindowInteger);
  imageWindow[0] = static_cast<double>(imageWindowInteger[0]);
  imageWindow[1] = static_cast<double>(imageWindowInteger[1]);
  imageWindow[2] = static_cast<double>(imageWindowInteger[2]);
  imageWindow[3] = static_cast<double>(imageWindowInteger[3]);
  d->CoordinatesWidget_ImageWindow->setCoordinates(imageWindow);

  d->CheckBox_UseImageWindow->setChecked(d->ParameterNode->GetImageWindowFlag());

  d->CheckBox_UseExponentialMapping->setChecked(d->ParameterNode->GetExponentialMappingFlag());
  d->CheckBox_AutoscalePixelsRange->setChecked(d->ParameterNode->GetAutoscaleFlag());

  switch (d->ParameterNode->GetAlgorithmReconstuction())
  {
    case vtkMRMLPlmDrrNode::AlgorithmReconstuctionType::EXACT:
      d->RadioButton_Exact->setChecked(true);
      break;
    case vtkMRMLPlmDrrNode::AlgorithmReconstuctionType::UNIFORM:
      d->RadioButton_Exact->setChecked(true);
      break;
    default:
      break;
  }

  switch (d->ParameterNode->GetHUConversion())
  {
    case vtkMRMLPlmDrrNode::HounsfieldUnitsConversionType::PREPROCESS:
      d->RadioButton_Preprocess->setChecked(true);
      break;
    case vtkMRMLPlmDrrNode::HounsfieldUnitsConversionType::INLINE:
      d->RadioButton_Inline->setChecked(true);
      break;
    case vtkMRMLPlmDrrNode::HounsfieldUnitsConversionType::NONE:
      d->RadioButton_None->setChecked(true);
      break;
    default:
      break;
  }

  switch (d->ParameterNode->GetThreading())
  {
    case vtkMRMLPlmDrrNode::ThreadingType::CPU:
      d->RadioButton_CPU->setChecked(true);
      break;
    case vtkMRMLPlmDrrNode::ThreadingType::CUDA:
      d->RadioButton_CUDA->setChecked(true);
      break;
    case vtkMRMLPlmDrrNode::ThreadingType::OPENCL:
      d->RadioButton_OpenCL->setChecked(true);
      break;
    default:
      break;
  }

  // Update DRR arguments
  this->onUpdatePlmDrrArgs();
*/
}

//-----------------------------------------------------------------------------
void qSlicerRtImageModuleWidget::onSceneImportedEvent()
{
  this->onEnter();
}

/// RTBeam Node (RTBeam or RTIonBeam) changed
void qSlicerRtImageModuleWidget::onRTBeamNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerRtImageModuleWidget);

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
  if (!beamNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid beam node";
    return;
  }

  vtkMRMLRTIonBeamNode* ionBeamNode = vtkMRMLRTIonBeamNode::SafeDownCast(node);
  Q_UNUSED(ionBeamNode);

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid RT Image node";
    return;
  }

  d->ParameterNode->DisableModifiedEventOn();
  d->ParameterNode->SetAndObserveBeamNode(beamNode);
  d->ParameterNode->DisableModifiedEventOff();

  // Update imager and image markups, DRR arguments
  d->logic()->UpdateMarkupsNodes(d->ParameterNode);
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerRtImageModuleWidget::enter()
{
  this->Superclass::enter();
  this->onEnter();
}

//-----------------------------------------------------------------------------
void qSlicerRtImageModuleWidget::exit()
{
  this->Superclass::exit();
  this->qvtkDisconnectAll();
}

//-----------------------------------------------------------------------------
void qSlicerRtImageModuleWidget::onCtVolumeNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerRtImageModuleWidget);
  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);
  if (!volumeNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid reference CT volume node";
    return;
  }

  d->CtVolumeNode = volumeNode;
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerRtImageModuleWidget::onParameterNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerRtImageModuleWidget);
  vtkMRMLRTImageNode* parameterNode = vtkMRMLRTImageNode::SafeDownCast(node);
  if (!parameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid RT Image node";
    return;
  }

  setParameterNode(parameterNode);
}

//-----------------------------------------------------------------------------
void qSlicerRtImageModuleWidget::onEnter()
{
  Q_D(qSlicerRtImageModuleWidget);

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

  if (!d->ParameterNode)
  {
    // Try to find one in the scene
    vtkMRMLNode* node = this->mrmlScene()->GetNthNodeByClass( 0, "vtkMRMLRTImageNode");
    if (node)
    {
      d->ParameterNode = vtkMRMLRTImageNode::SafeDownCast(node);
    }
    else 
    {
      qCritical() << Q_FUNC_INFO << ": Invalid RT Image node";
      return;
    }
  }

  if (!d->ParameterNode->GetBeamNode())
  {
    // Try to find one in the scene
    vtkMRMLNode* node = this->mrmlScene()->GetNthNodeByClass( 0, "vtkMRMLRTBeamNode");
    if (node)
    {
      d->RtBeamNode = vtkMRMLRTBeamNode::SafeDownCast(node);
      d->ParameterNode->SetAndObserveBeamNode(d->RtBeamNode);
    }
    else 
    {
      d->RtBeamNode = nullptr;
    }
  }

  d->logic()->CreateMarkupsNodes(d->ParameterNode);

  d->ModuleWindowInitialized = true;
  this->updateWidgetFromMRML();
}
