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
#include <QRadioButton>

// Slicer includes
#include <qSlicerSingletonViewFactory.h>
#include <qSlicerLayoutManager.h>
#include <qSlicerApplication.h>

#include "qSlicerIhepMlcControlModuleWidget.h"
#include "ui_qSlicerIhepMlcControlModuleWidget.h"

#include "qSlicerIhepMlcControlLayoutWidget.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLLayoutLogic.h>

// SlicerRT MRML Beams includes
#include <vtkMRMLRTBeamNode.h>
#include <vtkMRMLRTIonBeamNode.h>

// SlicerRT MRML IhepMlcControl includes
#include "vtkMRMLIhepMlcControlNode.h"

// Logic includes
#include "vtkSlicerIhepMlcControlLogic.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerIhepMlcControlModuleWidgetPrivate: public Ui_qSlicerIhepMlcControlModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerIhepMlcControlModuleWidget);
protected:
  qSlicerIhepMlcControlModuleWidget* const q_ptr;
public:
  qSlicerIhepMlcControlModuleWidgetPrivate(qSlicerIhepMlcControlModuleWidget &object);
  virtual ~qSlicerIhepMlcControlModuleWidgetPrivate();
  vtkSlicerIhepMlcControlLogic* logic() const;

  qSlicerIhepMlcControlLayoutWidget* MlcControlWidget{ nullptr };
  int PreviousLayoutId{ 0 };
  int MlcCustomLayoutId{ 507 };
  vtkWeakPointer<vtkMRMLIhepMlcControlNode> ParameterNode;
};

//-----------------------------------------------------------------------------
// qSlicerIhepMlcControlModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerIhepMlcControlModuleWidgetPrivate::qSlicerIhepMlcControlModuleWidgetPrivate(qSlicerIhepMlcControlModuleWidget &object)
  :
  q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
qSlicerIhepMlcControlModuleWidgetPrivate::~qSlicerIhepMlcControlModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
vtkSlicerIhepMlcControlLogic* qSlicerIhepMlcControlModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerIhepMlcControlModuleWidget);
  return vtkSlicerIhepMlcControlLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
// qSlicerIhepMlcControlModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerIhepMlcControlModuleWidget::qSlicerIhepMlcControlModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerIhepMlcControlModuleWidgetPrivate(*this) )
{
}

//-----------------------------------------------------------------------------
qSlicerIhepMlcControlModuleWidget::~qSlicerIhepMlcControlModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::setup()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  d->MlcControlWidget = new qSlicerIhepMlcControlLayoutWidget;

  qSlicerSingletonViewFactory* viewFactory = new qSlicerSingletonViewFactory();
  viewFactory->setWidget(d->MlcControlWidget);
  viewFactory->setTagName("MlcControlLayout");

  const char* layoutString = \
    "<layout type=\"vertical\">" \
    " <item>" \
    "  <MlcControlLayout></MlcControlLayout>" \
    " </item>" \
    "</layout>";

  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  layoutManager->registerViewFactory(viewFactory);
  // Save previous layout
  d->PreviousLayoutId = layoutManager->layout();

  vtkMRMLLayoutNode* layoutNode = layoutManager->layoutLogic()->GetLayoutNode();
  if (layoutNode)
  {
    layoutNode->AddLayoutDescription( d->MlcCustomLayoutId, layoutString);
  }

  // Buttons
  QObject::connect( d->PushButton_SwitchLayout, SIGNAL(toggled(bool)),
    this, SLOT(onSwitchToMlcControlLayoutToggled(bool)));
  QObject::connect( d->CheckBox_ParallelBeam, SIGNAL(toggled(bool)),
    this, SLOT(onParallelBeamToggled(bool)));
  QObject::connect( d->ButtonGroup_MlcLayers, SIGNAL(buttonClicked(QAbstractButton*)),
    this, SLOT(onMlcLeayersButtonClicked(QAbstractButton*)));
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::exit()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  this->Superclass::exit();

  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  layoutManager->setLayout(d->PreviousLayoutId);
}


//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::enter()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  this->Superclass::enter();

  this->onEnter();

  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  d->PreviousLayoutId = layoutManager->layout();
//  layoutManager->setLayout(d->MlcCustomLayoutId);
//  QTimer::singleShot(100, this, SLOT(onSetMlcControlLayout()));
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onSwitchToMlcControlLayoutToggled(bool toggled)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  layoutManager->setLayout(toggled ? d->MlcCustomLayoutId : d->PreviousLayoutId);
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onSetMlcControlLayout()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  layoutManager->setLayout(d->MlcCustomLayoutId);
  QSignalBlocker blocker1(d->PushButton_SwitchLayout);
  d->PushButton_SwitchLayout->setChecked(true);
}


//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
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
    else if (vtkMRMLNode* node = scene->GetFirstNodeByClass("vtkMRMLIhepMlcControlNode"))
    {
      this->setParameterNode(node);
    }
    else
    {
      vtkNew<vtkMRMLIhepMlcControlNode> newNode;
      std::string nodeName = this->mrmlScene()->GenerateUniqueName("IHEP_MLC_VRBS");
      newNode->SetName(nodeName.c_str());
      newNode->SetSingletonTag("IHEP_MLC");
      this->mrmlScene()->AddNode(newNode);
      this->setParameterNode(newNode);
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::setParameterNode(vtkMRMLNode *node)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);

  vtkMRMLIhepMlcControlNode* parameterNode = vtkMRMLIhepMlcControlNode::SafeDownCast(node);

  // Make sure the parameter set node is selected (in case the function was not called by the selector combobox signal)
  d->MRMLNodeComboBox_ParameterSet->setCurrentNode(node);

  // Set parameter node to children widgets (MlcControlWidget)
  d->MlcControlWidget->setParameterNode(node);
 
  // Each time the node is modified, the UI widgets are updated
  qvtkReconnect( parameterNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));
  d->ParameterNode = parameterNode;

  // Set selected MRML nodes in comboboxes in the parameter set if it was nullptr there
  // (then in the meantime the comboboxes selected the first one from the scene and we have to set that)
  if (parameterNode)
  {
    if (!parameterNode->GetBeamNode())
    {
      vtkMRMLRTBeamNode* beamNode = vtkMRMLRTBeamNode::SafeDownCast(d->MRMLNodeComboBox_Beam->currentNode());
//      qvtkConnect( beamNode, vtkMRMLRTBeamNode::BeamTransformModified, this, SLOT(updateNormalAndVupVectors()));
      parameterNode->SetAndObserveBeamNode(beamNode);
    }
  }
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);

  vtkMRMLIhepMlcControlNode* parameterNode = vtkMRMLIhepMlcControlNode::SafeDownCast(d->MRMLNodeComboBox_ParameterSet->currentNode());

  if (!this->mrmlScene())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid scene";
    return;
  }

  // Enable widgets
//  d->CheckBox_ShowDrrMarkups->setEnabled(parameterNode);
//  d->CollapsibleButton_ReferenceInput->setEnabled(parameterNode);
//  d->CollapsibleButton_GeometryBasicParameters->setEnabled(parameterNode);
//  d->PlastimatchParametersWidget->setEnabled(parameterNode);
//  d->PushButton_ComputeDrr->setEnabled(parameterNode);
  
  if (!parameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  if (!parameterNode->GetBeamNode())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid referenced parameter's beam node";
    return;
  }
  
  d->CheckBox_ParallelBeam->setChecked(parameterNode->GetParallelBeam());
/*
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

  d->GroupBox_ImageWindowParameters->setChecked(useImageWindow);
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
  
  // update RT beam from camera button
  vtkMRMLCameraNode* cameraNode = vtkMRMLCameraNode::SafeDownCast(d->MRMLNodeComboBox_Camera->currentNode());
  d->PushButton_UpdateBeamFromCamera->setEnabled(cameraNode);
*/
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onSceneImportedEvent()
{
  this->onEnter();
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onSceneClosedEvent()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onEnter()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);

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

  vtkMRMLIhepMlcControlNode* parameterNode = nullptr; 
  // Try to find one in the scene
  if (vtkMRMLNode* node = this->mrmlScene()->GetFirstNodeByClass("vtkMRMLIhepMlcControlNode"))
  {
    parameterNode = vtkMRMLIhepMlcControlNode::SafeDownCast(node);
  }

///  if (parameterNode && parameterNode->GetBeamNode())
///  {
    // First thing first: update normal and vup vectors for parameter node
    // in case observed beam node transformation has been modified
///    d->logic()->UpdateNormalAndVupVectors(parameterNode);
///  }

  // Create DRR markups nodes
///  d->logic()->CreateMarkupsNodes(parameterNode);

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onParameterNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  vtkMRMLIhepMlcControlNode* parameterNode = vtkMRMLIhepMlcControlNode::SafeDownCast(node);

  if (!parameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  this->setParameterNode(parameterNode);
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onParallelBeamToggled(bool toggled)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  d->ParameterNode->SetParallelBeam(toggled);
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onMlcLeayersButtonClicked(QAbstractButton* button)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  QRadioButton* rButton = qobject_cast<QRadioButton*>(button);
  if (rButton && rButton == d->RadioButton_OneLayer)
  {
    d->ParameterNode->SetLayers(vtkMRMLIhepMlcControlNode::OneLayer);
  }
  else if (rButton && rButton == d->RadioButton_TwoLayers)
  {
    d->ParameterNode->SetLayers(vtkMRMLIhepMlcControlNode::TwoLayers);
  }
  else
  {
  }
}

