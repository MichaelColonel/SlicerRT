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
#include "qSlicerPatientPositioningModuleWidget.h"
#include "ui_qSlicerPatientPositioningModuleWidget.h"

#include <qSlicerLayoutManager.h>
#include <qSlicerApplication.h>
#include <qMRMLSliceWidget.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLPatientPositioningNode.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLScalarVolumeNode.h>

// Logic includes
#include <vtkSlicerPatientPositioningLogic.h>

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

  // Nodes
  connect( d->MRMLNodeComboBox_ParameterSet, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onParameterNodeChanged(vtkMRMLNode*)));
  connect( d->MRMLNodeComboBox_DRR, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    this, SLOT(onDrrNodeChanged(vtkMRMLNode*)));
  connect( d->MRMLNodeComboBox_Xray, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    this, SLOT(onXrayImageNodeChanged(vtkMRMLNode*)));

  // Buttons
  connect( d->PushButton_SetImagesToSliceView, SIGNAL(clicked()), this, SLOT(onSetImagesToSliceViewClicked()));
  connect( d->ButtonGroup_XrayProjection, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(onXrayProjectionButtonGroupChanged(QAbstractButton*)));
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

  // Set parameter node to children widgets (PlastimatchParameters)
  d->FooBar->setParameterNode(node);
 
  // Each time the node is modified, the UI widgets are updated
  qvtkReconnect( parameterNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));

  // Set selected MRML nodes in comboboxes in the parameter set if it was nullptr there
  // (then in the meantime the comboboxes selected the first one from the scene and we have to set that)
  if (parameterNode)
  {
  }
  this->updateWidgetFromMRML();
}

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
    d->logic()->SetXrayImagesProjection( parameterNode, projType, sliceCompositeNode);
    layoutManager->ResetSliceViews();
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

  // All required data for GUI is initiated
  this->updateWidgetFromMRML();
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
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
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
void qSlicerPatientPositioningModuleWidget::onXrayProjectionButtonGroupChanged(QAbstractButton* but)
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

  QRadioButton* rButton = qobject_cast<QRadioButton*>(but);
  qMRMLSliceWidget* sliceWidget = nullptr;
  vtkMRMLPatientPositioningNode::XrayProjectionType projType = vtkMRMLPatientPositioningNode::XrayProjectionType_Last;

  if (rButton == d->RadioButton_Vertical)
  {
    qDebug() << Q_FUNC_INFO << ": Vertical";
    layoutManager->setLayout(vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView);
    sliceWidget = layoutManager->sliceWidget("Red");
    projType = vtkMRMLPatientPositioningNode::Vertical;
  }
  else if (rButton == d->RadioButton_Horizontal)
  {
    qDebug() << Q_FUNC_INFO << ": Horizontal";
    layoutManager->setLayout(vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView);
    sliceWidget = layoutManager->sliceWidget("Yellow");
    projType = vtkMRMLPatientPositioningNode::Horizontal;
  }
  else if (rButton == d->RadioButton_Angle)
  {
    qDebug() << Q_FUNC_INFO << ": Angle";
    layoutManager->setLayout(vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView);
    sliceWidget = layoutManager->sliceWidget("Green");
    projType = vtkMRMLPatientPositioningNode::Angle;
  }
  if (sliceWidget)
  {
    vtkMRMLSliceNode* sliceNode = sliceWidget->mrmlSliceNode();
    vtkMRMLSliceCompositeNode* sliceCompositeNode = sliceWidget->mrmlSliceCompositeNode();
    d->logic()->SetXrayImagesProjection( parameterNode, projType, sliceCompositeNode);
  }
}
