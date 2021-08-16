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

#include <vtkMRMLScalarVolumeNode.h>

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
  vtkMRMLScalarVolumeNode* VolumeNode;
  bool ModuleWindowInitialized;
};

//-----------------------------------------------------------------------------
// qSlicerScalarBarModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerScalarBarModuleWidgetPrivate::qSlicerScalarBarModuleWidgetPrivate(qSlicerScalarBarModuleWidget &object)
  :
  q_ptr(&object),
  VolumeNode(nullptr),
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
  connect( d->MRMLNodeComboBox_ScalarVolumeNode, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onScalarVolumeNodeChanged(vtkMRMLNode*)));
  connect( d->PushButton_AddColorBarDisplayNode, SIGNAL(clicked()), 
    this, SLOT(onAddColorBarDisplayNodeClicked()));

  // CheckButton
  connect( d->CheckBox_ShowColorBar, SIGNAL(toggled(bool)), 
    this, SLOT(onShowColorBarToggled(bool)));

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
    if (d->MRMLNodeComboBox_ScalarVolumeNode->currentNode())
    {
      qDebug() << Q_FUNC_INFO << "Scalar volume node is valid";
      d->VolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(d->MRMLNodeComboBox_ScalarVolumeNode->currentNode());
    }
    else
    {
      d->VolumeNode = nullptr;
    }
  }

  d->PushButton_AddColorBarDisplayNode->setEnabled(d->VolumeNode);

}

//-----------------------------------------------------------------------------
void qSlicerScalarBarModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerScalarBarModuleWidget);

  d->VolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(d->MRMLNodeComboBox_ScalarVolumeNode->currentNode());

  if (!this->mrmlScene())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid scene";
    d->PushButton_AddColorBarDisplayNode->setEnabled(false);
    d->CheckBox_ShowColorBar->setEnabled(false);
    return;
  }
  
  if (!d->VolumeNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node, or module window isn't initialized";
    d->PushButton_AddColorBarDisplayNode->setEnabled(false);
    d->CheckBox_ShowColorBar->setEnabled(false);
    return;
  }
  d->PushButton_AddColorBarDisplayNode->setEnabled(d->VolumeNode);
  d->CheckBox_ShowColorBar->setEnabled(d->VolumeNode);
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
void qSlicerScalarBarModuleWidget::onScalarVolumeNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerScalarBarModuleWidget);
  d->VolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);

  if (!d->VolumeNode || !d->ModuleWindowInitialized)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid volume node, or module window isn't initialized";
    return;
  }

  qDebug() << Q_FUNC_INFO;
}

//-----------------------------------------------------------------------------
void qSlicerScalarBarModuleWidget::onAddColorBarDisplayNodeClicked()
{
  Q_D(qSlicerScalarBarModuleWidget);
}

//-----------------------------------------------------------------------------
void qSlicerScalarBarModuleWidget::onShowColorBarToggled(bool toggled)
{
  Q_D(qSlicerScalarBarModuleWidget);
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

  // Try to find one in the scene
  if (vtkMRMLNode* node = this->mrmlScene()->GetFirstNodeByClass("vtkMRMLScalarVolumeNode"))
  {
    d->VolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);
  }
  else
  {
    d->VolumeNode = nullptr;
  }

  // If we have a parameter node select it
  if (!d->VolumeNode)
  {
    d->PushButton_AddColorBarDisplayNode->setEnabled(false);
    d->CheckBox_ShowColorBar->setEnabled(false);
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
