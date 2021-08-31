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
#include <vtkMRMLColorBarDisplayNode.h>

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
};

//-----------------------------------------------------------------------------
// qSlicerScalarBarModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerScalarBarModuleWidgetPrivate::qSlicerScalarBarModuleWidgetPrivate(qSlicerScalarBarModuleWidget &object)
  :
  q_ptr(&object),
  VolumeNode(nullptr)
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
  connect( d->CheckBox_ShowColorBar3D, SIGNAL(toggled(bool)), 
    this, SLOT(onShowColorBar3DToggled(bool)));
  connect( d->CheckBox_ShowColorBar2D, SIGNAL(toggled(bool)), 
    this, SLOT(onShowColorBar2DToggled(bool)));

  // ComboBox
  connect( d->ComboBox_ColorBarPosition, SIGNAL(currentIndexChanged(int)),
    this, SLOT(onColocrBarOrientationIndexChanged(int)));

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
    d->CheckBox_ShowColorBar2D->setEnabled(false);
    d->CheckBox_ShowColorBar3D->setEnabled(false);
    d->ComboBox_ColorBarPosition->setEnabled(false);
    return;
  }
  
  if (!d->VolumeNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node, or module window isn't initialized";
    d->PushButton_AddColorBarDisplayNode->setEnabled(false);
    d->CheckBox_ShowColorBar2D->setEnabled(false);
    d->CheckBox_ShowColorBar3D->setEnabled(false);
    d->ComboBox_ColorBarPosition->setEnabled(false);
    return;
  }
  else
  {
    d->PushButton_AddColorBarDisplayNode->setEnabled(true);
    d->CheckBox_ShowColorBar2D->setEnabled(false);
    d->CheckBox_ShowColorBar3D->setEnabled(false);
    d->ComboBox_ColorBarPosition->setEnabled(false);
  }
  
  if (vtkMRMLNode* node = d->VolumeNode->GetNodeReference("ColorBarRef"))
  {
    vtkMRMLColorBarDisplayNode* displayNode = vtkMRMLColorBarDisplayNode::SafeDownCast(node);
    if (displayNode)
    {
      d->PushButton_AddColorBarDisplayNode->setEnabled(false);
      d->CheckBox_ShowColorBar2D->setEnabled(true);
      d->CheckBox_ShowColorBar3D->setEnabled(true);
      d->ComboBox_ColorBarPosition->setEnabled(true);
      d->ComboBox_ColorBarPosition->setCurrentIndex(displayNode->GetOrientationPreset());
    }
    else
    {
      d->PushButton_AddColorBarDisplayNode->setEnabled(true);
      d->CheckBox_ShowColorBar2D->setEnabled(false);
      d->CheckBox_ShowColorBar3D->setEnabled(false);
      d->ComboBox_ColorBarPosition->setEnabled(false);
    }
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
void qSlicerScalarBarModuleWidget::onScalarVolumeNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerScalarBarModuleWidget);
  d->VolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);

  if (!d->VolumeNode)
  {
    d->PushButton_AddColorBarDisplayNode->setEnabled(false);
    d->CheckBox_ShowColorBar2D->setEnabled(false);
    d->CheckBox_ShowColorBar3D->setEnabled(false);
    d->ComboBox_ColorBarPosition->setEnabled(false);
    qCritical() << Q_FUNC_INFO << ": Invalid volume node, or module window isn't initialized";
    return;
  }
  else
  {
    vtkMRMLNode* node = d->VolumeNode->GetNodeReference("ColorBarRef");
    d->CheckBox_ShowColorBar2D->setEnabled(node);
    d->CheckBox_ShowColorBar3D->setEnabled(node);
    d->ComboBox_ColorBarPosition->setEnabled(node);
  }
  
  d->PushButton_AddColorBarDisplayNode->setEnabled(true);

  qDebug() << Q_FUNC_INFO;
}

//-----------------------------------------------------------------------------
void qSlicerScalarBarModuleWidget::onAddColorBarDisplayNodeClicked()
{
  Q_D(qSlicerScalarBarModuleWidget);

  if (!d->VolumeNode)
  {
    qDebug() << Q_FUNC_INFO << "Volume node is invalid";
    return;
  }

  vtkNew<vtkMRMLColorBarDisplayNode> cbNode;
  this->mrmlScene()->AddNode(cbNode);

  cbNode->SetOrientationPreset(vtkMRMLColorBarDisplayNode::Vertical);
  
  d->VolumeNode->SetNodeReferenceID( "ColorBarRef", cbNode->GetID());
  cbNode->SetAndObserveDisplayableNode(d->VolumeNode);
  d->CheckBox_ShowColorBar2D->setEnabled(true);
  d->CheckBox_ShowColorBar3D->setEnabled(true);
  d->ComboBox_ColorBarPosition->setEnabled(true);
}

//-----------------------------------------------------------------------------
void qSlicerScalarBarModuleWidget::onShowColorBar2DToggled(bool toggled)
{
  Q_D(qSlicerScalarBarModuleWidget);

  if (!d->VolumeNode)
  {
    qDebug() << Q_FUNC_INFO << "Volume node is invalid";
    return;
  }

  if (vtkMRMLNode* node = d->VolumeNode->GetNodeReference("ColorBarRef"))
  {
    vtkMRMLColorBarDisplayNode* displayNode = vtkMRMLColorBarDisplayNode::SafeDownCast(node);
    if (displayNode)
    {
      displayNode->SetVisibility2D(toggled);
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerScalarBarModuleWidget::onShowColorBar3DToggled(bool toggled)
{
  Q_D(qSlicerScalarBarModuleWidget);

  if (!d->VolumeNode)
  {
    qDebug() << Q_FUNC_INFO << "Volume node is invalid";
    return;
  }

  if (vtkMRMLNode* node = d->VolumeNode->GetNodeReference("ColorBarRef"))
  {
    vtkMRMLColorBarDisplayNode* displayNode = vtkMRMLColorBarDisplayNode::SafeDownCast(node);
    if (displayNode)
    {
      displayNode->SetVisibility3D(toggled);
    }
  }
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

  d->PushButton_AddColorBarDisplayNode->setEnabled(false);
  d->CheckBox_ShowColorBar2D->setEnabled(false);
  d->CheckBox_ShowColorBar3D->setEnabled(false);
  d->ComboBox_ColorBarPosition->setEnabled(false);

  // Try to find one in the scene
  if (vtkMRMLNode* node = this->mrmlScene()->GetFirstNodeByClass("vtkMRMLScalarVolumeNode"))
  {
    d->VolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);
  }
  else
  {
    d->VolumeNode = nullptr;
  }

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerScalarBarModuleWidget::onLogicModified()
{
  this->updateWidgetFromMRML();
  qDebug() << Q_FUNC_INFO;
}

void qSlicerScalarBarModuleWidget::onColocrBarOrientationIndexChanged(int orientationIndex)
{
  Q_D(qSlicerScalarBarModuleWidget);

  if (!d->VolumeNode)
  {
    qDebug() << Q_FUNC_INFO << "Volume node is invalid";
    return;
  }

  vtkMRMLColorBarDisplayNode* displayNode = nullptr;
  if (vtkMRMLNode* node = d->VolumeNode->GetNodeReference("ColorBarRef"))
  {
    displayNode = vtkMRMLColorBarDisplayNode::SafeDownCast(node);
  }

  if (!displayNode)
  {
    qDebug() << Q_FUNC_INFO << "Color bar display node is invalid";
    return;
  }

  switch (orientationIndex)
  {
  case 0:
    displayNode->SetOrientationPreset(vtkMRMLColorBarDisplayNode::Horizontal);
    break;
  case 1:
    displayNode->SetOrientationPreset(vtkMRMLColorBarDisplayNode::Vertical);
    break;
  default:
    break;
  }
}
