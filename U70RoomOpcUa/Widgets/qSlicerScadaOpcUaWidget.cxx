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

#include "ScadaOpcUaModel.h"
#include "ScadaOpcUaTreeItem.h"

#include <vtkMRMLScadaOpcUaNode.h>

namespace {

constexpr const char* const PARENT_NODE_DISPLAYED_NAME = "Модуль позиционирования";

const QString DOT = QString(QChar('.'));

static const char* const SCADA_SYS_TIME = "SysTimeScada";
static const char* const SCADA_SYS_TIME_MOD_POS = "SysTimeModPos";
static const char* const SCADA_PLAN_STRING = "PlanStr";
static const char* const SCADA_IDP = "IDP";
static const char* const SCADA_ECP = "ECP";
static const char* const SCADA_PLAN_OPEN = "PlanOpen";

const QString SCADA_SYS_TIME_NODE_NAME = PARENT_NODE_DISPLAYED_NAME + DOT + QLatin1String(SCADA_SYS_TIME);
const QString SCADA_SYS_TIME_MOD_POS_NODE_NAME = PARENT_NODE_DISPLAYED_NAME + DOT + QLatin1String(SCADA_SYS_TIME_MOD_POS);
const QString SCADA_PLAN_STRING_NODE_NAME = PARENT_NODE_DISPLAYED_NAME + DOT + QLatin1String(SCADA_PLAN_STRING);
const QString SCADA_IDP_NODE_NAME = PARENT_NODE_DISPLAYED_NAME + DOT + QLatin1String(SCADA_IDP);
const QString SCADA_ECP_NODE_NAME = PARENT_NODE_DISPLAYED_NAME + DOT + QLatin1String(SCADA_ECP);
const QString SCADA_PLAN_OPEN_NODE_NAME = PARENT_NODE_DISPLAYED_NAME + DOT + QLatin1String(SCADA_PLAN_OPEN);

const QOpcUaMonitoringParameters OpcUaMonitorParameters(1000);

}

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

  bool ConnectNodes();


  QScopedPointer< ScadaOpcUaModel > OpcUaModel;

  QWeakPointer< QOpcUaClient > OpcUaClient; // client
  vtkWeakPointer< vtkMRMLScadaOpcUaNode > ParameterNode; // MRML OPC UA parameters node

  // Parse parent node
  QScopedPointer< QOpcUaNode > ServerInterfacesNode;
  // Monitored node
  QScopedPointer< QOpcUaNode > CurrentTimeNode; // Siemens PLC monitored node to prevent session timeout ending
  QMap< QString, NodeData > NodeNameDataMap; // key - node unique full name, value - node data
  vtkSlicerU70RoomOpcUaLogic* opcUaLogic{ nullptr };

  QScopedPointer< QProgressDialog > ProgressDialog;
  QScopedPointer< QTimer > ProgressTimer;

  bool AutoConnectServerFlag{ false };

  const QStringList ReadValueFullNodeNames = {
    SCADA_SYS_TIME_NODE_NAME,
    SCADA_SYS_TIME_MOD_POS_NODE_NAME,
    SCADA_PLAN_STRING_NODE_NAME,
    SCADA_IDP_NODE_NAME,
    SCADA_ECP_NODE_NAME,
    SCADA_PLAN_OPEN_NODE_NAME
  };
};

// --------------------------------------------------------------------------
qSlicerScadaOpcUaWidgetPrivate::qSlicerScadaOpcUaWidgetPrivate(qSlicerScadaOpcUaWidget& object)
  : q_ptr(&object)
  , OpcUaModel(new ScadaOpcUaModel(&object))
  , ProgressDialog(new QProgressDialog(&object))
  , ProgressTimer(new QTimer(&object))
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
  
  this->TreeView_OpcUaModel->setModel(OpcUaModel.data());
  this->TreeView_OpcUaModel->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

//-----------------------------------------------------------------------------
bool qSlicerScadaOpcUaWidgetPrivate::ParseServerParentNode(const QString& serverNodeDispName)
{
  if (!this->ServerInterfacesNode)
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
bool qSlicerScadaOpcUaWidgetPrivate::ConnectNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLScadaOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Scada System Time node (SCADA system time value - read only)
  QOpcUaNode* scadaSysTimeNode = this->FindNodeFromFullDisplayName(SCADA_SYS_TIME_NODE_NAME);
  // Scada System Time Positioning Module node (Positioning module local system time - write only)
  QOpcUaNode* scadaSysTimeModPosNode = this->FindNodeFromFullDisplayName(SCADA_SYS_TIME_MOD_POS_NODE_NAME);

  // Scada System Time node
  if (!scadaSysTimeNode)
  {
    return false;
  }
  QObject::connect(scadaSysTimeNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< qulonglong >())
      {
        bool ok = false;
        qulonglong sysTimeValue = value.toULongLong(&ok);
        if (ok)
        {
          mrmlNode->SetSysTimeScada(static_cast< uint64_t >(sysTimeValue));
          qDebug() << Q_FUNC_INFO << "SCADA System time changed:" << sysTimeValue;
        }
      }
    }
  );
  QObject::connect(scadaSysTimeNode,
    &QOpcUaNode::attributeRead, [scadaSysTimeNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!scadaSysTimeNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (scadaSysTimeNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = scadaSysTimeNode->attribute(QOpcUa::NodeAttribute::Value);
          if (value.canConvert< qulonglong >())
          {
            bool ok = false;
            qulonglong sysTimeValue = value.toULongLong(&ok);
            if (ok)
            {
              mrmlNode->SetSysTimeScada(static_cast< uint64_t >(sysTimeValue));
              qDebug() << Q_FUNC_INFO << "SCADA System time read:" << sysTimeValue;
            }
          }
        }
      }
    }
  );
  // Subscribe to data changes
  scadaSysTimeNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  // Scada System Time Positioning Module node
  if (!scadaSysTimeModPosNode)
  {
    return false;
  }
  QObject::connect(scadaSysTimeModPosNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< qulonglong >())
      {
        bool ok = false;
        qulonglong sysTimeModPosValue = value.toULongLong(&ok);
        if (ok)
        {
          mrmlNode->SetSysTimeModPos(static_cast< uint64_t >(sysTimeModPosValue));
          qDebug() << Q_FUNC_INFO << "Positioning Module System time changed:" << sysTimeModPosValue;
        }
      }
    }
  );
  QObject::connect(scadaSysTimeModPosNode,
    &QOpcUaNode::attributeRead, [scadaSysTimeModPosNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!scadaSysTimeModPosNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (scadaSysTimeModPosNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = scadaSysTimeModPosNode->attribute(QOpcUa::NodeAttribute::Value);
          if (value.canConvert< qulonglong >())
          {
            bool ok = false;
            qulonglong sysTimeModPosValue = value.toULongLong(&ok);
            if (ok)
            {
              mrmlNode->SetSysTimeModPos(static_cast< uint64_t >(sysTimeModPosValue));
              qDebug() << Q_FUNC_INFO << "Positioning Module System time read:" << sysTimeModPosValue;
            }
          }
        }
      }
    }
  );
  // Subscribe to data changes
  scadaSysTimeModPosNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);
  return true;
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
void qSlicerScadaOpcUaWidget::onParseServerInterfacesClicked()
{
  Q_D(qSlicerScadaOpcUaWidget);
  
  QSharedPointer< QOpcUaClient > sharedClient = d->OpcUaClient.toStrongRef();
  if (!sharedClient)
  {
    return;
  }

  d->ServerInterfacesNode.reset(sharedClient->node(ScadaOpcUaTreeItem::SCADA_POSITION_MODULE_NODE_ID));
  if (!d->ServerInterfacesNode)
  {
    return;
  }

  QObject::connect(d->ServerInterfacesNode.data(), &QOpcUaNode::attributeRead,
    this, &qSlicerScadaOpcUaWidget::onPositionModuleNodesRead);

  d->ServerInterfacesNode->readAttributes(QOpcUa::NodeAttribute::DisplayName);

}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaWidget::onOpcUaClientDisconnected()
{
  Q_D(qSlicerScadaOpcUaWidget);

  d->PushButton_ReadNodesValues->setEnabled(false);
  if (d->CurrentTimeNode)
  {
    d->CurrentTimeNode->disableMonitoring(QOpcUa::NodeAttribute::Value);
  }
  d->OpcUaModel->clear();
}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaWidget::onPositionModuleNodesRead(QOpcUa::NodeAttributes attr)
{
  Q_D(qSlicerScadaOpcUaWidget);

  QHash< QString, QPointer< QOpcUaNode > > hashNodes;
  if (attr & QOpcUa::NodeAttribute::DisplayName)
  { // Make sure the value attribute has been read
    QString nodeDisplayName = d->ServerInterfacesNode->attribute(QOpcUa::NodeAttribute::DisplayName).value<QOpcUaLocalizedText>().text();
    bool res = d->ParseServerParentNode(nodeDisplayName);
    if (res)
    {
      for (auto& nodeData : d->NodeNameDataMap)
      {
        if (nodeData.getNode())
        {
          qDebug() << Q_FUNC_INFO << "Node ID:" << nodeData.getNodeId()
              << ", Name:" << nodeData.getNodeName()
              << ", Description:" << nodeData.getNodeDescription() << Qt::endl;
          hashNodes.insert(nodeData.getNodeName(), nodeData.getNode());
        }
      }
    }
    bool coords = d->ConnectNodes();
    if (coords)
    {
      qDebug() << Q_FUNC_INFO << "Nodes are OK";
    }
  }

  d->PushButton_ReadNodesValues->setEnabled(true);
  d->opcUaLogic->ServerConnected(d->OpcUaClient, hashNodes);

  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaWidget::onReadNodesValuesClicked()
{
  Q_D(qSlicerScadaOpcUaWidget);
/*
  for (const QString& nodeName : d->ReadValueFullNodeNames)
  {
    QOpcUaNode* readValueNode = d->FindNodeFromFullDisplayName(nodeName);
    if (readValueNode)
    {
      readValueNode->readAttributes(QOpcUa::NodeAttribute::Value);
    }
  }
  d->ParameterNode->Modified();
*/
}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaWidget::setOpcUaLogic(vtkSlicerU70RoomOpcUaLogic* logic)
{
  Q_D(qSlicerScadaOpcUaWidget);
  d->opcUaLogic = logic;
  if (d->opcUaLogic)
  {
    qDebug() << Q_FUNC_INFO << "Logic is OK";
  }
}
