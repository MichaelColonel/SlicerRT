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
#include <vtkMRMLLinearTransformNode.h>

//-----------------------------------------------------------------------------
class qSlicerTestMe2ModuleWidgetPrivate: public Ui_qSlicerTestMe2ModuleWidget
{
public:
  qSlicerTestMe2ModuleWidgetPrivate();
  vtkSmartPointer< vtkMRMLMarkupsFiducialNode > m_FiducialNode;
  vtkSmartPointer< vtkMRMLLinearTransformNode > m_TransformNode;
  double offset{ 0.0 };
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

  connect( d->InputFiducial, SIGNAL(nodeAboutToBeRemoved(vtkMRMLNode *)), this,
    SLOT(onFiducialNodeRemoved()));

  connect( d->InputTransform, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
    SLOT(onTransformNodeChanged(vtkMRMLNode*)));

   connect( d->InputTransform, SIGNAL(nodeAboutToBeRemoved(vtkMRMLNode *)), this,
    SLOT(onTransformNodeRemoved()));

  connect( d->CheckNodesButton, SIGNAL(clicked()), this,
    SLOT(onCheckNodesButtonClicked()));
}


void qSlicerTestMe2ModuleWidget::onFiducialNodeChanged(vtkMRMLNode *node)
{
  Q_D(qSlicerTestMe2ModuleWidget);
  if (node)
  {
    qDebug() << Q_FUNC_INFO << "Fiducial node is changed";
    d->m_FiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(node);
    if (d->m_FiducialNode)
    {
      qDebug() << Q_FUNC_INFO << "Fiducial node name is" << d->m_FiducialNode->GetName();
    }
  }
}

void qSlicerTestMe2ModuleWidget::onTransformNodeChanged(vtkMRMLNode *node)
{
  Q_D(qSlicerTestMe2ModuleWidget);
  if (node)
  {
    qDebug() << Q_FUNC_INFO << "Transform node is changed";
    d->m_TransformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
    if (d->m_TransformNode)
    {
      qDebug() << Q_FUNC_INFO << "Transform node name is" << d->m_TransformNode->GetName();
    }
  }
}

void qSlicerTestMe2ModuleWidget::onFiducialNodeRemoved()
  {
    Q_D(qSlicerTestMe2ModuleWidget);
    d->m_FiducialNode = nullptr;
    qDebug() << Q_FUNC_INFO << "Fiducial node pointer is" << d->m_FiducialNode.GetPointer();
  };

void qSlicerTestMe2ModuleWidget::onTransformNodeRemoved()
  {
    Q_D(qSlicerTestMe2ModuleWidget);
    d->m_TransformNode = nullptr;
    qDebug() << Q_FUNC_INFO << "Transform node pointer is" << d->m_TransformNode.GetPointer();
  };

void qSlicerTestMe2ModuleWidget::onCheckNodesButtonClicked()
{
  Q_D(qSlicerTestMe2ModuleWidget);
  if (d->m_FiducialNode && d->m_TransformNode)
  {
      qDebug() << Q_FUNC_INFO << "Fiducial & Transform nodes are valid";
      qDebug() << "Fiducial node name is" << d->m_FiducialNode->GetName();
      qDebug() << "Transform node name is" << d->m_TransformNode->GetName();
  }
  else
  {
      qWarning() << Q_FUNC_INFO << "Nodes are invalid";
      if (!d->m_FiducialNode) qWarning() << "Fiducial node is invalid";
      if (!d->m_TransformNode) qWarning() << "Transform node is invalid";
  }
}
