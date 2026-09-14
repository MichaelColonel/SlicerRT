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
#include <QOpcUaClient>

// SiemensPlcOpcUa Widgets includes
#include "qSlicerSiemensPlcOpcUaWidget.h"
#include "ui_qSlicerSiemensPlcOpcUaWidget.h"

#include <vtkMRMLSiemensPlcOpcUaNode.h>

//-----------------------------------------------------------------------------
class qSlicerSiemensPlcOpcUaWidgetPrivate : public Ui_qSlicerSiemensPlcOpcUaWidget
{
  Q_DECLARE_PUBLIC(qSlicerSiemensPlcOpcUaWidget)
protected:
  qSlicerSiemensPlcOpcUaWidget* const q_ptr;

public:
  qSlicerSiemensPlcOpcUaWidgetPrivate(qSlicerSiemensPlcOpcUaWidget& object);
  virtual void setupUi(qSlicerSiemensPlcOpcUaWidget*);
  bool ParseServerParentNode();

  QWeakPointer< QOpcUaClient > OpcUaClient;
  vtkWeakPointer< vtkMRMLSiemensPlcOpcUaNode > ParameterNode;

  // Messages nodes
  QScopedPointer<QOpcUaNode> ErrorMessagesNode;
  QScopedPointer<QOpcUaNode> ServiceMessagesNode;
  QScopedPointer<QOpcUaNode> MiscMessagesNode;

  // Parse parent node
  const QString SIEMENS_PLC_SERVER_INTERFACES_NODE_ID = "ns=3;s=ServerInterfaces";
  QScopedPointer<QOpcUaNode> ServerInterfacesNode;
  // Monitored node
  const QString SIEMENS_PLC_CURRENT_TIME_NODE_ID = "ns=0;i=2258";
  QScopedPointer<QOpcUaNode> CurrentTimeNode; // Siemens PLC monitored node to prevent session timeout ending

  QMap< QString, QString > NodeIdNameMap;
};

// --------------------------------------------------------------------------
qSlicerSiemensPlcOpcUaWidgetPrivate::qSlicerSiemensPlcOpcUaWidgetPrivate(qSlicerSiemensPlcOpcUaWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidgetPrivate::setupUi(qSlicerSiemensPlcOpcUaWidget* widget)
{
  this->Ui_qSlicerSiemensPlcOpcUaWidget::setupUi(widget);
}

bool qSlicerSiemensPlcOpcUaWidgetPrivate::ParseServerParentNode()
{
  return true;
}

//-----------------------------------------------------------------------------
// qSlicerScadaOpcUaRobotsControlWidget methods

//-----------------------------------------------------------------------------
qSlicerSiemensPlcOpcUaWidget::qSlicerSiemensPlcOpcUaWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerSiemensPlcOpcUaWidgetPrivate(*this) )
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerSiemensPlcOpcUaWidget::~qSlicerSiemensPlcOpcUaWidget()
{
}

void qSlicerSiemensPlcOpcUaWidget::setParameterNode(vtkMRMLNode* node)
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);

  vtkMRMLSiemensPlcOpcUaNode* parameterNode = vtkMRMLSiemensPlcOpcUaNode::SafeDownCast(node);
  // Each time the node is modified, the UI widgets are updated
  qvtkReconnect(d->ParameterNode, parameterNode, vtkCommand::ModifiedEvent, 
    this, SLOT(updateWidgetFromMRML()));

  d->ParameterNode = parameterNode;

  this->updateWidgetFromMRML();
}

void qSlicerSiemensPlcOpcUaWidget::setSiemensPlcOpcUaClient(const QSharedPointer< QOpcUaClient >& sharedClient)
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);

  d->OpcUaClient = sharedClient;
}

void qSlicerSiemensPlcOpcUaWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  if (!d->ParameterNode)
  {
    return;
  }
  switch (d->ParameterNode->GetMode())
  {
  case vtkMRMLSiemensPlcOpcUaNode::AUTOMATIC_MANUAL:
    break;
  case vtkMRMLSiemensPlcOpcUaNode::SERVICE:
    break;
  case vtkMRMLSiemensPlcOpcUaNode::KUKA_CONTROLLERS:
    break;
  case vtkMRMLSiemensPlcOpcUaNode::UNKNOWN:
  default:
    break;
  }

  qDebug() << Q_FUNC_INFO << "Update SiemensPlcOpcUa buttons";
}

void qSlicerSiemensPlcOpcUaWidget::onOpcUaClientConnected()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  
  QSharedPointer< QOpcUaClient > sharedClient = d->OpcUaClient.toStrongRef();
  if (!sharedClient)
  {
    return;
  }
  d->ServerInterfacesNode.reset(sharedClient->node(d->SIEMENS_PLC_SERVER_INTERFACES_NODE_ID));

  d->CurrentTimeNode.reset(sharedClient->node(d->SIEMENS_PLC_CURRENT_TIME_NODE_ID));
  // Subscribe to data changes
  d->CurrentTimeNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(1000));
}

void qSlicerSiemensPlcOpcUaWidget::onOpcUaClientDisconnected()
{
}
