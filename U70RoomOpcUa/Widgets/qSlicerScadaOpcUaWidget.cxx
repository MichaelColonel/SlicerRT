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
#include <QOpcUaClient>
#include <QWeakPointer>
#include <QSharedPointer>
#include <QScopedPointer>
#include <QOpcUaNode>
#include <QProgressDialog>
#include <QMessageBox>
#include <QTimer>

// Slicer includes
#include <qSlicerSingletonViewFactory.h>
#include <qSlicerLayoutManager.h>
#include <qSlicerApplication.h>

// ScadaOpcUa Widgets includes
#include "qSlicerScadaOpcUaWidget.h"
#include "ui_qSlicerScadaOpcUaWidget.h"

// U70RoomOpcUa Logic includes
#include "vtkSlicerU70RoomOpcUaLogic.h"
#include "vtkSlicerU70RoomScadaOpcUaLogic.h"
#include "vtkSlicerU70RoomScadaOpcUaNodesNames.h"

#include "ScadaOpcUaModel.h"
#include "ScadaOpcUaTreeItem.h"
#include "qSlicerOpcUaInitialValueNodeReader.h"
#include "qSlicerOpcUaInitialValueNodeLoader.h"

#include <vtkMRMLScadaOpcUaNode.h>

//-----------------------------------------------------------------------------
class NodeData {
public:
  NodeData() = default;
  NodeData(QOpcUaNode* node, const QString& id, const QString& name, const QString& desc)
    : nodePtr(node)
    , nodeId(id)
    , nodeName(name)
    , nodeDescription(desc)
  {
  }
  NodeData(const NodeData& obj)
    : nodePtr(obj.getNode())
    , nodeId(obj.getNodeId())
    , nodeName(obj.getNodeName())
    , nodeDescription(obj.getNodeDescription())
  {
  }
  NodeData& operator=(const NodeData& obj)
  {
    this->nodePtr = obj.getNodePtr();
    this->nodeId = obj.getNodeId();
    this->nodeName = obj.getNodeName();
    this->nodeDescription = obj.getNodeDescription();
    return *this;
  }
  virtual ~NodeData() {}
  QSharedPointer< QOpcUaNode > getNodePtr() const { return nodePtr; }
  void setNode(QOpcUaNode* node) { this->nodePtr.reset(node); }
  QOpcUaNode* getNode() { return nodePtr.data(); }
  QOpcUaNode* getNode() const { return nodePtr.data(); }
  QString getNodeId() const { return nodeId; }
  QString getNodeName() const { return nodeName; }
  QString getNodeDescription() const { return nodeDescription; }
protected:
  QSharedPointer< QOpcUaNode > nodePtr;
  QString nodeId;
  QString nodeName;
  QString nodeDescription;
};

//-----------------------------------------------------------------------------
class qSlicerScadaOpcUaWidgetPrivate : public Ui_qSlicerScadaOpcUaWidget
{
  Q_DECLARE_PUBLIC(qSlicerScadaOpcUaWidget)
protected:
  qSlicerScadaOpcUaWidget* const q_ptr;

public:
  qSlicerScadaOpcUaWidgetPrivate(qSlicerScadaOpcUaWidget& object);
  virtual void setupUi(qSlicerScadaOpcUaWidget*);

  bool ParseServerParentNode(const QString& nodeDisplayName);
  QOpcUaNode* FindNodeFromFullDisplayName(const QString& nodeDisplayName);
  void ExpandAllAsync(QTreeView *view, QAbstractItemModel* model,
    const QModelIndex &parentIndex = QModelIndex());

  QScopedPointer< ScadaOpcUaModel > OpcUaModel;

  QWeakPointer< QOpcUaClient > OpcUaClient; // client
  vtkWeakPointer< vtkMRMLScadaOpcUaNode > ParameterNode; // MRML OPC UA parameters node

  // Parse parent node
  QScopedPointer< QOpcUaNode > ServerParentNode;
  // Monitored node
  QScopedPointer< QOpcUaNode > CurrentTimeNode; // Siemens PLC monitored node to prevent session timeout ending
  QMap< QString, NodeData > NodeNameDataMap; // key - node unique full name, value - node data
  vtkSlicerU70RoomOpcUaLogic* opcUaLogic{ nullptr };

  QScopedPointer< QProgressDialog > ProgressDialog;
  QScopedPointer< QTimer > ProgressTimer;
  QScopedPointer< QTimer > ScadaExpiredTimer;
  QScopedPointer< qSlicerOpcUaInitialValueNodeReader > InitialValuesNodeReader;
  QScopedPointer< qSlicerOpcUaInitialValueNodeLoader > InitialValuesNodeLoader;

  bool AutoConnectServerFlag{ false };
};

// --------------------------------------------------------------------------
qSlicerScadaOpcUaWidgetPrivate::qSlicerScadaOpcUaWidgetPrivate(qSlicerScadaOpcUaWidget& object)
  : q_ptr(&object)
  , OpcUaModel(new ScadaOpcUaModel(&object))
  , ProgressDialog(new QProgressDialog(&object))
  , ProgressTimer(new QTimer(&object))
  , ScadaExpiredTimer(new QTimer(&object))
{
}

// --------------------------------------------------------------------------
void qSlicerScadaOpcUaWidgetPrivate::setupUi(qSlicerScadaOpcUaWidget* widget)
{
  Q_Q(qSlicerScadaOpcUaWidget);

  this->Ui_qSlicerScadaOpcUaWidget::setupUi(widget);

  this->ProgressDialog->setWindowTitle(QObject::tr("Scada OPC UA"));
  this->ProgressDialog->setWindowModality(Qt::NonModal);
  this->ProgressDialog->setMinimumDuration(500);
  this->ProgressDialog->setAutoReset(false);
  this->ProgressDialog->setAutoClose(false);
  this->ProgressDialog->close();

  this->ProgressTimer->setSingleShot(true);
  this->ProgressTimer->setInterval(300);

  this->ScadaExpiredTimer->setInterval(2900);

  this->TreeView_OpcUaModel->setModel(OpcUaModel.data());
  this->TreeView_OpcUaModel->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

//-----------------------------------------------------------------------------
bool qSlicerScadaOpcUaWidgetPrivate::ParseServerParentNode(const QString& serverNodeDispName)
{
  if (!this->ServerParentNode)
  {
    return false;
  }
  if (serverNodeDispName.isEmpty())
  {
    return false;
  }

  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();
  if (!opcUaClient)
  {
    return false;
  }
  ScadaOpcUaTreeItem* parentItem = nullptr;
  ScadaOpcUaTreeItem* rootItem = OpcUaModel->getRootItem();
  if (rootItem)
  {
    parentItem = ScadaOpcUaTreeItem::findParentItemByName(rootItem, serverNodeDispName);
  }
  else
  {
    return false;
  }
  this->NodeNameDataMap.clear();
  if (parentItem)
  {
    QList< QPointer< ScadaOpcUaTreeItem > > items = ScadaOpcUaTreeItem::findLeafValueItems(parentItem);
    for (QPointer< ScadaOpcUaTreeItem > item : items)
    {
      QStringList parentNames = ScadaOpcUaTreeItem::findParentNamesForItem(item.data(), parentItem);

      if (parentNames.size())
      {
        QString hierNames;

        for (auto iter = parentNames.rbegin(); iter != parentNames.rend(); ++iter)
        {
          if (iter != parentNames.rend() - 1)
          {
            hierNames += *iter + QString(".");
          }
          else
          {
            hierNames += *iter;
          }
        }
        QOpcUaNode* itemNode = opcUaClient->node(item->getNodeId());

        NodeData data(itemNode, item->getNodeId(), hierNames, item->getNodeDescription());
        this->NodeNameDataMap[hierNames] = data;
      }
    }
  }
  return true;
}

//-----------------------------------------------------------------------------
QOpcUaNode* qSlicerScadaOpcUaWidgetPrivate::FindNodeFromFullDisplayName(const QString& nodeDisplayName)
{
  if (!this->NodeNameDataMap.size())
  {
    return nullptr;
  }
  for (auto& nodeData : this->NodeNameDataMap)
  {
    const QString& fullName = nodeData.getNodeName();
    if (fullName == nodeDisplayName)
    {
      return nodeData.getNode();
    }
  }
  return nullptr;
}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaWidgetPrivate::ExpandAllAsync(QTreeView *view, QAbstractItemModel* model, const QModelIndex &parentIndex)
{
  if (parentIndex.isValid())
  {
    view->expand(parentIndex);
  }
  int rows = model->rowCount(parentIndex);
  for (int row = 0; row < rows; ++row)
  {
    this->ExpandAllAsync(view, model, model->index(row, 0, parentIndex));
  }
}

//-----------------------------------------------------------------------------
// qSlicerSiemensPlcOpcUaWidget methods

//-----------------------------------------------------------------------------
qSlicerScadaOpcUaWidget::qSlicerScadaOpcUaWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerScadaOpcUaWidgetPrivate(*this) )
{
  Q_D(qSlicerScadaOpcUaWidget);
  d->setupUi(this);

  ScadaOpcUaModel* opcUaModel = d->OpcUaModel.data();
  QTreeView* opcUaTreeView = d->TreeView_OpcUaModel;

  QObject::connect(d->ProgressTimer.data(), SIGNAL(timeout()), d->ProgressDialog.data(), SLOT(reset()));

  QObject::connect(opcUaModel, &QAbstractItemModel::rowsInserted, this,
    [this, opcUaModel, opcUaTreeView](const QModelIndex& parent, int first, int last)
    {
      opcUaTreeView->expand(parent);
      for (int row = first; row <= last; ++row)
      {
        QModelIndex childIndex = opcUaModel->index(row, 0, parent);
        opcUaTreeView->expand(childIndex);
        opcUaModel->rowCount(childIndex);
      }
    }
  );

  QProgressDialog* progressDialog = d->ProgressDialog.data();
  QTimer* progressTimer = d->ProgressTimer.data();
  QObject::connect(opcUaModel, &ScadaOpcUaModel::browseProgressChanged, this,
    [this, progressDialog, progressTimer](int finished, int started)
    {
      progressDialog->setLabelText(tr("Loading tree of nodes. Please, wait!"));
      progressDialog->setMaximum(started);
      progressDialog->setValue(finished);
      if (finished >= started)
      {
        progressTimer->start();
        progressDialog->close();
        emit this->nodesTreeLoadingFinished();
      }
      else
      {
        progressTimer->stop();
      }
    }
  );

  // Read nodes manually
  QObject::connect(d->PushButton_ReadNodesValues, SIGNAL(clicked()),
    this, SLOT(onReadNodesValuesClicked()));
  // SCADA Timeout expired
  QObject::connect(d->ScadaExpiredTimer.data(), SIGNAL(timeout()),
    this, SLOT(onScadaExpiredTimeout()));
}

//-----------------------------------------------------------------------------
qSlicerScadaOpcUaWidget::~qSlicerScadaOpcUaWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaWidget::setParameterNode(vtkMRMLNode* node)
{
  Q_D(qSlicerScadaOpcUaWidget);

  vtkMRMLScadaOpcUaNode* parameterNode = vtkMRMLScadaOpcUaNode::SafeDownCast(node);
  // Each time the node is modified, the UI widgets are updated
  qvtkReconnect(d->ParameterNode, parameterNode, vtkCommand::ModifiedEvent, 
    this, SLOT(updateWidgetFromMRML()));

  d->ParameterNode = parameterNode;

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaWidget::setOpcUaClient(const QSharedPointer< QOpcUaClient >& sharedClient)
{
  Q_D(qSlicerScadaOpcUaWidget);

  d->OpcUaClient = sharedClient;
}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerScadaOpcUaWidget);
  if (!d->ParameterNode)
  {
    return;
  }
  if (d->ParameterNode->GetSysTimeScada() > 0)
  {
    QDateTime dt = QDateTime::fromSecsSinceEpoch(d->ParameterNode->GetSysTimeScada());
    qDebug() << Q_FUNC_INFO << ": SCADA system time: " << dt.toString("dd.MM.yyyy hh:mm:ss");
  }
  if (d->ParameterNode->GetSysTimeModPos() > 0)
  {
    QDateTime dt = QDateTime::fromSecsSinceEpoch(d->ParameterNode->GetSysTimeModPos());
    qDebug() << Q_FUNC_INFO << ": ModPos system time: " << dt.toString("dd.MM.yyyy hh:mm:ss");
  }
  if (d->ParameterNode->GetTimeoutExpired())
  {
    qCritical() << Q_FUNC_INFO << ": SCADA timeout expired!";
  }
}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaWidget::onOpcUaClientConnected()
{
  Q_D(qSlicerScadaOpcUaWidget);
  
  QSharedPointer< QOpcUaClient > sharedClient = d->OpcUaClient.toStrongRef();
  if (!sharedClient)
  {
    return;
  }

  d->OpcUaModel->setOpcUaClient(sharedClient.data(), ScadaOpcUaTreeItem::SCADA_POSITION_MODULE_NODE_ID);
//  d->SiemensPlcOpcUaModel->setOpcUaClient(sharedClient.data(), QOpcUa::namespace0Id(QOpcUa::NodeIds::Namespace0::RootFolder));
//  d->TreeView_OpcUaModel->header()->setSectionResizeMode(0 /* BrowseName column*/, QHeaderView::Interactive);
  d->TreeView_OpcUaModel->header()->setSectionResizeMode(1 /* Value column*/, QHeaderView::Interactive);
//  d->TreeView_OpcUaModel->header()->setSectionResizeMode(2 /* NodeClass column*/, QHeaderView::Interactive);
//  d->TreeView_OpcUaModel->header()->setSectionResizeMode(3 /* DataType column*/, QHeaderView::Interactive);
//  d->TreeView_OpcUaModel->header()->setSectionResizeMode(4 /* NodeID column*/, QHeaderView::Interactive);
//  d->TreeView_OpcUaModel->header()->setSectionResizeMode(5 /* DisplayName column*/, QHeaderView::Interactive);
//  d->TreeView_OpcUaModel->header()->setSectionResizeMode(6 /* Description column*/, QHeaderView::Interactive);

  d->CurrentTimeNode.reset(sharedClient->node(QOpcUa::namespace0Id(QOpcUa::NodeIds::Namespace0::Server_ServerStatus_CurrentTime)));
  if (d->CurrentTimeNode)
  {
    // Subscribe to data changes
    d->CurrentTimeNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(1000));
  }
}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaWidget::onParseServerParentNodeClicked()
{
  Q_D(qSlicerScadaOpcUaWidget);
  
  QSharedPointer< QOpcUaClient > sharedClient = d->OpcUaClient.toStrongRef();
  if (!sharedClient)
  {
    return;
  }

  std::string parentNodeIdName = vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaOpcUaTreatmentRoomParentNodeId(3);

  d->ServerParentNode.reset(sharedClient->node(QString::fromStdString(parentNodeIdName)));
  if (!d->ServerParentNode)
  {
    return;
  }

  QObject::connect(d->ServerParentNode.data(), &QOpcUaNode::attributeRead,
    this, &qSlicerScadaOpcUaWidget::onPositionModuleNodesRead);

  d->ServerParentNode->readAttributes(QOpcUa::NodeAttribute::DisplayName);
}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaWidget::onOpcUaClientDisconnected()
{
  Q_D(qSlicerScadaOpcUaWidget);

  if (!d->ParameterNode)
  {
    qWarning() << Q_FUNC_INFO << ": Scada OPC UA node is invalid";
    return;
  }
  d->ScadaExpiredTimer->stop();
  d->PushButton_ReadNodesValues->setEnabled(false);
  if (d->CurrentTimeNode)
  {
    d->CurrentTimeNode->disableMonitoring(QOpcUa::NodeAttribute::Value);
  }
  d->OpcUaModel->clear();

  d->ParameterNode->SetTimeoutExpired(false);
}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaWidget::onPositionModuleNodesRead(QOpcUa::NodeAttributes attr)
{
  Q_D(qSlicerScadaOpcUaWidget);

  if (!d->ParameterNode)
  {
    return;
  }
  QHash< QString, QString > hashNodes;
  std::list< std::string > fullNamesList = vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaFullNodeNamesList();

  if (attr & QOpcUa::NodeAttribute::DisplayName)
  { // Make sure the value attribute has been read
    QString nodeDisplayName = d->ServerParentNode->attribute(QOpcUa::NodeAttribute::DisplayName).value<QOpcUaLocalizedText>().text();
    bool res = d->ParseServerParentNode(nodeDisplayName);
    if (res)
    {
      for (auto& nodeData : d->NodeNameDataMap)
      {
        if (nodeData.getNode())
        {
          hashNodes.insert(nodeData.getNodeName(), nodeData.getNodeId());
          auto iter = std::find(fullNamesList.begin(), fullNamesList.end(), nodeData.getNodeName().toStdString());
          if (iter != fullNamesList.end())
          {
            qDebug() << Q_FUNC_INFO << "Node ID:" << nodeData.getNodeId()
              << ", Name:" << nodeData.getNodeName()
              << ", Description:" << nodeData.getNodeDescription() << Qt::endl;
          }
        }
      }
    }
  }

  d->PushButton_ReadNodesValues->setEnabled(true);
  auto list = d->opcUaLogic->GetScadaOpcUaLogic()->ServerConnected(d->OpcUaClient, hashNodes);
  QList< QOpcUaNode* > nodesList(list.begin(), list.end());

  {
    qint64 currTimeSecs = QDateTime::currentSecsSinceEpoch();
    MRMLNodeModifyBlocker blocker(d->ParameterNode);
    d->opcUaLogic->GetScadaOpcUaLogic()->WriteSysTimeModPos(static_cast< int32_t >(currTimeSecs));
  }

  d->ProgressDialog->setLabelText(tr("Reading nodes initial value, Please wait!"));
  d->ProgressDialog->setMaximum(nodesList.size());
  d->ProgressDialog->setValue(0);

  d->InitialValuesNodeLoader.reset(new qSlicerOpcUaInitialValueNodeLoader(nodesList, this));
  QObject::connect(d->InitialValuesNodeLoader.data(), &qSlicerOpcUaInitialValueNodeLoader::progress,
    this, [d](int done, int total)
  {
    d->ProgressDialog->setValue(done);
    d->ProgressTimer->stop();
  });
  QObject::connect(d->InitialValuesNodeLoader.data(), &qSlicerOpcUaInitialValueNodeLoader::finished,
    this, [this, d]()
  {
    if (d->ProgressTimer && d->ProgressDialog)
    {
      d->ProgressTimer->start();
      d->ProgressDialog->close();
    }
    d->InitialValuesNodeLoader.reset(nullptr);
    QMessageBox::information(this, tr("Siemens PLC OPC UA"), tr("Client is ready for data exchange with Siemens PLC!"));
  });

//  this->onReadNodesValuesClicked();
  
  d->ScadaExpiredTimer->start();
}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaWidget::onReadNodesValuesClicked()
{
  Q_D(qSlicerScadaOpcUaWidget);

/*
  std::list< std::string > nodeFullNameList = vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaFullNodeNamesList();
  if (d->opcUaLogic && d->opcUaLogic->GetScadaOpcUaLogic())
  {
    bool res = d->opcUaLogic->GetScadaOpcUaLogic()->ReadNodesValues(nodeFullNameList);
    if (res)
    {
      //
    }
  }
*/
  std::list< std::string > fullNamesList = vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaFullNodeNamesList();
  QList< QOpcUaNode* > nodesList; 

  for (auto& nodeData : d->NodeNameDataMap)
  {
    if (nodeData.getNode())
    {
      auto iter = std::find(fullNamesList.begin(), fullNamesList.end(), nodeData.getNodeName().toStdString());
      if (iter != fullNamesList.end())
      {
        nodesList.push_back(nodeData.getNode());
      }
    }
  }

  d->ProgressDialog->setLabelText(tr("Reading nodes values, Please wait!"));
  d->ProgressDialog->setMaximum(nodesList.size());
  d->ProgressDialog->setValue(0);

  d->InitialValuesNodeReader.reset(new qSlicerOpcUaInitialValueNodeReader(nodesList, this));
  QObject::connect(d->InitialValuesNodeReader.data(), &qSlicerOpcUaInitialValueNodeReader::progress,
    this, [d](int done, int)
  {
    d->ProgressDialog->setValue(done);
    d->ProgressTimer->stop();
  });
  QObject::connect(d->InitialValuesNodeReader.data(), &qSlicerOpcUaInitialValueNodeReader::finished,
    this, [this, d]()
  {
    d->ProgressTimer->start();
    d->ProgressDialog->close();
    QMessageBox::information(this, tr("Siemens PLC OPC UA"), tr("Client is ready for data exchange with Siemens PLC!"));
  });

  d->InitialValuesNodeReader->start();
}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaWidget::setOpcUaLogic(vtkSlicerU70RoomOpcUaLogic* logic)
{
  Q_D(qSlicerScadaOpcUaWidget);
  d->opcUaLogic = logic;
}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaWidget::onScadaExpiredTimeout()
{
  Q_D(qSlicerScadaOpcUaWidget);
  if (!d->ParameterNode)
  {
    qWarning() << Q_FUNC_INFO << ": Scada OPC UA node is invalid";
    return;
  }

  if (d->opcUaLogic && d->opcUaLogic->GetScadaOpcUaLogic())
  {
    qint64 currTimeSecs = QDateTime::currentSecsSinceEpoch();
    if (d->opcUaLogic->GetScadaOpcUaLogic()->WriteSysTimeModPos(static_cast< int32_t >(currTimeSecs)))
    {
      qDebug() << Q_FUNC_INFO << ": ModPos systen time is written";
    }
  }
  bool timerExpiredSignal = (std::abs(d->ParameterNode->GetSysTimeModPos() - d->ParameterNode->GetSysTimeScada()) > 3);
  {
    MRMLNodeModifyBlocker blocker(d->ParameterNode);
    d->ParameterNode->SetTimeoutExpired(timerExpiredSignal);
  }
}
