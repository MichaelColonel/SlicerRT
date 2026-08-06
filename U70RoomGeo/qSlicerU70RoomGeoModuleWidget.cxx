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
#include "qSlicerU70RoomGeoModuleWidget.h"
#include "ui_qSlicerU70RoomGeoModuleWidget.h"

#include <vtkMRMLScene.h>

// U70RoomGeo logic and nodes
#include <vtkMRMLU70RoomGeoNode.h>
#include <vtkSlicerU70RoomGeoLogic.h>

//-----------------------------------------------------------------------------
/// \ingroup SlicerRt_QtModules_U70RoomGeo
class qSlicerU70RoomGeoModuleWidgetPrivate : public Ui_qSlicerU70RoomGeoModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerU70RoomGeoModuleWidget);
protected:
  qSlicerU70RoomGeoModuleWidget* const q_ptr;
public:
  qSlicerU70RoomGeoModuleWidgetPrivate(qSlicerU70RoomGeoModuleWidget& object);
  ~qSlicerU70RoomGeoModuleWidgetPrivate()  = default;
  vtkSmartPointer<vtkSlicerU70RoomGeoLogic> logic() const;

  bool ModuleWindowInitialized;
};

//-----------------------------------------------------------------------------
// qSlicerU70RoomGeoModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerU70RoomGeoModuleWidgetPrivate::qSlicerU70RoomGeoModuleWidgetPrivate(qSlicerU70RoomGeoModuleWidget& object)
  : q_ptr(&object)
  , ModuleWindowInitialized(false)
{
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkSlicerU70RoomGeoLogic> qSlicerU70RoomGeoModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerU70RoomGeoModuleWidget);
  return vtkSlicerU70RoomGeoLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
// qSlicerU70RoomGeoModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerU70RoomGeoModuleWidget::qSlicerU70RoomGeoModuleWidget(QWidget* _parent)
: Superclass(_parent)
, d_ptr(new qSlicerU70RoomGeoModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerU70RoomGeoModuleWidget::~qSlicerU70RoomGeoModuleWidget() = default;

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::setup()
{
  Q_D(qSlicerU70RoomGeoModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  // buttons
  QObject::connect(d->PushButton_LoadModels, SIGNAL(clicked()), this, SLOT(onLoadTreatmentRoomButtonClicked()));
  QObject::connect(d->PushButton_UnloadModels, SIGNAL(clicked()), this, SLOT(onUnloadTreatmentRoomButtonClicked()));
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::onLoadTreatmentRoomButtonClicked()
{
  Q_D(qSlicerU70RoomGeoModuleWidget);
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::onUnloadTreatmentRoomButtonClicked()
{
  Q_D(qSlicerU70RoomGeoModuleWidget);
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerU70RoomGeoModuleWidget);

  this->Superclass::setMRMLScene(scene);
  qvtkReconnect(d->logic(), scene, vtkMRMLScene::EndImportEvent, this, SLOT(onSceneImportedEvent()));
  qvtkReconnect(d->logic(), scene, vtkMRMLScene::EndCloseEvent, this, SLOT(onSceneClosedEvent()));

  // Find parameters node or create it if there is none in the scene
  if (scene)
  {
    vtkMRMLNode* node = scene->GetFirstNodeByClass("vtkMRMLChannel26GeometryNode");
    if (node)
    {
      this->setParameterNode(node);
    }
    else
    {
      vtkSmartPointer<vtkMRMLU70RoomGeoNode> newNode = vtkSmartPointer<vtkMRMLU70RoomGeoNode>::New();
      this->mrmlScene()->AddNode(newNode);
      this->setParameterNode(newNode);
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::setParameterNode(vtkMRMLNode* node)
{
  vtkMRMLU70RoomGeoNode* chanGeoNode = vtkMRMLU70RoomGeoNode::SafeDownCast(node);
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::onSceneImportedEvent()
{
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::onSceneClosedEvent()
{
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::updateWidgetFromMRML()
{
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::enter()
{
  Q_D(qSlicerU70RoomGeoModuleWidget);
  this->Superclass::enter();
  this->onEnter();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::onEnter()
{
  Q_D(qSlicerU70RoomGeoModuleWidget);

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

  // Select or create parameter node
  this->setMRMLScene(this->mrmlScene());

  d->ModuleWindowInitialized = true;
}
