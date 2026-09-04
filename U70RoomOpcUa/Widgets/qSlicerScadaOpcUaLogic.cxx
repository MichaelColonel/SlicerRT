/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// MLC device includes
#include "qSlicerScadaOpcUaLogic.h"

// SlicerRT ScadaOpcUa MRML includes
#include <vtkMRMLScadaOpcUaNode.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLSubjectHierarchyNode.h>
#include <vtkMRMLSubjectHierarchyConstants.h>
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLTableNode.h>

// Slicer includes
#include "qSlicerCoreApplication.h"
#include "vtkSlicerApplicationLogic.h"

// OPC UA headers
#include <QOpcUaProvider>

// VTK includes
#include <vtkSmartPointer.h>

// Qt includes
#include <QDebug>
#include <QVariant>

// STD includes
#include <cstring>
#include <bitset>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy
class qSlicerScadaOpcUaLogicPrivate
{
  Q_DECLARE_PUBLIC(qSlicerScadaOpcUaLogic);
protected:
  qSlicerScadaOpcUaLogic* const q_ptr;
public:
  qSlicerScadaOpcUaLogicPrivate(qSlicerScadaOpcUaLogic& object);
  ~qSlicerScadaOpcUaLogicPrivate();
  void loadApplicationSettings();
  
  vtkWeakPointer<vtkMRMLScadaOpcUaNode> ParameterNode;

  QScopedPointer< QOpcUaProvider > OpcUaProvider;
  QScopedPointer< QOpcUaClient > OpcUaClient;
  bool ClientConnectedFlag{ false };

  // Messages
  QScopedPointer<QOpcUaNode> ErrorMessagesNode;
  QScopedPointer<QOpcUaNode> ServiceMessagesNode;
  QScopedPointer<QOpcUaNode> MiscMessagesNode;
  QScopedPointer<QOpcUaNode> ServerInterfacesNode;
  QScopedPointer<QOpcUaNode> SiemensPlcHeartbeatNode;
};

//-----------------------------------------------------------------------------
// qSlicerIhepMlcDeviceLogicPrivate methods

//-----------------------------------------------------------------------------
qSlicerScadaOpcUaLogicPrivate::qSlicerScadaOpcUaLogicPrivate(qSlicerScadaOpcUaLogic& object)
  : q_ptr(&object),
  OpcUaProvider(new QOpcUaProvider(&object))
{
  qDebug() << Q_FUNC_INFO << this->OpcUaProvider->availableBackends();
}

//-----------------------------------------------------------------------------
qSlicerScadaOpcUaLogicPrivate::~qSlicerScadaOpcUaLogicPrivate()
{
  if (this->OpcUaClient && this->ClientConnectedFlag)
  {
    this->OpcUaClient->disconnectFromEndpoint();
  }
}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaLogicPrivate::loadApplicationSettings()
{
  //TODO: Implement if there are application settings (such as default dose engine)
  //      See qSlicerSubjectHierarchyPluginLogicPrivate::loadApplicationSettings
}

//-----------------------------------------------------------------------------
// qSlicerScadaOpcUaLogic methods

//----------------------------------------------------------------------------
qSlicerScadaOpcUaLogic::qSlicerScadaOpcUaLogic(QObject* parent)
  : Superclass(parent)
  , d_ptr( new qSlicerScadaOpcUaLogicPrivate(*this) )
{
}

//----------------------------------------------------------------------------
qSlicerScadaOpcUaLogic::~qSlicerScadaOpcUaLogic() = default;

//-----------------------------------------------------------------------------

void qSlicerScadaOpcUaLogic::setServerUrl(const QString& url)
{
  Q_D(qSlicerScadaOpcUaLogic);
  d->ServerUrl = url;
}

void qSlicerScadaOpcUaLogic::setOpcUaPlugin(const QString& plugin)
{
  Q_D(qSlicerScadaOpcUaLogic);
  d->OpcUaPlugin = plugin;
}
  
void qSlicerScadaOpcUaLogic::setMRMLScene(vtkMRMLScene* scene)
{
  this->qSlicerObject::setMRMLScene(scene);

  // Connect scene node added event so that the new subject hierarchy nodes can be claimed by a plugin
  qvtkReconnect(scene, vtkMRMLScene::NodeAddedEvent, this, SLOT( onNodeAdded(vtkObject*,vtkObject*) ) );
  // Connect scene import ended event so that subject hierarchy nodes can be created for supported data nodes if missing (backwards compatibility)
  qvtkReconnect(scene, vtkMRMLScene::EndImportEvent, this, SLOT( onSceneImportEnded(vtkObject*) ) );
}

//-----------------------------------------------------------------------------

void qSlicerScadaOpcUaLogic::onNodeAdded(vtkObject* sceneObject, vtkObject* nodeObject)
{
  vtkMRMLScene* scene = vtkMRMLScene::SafeDownCast(sceneObject);
  if (!scene)
  {
    return;
  }

  if (nodeObject->IsA("vtkMRMLScadaOpcUaNode"))
  {
    qDebug() << Q_FUNC_INFO << "Parameter node added";
    // Observe MLC control node changes
//    vtkMRMLIhepMlcControlNode* mlcNode = vtkMRMLIhepMlcControlNode::SafeDownCast(nodeObject);
//    qvtkConnect( mlcNode, vtkMRMLIhepMlcControlNode::Modified, this, SLOT( applyDoseEngineInPlan(vtkObject*) ) );
  }
}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaLogic::onSceneImportEnded(vtkObject* sceneObject)
{
  vtkMRMLScene* scene = vtkMRMLScene::SafeDownCast(sceneObject);
  if (!scene)
  {
    return;
  }

  // Traverse all plan nodes in the scene and observe dose engine changed event so that default
  // beam parameters can be added for the newly selected engine in the beams contained by the plan
//  std::vector<vtkMRMLNode*> planNodes;
//  scene->GetNodesByClass("vtkMRMLRTPlanNode", planNodes);
//  for (std::vector<vtkMRMLNode*>::iterator planNodeIt = planNodes.begin(); planNodeIt != planNodes.end(); ++planNodeIt)
//  {
//    vtkMRMLNode* planNode = (*planNodeIt);
//    qvtkConnect( planNode, vtkMRMLRTPlanNode::DoseEngineChanged, this, SLOT( applyDoseEngineInPlan(vtkObject*) ) );
//  }
}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaLogic::setParameterNode(vtkMRMLNode* node)
{
  Q_D(qSlicerScadaOpcUaLogic);

  vtkMRMLScadaOpcUaNode* parameterNode = vtkMRMLScadaOpcUaNode::SafeDownCast(node);
  // Each time the node is modified, the UI widgets are updated
  qvtkReconnect(d->ParameterNode, parameterNode, vtkCommand::ModifiedEvent, 
    this, SLOT(updateLogicFromMRML()));

  d->ParameterNode = parameterNode;
  qDebug() << Q_FUNC_INFO << "Parameter node added";
  this->updateLogicFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaLogic::updateLogicFromMRML()
{
  Q_D(qSlicerScadaOpcUaLogic);
  qDebug() << Q_FUNC_INFO << "Update SCADA OPC UA logic from node";
}

void qSlicerScadaOpcUaLogic::createClient()
{
  Q_D(qSlicerScadaOpcUaLogic);
  if (!d->OpcUaClient)
  {
    d->OpcUaClient.reset(OpcUaProvider->createClient(d->OpcUaPlugin));
    if (!OpcUaClient)
    {
      return;
    }

    QObject::connect(OpcUaClient.data(), &QOpcUaClient::connectError, this, &qSlicerScadaOpcUaLogic::processConnectError);
    OpcUaClient->setApplicationIdentity(OpcUaAppIdentity);

    if (OpcUaClient->supportedUserTokenTypes().contains(QOpcUaUserTokenPolicy::TokenType::Certificate))
    {
      QOpcUaAuthenticationInformation authInfo;
      authInfo.setCertificateAuthentication();
      OpcUaClient->setAuthenticationInformation(authInfo);
    }

    QObject::connect(OpcUaClient, &QOpcUaClient::connected, this, &MainWindow::clientConnected);
    QObject::connect(OpcUaClient, &QOpcUaClient::disconnected, this, &MainWindow::clientDisconnected);
    QObject::connect(OpcUaClient, &QOpcUaClient::errorChanged, this, &MainWindow::clientError);
    QObject::connect(OpcUaClient, &QOpcUaClient::stateChanged, this, &MainWindow::clientState);
    QObject::connect(OpcUaClient, &QOpcUaClient::endpointsRequestFinished, this, &MainWindow::getEndpointsComplete);
    QObject::connect(OpcUaClient, &QOpcUaClient::findServersFinished, this, &MainWindow::findServersComplete);
  }
}

void MainWindow::findServers()
{
  Q_D(qSlicerScadaOpcUaLogic);
  QStringList localeIds;
  QStringList serverUris;
  QUrl url(d->serverUrl);

  createClient();
  // set default port if missing
  if (url.port() == -1)
  {
    url.setPort(62544);
  }

  if (OpcUaClient)
  {
    OpcUaClient->findServers(url, localeIds, serverUris);
  }
}

void MainWindow::findServersComplete(const QVector<QOpcUaApplicationDescription> &servers, QOpcUa::UaStatusCode statusCode)
{
  if (isSuccessStatus(statusCode))
  {
    ui->ComboBox_Servers->clear();
    for (QOpcUaApplicationDescription server : servers)
    {
      QVector<QString> urls = server.discoveryUrls();
      for (const auto &url : qAsConst(urls))
      {
        ui->ComboBox_Servers->addItem(url);
      }
    }
  }
  updateUiState();
}

void MainWindow::getEndpoints()
{
  ui->ComboBox_Endpoints->clear();
  updateUiState();

  if (ui->ComboBox_Servers->currentIndex() >= 0)
  {

    const QString serverUrl = ui->ComboBox_Servers->currentText();
    createClient();
    OpcUaClient->requestEndpoints(serverUrl);
  }
}

void MainWindow::getEndpointsComplete(const QVector<QOpcUaEndpointDescription> &endpoints, QOpcUa::UaStatusCode statusCode)
{
  qDebug() << Q_FUNC_INFO << "Status code: " << statusCode << ", end points size: " << endpoints.size();

  int index = 0;
  const char *modes[] = { "Invalid", "None", "Sign", "SignAndEncrypt" };

  if (isSuccessStatus(statusCode))
  {
    OpcUaEndpointList = endpoints;
    for (const auto &endpoint : endpoints)
    {
      if (endpoint.securityMode() > sizeof(modes))
      {
        qWarning() << "Invalid security mode";
        continue;
      }
      const QString EndpointName = QString("%1 (%2)").arg(endpoint.securityPolicy(), modes[endpoint.securityMode()]);
      ui->ComboBox_Endpoints->addItem(EndpointName, index++);
    }
  }
  updateUiState();
}

void MainWindow::connectToServer()
{
  if (this->ClientConnectedFlag)
  {
    OpcUaClient->disconnectFromEndpoint();
    return;
  }

  if (ui->ComboBox_Endpoints->currentIndex() >= 0)
  {
    OpcUaEndpoint = OpcUaEndpointList[ui->ComboBox_Endpoints->currentIndex()];
    createClient();
    OpcUaClient->connectToEndpoint(OpcUaEndpoint);
  }
}

void MainWindow::clientConnected()
{
  this->ClientConnectedFlag = true;
  this->updateUiState();
  QObject::connect(OpcUaClient, &QOpcUaClient::namespaceArrayUpdated, this, &MainWindow::namespacesArrayUpdated);
  OpcUaClient->updateNamespaceArray();
}

void MainWindow::clientDisconnected()
{
  this->ClientConnectedFlag = false;
  OpcUaClient->deleteLater();
  OpcUaClient = nullptr;
  OpcUaModelData->setOpcUaClient(nullptr);
  this->OpcUaScadaErrorMessagesNode.reset(nullptr);
  this->OpcUaScadaServiceMessagesNode.reset(nullptr);
  this->OpcUaScadaMiscMessagesNode.reset(nullptr);
  this->SiemensPlcTimeNode.reset(nullptr);
  this->OpcUaScadaServerInterfacesNode.reset(nullptr);
  updateUiState();
}

void MainWindow::namespacesArrayUpdated(const QStringList &namespaceArray)
{
  if (namespaceArray.isEmpty())
  {
    qWarning() << "Failed to retrieve the namespaces array";
    return;
  }

  QObject::disconnect(OpcUaClient, &QOpcUaClient::namespaceArrayUpdated, this, &MainWindow::namespacesArrayUpdated);
  OpcUaModelData->setOpcUaClient(OpcUaClient);

  this->OpcUaModelData->setOpcUaClient(this->OpcUaClient);
  this->ui->OpcUaTreeView->header()->setSectionResizeMode(1 /* Value column*/, QHeaderView::Interactive);


  // Error messages
  this->OpcUaScadaErrorMessagesNode.reset(this->OpcUaClient->node(SCADA_ERRORMESSAGES_NODE_ID));
  // Connect signal handlers for subscribed values
  QObject::connect(this->OpcUaScadaErrorMessagesNode.data(), &QOpcUaNode::dataChangeOccurred, this, &MainWindow::ErrorMessagesChanged);
  // Subscribe to data changes
  this->OpcUaScadaErrorMessagesNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(1000));
  // Connect the handler for async reading
  QObject::connect(this->OpcUaScadaErrorMessagesNode.data(), &QOpcUaNode::attributeRead, this, &MainWindow::ErrorMessagesRead);

  // Service messages
  this->OpcUaScadaServiceMessagesNode.reset(this->OpcUaClient->node(SCADA_SERVICEMESSAGES_NODE_ID));
  // Connect signal handlers for subscribed values
  QObject::connect(this->OpcUaScadaServiceMessagesNode.data(), &QOpcUaNode::dataChangeOccurred, this, &MainWindow::ServiceMessagesChanged);
  // Subscribe to data changes
  this->OpcUaScadaServiceMessagesNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(1000));
  // Connect the handler for async reading
  QObject::connect(this->OpcUaScadaServiceMessagesNode.data(), &QOpcUaNode::attributeRead, this, &MainWindow::ServiceMessagesRead);

  // Misc messages
  this->OpcUaScadaMiscMessagesNode.reset(this->OpcUaClient->node(SCADA_MESSAGES_NODE_ID));
  // Connect signal handlers for subscribed values
  QObject::connect(this->OpcUaScadaMiscMessagesNode.data(), &QOpcUaNode::dataChangeOccurred, this, &MainWindow::MiscMessagesChanged);
  // Subscribe to data changes
  this->OpcUaScadaMiscMessagesNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(1000));
  // Connect the handler for async reading
  QObject::connect(this->OpcUaScadaMiscMessagesNode.data(), &QOpcUaNode::attributeRead, this, &MainWindow::MiscMessagesRead);

  this->OpcUaScadaServerInterfacesNode.reset(this->OpcUaClient->node("ns=3;s=ServerInterfaces"));
  QObject::connect(this->OpcUaScadaServerInterfacesNode.data(), &QOpcUaNode::attributeRead, this, &MainWindow::ServerInterfacesRead);
}

//-----------------------------------------------------------------------------
bool qSlicerScadaOpcUaLogic::disconnectFromServer()
{
  Q_D(qSlicerScadaOpcUaLogic);
  if (!d->ClientConnectedFlag)
  {
    d->ClientConnectedFlag = false;
    emit opcUaClientConnected(false);
    return false;
  }
  
  d->OpcUaClient->disconnectFromEndpoint();
  return true;
}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaLogic::clientConnected()
{
  Q_D(qSlicerScadaOpcUaLogic);

  d->ClientConnectedFlag = true;

  // emit signal before node connection
  emit opcUaClientConnected(true);
}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaLogic::clientDisconnected()
{
  Q_D(qSlicerScadaOpcUaLogic);

  d->ClientConnectedFlag = false;
  d->OpcUaClient->deleteLater();
  d->OpcUaClient.take();

  // emit signal before node connection
  emit opcUaClientConnected(false);
}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaLogic::clientError(QOpcUaClient::ClientError)
{
}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaLogic::clientState(QOpcUaClient::ClientState)
{
}
