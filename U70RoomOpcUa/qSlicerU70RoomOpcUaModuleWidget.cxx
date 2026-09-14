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
#include <QTextStream>
#include <QMessageBox>

#include <QOpcUaProvider>
#include <QOpcUaAuthenticationInformation>
#include <QOpcUaErrorState>
#include <QOpcUaExtensionObject>

// Slicer includes
#include "qSlicerU70RoomOpcUaModuleWidget.h"
#include "ui_qSlicerU70RoomOpcUaModuleWidget.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLLayoutLogic.h>

// Slicer includes
#include <qSlicerSingletonViewFactory.h>
#include <qSlicerLayoutManager.h>
#include <qSlicerApplication.h>

// STD includes
#include <bitset>

// vtkSlicer and qSlicer logic includes
#include <vtkSlicerU70RoomOpcUaLogic.h>
//#include "qSlicerScadaOpcUaLogic.h"
#include "qSlicerSiemensPlcOpcUaWidget.h"
#include "qSlicerOpcUaRobotsMessagesListModel.h"

// SlicerRT U70RoomOpcUa MRML includes
#include <vtkMRMLSiemensPlcOpcUaNode.h>

// Local widgets includes
//#include "opcuamodel.h"

class QAbstractButton;


//-----------------------------------------------------------------------------
class qSlicerU70RoomOpcUaModuleWidgetPrivate: public Ui_qSlicerU70RoomOpcUaModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerU70RoomOpcUaModuleWidget)
protected:
  qSlicerU70RoomOpcUaModuleWidget* const q_ptr;
public:
  qSlicerU70RoomOpcUaModuleWidgetPrivate(qSlicerU70RoomOpcUaModuleWidget& object);
  vtkSlicerU70RoomOpcUaLogic* logic() const;
  void createClient();
  void updateUiState();

  const char* SIEMENS_PLC_OPCUA_LAYOUT_DESCRIPTION = \
    "<layout type=\"vertical\">" \
    " <item>" \
    "  <SiemensPlcOpcUa></SiemensPlcOpcUa>" \
    " </item>" \
    "</layout>";
  const int SIEMENS_PLC_OPCUA_LAYOUT_ID = 1021;

  int PreviousLayoutId{ -1 };
  bool ModuleWindowInitialized{ false };

  vtkSmartPointer< vtkMRMLSiemensPlcOpcUaNode > SiemensPlcOpcUaNode;
//  QSharedPointer< qSlicerScadaOpcUaLogic > ScadaOpcUaLogic;

  QScopedPointer< qSlicerSiemensPlcOpcUaWidget > SiemensPlcOpcUaControlWidget;

  QScopedPointer< QOpcUaProvider > OpcUaProvider;
  QSharedPointer< QOpcUaClient > OpcUaClient;
  bool ClientConnectedFlag{ false };
  QVector<QOpcUaEndpointDescription> OpcUaEndpointList;
  QOpcUaApplicationIdentity OpcUaAppIdentity;
  QOpcUaEndpointDescription OpcUaEndpoint; // current endpoint used to connect
  
  QScopedPointer<qSlicerOpcUaRobotsMessagesListModel> SiemensPlcMessagesModel;
};

//-----------------------------------------------------------------------------
// qSlicerU70RoomOpcUaModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
vtkSlicerU70RoomOpcUaLogic* qSlicerU70RoomOpcUaModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerU70RoomOpcUaModuleWidget);
  return vtkSlicerU70RoomOpcUaLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
qSlicerU70RoomOpcUaModuleWidgetPrivate::qSlicerU70RoomOpcUaModuleWidgetPrivate(qSlicerU70RoomOpcUaModuleWidget& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidgetPrivate::createClient()
{
  Q_Q(qSlicerU70RoomOpcUaModuleWidget);
  if (!this->OpcUaClient)
  {
    QOpcUaClient* newClient = this->OpcUaProvider->createClient(this->ComboBox_OpcUaPlugin->currentText());
    this->OpcUaClient = QSharedPointer< QOpcUaClient >(newClient, &QOpcUaClient::deleteLater);
    if (!this->OpcUaClient)
    {
      const QString message(QObject::tr("Connecting to the given sever failed. See the log for details."));
      QMessageBox::critical(q, QObject::tr("Failed to connect to server"), message);
      return;
    }

    QObject::connect(this->OpcUaClient.data(), &QOpcUaClient::connectError, q, &qSlicerU70RoomOpcUaModuleWidget::showErrorDialog);
    OpcUaClient->setApplicationIdentity(OpcUaAppIdentity);

    if (OpcUaClient->supportedUserTokenTypes().contains(QOpcUaUserTokenPolicy::TokenType::Certificate))
    {
      QOpcUaAuthenticationInformation authInfo;
      authInfo.setCertificateAuthentication();
      OpcUaClient->setAuthenticationInformation(authInfo);
    }

    QObject::connect(this->OpcUaClient.data(), &QOpcUaClient::connected, q, &qSlicerU70RoomOpcUaModuleWidget::clientConnected);
    QObject::connect(this->OpcUaClient.data(), &QOpcUaClient::disconnected, q, &qSlicerU70RoomOpcUaModuleWidget::clientDisconnected);
    QObject::connect(this->OpcUaClient.data(), &QOpcUaClient::errorChanged, q, &qSlicerU70RoomOpcUaModuleWidget::clientError);
    QObject::connect(this->OpcUaClient.data(), &QOpcUaClient::stateChanged, q, &qSlicerU70RoomOpcUaModuleWidget::clientState);
    QObject::connect(this->OpcUaClient.data(), &QOpcUaClient::endpointsRequestFinished, q, &qSlicerU70RoomOpcUaModuleWidget::getEndpointsComplete);
    QObject::connect(this->OpcUaClient.data(), &QOpcUaClient::findServersFinished, q, &qSlicerU70RoomOpcUaModuleWidget::findServersComplete);
  }
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidgetPrivate::updateUiState()
{
  Q_Q(qSlicerU70RoomOpcUaModuleWidget);

  this->PushButton_Connect->setText(this->ClientConnectedFlag ? QObject::tr("&Disconnect") : QObject::tr("&Connect"));
  this->ComboBox_OpcUaPlugin->setDisabled(this->ClientConnectedFlag);
  this->LineEdit_OpcUaServerUrl->setDisabled(this->ClientConnectedFlag);
  if (!this->ClientConnectedFlag)
  {
    this->SiemensPlcMessagesModel->clear();
  }
}

//-----------------------------------------------------------------------------
// qSlicerU70RoomOpcUaModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerU70RoomOpcUaModuleWidget::qSlicerU70RoomOpcUaModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerU70RoomOpcUaModuleWidgetPrivate(*this) )
{
}

//-----------------------------------------------------------------------------
qSlicerU70RoomOpcUaModuleWidget::~qSlicerU70RoomOpcUaModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::setup()
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

//  d->ScadaOpcUaLogic.reset(new qSlicerScadaOpcUaLogic(this));
  d->SiemensPlcOpcUaControlWidget.reset(new qSlicerSiemensPlcOpcUaWidget(this));
//  d->ScadaOpcUaModel.reset(new OpcUaModel(this));
  d->SiemensPlcMessagesModel.reset(new qSlicerOpcUaRobotsMessagesListModel(this));
  d->OpcUaProvider.reset(new QOpcUaProvider(this));

  d->LineEdit_OpcUaServerUrl->setText("opc.tcp://172.31.1.1:4840");
  d->ComboBox_OpcUaPlugin->addItems(d->OpcUaProvider->availableBackends());

//  d->SiemensPlcOpcUaControlWidget->setScadaOpcUaLogic(d->ScadaOpcUaLogic);

  d->ListView_SiemensPlcMessages->setModel(d->SiemensPlcMessagesModel.data());

  if (d->ComboBox_OpcUaPlugin->count() == 0)
  {
    d->ComboBox_OpcUaPlugin->setDisabled(true);
    d->PushButton_Connect->setDisabled(true);
    QMessageBox::critical(this, tr("No OPCUA plugins available"), tr("The list of available OPCUA plugins is empty. No connection possible."));
  }

  QObject::connect(d->PushButton_Connect, &QPushButton::clicked, this, &qSlicerU70RoomOpcUaModuleWidget::connectToServer);
  QObject::connect(d->PushButton_FindServers, &QPushButton::clicked, this, &qSlicerU70RoomOpcUaModuleWidget::findServers);
  QObject::connect(d->PushButton_GetEndpoints, &QPushButton::clicked, this, &qSlicerU70RoomOpcUaModuleWidget::getEndpoints);
  QObject::connect(d->LineEdit_OpcUaServerUrl, &QLineEdit::returnPressed, d->PushButton_FindServers,
    [d]() { d->PushButton_FindServers->animateClick(); });

  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();

  qSlicerSingletonViewFactory* viewFactory = new qSlicerSingletonViewFactory();
  viewFactory->setWidget(d->SiemensPlcOpcUaControlWidget.get());
  viewFactory->setTagName("SiemensPlcOpcUa");

  layoutManager->registerViewFactory(viewFactory);

  // Save previous layout
  d->PreviousLayoutId = layoutManager->layout();

  vtkMRMLLayoutNode* layoutNode = layoutManager->layoutLogic()->GetLayoutNode();
  if (layoutNode)
  {
//    if (!layoutNode->SetLayoutDescription(d->SIEMENS_PLC_OPCUA_LAYOUT_ID, d->SIEMENS_PLC_OPCUA_LAYOUT_DESCRIPTION))
    {
      layoutNode->AddLayoutDescription(d->SIEMENS_PLC_OPCUA_LAYOUT_ID, d->SIEMENS_PLC_OPCUA_LAYOUT_DESCRIPTION);
    }
  }
  
  // Nodes
  // Widgets
  QObject::connect(d->PushButton_ShowSiemensPlcControls, &QPushButton::clicked, this, &qSlicerU70RoomOpcUaModuleWidget::onShowSiemensPlcControlsClicked);

  // qLogic
//  QObject::connect(d->ScadaOpcUaLogic.data(), SIGNAL(opcUaClientConnected(bool)),
//    this, SLOT(onScadaLogicConnected(bool)));
    
  // Handle scene change event if occurs
//  qvtkConnect(d->logic(), vtkCommand::ModifiedEvent, this, SLOT(onScadaOpcUaLogicModified()));
}

void qSlicerU70RoomOpcUaModuleWidget::onShowSiemensPlcControlsClicked()
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);
  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();

  layoutManager->pauseRender();
  if (d->PushButton_ShowSiemensPlcControls->isChecked())
  {
    if (layoutManager)
    {
      layoutManager->setLayout(d->SIEMENS_PLC_OPCUA_LAYOUT_ID);
    }
  }
  else
  {
    if (layoutManager)
    {
      layoutManager->setLayout(d->PreviousLayoutId);
    }
  }
  vtkMRMLLayoutNode* layoutNode = layoutManager->layoutLogic()->GetLayoutNode();
  layoutNode->Modified();
  layoutManager->resumeRender();

  slicerApplication->processEvents();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);

  this->Superclass::setMRMLScene(scene);

  qvtkReconnect(d->logic(), scene, vtkMRMLScene::EndImportEvent, this, SLOT(onSceneImportedEvent()));
  qvtkReconnect(d->logic(), scene, vtkMRMLScene::EndCloseEvent, this, SLOT(onSceneClosedEvent()));

  // Find parameters node or create it if there is none in the scene
  if (scene)
  {
    if (vtkMRMLNode* node = scene->GetFirstNodeByClass("vtkMRMLSiemensPlcOpcUaNode"))
    {
      vtkMRMLSiemensPlcOpcUaNode* newNode = vtkMRMLSiemensPlcOpcUaNode::SafeDownCast(node);
      if (newNode)
      {
        d->SiemensPlcOpcUaNode = vtkSmartPointer<vtkMRMLSiemensPlcOpcUaNode>::Take(newNode);
        d->SiemensPlcOpcUaControlWidget->setParameterNode(d->SiemensPlcOpcUaNode);
      }
    }
    else
    {
      d->SiemensPlcOpcUaNode = vtkSmartPointer<vtkMRMLSiemensPlcOpcUaNode>::New();
      std::string nodeName = this->mrmlScene()->GenerateUniqueName("SiemensPlcOpcUa");
      d->SiemensPlcOpcUaNode->SetName(nodeName.c_str());
      this->mrmlScene()->AddNode(d->SiemensPlcOpcUaNode);
      d->SiemensPlcOpcUaControlWidget->setParameterNode(d->SiemensPlcOpcUaNode);
      // Each time the node is modified, the UI widgets are updated
      qvtkReconnect(d->SiemensPlcOpcUaNode, vtkCommand::ModifiedEvent, 
        this, SLOT(updateWidgetFromMRML()));
    }
  }
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::onSceneImportedEvent()
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);

  this->onEnter();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::onSceneClosedEvent()
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);

  qDebug() << Q_FUNC_INFO << "Scene is closed";
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::onScadaOpcUaLogicModified()
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);

  qDebug() << Q_FUNC_INFO << "Logic modified";
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::enter()
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);

  this->onEnter();
  this->Superclass::enter();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::onEnter()
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid scene!";
    return;
  }

  // First check the logic if it has a parameter node
  if (!d->logic())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid logic!";
    return;
  }

  d->ModuleWindowInitialized = true;

  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();

  layoutManager->pauseRender();
  {
    QSignalBlocker block(d->PushButton_ShowSiemensPlcControls);
    d->PushButton_ShowSiemensPlcControls->setChecked(true);
  }
  layoutManager->setLayout(d->SIEMENS_PLC_OPCUA_LAYOUT_ID);
  vtkMRMLLayoutNode* layoutNode = layoutManager->layoutLogic()->GetLayoutNode();
  layoutNode->Modified();
  layoutManager->resumeRender();

  slicerApplication->processEvents();

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);

  if (!this->mrmlScene())
  {
    return;
  }
  if (!d->SiemensPlcOpcUaNode)
  {
    qCritical() << Q_FUNC_INFO << "Scada node is invalid";
    return;
  }
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::findServers()
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);

  QStringList localeIds;
  QStringList serverUris;
  QUrl url(d->LineEdit_OpcUaServerUrl->text());

  d->updateUiState();

  d->createClient();
  // set default port if missing
  if (url.port() == -1)
  {
    url.setPort(62544);
  }

  if (d->OpcUaClient)
  {
    d->OpcUaClient->findServers(url, localeIds, serverUris);
    qDebug() << "Discovering servers on " << url.toString();
  }
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::findServersComplete(const QVector<QOpcUaApplicationDescription> &servers, QOpcUa::UaStatusCode statusCode)
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);

  if (isSuccessStatus(statusCode))
  {
    d->ComboBox_Servers->clear();
    for (QOpcUaApplicationDescription server : servers)
    {
      QVector<QString> urls = server.discoveryUrls();
      for (const auto &url : qAsConst(urls))
      {
        d->ComboBox_Servers->addItem(url);
      }
    }
  }
  d->updateUiState();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::getEndpoints()
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);

  d->ComboBox_Endpoints->clear();
  d->updateUiState();

  if (d->ComboBox_Servers->currentIndex() >= 0)
  {

    const QString serverUrl = d->ComboBox_Servers->currentText();
    d->createClient();
    d->OpcUaClient->requestEndpoints(serverUrl);
  }
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::getEndpointsComplete(const QVector<QOpcUaEndpointDescription> &endpoints, QOpcUa::UaStatusCode statusCode)
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);

  int index = 0;
  const char *modes[] = { "Invalid", "None", "Sign", "SignAndEncrypt" };

  if (isSuccessStatus(statusCode))
  {
    d->OpcUaEndpointList = endpoints;
    for (const auto& endpoint : endpoints)
    {
      if (endpoint.securityMode() > sizeof(modes))
      {
        qWarning() << "Invalid security mode";
        continue;
      }
      const QString EndpointName = QString("%1 (%2)").arg(endpoint.securityPolicy(), modes[endpoint.securityMode()]);
      d->ComboBox_Endpoints->addItem(EndpointName, index++);
    }
  }
  d->updateUiState();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::connectToServer()
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);

  if (d->ClientConnectedFlag)
  {
    d->OpcUaClient->disconnectFromEndpoint();
    return;
  }

  if (d->ComboBox_Endpoints->currentIndex() >= 0)
  {
    d->OpcUaEndpoint = d->OpcUaEndpointList[d->ComboBox_Endpoints->currentIndex()];
    d->createClient();
    d->OpcUaClient->connectToEndpoint(d->OpcUaEndpoint);
  }
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::clientConnected()
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);

  d->ClientConnectedFlag = true;
  d->updateUiState();
  QObject::connect(d->OpcUaClient.data(), &QOpcUaClient::namespaceArrayUpdated, this, &qSlicerU70RoomOpcUaModuleWidget::namespacesArrayUpdated);
  d->OpcUaClient->updateNamespaceArray();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::clientDisconnected()
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);

  d->ClientConnectedFlag = false;
//  d->OpcUaClient->deleteLater();
  d->OpcUaClient.clear();
  d->updateUiState();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::namespacesArrayUpdated(const QStringList &namespaceArray)
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);

  if (namespaceArray.isEmpty())
  {
    qWarning() << "Failed to retrieve the namespaces array";
    return;
  }

  // set client for Siemens PLC widget
  d->SiemensPlcOpcUaControlWidget->setSiemensPlcOpcUaClient(d->OpcUaClient);

  QObject::disconnect(d->OpcUaClient.data(), &QOpcUaClient::namespaceArrayUpdated, this, &qSlicerU70RoomOpcUaModuleWidget::namespacesArrayUpdated);
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::showErrorDialog(QOpcUaErrorState *errorState)
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);
  const QString statuscode = QOpcUa::statusToString(errorState->errorCode());

  QString msg = errorState->isClientSideError() ? tr("The client reported: ") : tr("The server reported: ");

  switch (errorState->connectionStep())
  {
  case QOpcUaErrorState::ConnectionStep::Unknown:
    break;
  case QOpcUaErrorState::ConnectionStep::CertificateValidation:
    {
      msg += tr("Server certificate validation failed with error 0x%1 (%2).\nClick 'Abort' to abort the connect, or 'Ignore' to continue connecting.")
        .arg(static_cast<ulong>(errorState->errorCode()), 8, 16, QLatin1Char('0')).arg(statuscode);
      qDebug() << Q_FUNC_INFO << msg;
    }
    break;
  case QOpcUaErrorState::ConnectionStep::OpenSecureChannel:
    msg += tr("OpenSecureChannel failed with error 0x%1 (%2).").arg(errorState->errorCode(), 8, 16, QLatin1Char('0')).arg(statuscode);
    QMessageBox::warning(this, tr("Connection Error"), msg);
    break;
  case QOpcUaErrorState::ConnectionStep::CreateSession:
    msg += tr("CreateSession failed with error 0x%1 (%2).").arg(errorState->errorCode(), 8, 16, QLatin1Char('0')).arg(statuscode);
    QMessageBox::warning(this, tr("Connection Error"), msg);
    break;
  case QOpcUaErrorState::ConnectionStep::ActivateSession:
    msg += tr("ActivateSession failed with error 0x%1 (%2).").arg(errorState->errorCode(), 8, 16, QLatin1Char('0')).arg(statuscode);
    QMessageBox::warning(this, tr("Connection Error"), msg);
    break;
  default:
    break;
  }
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::clientError(QOpcUaClient::ClientError error)
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);
  qDebug() << "Client error changed" << error;
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::clientState(QOpcUaClient::ClientState state)
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);
  qDebug() << "Client state changed" << state;
  switch (state)
  {
  case QOpcUaClient::ClientState::Closing:
      break;
  case QOpcUaClient::ClientState::Disconnected:
    QMessageBox::information(this, tr("OPC UA"), tr("Client disconnected"));
    break;
  default:
    break;
  }
}
