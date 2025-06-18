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

#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLLinearTransformNode.h>

#include <vtkMRMLRTBeamNode.h>

#include <vtkTransform.h>

//-----------------------------------------------------------------------------
class qSlicerTestMe2ModuleWidgetPrivate: public Ui_qSlicerTestMe2ModuleWidget
{
public:
  qSlicerTestMe2ModuleWidgetPrivate();
  vtkSmartPointer< vtkMRMLMarkupsFiducialNode > m_FiducialNode;
  vtkSmartPointer< vtkMRMLLinearTransformNode > m_TransformNode;
  vtkSmartPointer< vtkMRMLRTBeamNode > m_BeamNode;
  vtkSmartPointer< vtkMRMLTransformNode > m_BeamParentTransformNode;
  
  double offset{ 0.0 };
  double rotateX{ 0.0 };
  double rotateY{ 0.0 };
  double rotateZ{ 0.0 };
};

//-----------------------------------------------------------------------------
// qSlicerTestMe2ModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerTestMe2ModuleWidgetPrivate::qSlicerTestMe2ModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerTestMe2ModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerTestMe2ModuleWidget::qSlicerTestMe2ModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerTestMe2ModuleWidgetPrivate )
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

  connect( d->InputFiducial, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
    SLOT(onFiducialNodeChanged(vtkMRMLNode*)));

  connect( d->InputTransform, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
    SLOT(onTransformNodeChanged(vtkMRMLNode*)));

  connect( d->CheckNodesButton, SIGNAL(clicked()), this,
    SLOT(onCheckNodesButtonClicked()));

  connect( d->MRMLNodeComboBox_Beam, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
    SLOT(onBeamNodeChanged(vtkMRMLNode*)));
  connect( d->MRMLNodeComboBox_ParentTransform, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
    SLOT(onParentTransformNodeChanged(vtkMRMLNode*)));

  connect( d->SliderWidget_RotateX, SIGNAL(valueChanged(double)), this,
    SLOT(onParentRotationXChanged(double)));
  connect( d->SliderWidget_RotateY, SIGNAL(valueChanged(double)), this,
    SLOT(onParentRotationYChanged(double)));
  connect( d->SliderWidget_RotateZ, SIGNAL(valueChanged(double)), this,
    SLOT(onParentRotationZChanged(double)));
}


void qSlicerTestMe2ModuleWidget::onFiducialNodeChanged(vtkMRMLNode *node)
{
  Q_D(qSlicerTestMe2ModuleWidget);
  d->m_FiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(node);
  if (d->m_FiducialNode)
  {
    qDebug() << Q_FUNC_INFO << "Fiducial node name is:" << d->m_FiducialNode->GetName();
  }
  else
  {
    qWarning() << Q_FUNC_INFO << "Fiducial node is invalid";
  }
}

void qSlicerTestMe2ModuleWidget::onTransformNodeChanged(vtkMRMLNode *node)
{
  Q_D(qSlicerTestMe2ModuleWidget);
  d->m_TransformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  if (d->m_TransformNode)
  {
    qDebug() << Q_FUNC_INFO << "Transform node name is" << d->m_TransformNode->GetName();
  }
  else
  {
    qWarning() << Q_FUNC_INFO << "Transform node is invalid";
  }
}

void qSlicerTestMe2ModuleWidget::onBeamNodeChanged(vtkMRMLNode *node)
{
  Q_D(qSlicerTestMe2ModuleWidget);
  d->m_BeamNode = vtkMRMLRTBeamNode::SafeDownCast(node);
  this->updateParentTransform();
}

void qSlicerTestMe2ModuleWidget::onParentTransformNodeChanged(vtkMRMLNode *node)
{
  Q_D(qSlicerTestMe2ModuleWidget);
  d->m_BeamParentTransformNode = vtkMRMLTransformNode::SafeDownCast(node);
  this->updateParentTransform();
}

void qSlicerTestMe2ModuleWidget::onParentRotationXChanged(double x)
{
  Q_D(qSlicerTestMe2ModuleWidget);
  d->rotateX = x;
  this->updateParentTransform();
}

void qSlicerTestMe2ModuleWidget::onParentRotationYChanged(double y)
{
  Q_D(qSlicerTestMe2ModuleWidget);
  d->rotateY = y;
  this->updateParentTransform();
}

void qSlicerTestMe2ModuleWidget::onParentRotationZChanged(double z)
{
  Q_D(qSlicerTestMe2ModuleWidget);
  d->rotateZ = z;
  this->updateParentTransform();
}

void qSlicerTestMe2ModuleWidget::updateParentTransform()
{
  Q_D(qSlicerTestMe2ModuleWidget);
  if (!d->m_BeamNode || !d->m_BeamParentTransformNode)
  {
    qWarning() << Q_FUNC_INFO << "Beam and Parent transform nodes are invalid";
    return;
  }
  double isocenter[3] = {};
  d->m_BeamNode->GetPlanIsocenterPosition(isocenter);
  vtkNew< vtkTransform > transform, translate;
  
  translate->Identity();
  translate->Translate(isocenter[0], isocenter[1], isocenter[2]);
  translate->Update();

  transform->Identity();
  transform->RotateX(d->rotateX);
  transform->RotateY(d->rotateY);
  transform->RotateZ(d->rotateZ);
  transform->Update();
  vtkMRMLLinearTransformNode* beamTransform = vtkMRMLLinearTransformNode::SafeDownCast(d->m_BeamNode->GetParentTransformNode());
  if (beamTransform)
  {
    qDebug() << Q_FUNC_INFO << "Beam transform is valid";
    d->m_BeamParentTransformNode->SetAndObserveTransformToParent(transform);
    beamTransform->SetAndObserveTransformNodeID(d->m_BeamParentTransformNode->GetID());
  }
}

void qSlicerTestMe2ModuleWidget::onCheckNodesButtonClicked()
{
  Q_D(qSlicerTestMe2ModuleWidget);
  if (d->m_FiducialNode && d->m_TransformNode)
  {
    qDebug() << Q_FUNC_INFO << "Nodes are valid. " \
             << "fiducial node name is:" << d->m_FiducialNode->GetName() \
             << "transform node name is:" << d->m_TransformNode->GetName();
  }
  else
  {
    qWarning() << Q_FUNC_INFO << "Nodes are invalid";
    if (!d->m_FiducialNode)
    {
      qWarning() << Q_FUNC_INFO << "Fiducial node is invalid";
    }
    if (!d->m_TransformNode)
    {
      qWarning() << Q_FUNC_INFO << "Transform node is invalid";
    }
  }
}
