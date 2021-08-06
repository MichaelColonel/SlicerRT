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
//    if (d->MRMLNodeComboBox_ParameterSet->currentNode())
//    {
//      this->setParameterNode(d->MRMLNodeComboBox_ParameterSet->currentNode());
//    }
//    else if (vtkMRMLNode* node = scene->GetNthNodeByClass( 0, "vtkMRMLDrrImageComputationNode"))
//    {
//      this->setParameterNode(node);
//    }
//    else
//    {
//      vtkNew<vtkMRMLDrrImageComputationNode> newNode;
//      this->mrmlScene()->AddNode(newNode);
//      this->setParameterNode(newNode);
//    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerScalarBarModuleWidget::exit()
{
  Q_D(qSlicerScalarBarModuleWidget);
  this->Superclass::exit();
}

//-----------------------------------------------------------------------------
void qSlicerScalarBarModuleWidget::enter()
{
  Q_D(qSlicerScalarBarModuleWidget);
  this->Superclass::enter();
}
