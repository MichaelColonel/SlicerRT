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
#include "qSlicerTestMe2ModuleWidget.h"
#include "ui_qSlicerTestMe2ModuleWidget.h"


#include <vtkMRMLScene.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLRTBeamNode.h>

#include <vtkMRMLSliceNode.h>
#include <qSlicerLayoutManager.h>
#include <qSlicerApplication.h>
#include <qMRMLSliceWidget.h>
#include <vtkMRMLSliceLogic.h>

//#include <vtkMatrix4x4.h>
#include <vtkTransform.h>

// MRML
#include <vtkMRMLTestMe2Node.h>

// Logic
#include <vtkSlicerTestMe2Logic.h>
//-----------------------------------------------------------------------------
class qSlicerTestMe2ModuleWidgetPrivate: public Ui_qSlicerTestMe2ModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerTestMe2ModuleWidget);
protected:
  qSlicerTestMe2ModuleWidget* const q_ptr;
public:
  qSlicerTestMe2ModuleWidgetPrivate(qSlicerTestMe2ModuleWidget &object);
  vtkSmartPointer<vtkMRMLTestMe2Node> ParameterNode;

  vtkSlicerTestMe2Logic* logic() const;

  bool ModuleWindowInitialized{ false };

  double offset{ 0.0 };
};

//-----------------------------------------------------------------------------
// qSlicerTestMe2ModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerTestMe2ModuleWidgetPrivate::qSlicerTestMe2ModuleWidgetPrivate(qSlicerTestMe2ModuleWidget &object)
:
q_ptr(&object)
{
}

vtkSlicerTestMe2Logic* qSlicerTestMe2ModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerTestMe2ModuleWidget);
  return vtkSlicerTestMe2Logic::SafeDownCast(q->logic());
}
//-----------------------------------------------------------------------------
// qSlicerTestMe2ModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerTestMe2ModuleWidget::qSlicerTestMe2ModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerTestMe2ModuleWidgetPrivate(*this) )
{
}

//-----------------------------------------------------------------------------
qSlicerTestMe2ModuleWidget::~qSlicerTestMe2ModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerTestMe2ModuleWidget::setup()
{
  Q_D(qSlicerTestMe2ModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  connect( d->InputParameterNode, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
    SLOT(onParameterNodeChanged(vtkMRMLNode*)));

  connect( d->InputFiducial, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
    SLOT(onFiducialNodeChanged(vtkMRMLNode*)));

//  connect( d->InputFiducial, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
//    SLOT(onCheckNodesButtonClicked()));

  connect( d->InputTransform, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
    SLOT(onTransformNodeChanged(vtkMRMLNode*)));

//  connect( d->InputTransform, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
 //   SLOT(onCheckNodesButtonClicked()));

  connect( d->CheckNodesButton, SIGNAL(clicked()), this,
    SLOT(onCheckNodesButtonClicked()));

  connect( d->HeightSlider, SIGNAL(valueChanged(double)), this,
    SLOT(onHeightSliderMove(double)));

  connect( d->RotateXSlider, SIGNAL(valueChanged(double)), this,
    SLOT(onRotateXSliderMove(double)));

  connect( d->InputDRR, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
    SLOT(onDRRNodeChanged(vtkMRMLNode*)));

  connect( d->InputBeam, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
    SLOT(onBeamNodeChanged(vtkMRMLNode*)));

  connect( d->ShowDRRButton, SIGNAL(clicked()), this,
    SLOT(onShowDRRButtonClicked()));



 // connect( d->InputParameterNode, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
 //   SLOT(onCheckNodesButtonClicked()));

}
void qSlicerTestMe2ModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerTestMe2ModuleWidget);
  this->Superclass::setMRMLScene(scene);

  qvtkReconnect( d->logic(), scene, vtkMRMLScene::EndImportEvent, this, SLOT(onSceneImportedEvent()));
  qvtkReconnect( d->logic(), scene, vtkMRMLScene::EndCloseEvent, this, SLOT(onSceneClosedEvent()));

  // Find parameters node or create it if there is none in the scene
  if (scene)
  {
    if (d->InputParameterNode->currentNode())
    {
      this->setParameterNode(d->InputParameterNode->currentNode());
    }
    else if (vtkMRMLNode* node = scene->GetNthNodeByClass( 0, "vtkMRMLTestMe2Node"))
    {
      this->setParameterNode(node);
    }
    else
    {
      vtkMRMLNode* newNode = scene->AddNewNodeByClass("vtkMRMLTestMe2Node");
      this->setParameterNode(newNode);
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerTestMe2ModuleWidget::setParameterNode(vtkMRMLNode *node)
{
  Q_D(qSlicerTestMe2ModuleWidget);

  vtkMRMLTestMe2Node* parameterNode = vtkMRMLTestMe2Node::SafeDownCast(node);

  // Make sure the parameter set node is selected (in case the function was not called by the selector combobox signal)
  d->InputParameterNode->setCurrentNode(node);

  // Each time the node is modified, the UI widgets are updated
  qvtkReconnect( d->ParameterNode, parameterNode, vtkCommand::ModifiedEvent,
    this, SLOT( updateWidgetFromMRML() ) );

  d->ParameterNode = parameterNode;

  if (d->ParameterNode)
  {
    vtkMRMLMarkupsFiducialNode* fiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(d->InputFiducial->currentNode());
    d->ParameterNode->SetAndObserveFiducialNode(fiducialNode);
    vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(d->InputTransform->currentNode());
    d->ParameterNode->SetAndObserveTransformNode(transformNode);
    d->ParameterNode->SetHeight(d->HeightSlider->value());
    d->ParameterNode->SetRotateXAngle(d->RotateXSlider->value());
    vtkMRMLScalarVolumeNode* drrNode = vtkMRMLScalarVolumeNode::SafeDownCast(d->InputDRR->currentNode());
    d->ParameterNode->SetAndObserveDRRNode(drrNode);
    vtkMRMLRTBeamNode* beamNode = vtkMRMLRTBeamNode::SafeDownCast(d->InputBeam->currentNode());
    d->ParameterNode->SetAndObserveBeamNode(beamNode);
  }

  this->updateWidgetFromMRML();
}

void qSlicerTestMe2ModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerTestMe2ModuleWidget);

  vtkMRMLTestMe2Node* parameterNode = vtkMRMLTestMe2Node::SafeDownCast(d->InputParameterNode->currentNode());

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

  // Update widgets


  d->InputFiducial->setCurrentNode(parameterNode->GetFiducialNode());
  d->InputTransform->setCurrentNode(parameterNode->GetTransformNode());
  d->HeightSlider->setValue(parameterNode->GetHeight());
  d->RotateXSlider->setValue(parameterNode->GetRotateXAngle());
  d->InputDRR->setCurrentNode(parameterNode->GetDRRNode());
  d->InputBeam->setCurrentNode(parameterNode->GetBeamNode());

  if (d->ParameterNode->GetFiducialNode() && d->ParameterNode->GetTransformNode())
  {
    d->HeightSlider->setEnabled(true);
    d->RotateXSlider->setEnabled(true);
  }
  else
  {
    d->HeightSlider->setEnabled(false);
    d->RotateXSlider->setEnabled(false);
  }

  if (d->ParameterNode->GetDRRNode() && d->ParameterNode->GetBeamNode())
  {
    d->ShowDRRButton->setEnabled(true);
  }
  else
  {
    d->ShowDRRButton->setEnabled(false);
  }
//  this->onCheckNodesButtonClicked();
//  qDebug() << Q_FUNC_INFO << "Update";
}

void qSlicerTestMe2ModuleWidget::onSceneImportedEvent()
{
  this->onEnter();
}

//-----------------------------------------------------------------------------
void qSlicerTestMe2ModuleWidget::onSceneClosedEvent()
{
  Q_D(qSlicerTestMe2ModuleWidget);
  this->updateWidgetFromMRML();
}

void qSlicerTestMe2ModuleWidget::enter()
{
  Q_D(qSlicerTestMe2ModuleWidget);
  this->Superclass::enter();
  this->onEnter();
}

//-----------------------------------------------------------------------------
void qSlicerTestMe2ModuleWidget::exit()
{
  Q_D(qSlicerTestMe2ModuleWidget);

  this->Superclass::exit();
}

void qSlicerTestMe2ModuleWidget::onParameterNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerTestMe2ModuleWidget);
  vtkMRMLTestMe2Node* parameterNode = vtkMRMLTestMe2Node::SafeDownCast(node);

  if (!parameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  this->setParameterNode(parameterNode);
//  this->onCheckNodesButtonClicked();
}

void qSlicerTestMe2ModuleWidget::onEnter()
{
  Q_D(qSlicerTestMe2ModuleWidget);

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

  vtkMRMLTestMe2Node* parameterNode = nullptr;
  // Try to find one in the scene
  if (vtkMRMLNode* node = this->mrmlScene()->GetNthNodeByClass( 0, "vtkMRMLTestMe2Node"))
  {
    parameterNode = vtkMRMLTestMe2Node::SafeDownCast(node);
  }

  // All required data for GUI is initiated
  this->updateWidgetFromMRML();

  d->ModuleWindowInitialized = true;
}

void qSlicerTestMe2ModuleWidget::onFiducialNodeChanged(vtkMRMLNode *node)
{
  Q_D(qSlicerTestMe2ModuleWidget);
  vtkMRMLMarkupsFiducialNode* fiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(node);

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  if (fiducialNode)
  {
    d->ParameterNode->SetAndObserveFiducialNode(fiducialNode);
    d->logic()->createControlPoint(d->ParameterNode->GetFiducialNode());
    if (d->ParameterNode->GetTransformNode())
    {
      d->logic()->updateFiducialTransformLink(d->ParameterNode->GetFiducialNode(), d->ParameterNode->GetTransformNode());
//      d->ParameterNode->Modified();
//      d->logic()->updateTransform(d->ParameterNode->GetTransformNode(), d->ParameterNode->GetHeight(), d->ParameterNode->GetRotateXAngle());
    }
    qDebug() << Q_FUNC_INFO << "Fiducial node is changed";
  }

//  this->onCheckNodesButtonClicked();
//    qDebug() << Q_FUNC_INFO << "Fiducial node name is" << d->ParameterNode->GetFiducialNode();

}

void qSlicerTestMe2ModuleWidget::onTransformNodeChanged(vtkMRMLNode *node)
{
  Q_D(qSlicerTestMe2ModuleWidget);
  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);

  if (!d->ParameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  if (transformNode)
  {
    d->ParameterNode->SetAndObserveTransformNode(transformNode);
    if (d->ParameterNode->GetFiducialNode() && d->ParameterNode->GetTransformNode())
    {
      d->logic()->updateFiducialTransformLink(d->ParameterNode->GetFiducialNode(), d->ParameterNode->GetTransformNode());
//      d->ParameterNode->Modified();
//      d->logic()->updateTransform(d->ParameterNode->GetTransformNode(), d->ParameterNode->GetHeight(), d->ParameterNode->GetRotateXAngle());
    }
    qDebug() << Q_FUNC_INFO << "Transform node is changed";
  }

//  this->onCheckNodesButtonClicked();
//    qDebug() << Q_FUNC_INFO << "Transform node name is" << d->ParameterNode->GetTransformNode();
}

void qSlicerTestMe2ModuleWidget::onCheckNodesButtonClicked()
{
  Q_D(qSlicerTestMe2ModuleWidget);
  if (!d->ParameterNode)
    {
      qWarning() << "Parameter node is invalid";
      return;
    }
  if (d->ParameterNode->GetFiducialNode() && d->ParameterNode->GetTransformNode())
  {
    d->HeightSlider->setEnabled(true);
    d->RotateXSlider->setEnabled(true);
    qDebug() << Q_FUNC_INFO << "Fiducial & Transform nodes are valid";
    qDebug() << "Fiducial node name is" << d->ParameterNode->GetFiducialNode()->GetName();
    qDebug() << "Transform node name is" << d->ParameterNode->GetTransformNode()->GetName();

    d->logic()->updateTransform(d->ParameterNode->GetTransformNode(), d->ParameterNode->GetHeight(), d->ParameterNode->GetRotateXAngle());
  }

  else
  {
    d->HeightSlider->setEnabled(false);
    d->RotateXSlider->setEnabled(false);
 //   qWarning() << Q_FUNC_INFO << "Nodes are invalid";
    if (!d->ParameterNode->GetFiducialNode())
    {
      qWarning() << "Fiducial node is invalid";
    }
    if (!d->ParameterNode->GetTransformNode())
    {
      qWarning() << "Transform node is invalid";
    }
  }
}

void qSlicerTestMe2ModuleWidget::onHeightSliderMove(double height)
{
  Q_D(qSlicerTestMe2ModuleWidget);
  d->ParameterNode->SetHeight(height);
//  if (d->ParameterNode->GetFiducialNode() && d->ParameterNode->GetTransformNode())
//  {
//    d->logic()->updateTransform(d->ParameterNode->GetTransformNode(), d->ParameterNode->GetHeight(), d->ParameterNode->GetRotateXAngle());
//  }
}

void qSlicerTestMe2ModuleWidget::onRotateXSliderMove(double rotateXAngle)
{
  Q_D(qSlicerTestMe2ModuleWidget);
  d->ParameterNode->SetRotateXAngle(rotateXAngle);
//  if (d->ParameterNode->GetFiducialNode() && d->ParameterNode->GetTransformNode())
//  {
//    d->logic()->updateTransform(d->ParameterNode->GetTransformNode(), d->ParameterNode->GetHeight(), d->ParameterNode->GetRotateXAngle());
//  }
}

void qSlicerTestMe2ModuleWidget::onDRRNodeChanged(vtkMRMLNode *node)
{
  Q_D(qSlicerTestMe2ModuleWidget);
  vtkMRMLScalarVolumeNode* drrNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  if (drrNode)
  {
    d->ParameterNode->SetAndObserveDRRNode(drrNode);
    qDebug() << Q_FUNC_INFO << "DRR node is changed";
  }
}

void qSlicerTestMe2ModuleWidget::onBeamNodeChanged(vtkMRMLNode *node)
{
  Q_D(qSlicerTestMe2ModuleWidget);
  vtkMRMLRTBeamNode* beamNode = vtkMRMLRTBeamNode::SafeDownCast(node);

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  if (beamNode)
  {
    d->ParameterNode->SetAndObserveBeamNode(beamNode);
    qDebug() << Q_FUNC_INFO << "Beam node is changed";
  }
}

void qSlicerTestMe2ModuleWidget::onShowDRRButtonClicked()
{
  Q_D(qSlicerTestMe2ModuleWidget);
  if (!d->ParameterNode)
    {
      qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
      return;
    }

  vtkMRMLScalarVolumeNode* drrNode = vtkMRMLScalarVolumeNode::SafeDownCast(d->ParameterNode->GetDRRNode());
  vtkMRMLRTBeamNode* beamNode = vtkMRMLRTBeamNode::SafeDownCast(d->ParameterNode->GetBeamNode());

  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(d->InputSlice->currentNode());


  if (drrNode && beamNode && sliceNode)
  {
    d->logic()->showDRR(drrNode, beamNode, sliceNode);
  }

  else
  {
    if (!d->ParameterNode->GetDRRNode())
    {
      qWarning() << "DRR node is invalid";
    }
    if (!d->ParameterNode->GetBeamNode())
    {
      qWarning() << "Beam node is invalid";
    }
    if (!sliceNode)
    {
      qWarning() << "Slice node is invalid";
    }
  }
}
