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
#include "qSlicerScalarBarModuleWidget.h"
#include "ui_qSlicerScalarBarModuleWidget.h"

#include "vtkMRMLScene.h"
 
#include "vtkSlicerScalarBarLogic.h"

#include "vtkMRMLScalarBarDisplayNode.h"

#include <vtkMRMLColorNode.h>
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLProceduralColorNode.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerScalarBarModuleWidgetPrivate: public Ui_qSlicerScalarBarModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerScalarBarModuleWidget);
protected:
  qSlicerScalarBarModuleWidget* const q_ptr;

public:
  qSlicerScalarBarModuleWidgetPrivate(qSlicerScalarBarModuleWidget &object);
  virtual ~qSlicerScalarBarModuleWidgetPrivate();

  vtkSlicerScalarBarLogic* logic() const;
  bool ModuleWindowInitialized;
};

//-----------------------------------------------------------------------------
// qSlicerScalarBarModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerScalarBarModuleWidgetPrivate::qSlicerScalarBarModuleWidgetPrivate(qSlicerScalarBarModuleWidget &object)
  :
  q_ptr(&object),
  ModuleWindowInitialized(false)
{
}

//-----------------------------------------------------------------------------
qSlicerScalarBarModuleWidgetPrivate::~qSlicerScalarBarModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
vtkSlicerScalarBarLogic* qSlicerScalarBarModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerScalarBarModuleWidget);
  return vtkSlicerScalarBarLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
// qSlicerScalarBarModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerScalarBarModuleWidget::qSlicerScalarBarModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerScalarBarModuleWidgetPrivate(*this) )
{
}

//-----------------------------------------------------------------------------
qSlicerScalarBarModuleWidget::~qSlicerScalarBarModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerScalarBarModuleWidget::setup()
{
  Q_D(qSlicerScalarBarModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  // Nodes
  connect( d->MRMLNodeComboBox_ParameterNode, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onParameterNodeChanged(vtkMRMLNode*)));
  connect( d->MRMLNodeComboBox_ColorTableNode, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onColorTableNodeChanged(vtkMRMLNode*)));

  // CheckButton
  connect( d->CheckBox_ShowScalarBar, SIGNAL(toggled(bool)), 
    this, SLOT(onShowScalarBarToggled(bool)));

  // Handle scene change event if occurs
  qvtkConnect( d->logic(), vtkCommand::ModifiedEvent, this, SLOT(onLogicModified()));
  qDebug() << Q_FUNC_INFO;
}

//-----------------------------------------------------------------------------
void qSlicerScalarBarModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerScalarBarModuleWidget);
  this->Superclass::setMRMLScene(scene);

  qvtkReconnect( d->logic(), scene, vtkMRMLScene::EndImportEvent, this, SLOT(onSceneImportedEvent()));
  qvtkReconnect( d->logic(), scene, vtkMRMLScene::EndCloseEvent, this, SLOT(onSceneClosedEvent()));

  // Find parameters node or create it if there is none in the scene
  if (scene)
  {
    if (d->MRMLNodeComboBox_ParameterNode->currentNode())
    {
      qDebug() << Q_FUNC_INFO << "Current node is valid";
      this->setParameterNode(d->MRMLNodeComboBox_ParameterNode->currentNode());
    }
    else if (vtkMRMLNode* node = scene->GetFirstNodeByClass("vtkMRMLScalarBarDisplayNode"))
    {
      qDebug() << Q_FUNC_INFO << "Get first node";
      this->setParameterNode(node);
    }
    else
    {
      qDebug() << Q_FUNC_INFO << "Create a new node";
      vtkNew<vtkMRMLScalarBarDisplayNode> newNode;
      this->mrmlScene()->AddNode(newNode);
      this->setParameterNode(newNode);
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerScalarBarModuleWidget::setParameterNode(vtkMRMLNode *node)
{
  Q_D(qSlicerScalarBarModuleWidget);

  vtkMRMLScalarBarDisplayNode* parameterNode = vtkMRMLScalarBarDisplayNode::SafeDownCast(node);

  // Make sure the parameter set node is selected (in case the function was not called by the selector combobox signal)
  d->MRMLNodeComboBox_ParameterNode->setCurrentNode(node);
//  d->MRMLNodeComboBox_ParameterNode->setEnabled(node);
 
  // Each time the node is modified, the UI widgets are updated
  qvtkReconnect( parameterNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));

  // Set selected MRML nodes in comboboxes in the parameter set if it was nullptr there
  // (then in the meantime the comboboxes selected the first one from the scene and we have to set that)
  if (parameterNode)
  {
    qDebug() << Q_FUNC_INFO << "Current node is valid";
  }
  this->updateWidgetFromMRML();
  qDebug() << Q_FUNC_INFO;
}

//-----------------------------------------------------------------------------
void qSlicerScalarBarModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerScalarBarModuleWidget);

  vtkMRMLScalarBarDisplayNode* parameterNode = vtkMRMLScalarBarDisplayNode::SafeDownCast(d->MRMLNodeComboBox_ParameterNode->currentNode());

  if (!this->mrmlScene())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid scene";
    return;
  }
  
  if (!parameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node, or module window isn't initialized";
    return;
  }

}

//-----------------------------------------------------------------------------
void qSlicerScalarBarModuleWidget::onSceneImportedEvent()
{
  this->onEnter();
  qDebug() << Q_FUNC_INFO;
}

//-----------------------------------------------------------------------------
void qSlicerScalarBarModuleWidget::onSceneClosedEvent()
{
  Q_D(qSlicerScalarBarModuleWidget);
  this->updateWidgetFromMRML();
  qDebug() << Q_FUNC_INFO;
}

//-----------------------------------------------------------------------------
void qSlicerScalarBarModuleWidget::enter()
{
  Q_D(qSlicerScalarBarModuleWidget);
  this->Superclass::enter();
  this->onEnter();
  qDebug() << Q_FUNC_INFO;
}

//-----------------------------------------------------------------------------
void qSlicerScalarBarModuleWidget::exit()
{
  Q_D(qSlicerScalarBarModuleWidget);

  this->Superclass::exit();
  qDebug() << Q_FUNC_INFO;
}

//-----------------------------------------------------------------------------
void qSlicerScalarBarModuleWidget::onParameterNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerScalarBarModuleWidget);
  vtkMRMLScalarBarDisplayNode* parameterNode = vtkMRMLScalarBarDisplayNode::SafeDownCast(node);

  if (!parameterNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node, or module window isn't initialized";
    return;
  }

  this->setParameterNode(parameterNode);
  qDebug() << Q_FUNC_INFO;
}

//-----------------------------------------------------------------------------
void qSlicerScalarBarModuleWidget::onColorTableNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerScalarBarModuleWidget);


  vtkMRMLScalarBarDisplayNode* parameterNode = vtkMRMLScalarBarDisplayNode::SafeDownCast(d->MRMLNodeComboBox_ParameterNode->currentNode());

  vtkMRMLColorNode* colorNode = vtkMRMLColorNode::SafeDownCast(node);
  if (!colorNode)
  {
    qDebug() << Q_FUNC_INFO << "Colour node is invalid";
    return;
  }

  vtkMRMLColorTableNode *colorTableNode = vtkMRMLColorTableNode::SafeDownCast(colorNode);
  vtkMRMLProceduralColorNode *procColorNode = vtkMRMLProceduralColorNode::SafeDownCast(colorNode);

  if (colorTableNode)
  {
    qDebug() << Q_FUNC_INFO << "Set and observe colour table node";
    parameterNode->SetAndObserveColorTableNode(colorTableNode);
//    parameterNode->Modified();
  }
}

//-----------------------------------------------------------------------------
void qSlicerScalarBarModuleWidget::onShowScalarBarToggled(bool toggled)
{
  Q_D(qSlicerScalarBarModuleWidget);

  vtkMRMLScalarBarDisplayNode* parameterNode = vtkMRMLScalarBarDisplayNode::SafeDownCast(d->MRMLNodeComboBox_ParameterNode->currentNode());
  
  if (!parameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  parameterNode->SetVisibilityOnSliceViewsFlag(toggled);
}

//-----------------------------------------------------------------------------
void qSlicerScalarBarModuleWidget::onEnter()
{
  Q_D(qSlicerScalarBarModuleWidget);

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

  vtkMRMLScalarBarDisplayNode* parameterNode = nullptr; 
  // Try to find one in the scene
  if (vtkMRMLNode* node = this->mrmlScene()->GetFirstNodeByClass("vtkMRMLScalarBarDisplayNode"))
  {
    parameterNode = vtkMRMLScalarBarDisplayNode::SafeDownCast(node);
  }

  vtkMRMLScalarBarDisplayNode* paramNode = vtkMRMLScalarBarDisplayNode::SafeDownCast(d->MRMLNodeComboBox_ParameterNode->currentNode());

  // If we have a parameter node select it
  if (!paramNode)
  {
    vtkMRMLNode* node = this->mrmlScene()->GetFirstNodeByClass("vtkMRMLScalarBarDisplayNode");
    if (node)
    {
      this->setParameterNode(node);
    }
    else 
    {
      qDebug() << Q_FUNC_INFO << "Create a new node";
      vtkNew<vtkMRMLScalarBarDisplayNode> newNode;
      this->mrmlScene()->AddNode(newNode);
      this->setParameterNode(newNode);
    }
  }
  else
  {
    this->updateWidgetFromMRML();
  }

  // All required data for GUI is initiated
  d->ModuleWindowInitialized = true;
}

//-----------------------------------------------------------------------------
void qSlicerScalarBarModuleWidget::onLogicModified()
{
  this->updateWidgetFromMRML();
  qDebug() << Q_FUNC_INFO;
}
