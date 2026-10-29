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
#include "qSlicerSiemensPlcOpcUaWidget.h"
#include "qSlicerOpcUaRobotsMessagesListModel.h"

// SlicerRT U70RoomOpcUa MRML includes
#include <vtkMRMLSiemensPlcOpcUaNode.h>
#include <vtkMRMLScadaOpcUaNode.h>

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
  void siemensCreateClient();
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

  // Siemens PLC
  vtkSmartPointer< vtkMRMLSiemensPlcOpcUaNode > SiemensPlcOpcUaNode;
//  QSharedPointer< qSlicerScadaOpcUaLogic > ScadaOpcUaLogic;
  QScopedPointer< qSlicerSiemensPlcOpcUaWidget > SiemensPlcOpcUaControlWidget;
  QScopedPointer< QOpcUaProvider > SiemensOpcUaProvider;
  QSharedPointer< QOpcUaClient > SiemensOpcUaClient;
  bool SiemensClientConnectedFlag{ false };
  QVector<QOpcUaEndpointDescription> SiemensOpcUaEndpointList;
  QOpcUaApplicationIdentity SiemensOpcUaAppIdentity;
  QOpcUaEndpointDescription SiemensOpcUaEndpoint; // current endpoint used to connect
  QScopedPointer< qSlicerOpcUaRobotsMessagesListModel > SiemensPlcMessagesModel;

  // Scada
  vtkSmartPointer< vtkMRMLScadaOpcUaNode > ScadaOpcUaNode;
//  QSharedPointer< qSlicerScadaOpcUaLogic > ScadaOpcUaLogic;
//  QScopedPointer< qSlicerSiemensPlcOpcUaWidget > SiemensPlcOpcUaControlWidget;
  QScopedPointer< QOpcUaProvider > ScadaOpcUaProvider;
  QSharedPointer< QOpcUaClient > ScadaOpcUaClient;
  bool ScadaClientConnectedFlag{ false };
  QVector<QOpcUaEndpointDescription> ScadaOpcUaEndpointList;
  QOpcUaApplicationIdentity ScadaOpcUaAppIdentity;
  QOpcUaEndpointDescription ScadaOpcUaEndpoint; // current endpoint used to connect
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
void qSlicerU70RoomOpcUaModuleWidgetPrivate::siemensCreateClient()
{
  Q_Q(qSlicerU70RoomOpcUaModuleWidget);
  if (!this->SiemensOpcUaClient)
  {
    QOpcUaClient* newClient = this->SiemensOpcUaProvider->createClient(this->ComboBox_SiemensOpcUaPlugin->currentText());
    this->SiemensOpcUaClient = QSharedPointer< QOpcUaClient >(newClient, &QOpcUaClient::deleteLater);
    if (!this->SiemensOpcUaClient)
    {
      const QString message(QObject::tr("Connecting to the given sever failed. See the log for details."));
      QMessageBox::critical(q, QObject::tr("Failed to connect to server"), message);
      return;
    }

    QObject::connect(this->SiemensOpcUaClient.data(), &QOpcUaClient::connectError,
      q, &qSlicerU70RoomOpcUaModuleWidget::siemensShowOpcUaErrorDialog);
    this->SiemensOpcUaClient->setApplicationIdentity(SiemensOpcUaAppIdentity);

    if (this->SiemensOpcUaClient->supportedUserTokenTypes().contains(QOpcUaUserTokenPolicy::TokenType::Certificate))
    {
      QOpcUaAuthenticationInformation authInfo;
      authInfo.setCertificateAuthentication();
      this->SiemensOpcUaClient->setAuthenticationInformation(authInfo);
    }

    QObject::connect(this->SiemensOpcUaClient.data(), &QOpcUaClient::connected,
      q, &qSlicerU70RoomOpcUaModuleWidget::siemensClientConnected);
//    QObject::connect(this->OpcUaClient.data(), &QOpcUaClient::connected,
//      this->SiemensPlcOpcUaControlWidget.data(), &qSlicerSiemensPlcOpcUaWidget::onOpcUaClientConnected);

    QObject::connect(this->SiemensOpcUaClient.data(), &QOpcUaClient::disconnected,
      q, &qSlicerU70RoomOpcUaModuleWidget::siemensClientDisconnected);
    QObject::connect(this->SiemensOpcUaClient.data(), &QOpcUaClient::disconnected,
      this->SiemensPlcOpcUaControlWidget.data(), &qSlicerSiemensPlcOpcUaWidget::onOpcUaClientDisconnected);

    QObject::connect(this->SiemensOpcUaClient.data(), &QOpcUaClient::errorChanged,
      q, &qSlicerU70RoomOpcUaModuleWidget::siemensClientError);
    QObject::connect(this->SiemensOpcUaClient.data(), &QOpcUaClient::stateChanged,
      q, &qSlicerU70RoomOpcUaModuleWidget::siemensClientState);
    QObject::connect(this->SiemensOpcUaClient.data(), &QOpcUaClient::endpointsRequestFinished,
      q, &qSlicerU70RoomOpcUaModuleWidget::siemensGetEndpointsComplete);
    QObject::connect(this->SiemensOpcUaClient.data(), &QOpcUaClient::findServersFinished,
      q, &qSlicerU70RoomOpcUaModuleWidget::siemensFindServersComplete);
  }
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidgetPrivate::updateUiState()
{
  Q_Q(qSlicerU70RoomOpcUaModuleWidget);

  this->PushButton_SiemensConnect->setText(this->SiemensClientConnectedFlag ? QObject::tr("&Disconnect") : QObject::tr("&Connect"));
  this->ComboBox_SiemensOpcUaPlugin->setDisabled(this->SiemensClientConnectedFlag);
  this->LineEdit_SiemensOpcUaServerUrl->setDisabled(this->SiemensClientConnectedFlag);
  if (!this->SiemensClientConnectedFlag)
  {
    this->SiemensPlcMessagesModel->clear();

    this->Label_ModeUnknown->setPixmap(QPixmap(":/Icons/gray.png"));
    this->Label_ModeAutomaticManual->setPixmap(QPixmap(":/Icons/gray.png"));
    this->Label_ModeService->setPixmap(QPixmap(":/Icons/gray.png"));
    this->Label_ModeKukaControllers->setPixmap(QPixmap(":/Icons/gray.png"));
    this->Label_ErrorsFlag->setPixmap(QPixmap(":/Icons/gray.png"));
    this->Label_ServiceFlags->setPixmap(QPixmap(":/Icons/gray.png"));
    this->Label_MessagesFlags->setPixmap(QPixmap(":/Icons/gray.png"));
    this->Label_CoordsAcceptedR1->setPixmap(QPixmap(":/Icons/gray.png"));
    this->Label_CoordsNotAcceptedR1->setPixmap(QPixmap(":/Icons/gray.png"));
    this->Label_CoordsAcceptedR2->setPixmap(QPixmap(":/Icons/gray.png"));
    this->Label_CoordsNotAcceptedR2->setPixmap(QPixmap(":/Icons/gray.png"));

    this->PushButton_GetServerInterfaces->setEnabled(false);
    this->PushButton_ModeAutoAndManual->setEnabled(false);
    this->PushButton_ModeService->setEnabled(false);
    this->PushButton_ModeKukaControllers->setEnabled(false);
    this->PushButton_ModeUnknown->setEnabled(false);
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
  d->SiemensOpcUaProvider.reset(new QOpcUaProvider(this));
  d->ScadaOpcUaProvider.reset(new QOpcUaProvider(this));

  d->LineEdit_SiemensOpcUaServerUrl->setText("opc.tcp://172.31.1.1:4840");
  d->ComboBox_SiemensOpcUaPlugin->addItems(d->SiemensOpcUaProvider->availableBackends());
  d->ComboBox_ScadaOpcUaPlugin->addItems(d->ScadaOpcUaProvider->availableBackends());

//  d->SiemensPlcOpcUaControlWidget->setScadaOpcUaLogic(d->ScadaOpcUaLogic);

  d->ListView_SiemensPlcMessages->setModel(d->SiemensPlcMessagesModel.data());
  d->ListView_SiemensPlcMessages->hide();

  if (d->ComboBox_SiemensOpcUaPlugin->count() == 0)
  {
    d->ComboBox_SiemensOpcUaPlugin->setDisabled(true);
    d->PushButton_SiemensConnect->setDisabled(true);
    QMessageBox::critical(this, tr("No OPCUA plugins available"), tr("The list of available OPCUA plugins is empty. No connection possible."));
  }

  QObject::connect(d->PushButton_SiemensConnect, &QPushButton::clicked,
    this, &qSlicerU70RoomOpcUaModuleWidget::siemensConnectToServer);
  QObject::connect(d->PushButton_SiemensFindServers, &QPushButton::clicked,
    this, &qSlicerU70RoomOpcUaModuleWidget::siemensFindServers);
  QObject::connect(d->PushButton_SiemensGetEndpoints, &QPushButton::clicked,
    this, &qSlicerU70RoomOpcUaModuleWidget::siemensGetEndpoints);
  QObject::connect(d->LineEdit_SiemensOpcUaServerUrl, &QLineEdit::returnPressed, d->PushButton_SiemensFindServers,
    [d]() { d->PushButton_SiemensFindServers->animateClick(); });

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
//    {
    layoutNode->AddLayoutDescription(d->SIEMENS_PLC_OPCUA_LAYOUT_ID, d->SIEMENS_PLC_OPCUA_LAYOUT_DESCRIPTION);
//    }
  }

  // Widgets
  QObject::connect(d->PushButton_ShowSiemensPlcControls, SIGNAL(clicked()),
    this, SLOT(onShowSiemensPlcControlsClicked()));
  QObject::connect(d->PushButton_GetServerInterfaces, SIGNAL(clicked()),
    d->SiemensPlcOpcUaControlWidget.data(), SLOT(onParseServerInterfacesClicked()));

  QObject::connect(d->PushButton_ModeUnknown, &QPushButton::clicked,
    [this](){ emit this->siemensPlcOpcUaModeChanged(vtkMRMLSiemensPlcOpcUaNode::UNKNOWN); });
  QObject::connect(d->PushButton_ModeAutoAndManual, &QPushButton::clicked,
    [this](){ emit this->siemensPlcOpcUaModeChanged(vtkMRMLSiemensPlcOpcUaNode::AUTOMATIC_MANUAL); });
  QObject::connect(d->PushButton_ModeService, &QPushButton::clicked,
    [this](){ emit this->siemensPlcOpcUaModeChanged(vtkMRMLSiemensPlcOpcUaNode::SERVICE); });
  QObject::connect(d->PushButton_ModeKukaControllers, &QPushButton::clicked,
    [this](){ emit this->siemensPlcOpcUaModeChanged(vtkMRMLSiemensPlcOpcUaNode::KUKA_CONTROLLERS); });

  QObject::connect(this, SIGNAL(siemensPlcOpcUaModeChanged(vtkMRMLSiemensPlcOpcUaNode::ModeType)),
    d->SiemensPlcOpcUaControlWidget.data(), SLOT(onOpcUaModeChanged(vtkMRMLSiemensPlcOpcUaNode::ModeType)));

  // Parse nodes automatically if auto connection flag is set
  QObject::connect(d->SiemensPlcOpcUaControlWidget.data(), &qSlicerSiemensPlcOpcUaWidget::nodesTreeLoadingFinished,
    [d]()
    {
      if (d->CheckBox_SiemensHostAutoConnect->isChecked())
      {
        d->SiemensPlcOpcUaControlWidget->onParseServerInterfacesClicked();
      }
    }
  );

  // Handle scene change event if occurs
  qvtkConnect(d->logic(), vtkCommand::ModifiedEvent, this, SLOT(onOpcUaLogicModified()));
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

  d->SiemensPlcOpcUaNode->Modified();
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
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::onOpcUaLogicModified()
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);
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
    qCritical() << Q_FUNC_INFO << tr(": Invalid scene!");
    return;
  }

  // First check the logic if it has a parameter node
  if (!d->logic())
  {
    qCritical() << Q_FUNC_INFO << tr(": Invalid logic!");
    return;
  }

  d->ModuleWindowInitialized = true;
/*
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
*/
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
    qCritical() << Q_FUNC_INFO << tr("Siemens PLC node is invalid");
    return;
  }
  if (!d->SiemensClientConnectedFlag)
  {
    d->SiemensPlcMessagesModel->clear();
    d->Label_ModeUnknown->setPixmap(QPixmap(":/Icons/gray.png"));
    d->Label_ModeAutomaticManual->setPixmap(QPixmap(":/Icons/gray.png"));
    d->Label_ModeService->setPixmap(QPixmap(":/Icons/gray.png"));
    d->Label_ModeKukaControllers->setPixmap(QPixmap(":/Icons/gray.png"));

    d->PushButton_GetServerInterfaces->setEnabled(false);
    d->PushButton_ModeAutoAndManual->setEnabled(false);
    d->PushButton_ModeService->setEnabled(false);
    d->PushButton_ModeKukaControllers->setEnabled(false);
    d->PushButton_ModeUnknown->setEnabled(false);

    return;
  }

  uint64_t errBits = d->SiemensPlcOpcUaNode->GetErrorMessages();
  uint64_t servBits = d->SiemensPlcOpcUaNode->GetServiceMessages();
  uint64_t miscBits = d->SiemensPlcOpcUaNode->GetMiscMessages();
  d->Label_ErrorsFlag->setPixmap(errBits ? QPixmap(":/Icons/red.png") : QPixmap(":/Icons/gray.png"));
  d->Label_ServiceFlags->setPixmap(servBits ? QPixmap(":/Icons/yellow.png") : QPixmap(":/Icons/gray.png"));
  d->Label_MessagesFlags->setPixmap(miscBits ? QPixmap(":/Icons/blue.png") : QPixmap(":/Icons/gray.png"));
  
  d->SiemensPlcMessagesModel->updateMessagesBits(errBits, servBits, miscBits);

  switch (d->SiemensPlcOpcUaNode->GetMode())
  {
  case vtkMRMLSiemensPlcOpcUaNode::UNKNOWN:
    d->Label_ModeUnknown->setPixmap(QPixmap(":/Icons/green.png"));
    d->Label_ModeAutomaticManual->setPixmap(QPixmap(":/Icons/gray.png"));
    d->Label_ModeService->setPixmap(QPixmap(":/Icons/gray.png"));
    d->Label_ModeKukaControllers->setPixmap(QPixmap(":/Icons/gray.png"));
    break;
  case vtkMRMLSiemensPlcOpcUaNode::AUTOMATIC_MANUAL:
    d->Label_ModeUnknown->setPixmap(QPixmap(":/Icons/gray.png"));
    d->Label_ModeAutomaticManual->setPixmap(QPixmap(":/Icons/green.png"));
    d->Label_ModeService->setPixmap(QPixmap(":/Icons/gray.png"));
    d->Label_ModeKukaControllers->setPixmap(QPixmap(":/Icons/gray.png"));
    break;
  case vtkMRMLSiemensPlcOpcUaNode::SERVICE:
    d->Label_ModeUnknown->setPixmap(QPixmap(":/Icons/gray.png"));
    d->Label_ModeAutomaticManual->setPixmap(QPixmap(":/Icons/gray.png"));
    d->Label_ModeService->setPixmap(QPixmap(":/Icons/green.png"));
    d->Label_ModeKukaControllers->setPixmap(QPixmap(":/Icons/gray.png"));
    break;
  case vtkMRMLSiemensPlcOpcUaNode::KUKA_CONTROLLERS:
    d->Label_ModeUnknown->setPixmap(QPixmap(":/Icons/gray.png"));
    d->Label_ModeAutomaticManual->setPixmap(QPixmap(":/Icons/gray.png"));
    d->Label_ModeService->setPixmap(QPixmap(":/Icons/gray.png"));
    d->Label_ModeKukaControllers->setPixmap(QPixmap(":/Icons/green.png"));
    break;
  default:
    d->Label_ModeUnknown->setPixmap(QPixmap(":/Icons/gray.png"));
    d->Label_ModeAutomaticManual->setPixmap(QPixmap(":/Icons/gray.png"));
    d->Label_ModeService->setPixmap(QPixmap(":/Icons/gray.png"));
    d->Label_ModeKukaControllers->setPixmap(QPixmap(":/Icons/gray.png"));
    break;
  }
  bool coordsAcceptenceFlags[4] = {};
  d->SiemensPlcOpcUaNode->GetCoordsAccepted(coordsAcceptenceFlags);
  d->Label_CoordsAcceptedR1->setPixmap(coordsAcceptenceFlags[0] ? QPixmap(":/Icons/green.png") : QPixmap(":/Icons/gray.png"));
  d->Label_CoordsNotAcceptedR1->setPixmap(coordsAcceptenceFlags[1] ? QPixmap(":/Icons/red.png") : QPixmap(":/Icons/gray.png"));
  d->Label_CoordsAcceptedR2->setPixmap(coordsAcceptenceFlags[2] ? QPixmap(":/Icons/green.png") : QPixmap(":/Icons/gray.png"));
  d->Label_CoordsNotAcceptedR2->setPixmap(coordsAcceptenceFlags[3] ? QPixmap(":/Icons/red.png") : QPixmap(":/Icons/gray.png"));
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::siemensFindServers()
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);

  QStringList localeIds;
  QStringList serverUris;
  QUrl url(d->LineEdit_SiemensOpcUaServerUrl->text());

  d->updateUiState();

  d->siemensCreateClient();
  // set default port if missing
  if (url.port() == -1)
  {
    url.setPort(62544);
  }

  if (d->SiemensOpcUaClient)
  {
    d->SiemensOpcUaClient->findServers(url, localeIds, serverUris);
    qDebug() << Q_FUNC_INFO << tr("Discovering servers on") << url.toString();
  }
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::siemensFindServersComplete(const QVector<QOpcUaApplicationDescription> &servers, QOpcUa::UaStatusCode statusCode)
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);

  if (isSuccessStatus(statusCode))
  {
    d->ComboBox_SiemensServers->clear();
    for (QOpcUaApplicationDescription server : servers)
    {
      QVector<QString> urls = server.discoveryUrls();
      for (const auto &url : qAsConst(urls))
      {
        d->ComboBox_SiemensServers->addItem(url);
      }
    }
  }
  d->updateUiState();
  emit d->PushButton_SiemensGetEndpoints->clicked();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::siemensGetEndpoints()
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);

  d->ComboBox_SiemensEndpoints->clear();
  d->updateUiState();

  if (d->ComboBox_SiemensServers->currentIndex() >= 0)
  {
    const QString serverUrl = d->ComboBox_SiemensServers->currentText();
    d->siemensCreateClient();
    d->SiemensOpcUaClient->requestEndpoints(serverUrl);
  }
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::siemensGetEndpointsComplete(const QVector<QOpcUaEndpointDescription> &endpoints, QOpcUa::UaStatusCode statusCode)
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);

  int index = 0;
  const char *modes[] = { "Invalid", "None", "Sign", "SignAndEncrypt" };

  if (isSuccessStatus(statusCode))
  {
    d->SiemensOpcUaEndpointList = endpoints;
    for (const auto& endpoint : endpoints)
    {
      if (endpoint.securityMode() > sizeof(modes))
      {
        qWarning() << Q_FUNC_INFO << tr("Invalid security mode");
        continue;
      }
      const QString EndpointName = QString("%1 (%2)").arg(endpoint.securityPolicy(), modes[endpoint.securityMode()]);
      d->ComboBox_SiemensEndpoints->addItem(EndpointName, index++);
    }
  }
  d->updateUiState();
  emit d->PushButton_SiemensConnect->clicked();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::siemensConnectToServer()
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);

  if (d->SiemensClientConnectedFlag)
  {
    d->SiemensOpcUaClient->disconnectFromEndpoint();
    return;
  }

  if (d->ComboBox_SiemensEndpoints->currentIndex() >= 0)
  {
    d->SiemensOpcUaEndpoint = d->SiemensOpcUaEndpointList[d->ComboBox_SiemensEndpoints->currentIndex()];
    d->siemensCreateClient();
    d->SiemensOpcUaClient->connectToEndpoint(d->SiemensOpcUaEndpoint);
  }
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::siemensClientConnected()
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);

  d->SiemensClientConnectedFlag = true;
  d->updateUiState();
  QObject::connect(d->SiemensOpcUaClient.data(), &QOpcUaClient::namespaceArrayUpdated,
    this, &qSlicerU70RoomOpcUaModuleWidget::siemensNamespacesArrayUpdated);
  d->SiemensOpcUaClient->updateNamespaceArray();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::siemensClientDisconnected()
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);

  d->SiemensClientConnectedFlag = false;
  d->SiemensOpcUaClient.clear();
  d->updateUiState();
  d->SiemensPlcOpcUaNode->SetMode(vtkMRMLSiemensPlcOpcUaNode::ModeType_Last);
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::siemensNamespacesArrayUpdated(const QStringList &namespaceArray)
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);

  if (namespaceArray.isEmpty())
  {
    qWarning() << Q_FUNC_INFO << tr("Failed to retrieve the namespaces array");
    return;
  }

  // set client for Siemens PLC widget
  d->PushButton_GetServerInterfaces->setEnabled(true);
  d->PushButton_ModeAutoAndManual->setEnabled(true);
  d->PushButton_ModeService->setEnabled(true);
  d->PushButton_ModeKukaControllers->setEnabled(true);
  d->PushButton_ModeUnknown->setEnabled(true);
  
  d->SiemensPlcOpcUaControlWidget->setSiemensPlcOpcUaClient(d->SiemensOpcUaClient);
  d->SiemensPlcOpcUaControlWidget->onOpcUaClientConnected();

  QObject::disconnect(d->SiemensOpcUaClient.data(), &QOpcUaClient::namespaceArrayUpdated,
    this, &qSlicerU70RoomOpcUaModuleWidget::siemensNamespacesArrayUpdated);
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::siemensShowOpcUaErrorDialog(QOpcUaErrorState *errorState)
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
void qSlicerU70RoomOpcUaModuleWidget::siemensClientError(QOpcUaClient::ClientError error)
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);
  Q_UNUSED(error);
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::siemensClientState(QOpcUaClient::ClientState state)
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);
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
