/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QWeakPointer>

// ScadaOpcUaRobotsControl Widgets includes
#include "qSlicerScadaOpcUaRobotsControlWidget.h"
#include "ui_qSlicerScadaOpcUaRobotsControlWidget.h"

#include <qSlicerScadaOpcUaLogic.h>

#include <vtkMRMLScadaOpcUaNode.h>

//-----------------------------------------------------------------------------
class qSlicerScadaOpcUaRobotsControlWidgetPrivate : public Ui_qSlicerScadaOpcUaRobotsControlWidget
{
  Q_DECLARE_PUBLIC(qSlicerScadaOpcUaRobotsControlWidget);
protected:
  qSlicerScadaOpcUaRobotsControlWidget* const q_ptr;

public:
  qSlicerScadaOpcUaRobotsControlWidgetPrivate(qSlicerScadaOpcUaRobotsControlWidget& object);
  virtual void setupUi(qSlicerScadaOpcUaRobotsControlWidget*);

  QWeakPointer< qSlicerScadaOpcUaLogic > opcUaLogic;
  vtkWeakPointer< vtkMRMLScadaOpcUaNode > ParameterNode;
};

// --------------------------------------------------------------------------
qSlicerScadaOpcUaRobotsControlWidgetPrivate::qSlicerScadaOpcUaRobotsControlWidgetPrivate(qSlicerScadaOpcUaRobotsControlWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerScadaOpcUaRobotsControlWidgetPrivate::setupUi(qSlicerScadaOpcUaRobotsControlWidget* widget)
{
  this->Ui_qSlicerScadaOpcUaRobotsControlWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerScadaOpcUaRobotsControlWidget methods

//-----------------------------------------------------------------------------
qSlicerScadaOpcUaRobotsControlWidget::qSlicerScadaOpcUaRobotsControlWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerScadaOpcUaRobotsControlWidgetPrivate(*this) )
{
  Q_D(qSlicerScadaOpcUaRobotsControlWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerScadaOpcUaRobotsControlWidget::~qSlicerScadaOpcUaRobotsControlWidget()
{
}

void qSlicerScadaOpcUaRobotsControlWidget::setParameterNode(vtkMRMLNode* node)
{
  Q_D(qSlicerScadaOpcUaRobotsControlWidget);

  vtkMRMLScadaOpcUaNode* parameterNode = vtkMRMLScadaOpcUaNode::SafeDownCast(node);
  // Each time the node is modified, the UI widgets are updated
  qvtkReconnect(d->ParameterNode, parameterNode, vtkCommand::ModifiedEvent, 
    this, SLOT(updateWidgetFromMRML()));

  d->ParameterNode = parameterNode;

  this->updateWidgetFromMRML();
}

void qSlicerScadaOpcUaRobotsControlWidget::setScadaOpcUaLogic(const QSharedPointer< qSlicerScadaOpcUaLogic >& sharedLogic)
{
  Q_D(qSlicerScadaOpcUaRobotsControlWidget);
  d->opcUaLogic = sharedLogic;
}

void qSlicerScadaOpcUaRobotsControlWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerScadaOpcUaRobotsControlWidget);
  qDebug() << Q_FUNC_INFO << "Update ScadaOpcUaRobotsControl buttons";
}
