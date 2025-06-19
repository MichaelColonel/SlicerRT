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
#include <vtkMatrix4x4.h>

//-----------------------------------------------------------------------------
class qSlicerTestMe2ModuleWidgetPrivate: public Ui_qSlicerTestMe2ModuleWidget
{
public:
  qSlicerTestMe2ModuleWidgetPrivate();
  vtkSmartPointer< vtkMRMLRTBeamNode > m_BeamNode;
  vtkSmartPointer< vtkMRMLTransformNode > m_BeamParentTransformNode;

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

void qSlicerTestMe2ModuleWidget::onBeamNodeChanged(vtkMRMLNode *node)
{
  Q_D(qSlicerTestMe2ModuleWidget);
  d->m_BeamNode = vtkMRMLRTBeamNode::SafeDownCast(node);
  if (d->m_BeamParentTransformNode)
  {
    vtkMRMLLinearTransformNode* beamTransform = vtkMRMLLinearTransformNode::SafeDownCast(d->m_BeamNode->GetParentTransformNode());
    if (beamTransform)
    {
      if (!beamTransform->GetParentTransformNode())
      {
        qDebug() << Q_FUNC_INFO << "Set new parent transform";
        beamTransform->SetAndObserveTransformNodeID(d->m_BeamParentTransformNode->GetID());
      }
      else
      {
        qDebug() << Q_FUNC_INFO << "Change parent transform here";
      }
    }
  }
}

void qSlicerTestMe2ModuleWidget::onParentTransformNodeChanged(vtkMRMLNode *node)
{
  Q_D(qSlicerTestMe2ModuleWidget);
  d->m_BeamParentTransformNode = vtkMRMLTransformNode::SafeDownCast(node);
  if (d->m_BeamNode)
  {
    vtkMRMLLinearTransformNode* beamTransform = vtkMRMLLinearTransformNode::SafeDownCast(d->m_BeamNode->GetParentTransformNode());
    if (d->m_BeamParentTransformNode && beamTransform)
    {
      qDebug() << Q_FUNC_INFO << "Set Parent transform";
      beamTransform->SetAndObserveTransformNodeID(d->m_BeamParentTransformNode->GetID());
    }
  }
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
    qWarning() << Q_FUNC_INFO << "Beam or Parent transform nodes are invalid";
    return;
  }
  double isocenter[4] = {};
  d->m_BeamNode->GetPlanIsocenterPosition(isocenter);
  vtkNew< vtkTransform > transform;
  transform->Identity();
  transform->PostMultiply();
  transform->RotateX(d->rotateX);
  transform->RotateY(d->rotateY);
  transform->RotateZ(d->rotateZ);
  transform->Update();
  double tmp[4] = {}; // transformed isocenter
  transform->MultiplyPoint( isocenter, tmp);
  // translate to new position so overall transform accurs around isocenter
  transform->Translate(isocenter[0] - tmp[0], isocenter[1] - tmp[1], isocenter[2] - tmp[2]);
  transform->Update();
  d->m_BeamParentTransformNode->SetAndObserveTransformToParent(transform);
}
