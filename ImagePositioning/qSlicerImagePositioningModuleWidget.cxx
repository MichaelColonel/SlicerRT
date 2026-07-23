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
#include "qSlicerImagePositioningModuleWidget.h"
#include "ui_qSlicerImagePositioningModuleWidget.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLLayoutLogic.h>

// Slicer includes
#include <qSlicerSingletonViewFactory.h>
#include <qSlicerLayoutManager.h>
#include <qSlicerApplication.h>

// Qt includes
#include <QOpcUaProvider>

// vtkSlicer and qSlicer logic includes
#include <vtkSlicerImagePositioningLogic.h>
#include "qSlicerScadaOpcUaLogic.h"
#include "qSlicerScadaOpcUaRobotsControlWidget.h"

// Local widgets includes
#include "opcuamodel.h"

class QAbstractButton;

//-----------------------------------------------------------------------------
class qSlicerImagePositioningModuleWidgetPrivate: public Ui_qSlicerImagePositioningModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerImagePositioningModuleWidget);
protected:
  qSlicerImagePositioningModuleWidget* const q_ptr;
public:
  qSlicerImagePositioningModuleWidgetPrivate(qSlicerImagePositioningModuleWidget& object);
  vtkSlicerImagePositioningLogic* logic() const;

  const char* GENERIC_LAYOUT_DESCRIPTION = \
    "<layout type=\"vertical\" split=\"true\" >"\
    "  <item splitSize=\"500\"> "\
    "    <layout type=\"horizontal\">"\
    "    <item>" \
    "     <view class=\"vtkMRMLViewNode\" singletontag=\"1\">" \
    "       <property name=\"viewlabel\" action=\"default\">1</property>" \
    "     </view>" \
    "    </item>" \
    "     <item>" \
    "       <view class=\"vtkMRMLSliceNode\" singletontag=\"XrayDetectorSlice\">" \
    "         <property name=\"orientation\" action=\"default\">Axial</property>" \
    "         <property name=\"viewlabel\" action=\"default\">C</property>" \
    "         <property name=\"viewcolor\" action=\"default\">#C3B1E1</property>" \
    "         <property name=\"viewgroup\" action=\"default\">101</property>" \
    "       </view>" \
    "     </item>" \
    "    </layout>" \
    "  </item>" \
    "  <item splitSize=\"350\">" \
    "    <layout type=\"horizontal\">" \
    "      <item>" \
    "         <view class=\"vtkMRMLSliceNode\" singletontag=\"Red\">" \
    "           <property name=\"orientation\" action=\"default\">Axial</property>" \
    "           <property name=\"viewlabel\" action=\"default\">R</property>" \
    "           <property name=\"viewcolor\" action=\"default\">#F34A33</property>" \
    "        </view>" \
    "      </item>" \
    "      <item>" \
    "         <view class=\"vtkMRMLSliceNode\" singletontag=\"Green\">" \
    "           <property name=\"orientation\" action=\"default\">Coronal</property>" \
    "           <property name=\"viewlabel\" action=\"default\">G</property>" \
    "           <property name=\"viewcolor\" action=\"default\">#6EB04B</property>" \
    "        </view>" \
    "      </item>" \
    "      <item>" \
    "         <view class=\"vtkMRMLSliceNode\" singletontag=\"Yellow\">" \
    "           <property name=\"orientation\" action=\"default\">Sagittal</property>" \
    "           <property name=\"viewlabel\" action=\"default\">Y</property>" \
    "           <property name=\"viewcolor\" action=\"default\">#EDD54C</property>" \
    "        </view>" \
    "      </item>" \
    "    </layout>" \
    "  </item>" \
    "  <item splitSize=\"0\">" \
    "    <layout type=\"horizontal\">" \
    "      <item>" \
    "        <view class=\"vtkMRMLViewNode\" singletontag=\"XrayDetectorSlice\">" \
    "           <property name=\"viewlabel\" action=\"default\">F</property>" \
    "           <property name=\"viewcolor\" action=\"default\">#C3B1E1</property>" \
    "           <property name=\"viewgroup\" action=\"default\">100</property>" \
    "        </view>" \
    "       </item>" \
    "    </layout>" \
    "  </item>" \
    "</layout>";
  const int GENERIC_LAYOUT_ID = 1020;

  const char* SCADA_OPCUA_ROBOTS_CONTROL_LAYOUT_DESCRIPTION = \
    "<layout type=\"vertical\">" \
    " <item>" \
    "  <ScadaOpcUaRobotsControl></ScadaOpcUaRobotsControl>" \
    " </item>" \
    "</layout>";
  const int SCADA_OPCUA_ROBOTS_CONTROL_LAYOUT_ID = 1021;

  const QString SCADA_PATPOS_NODE_ID = "ns=1;s=Модуль Позиционирования";
  const QString SCADA_TEST_READ_NODE_ID = SCADA_PATPOS_NODE_ID + ".SysTimeScada";

  int PreviousLayoutId{ -1 };
  bool ModuleWindowInitialized{ false };

  QScopedPointer< qSlicerScadaOpcUaLogic > ScadaOpcUaLogic;
  QScopedPointer< qSlicerScadaOpcUaRobotsControlWidget > ScadaOpcUaRobotsControlWidget;
  vtkSmartPointer< vtkMRMLScadaOpcUaNode > ScadaOpcUaNode;
  QScopedPointer< OpcUaModel > ScadaOpcUaModel;
  QScopedPointer<QOpcUaProvider> ScadaOpcUaProvider;
};

//-----------------------------------------------------------------------------
// qSlicerImagePositioningModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
vtkSlicerImagePositioningLogic* qSlicerImagePositioningModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerImagePositioningModuleWidget);
  return vtkSlicerImagePositioningLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
qSlicerImagePositioningModuleWidgetPrivate::qSlicerImagePositioningModuleWidgetPrivate(qSlicerImagePositioningModuleWidget& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
// qSlicerImagePositioningModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerImagePositioningModuleWidget::qSlicerImagePositioningModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerImagePositioningModuleWidgetPrivate(*this) )
{
}

//-----------------------------------------------------------------------------
qSlicerImagePositioningModuleWidget::~qSlicerImagePositioningModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::setup()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  d->ScadaOpcUaLogic.reset(new qSlicerScadaOpcUaLogic(this));
  d->ScadaOpcUaRobotsControlWidget.reset(new qSlicerScadaOpcUaRobotsControlWidget(this));
  d->ScadaOpcUaModel.reset(new OpcUaModel(this));
  d->ScadaOpcUaProvider.reset(new QOpcUaProvider(this));

  d->OpcUaServerUrlLineEdit->setText("opc.tcp://192.168.26.128:62544");
  d->OpcUaPluginComboBox->addItems(d->ScadaOpcUaProvider->availableBackends());
  
//  d->OpcUaTreeView->setModel(d->ScadaOpcUaModel.get());
//  d->OpcUaTreeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
//  d->OpcUaTreeView->setTextElideMode(Qt::ElideRight);
//  d->OpcUaTreeView->setAlternatingRowColors(true);
//  d->OpcUaTreeView->setSelectionBehavior(QAbstractItemView::SelectItems);

  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();

  // Save previous layout
  d->PreviousLayoutId = layoutManager->layout();

  vtkMRMLLayoutNode* layoutNode = layoutManager->layoutLogic()->GetLayoutNode();
  if (layoutNode)
  {
    if (!layoutNode->SetLayoutDescription(d->GENERIC_LAYOUT_ID, d->GENERIC_LAYOUT_DESCRIPTION))
    {
      layoutNode->AddLayoutDescription(d->GENERIC_LAYOUT_ID, d->GENERIC_LAYOUT_DESCRIPTION);
    }
  }


  qSlicerSingletonViewFactory* viewFactory = new qSlicerSingletonViewFactory();
  viewFactory->setWidget(d->ScadaOpcUaRobotsControlWidget.get());
  viewFactory->setTagName("ScadaOpcUaRobotsControl");

  layoutManager->registerViewFactory(viewFactory);
  // Save previous layout
  d->PreviousLayoutId = layoutManager->layout();

  if (layoutNode)
  {
    if (!layoutNode->SetLayoutDescription(d->SCADA_OPCUA_ROBOTS_CONTROL_LAYOUT_ID, d->SCADA_OPCUA_ROBOTS_CONTROL_LAYOUT_DESCRIPTION))
    {
      layoutNode->AddLayoutDescription(d->SCADA_OPCUA_ROBOTS_CONTROL_LAYOUT_ID, d->SCADA_OPCUA_ROBOTS_CONTROL_LAYOUT_DESCRIPTION);
    }
  }
  
  // Nodes

  // Buttons
  QObject::connect(d->PushButton_CustomLayout, SIGNAL(clicked()),
    this, SLOT(onSetCustomLayoutClicked()));
  QObject::connect(d->PushButton_ScadaConnect, SIGNAL(clicked()),
    this, SLOT(onConnectClicked()));
  QObject::connect(d->PushButton_SetLocalTime, SIGNAL(clicked()),
    this, SLOT(onSetLocalTimeClicked()));
  QObject::connect(d->PushButton_SetErrorMessage, SIGNAL(clicked()),
    this, SLOT(onSetErrorMessageClicked()));
  QObject::connect(d->PushButton_SetEventMessage, SIGNAL(clicked()),
    this, SLOT(onSetEventMessageClicked()));

  // qLogic
  QObject::connect(d->ScadaOpcUaLogic.data(), SIGNAL(opcUaClientConnected(bool)),
    this, SLOT(onScadaLogicConnected(bool)));
    
  // Handle scene change event if occurs
//  qvtkConnect(d->logic(), vtkCommand::ModifiedEvent, this, SLOT(onScadaOpcUaLogicModified()));
}

void qSlicerImagePositioningModuleWidget::onSetCustomLayoutClicked()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();

  if (d->PushButton_CustomLayout->isChecked())
  {
    if (layoutManager)
    {
      layoutManager->setLayout(d->SCADA_OPCUA_ROBOTS_CONTROL_LAYOUT_ID);
    }
  }
  else
  {
    if (layoutManager)
    {
      layoutManager->setLayout(d->PreviousLayoutId);
    }
  }
  slicerApplication->processEvents();
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerImagePositioningModuleWidget);

  this->Superclass::setMRMLScene(scene);

  qvtkReconnect(d->logic(), scene, vtkMRMLScene::EndImportEvent, this, SLOT(onSceneImportedEvent()));
  qvtkReconnect(d->logic(), scene, vtkMRMLScene::EndCloseEvent, this, SLOT(onSceneClosedEvent()));

  // Find parameters node or create it if there is none in the scene
  if (scene)
  {
    if (vtkMRMLNode* node = scene->GetFirstNodeByClass("vtkMRMLScadaOpcUaNode"))
    {
      vtkMRMLScadaOpcUaNode* newNode = vtkMRMLScadaOpcUaNode::SafeDownCast(node);
      if (newNode)
      {
        d->ScadaOpcUaNode = vtkSmartPointer<vtkMRMLScadaOpcUaNode>::Take(newNode);
//        d->ScadaOpcUaLogic->setParameterNode(d->ScadaOpcUaNode);
      }
    }
    else
    {
      d->ScadaOpcUaNode = vtkSmartPointer<vtkMRMLScadaOpcUaNode>::New();
      std::string nodeName = this->mrmlScene()->GenerateUniqueName("ScadaOpcUa");
      d->ScadaOpcUaNode->SetName(nodeName.c_str());
      this->mrmlScene()->AddNode(d->ScadaOpcUaNode);
      d->ScadaOpcUaLogic->setParameterNode(d->ScadaOpcUaNode);
      // Each time the node is modified, the UI widgets are updated
      qvtkReconnect(d->ScadaOpcUaNode, vtkCommand::ModifiedEvent, 
        this, SLOT(updateWidgetFromMRML()));
    }
  }
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onSceneImportedEvent()
{
  Q_D(qSlicerImagePositioningModuleWidget);

  this->onEnter();
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onSceneClosedEvent()
{
  Q_D(qSlicerImagePositioningModuleWidget);

  qDebug() << Q_FUNC_INFO << "Scene is closed";
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onScadaOpcUaLogicModified()
{
  Q_D(qSlicerImagePositioningModuleWidget);

  qDebug() << Q_FUNC_INFO << "Logic modified";
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::enter()
{
  Q_D(qSlicerImagePositioningModuleWidget);

  this->onEnter();
  this->Superclass::enter();
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onEnter()
{
  Q_D(qSlicerImagePositioningModuleWidget);

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

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerImagePositioningModuleWidget);

  if (!this->mrmlScene())
  {
    return;
  }
  if (!d->ScadaOpcUaNode)
  {
    qCritical() << Q_FUNC_INFO << "Scada node is invalid";
    return;
  }
  
  d->Label_ScadaSystemTime->setText(tr("SCADA System Time: %1").arg(d->ScadaOpcUaNode->GetSysTime()));
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onConnectClicked()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  
  if (!d->ScadaOpcUaLogic)
  {
    return;
  }
  
  if (d->ScadaOpcUaLogic->getClientConnectedFlag())
  {
    d->ScadaOpcUaLogic->disconnectFromServer();
    return;
  }
  d->ScadaOpcUaLogic->connectToServer(d->OpcUaPluginComboBox->currentText(), d->OpcUaServerUrlLineEdit->text());
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::clientConnected()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  d->PushButton_ScadaConnect->setText(tr("Disconnect"));

//  d->ScadaOpcUaModel->setOpcUaClient(d->ScadaOpcUaLogic->getClientConnected());
//  d->OpcUaTreeView->header()->setSectionResizeMode(1 /* Value column*/, QHeaderView::Interactive);
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::clientDisconnected()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  d->PushButton_ScadaConnect->setText(tr("Connect"));
  d->ScadaOpcUaModel->setOpcUaClient(nullptr);
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::clientError(QOpcUaClient::ClientError)
{
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::clientState(QOpcUaClient::ClientState)
{
}

void qSlicerImagePositioningModuleWidget::onScadaLogicConnected(bool connected)
{
  Q_D(qSlicerImagePositioningModuleWidget);
  if (!d->ScadaOpcUaLogic)
  {
    return;
  }
  if (connected)
  {
    d->PushButton_ScadaConnect->setText(tr("Disconnect"));
  }
  else
  {
    d->PushButton_ScadaConnect->setText(tr("Connect"));
    return;
  }
}

void qSlicerImagePositioningModuleWidget::onSetLocalTimeClicked()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  if (!d->ScadaOpcUaLogic)
  {
    return;
  }
  d->ScadaOpcUaLogic->setPatientPositioningLocalTime(QDateTime::currentDateTimeUtc());
}

void qSlicerImagePositioningModuleWidget::onSetErrorMessageClicked()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  if (!d->ScadaOpcUaLogic)
  {
    return;
  }
  d->ScadaOpcUaLogic->setPatientPositioningErrorMessage(d->LineEdit_ErrorMessage->text());
}

void qSlicerImagePositioningModuleWidget::onSetEventMessageClicked()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  if (!d->ScadaOpcUaLogic)
  {
    return;
  }
  d->ScadaOpcUaLogic->setPatientPositioningEventMessage(d->LineEdit_EventMessage->text());
}
