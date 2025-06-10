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

  vtkSmartPointer< vtkMRMLMarkupsFiducialNode > m_FiducialNode;
  vtkSmartPointer< vtkMRMLLinearTransformNode > m_TransformNode;

  vtkSlicerTestMe2Logic* logic() const;
  double offset{ 0.0 };
};

//-----------------------------------------------------------------------------
// qSlicerTestMe2ModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerTestMe2ModuleWidgetPrivate::qSlicerTestMe2ModuleWidgetPrivate(qSlicerTestMe2ModuleWidget &object)
:
q_ptr(&object)
//,
//m_matrixTransform(vtkSmartPointer<vtkMatrix4x4>::New())
{
//    m_matrixTransform->Identity();
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
  connect( d->InputFiducial, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
    SLOT(onFiducialNodeChanged(vtkMRMLNode*)));

  connect( d->InputFiducial, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
    SLOT(onCheckNodesButtonClicked()));

  connect( d->InputTransform, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
    SLOT(onTransformNodeChanged(vtkMRMLNode*)));


  connect( d->InputTransform, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
    SLOT(onCheckNodesButtonClicked()));

  connect( d->CheckNodesButton, SIGNAL(clicked()), this,
    SLOT(onCheckNodesButtonClicked()));

  connect( d->HeightSlider, SIGNAL(valueChanged(double)), this,
    SLOT(onHeightSliderMove(double)));


}


void qSlicerTestMe2ModuleWidget::onFiducialNodeChanged(vtkMRMLNode *node)
{
  Q_D(qSlicerTestMe2ModuleWidget);
  d->ParameterNode->SetAndObserveFiducialNode(vtkMRMLMarkupsFiducialNode::SafeDownCast(node));
  if (node)
  {
    qDebug() << Q_FUNC_INFO << "Fiducial node is changed";
  }

    qDebug() << Q_FUNC_INFO << "Fiducial node name is" << d->ParameterNode->GetFiducialNode();

}

void qSlicerTestMe2ModuleWidget::onTransformNodeChanged(vtkMRMLNode *node)
{
  Q_D(qSlicerTestMe2ModuleWidget);
  d->ParameterNode->SetAndObserveTransformNode(vtkMRMLLinearTransformNode::SafeDownCast(node));
  if (node)
  {
    qDebug() << Q_FUNC_INFO << "Transform node is changed";
  }
    qDebug() << Q_FUNC_INFO << "Transform node name is" << d->ParameterNode->GetTransformNode();
  /*
  if (d->ParameterNode->GetTransformNode())
  {
    if (d->ParameterNode->GetFiducialNode())
    {
      d->{m_FiducialNode}->SetAndObserveTransformNodeID(d->m_TransformNode->GetID());
    }
    qDebug() << Q_FUNC_INFO << "Transform node name is" << d->m_TransformNode->GetName();
  }
    */
}

void qSlicerTestMe2ModuleWidget::onCheckNodesButtonClicked()
{
  Q_D(qSlicerTestMe2ModuleWidget);
  if (d->m_FiducialNode && d->m_TransformNode)
  {
    d->HeightSlider->setEnabled(true);
    qDebug() << Q_FUNC_INFO << "Fiducial & Transform nodes are valid";
    qDebug() << "Fiducial node name is" << d->ParameterNode->GetFiducialNode();
    qDebug() << "Transform node name is" << d->ParameterNode->GetTransformNode();
  }
  else
  {
    qWarning() << Q_FUNC_INFO << "Nodes are invalid";
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
  if (d->ParameterNode->GetFiducialNode() && d->ParameterNode->GetTransformNode())
  {
    d->logic()->updateHeight(d->m_TransformNode, height);
  }
}
