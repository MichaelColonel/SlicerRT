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
#include <QSharedPointer>
#include <QOpcUaNode>

// SiemensPlcOpcUa Widgets includes
#include "qSlicerSiemensPlcOpcUaWidget.h"
#include "ui_qSlicerSiemensPlcOpcUaWidget.h"

#include "OpcUaModel.h"
#include "OpcUaTreeItem.h"

#include <vtkMRMLSiemensPlcOpcUaNode.h>

#include <bitset>

namespace {

const QString PARENT_NODE_DISPLAYED_NAME = "ServerInterfaces";

const QString RTK_PLC_NODE_NAME = PARENT_NODE_DISPLAYED_NAME + ".RTK_PLC";
const QString ASU_NODE_NAME = RTK_PLC_NODE_NAME + ".ASU";
const QString ASU_ERROR_MESSAGES_NODE_NAME = ASU_NODE_NAME + ".ErrorMes";
const QString ASU_SERVICE_MESSAGES_NODE_NAME = ASU_NODE_NAME + ".ServMes";
const QString ASU_MESSAGES_NODE_NAME = ASU_NODE_NAME + ".Messages";
const QString ASU_MODE_NODE_NAME = ASU_NODE_NAME + ".Mode";
const QString ASU_STATUS_RTK_NODE_NAME = ASU_NODE_NAME + ".Status_RTK";
const QString ASU_STATUS_R1_TABLE_NODE_NAME = ASU_NODE_NAME + ".Status_R1_deka";
const QString ASU_STATUS_R2_CARM_NODE_NAME = ASU_NODE_NAME + ".Status_R2_C_Duga";
const QString ASU_COORDS_NODE_NAME = ASU_NODE_NAME + ".CoordFromASU";
const QString ASU_COORDS_X_NODE_NAME = ASU_COORDS_NODE_NAME + ".X";
const QString ASU_COORDS_Y_NODE_NAME = ASU_COORDS_NODE_NAME + ".Y";
const QString ASU_COORDS_Z_NODE_NAME = ASU_COORDS_NODE_NAME + ".Z";
const QString ASU_COORDS_XRAY_Z_NODE_NAME = ASU_COORDS_NODE_NAME + ".XRAY_Z_correction";
const QString ASU_COORDS_R1_A_NODE_NAME = ASU_COORDS_NODE_NAME + ".AngleA_R1";
const QString ASU_COORDS_R1_B_NODE_NAME = ASU_COORDS_NODE_NAME + ".AngleB_R1";
const QString ASU_COORDS_R1_C_NODE_NAME = ASU_COORDS_NODE_NAME + ".AngleC_R1";

const QString IS_PRESSED = ".isPressed";
const QString IS_ENABLED = ".isEnable";

const QString BUTTONS_NODE_NAME = ASU_NODE_NAME + ".Buttons";
const QString BUTTONS_AM_NODE_NAME = BUTTONS_NODE_NAME + ".AM_MM";

const QString BUTTONS_AM_R1_LOADTOISO_NODE_NAME = BUTTONS_AM_NODE_NAME + ".R1_LoadToIso";
const QString BUTTONS_AM_R1_LOADTOISO_PRESSED_NODE_NAME = BUTTONS_AM_R1_LOADTOISO_NODE_NAME + IS_PRESSED;
const QString BUTTONS_AM_R1_LOADTOISO_ENABLED_NODE_NAME = BUTTONS_AM_R1_LOADTOISO_NODE_NAME + IS_ENABLED;

const QString BUTTONS_AM_R1_NEWCOORDS_NODE_NAME = BUTTONS_AM_NODE_NAME + ".R1_ToNewCoords";
const QString BUTTONS_AM_R1_NEWCOORDS_PRESSED_NODE_NAME = BUTTONS_AM_R1_NEWCOORDS_NODE_NAME + IS_PRESSED;
const QString BUTTONS_AM_R1_NEWCOORDS_ENABLED_NODE_NAME = BUTTONS_AM_R1_NEWCOORDS_NODE_NAME + IS_ENABLED;

const QString BUTTONS_AM_R1_TOLOAD_NODE_NAME = BUTTONS_AM_NODE_NAME + ".R1_ToLoad";
const QString BUTTONS_AM_R1_TOLOAD_PRESSED_NODE_NAME = BUTTONS_AM_R1_TOLOAD_NODE_NAME + IS_PRESSED;
const QString BUTTONS_AM_R1_TOLOAD_ENABLED_NODE_NAME = BUTTONS_AM_R1_TOLOAD_NODE_NAME + IS_ENABLED;

const QString BUTTONS_AM_R2_TOPLANE1_NODE_NAME = BUTTONS_AM_NODE_NAME + ".R2_ToPL1";
const QString BUTTONS_AM_R2_TOPLANE1_PRESSED_NODE_NAME = BUTTONS_AM_R2_TOPLANE1_NODE_NAME + IS_PRESSED;
const QString BUTTONS_AM_R2_TOPLANE1_ENABLED_NODE_NAME = BUTTONS_AM_R2_TOPLANE1_NODE_NAME + IS_ENABLED;

const QString BUTTONS_AM_R2_TOPLANE2_NODE_NAME = BUTTONS_AM_NODE_NAME + ".R2_ToPL2";
const QString BUTTONS_AM_R2_TOPLANE2_PRESSED_NODE_NAME = BUTTONS_AM_R2_TOPLANE2_NODE_NAME + IS_PRESSED;
const QString BUTTONS_AM_R2_TOPLANE2_ENABLED_NODE_NAME = BUTTONS_AM_R2_TOPLANE2_NODE_NAME + IS_ENABLED;

const QString BUTTONS_AM_R2_TOHOME_NODE_NAME = BUTTONS_AM_NODE_NAME + ".R2_ToHome";
const QString BUTTONS_AM_R2_TOHOME_PRESSED_NODE_NAME = BUTTONS_AM_R2_TOHOME_NODE_NAME + IS_PRESSED;
const QString BUTTONS_AM_R2_TOHOME_ENABLED_NODE_NAME = BUTTONS_AM_R2_TOHOME_NODE_NAME + IS_ENABLED;

const QString BUTTONS_AM_R1R2_EMEVAC_NODE_NAME = BUTTONS_AM_NODE_NAME + ".R1R2_EmEvac";
const QString BUTTONS_AM_R1R2_EMEVAC_PRESSED_NODE_NAME = BUTTONS_AM_R1R2_EMEVAC_NODE_NAME + IS_PRESSED;
const QString BUTTONS_AM_R1R2_EMEVAC_ENABLED_NODE_NAME = BUTTONS_AM_R1R2_EMEVAC_NODE_NAME + IS_ENABLED;

const QString BUTTONS_AM_APPLYTABLEPOSITION_NODE_NAME = BUTTONS_AM_NODE_NAME + ".ApplyDekaPosition";
const QString BUTTONS_AM_APPLYTABLEPOSITION_PRESSED_NODE_NAME = BUTTONS_AM_APPLYTABLEPOSITION_NODE_NAME + IS_PRESSED;
const QString BUTTONS_AM_APPLYTABLEPOSITION_ENABLED_NODE_NAME = BUTTONS_AM_APPLYTABLEPOSITION_NODE_NAME + IS_ENABLED;

const QString BUTTONS_SM_NODE_NAME = BUTTONS_NODE_NAME + ".SM";

const QString BUTTONS_SM_R1_BREAKTEST_NODE_NAME = BUTTONS_SM_NODE_NAME + ".BrakeTestR1";
const QString BUTTONS_SM_R1_BREAKTEST_PRESSED_NODE_NAME = BUTTONS_SM_R1_BREAKTEST_NODE_NAME + IS_PRESSED;
const QString BUTTONS_SM_R1_BREAKTEST_ENABLED_NODE_NAME = BUTTONS_SM_R1_BREAKTEST_NODE_NAME + IS_ENABLED;

const QString BUTTONS_SM_R1_MASREFTEST_NODE_NAME = BUTTONS_SM_NODE_NAME + ".MasterRefTestR1";
const QString BUTTONS_SM_R1_MASREFTEST_PRESSED_NODE_NAME = BUTTONS_SM_R1_MASREFTEST_NODE_NAME + IS_PRESSED;
const QString BUTTONS_SM_R1_MASREFTEST_ENABLED_NODE_NAME = BUTTONS_SM_R1_MASREFTEST_NODE_NAME + IS_ENABLED;

const QString BUTTONS_SM_R1_LOAD_NODE_NAME = BUTTONS_SM_NODE_NAME + ".LoadR1";
const QString BUTTONS_SM_R1_LOAD_PRESSED_NODE_NAME = BUTTONS_SM_R1_LOAD_NODE_NAME + IS_PRESSED;
const QString BUTTONS_SM_R1_LOAD_ENABLED_NODE_NAME = BUTTONS_SM_R1_LOAD_NODE_NAME + IS_ENABLED;

const QString BUTTONS_SM_R1_SERVICEPOS1_NODE_NAME = BUTTONS_SM_NODE_NAME + ".ServicePos1_R1";
const QString BUTTONS_SM_R1_SERVICEPOS1_PRESSED_NODE_NAME = BUTTONS_SM_R1_SERVICEPOS1_NODE_NAME + IS_PRESSED;
const QString BUTTONS_SM_R1_SERVICEPOS1_ENABLED_NODE_NAME = BUTTONS_SM_R1_SERVICEPOS1_NODE_NAME + IS_ENABLED;

const QString BUTTONS_SM_R1_SERVICEPOS2_NODE_NAME = BUTTONS_SM_NODE_NAME + ".ServicePos2_R1";
const QString BUTTONS_SM_R1_SERVICEPOS2_PRESSED_NODE_NAME = BUTTONS_SM_R1_SERVICEPOS2_NODE_NAME + IS_PRESSED;
const QString BUTTONS_SM_R1_SERVICEPOS2_ENABLED_NODE_NAME = BUTTONS_SM_R1_SERVICEPOS2_NODE_NAME + IS_ENABLED;

const QString BUTTONS_SM_R1_SERVICEPOS3_NODE_NAME = BUTTONS_SM_NODE_NAME + ".ServicePos3_R1";
const QString BUTTONS_SM_R1_SERVICEPOS3_PRESSED_NODE_NAME = BUTTONS_SM_R1_SERVICEPOS3_NODE_NAME + IS_PRESSED;
const QString BUTTONS_SM_R1_SERVICEPOS3_ENABLED_NODE_NAME = BUTTONS_SM_R1_SERVICEPOS3_NODE_NAME + IS_ENABLED;

const QString BUTTONS_SM_R2_BREAKTEST_NODE_NAME = BUTTONS_SM_NODE_NAME + ".BrakeTestR2";
const QString BUTTONS_SM_R2_BREAKTEST_PRESSED_NODE_NAME = BUTTONS_SM_R2_BREAKTEST_NODE_NAME + IS_PRESSED;
const QString BUTTONS_SM_R2_BREAKTEST_ENABLED_NODE_NAME = BUTTONS_SM_R2_BREAKTEST_NODE_NAME + IS_ENABLED;

const QString BUTTONS_SM_R2_MASREFTEST_NODE_NAME = BUTTONS_SM_NODE_NAME + ".MasterRefTestR2";
const QString BUTTONS_SM_R2_MASREFTEST_PRESSED_NODE_NAME = BUTTONS_SM_R2_MASREFTEST_NODE_NAME + IS_PRESSED;
const QString BUTTONS_SM_R2_MASREFTEST_ENABLED_NODE_NAME = BUTTONS_SM_R2_MASREFTEST_NODE_NAME + IS_ENABLED;

const QString BUTTONS_SM_R2_HOME_NODE_NAME = BUTTONS_SM_NODE_NAME + ".HomeR2";
const QString BUTTONS_SM_R2_HOME_PRESSED_NODE_NAME = BUTTONS_SM_R2_HOME_NODE_NAME + IS_PRESSED;
const QString BUTTONS_SM_R2_HOME_ENABLED_NODE_NAME = BUTTONS_SM_R2_HOME_NODE_NAME + IS_ENABLED;

const QString BUTTONS_SM_R2_SERVICEPOS1_NODE_NAME = BUTTONS_SM_NODE_NAME + ".ServicePos1_R2";
const QString BUTTONS_SM_R2_SERVICEPOS1_PRESSED_NODE_NAME = BUTTONS_SM_R2_SERVICEPOS1_NODE_NAME + IS_PRESSED;
const QString BUTTONS_SM_R2_SERVICEPOS1_ENABLED_NODE_NAME = BUTTONS_SM_R2_SERVICEPOS1_NODE_NAME + IS_ENABLED;

const QString BUTTONS_SM_R2_SERVICEPOS2_NODE_NAME = BUTTONS_SM_NODE_NAME + ".ServicePos2_R2";
const QString BUTTONS_SM_R2_SERVICEPOS2_PRESSED_NODE_NAME = BUTTONS_SM_R2_SERVICEPOS2_NODE_NAME + IS_PRESSED;
const QString BUTTONS_SM_R2_SERVICEPOS2_ENABLED_NODE_NAME = BUTTONS_SM_R2_SERVICEPOS2_NODE_NAME + IS_ENABLED;

const QString BUTTONS_SM_R2_SERVICEPOS3_NODE_NAME = BUTTONS_SM_NODE_NAME + ".ServicePos3_R2";
const QString BUTTONS_SM_R2_SERVICEPOS3_PRESSED_NODE_NAME = BUTTONS_SM_R2_SERVICEPOS3_NODE_NAME + IS_PRESSED;
const QString BUTTONS_SM_R2_SERVICEPOS3_ENABLED_NODE_NAME = BUTTONS_SM_R2_SERVICEPOS3_NODE_NAME + IS_ENABLED;

const QString BUTTONS_SM_RESTARTSM_NODE_NAME = BUTTONS_SM_NODE_NAME + ".RestartSM";
const QString BUTTONS_SM_RESTARTSM_PRESSED_NODE_NAME = BUTTONS_SM_RESTARTSM_NODE_NAME + IS_PRESSED;
const QString BUTTONS_SM_RESTARTSM_ENABLED_NODE_NAME = BUTTONS_SM_RESTARTSM_NODE_NAME + IS_ENABLED;

const QString BUTTONS_KCM_NODE_NAME = BUTTONS_NODE_NAME + ".KCM";

const QString BUTTONS_KCM_ALLOWT1_NODE_NAME = BUTTONS_KCM_NODE_NAME + ".AllowT1";
const QString BUTTONS_KCM_ALLOWT1_PRESSED_NODE_NAME = BUTTONS_KCM_ALLOWT1_NODE_NAME + IS_PRESSED;
const QString BUTTONS_KCM_ALLOWT1_ENABLED_NODE_NAME = BUTTONS_KCM_ALLOWT1_NODE_NAME + IS_ENABLED;

const QString BUTTONS_KCM_ALLOWXRAY_NODE_NAME = BUTTONS_KCM_NODE_NAME + ".AllowXRay";
const QString BUTTONS_KCM_ALLOWXRAY_PRESSED_NODE_NAME = BUTTONS_KCM_ALLOWXRAY_NODE_NAME + IS_PRESSED;
const QString BUTTONS_KCM_ALLOWXRAY_ENABLED_NODE_NAME = BUTTONS_KCM_ALLOWXRAY_NODE_NAME + IS_ENABLED;

const QString BUTTONS_KCM_ALLOWBEAM_NODE_NAME = BUTTONS_KCM_NODE_NAME + ".AllowBeam";
const QString BUTTONS_KCM_ALLOWBEAM_PRESSED_NODE_NAME = BUTTONS_KCM_ALLOWBEAM_NODE_NAME + IS_PRESSED;
const QString BUTTONS_KCM_ALLOWBEAM_ENABLED_NODE_NAME = BUTTONS_KCM_ALLOWBEAM_NODE_NAME + IS_ENABLED;

const QString BUTTONS_RESETERRORS_NODE_NAME = BUTTONS_NODE_NAME + ".ResetErrors";
const QString BUTTONS_RESETERRORS_PRESSED_NODE_NAME = BUTTONS_RESETERRORS_NODE_NAME + IS_PRESSED;
const QString BUTTONS_RESETERRORS_ENABLED_NODE_NAME = BUTTONS_RESETERRORS_NODE_NAME + IS_ENABLED;

const QString BUTTONS_MAKEXRAY_NODE_NAME = BUTTONS_NODE_NAME + ".MakeXRay";
const QString BUTTONS_MAKEXRAY_PRESSED_NODE_NAME = BUTTONS_MAKEXRAY_NODE_NAME + IS_PRESSED;
const QString BUTTONS_MAKEXRAY_ENABLED_NODE_NAME = BUTTONS_MAKEXRAY_NODE_NAME + IS_ENABLED;

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
class qSlicerSiemensPlcOpcUaWidgetPrivate : public Ui_qSlicerSiemensPlcOpcUaWidget
{
  Q_DECLARE_PUBLIC(qSlicerSiemensPlcOpcUaWidget)
protected:
  qSlicerSiemensPlcOpcUaWidget* const q_ptr;

public:
  qSlicerSiemensPlcOpcUaWidgetPrivate(qSlicerSiemensPlcOpcUaWidget& object);
  virtual void setupUi(qSlicerSiemensPlcOpcUaWidget*);
  bool ParseServerParentNode(const QString& nodeDisplayName);
  QOpcUaNode* FindNodeFromFullDisplayName(const QString& nodeDisplayName);

  bool ConnectMessagesNodes();
  bool ConnectModeAndStatusNodes();

  bool ConnectAutoManualR1ToLoadNodes();
  bool ConnectAutoManualR1LoadToIsoNodes();
  bool ConnectAutoManualR1ToNewCoordsNodes();
  bool ConnectAutoManualR2ToHomeNodes();

  bool ConnectResetErrorsNodes();
  bool ConnectMakeXrayNodes();

  QScopedPointer< OpcUaModel > SiemensPlcOpcUaModel;
  QWeakPointer< QOpcUaClient > OpcUaClient;
  vtkWeakPointer< vtkMRMLSiemensPlcOpcUaNode > ParameterNode;

  // Parse parent node
  const QString SIEMENS_PLC_SERVER_INTERFACES_NODE_ID = "ns=3;s=ServerInterfaces";
  QScopedPointer<QOpcUaNode> ServerInterfacesNode;
  // Monitored node
  const QString SIEMENS_PLC_CURRENT_TIME_NODE_ID = "ns=0;i=2258";
  QScopedPointer<QOpcUaNode> CurrentTimeNode; // Siemens PLC monitored node to prevent session timeout ending

  QMap< QString, NodeData > NodeNameDataMap; // key - node unique full name, value - node data
};

// --------------------------------------------------------------------------
qSlicerSiemensPlcOpcUaWidgetPrivate::qSlicerSiemensPlcOpcUaWidgetPrivate(qSlicerSiemensPlcOpcUaWidget& object)
  : q_ptr(&object)
  , SiemensPlcOpcUaModel(new OpcUaModel(&object))
{
}

// --------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidgetPrivate::setupUi(qSlicerSiemensPlcOpcUaWidget* widget)
{
  Q_Q(qSlicerSiemensPlcOpcUaWidget);

  this->Ui_qSlicerSiemensPlcOpcUaWidget::setupUi(widget);

  this->TreeView_OpcUaModel->setModel(SiemensPlcOpcUaModel.data());
  this->TreeView_OpcUaModel->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ParseServerParentNode(const QString& serverNodeDispName)
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
  OpcUaTreeItem* parentItem = nullptr;
  OpcUaTreeItem* rootItem = SiemensPlcOpcUaModel->getRootItem();
  if (rootItem)
  {
    parentItem = OpcUaTreeItem::findParentItemByName(rootItem, serverNodeDispName);
  }
  else
  {
    return false;
  }
  this->NodeNameDataMap.clear();
  if (parentItem)
  {
    QList< QPointer< OpcUaTreeItem > > items = OpcUaTreeItem::findLeafValueItems(parentItem);
    for (QPointer< OpcUaTreeItem > item : items)
    {
      QStringList parentNames = OpcUaTreeItem::findParentNamesForItem(item.data(), parentItem);

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
        if (itemNode)
        {
          qDebug() << Q_FUNC_INFO << "QOpcUaNode node is created for ID:" << item->getNodeId();
        }
        NodeData data(itemNode, item->getNodeId(), hierNames, item->getNodeDescription());
        this->NodeNameDataMap[hierNames] = data;
      }
    }
  }
  return true;
}

//-----------------------------------------------------------------------------
QOpcUaNode* qSlicerSiemensPlcOpcUaWidgetPrivate::FindNodeFromFullDisplayName(const QString& nodeDisplayName)
{
  if (this->NodeNameDataMap.size())
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
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectMessagesNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // write values
  // Error messages
  QOpcUaNode* errMessagesNode = this->FindNodeFromFullDisplayName(ASU_ERROR_MESSAGES_NODE_NAME);
  // Service messages
  QOpcUaNode* servMessagesNode = this->FindNodeFromFullDisplayName(ASU_SERVICE_MESSAGES_NODE_NAME);
  // Miscellaneous messages
  QOpcUaNode* miscMessagesNode = this->FindNodeFromFullDisplayName(ASU_MESSAGES_NODE_NAME);

  // Connect signal handlers for subscribed values
  // Error messages
  if (!errMessagesNode)
  {
    return false;
  }
  QObject::connect(errMessagesNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert<QVariantList>())
      {
        std::bitset< vtkMRMLSiemensPlcOpcUaNode::MESSAGES_SIZE > errors;
        QVariantList errList = value.toList(); // Get the attribute from the cache
        if (errList.size() == vtkMRMLSiemensPlcOpcUaNode::MESSAGES_SIZE)
        {
          int i = 0;
          for (const QVariant& errFlag : errList)
          {
            bool value = errFlag.toBool();
            errors.set(i, value);
            ++i;
          }
          uint64_t errValue = errors.to_ullong();
          mrmlNode->SetErrorMessages(errValue);
          qDebug() << Q_FUNC_INFO << "Error messages value:" << errValue;
        }
      }
    }
  );
  QObject::connect(errMessagesNode,
    &QOpcUaNode::attributeRead, [errMessagesNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (errMessagesNode && errMessagesNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = errMessagesNode->attribute(QOpcUa::NodeAttribute::Value);
          if (value.canConvert<QVariantList>())
          {
            std::bitset< vtkMRMLSiemensPlcOpcUaNode::MESSAGES_SIZE > errors;
            QVariantList errList = value.toList(); // Get the attribute from the cache
            if (errList.size() == vtkMRMLSiemensPlcOpcUaNode::MESSAGES_SIZE)
            {
              int i = 0;
              for (const QVariant& errFlag : errList)
              {
                bool value = errFlag.toBool();
                errors.set(i, value);
                ++i;
              }
              uint64_t errValue = errors.to_ullong();
              mrmlNode->SetErrorMessages(errValue);
              qDebug() << Q_FUNC_INFO << "Error messages value:" << errValue;
            }
          }
        }
      }
    }
  );
  // Subscribe to data changes
  errMessagesNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(1000));

  // Service messages
  if (!servMessagesNode)
  {
    return false;
  }
  QObject::connect(servMessagesNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert<QVariantList>())
      {
        std::bitset< vtkMRMLSiemensPlcOpcUaNode::MESSAGES_SIZE > servFlags;
        QVariantList servList = value.toList(); // Get the attribute from the cache
        if (servList.size() == vtkMRMLSiemensPlcOpcUaNode::MESSAGES_SIZE)
        {
          int i = 0;
          for (const QVariant& servFlag : servList)
          {
            bool value = servFlag.toBool();
            servFlags.set(i, value);
            ++i;
          }
          uint64_t servValue = servFlags.to_ullong();
          mrmlNode->SetServiceMessages(servValue);
          qDebug() << Q_FUNC_INFO << "Service messages value:" << servValue;
        }
      }
    }
  );
  QObject::connect(servMessagesNode,
    &QOpcUaNode::attributeRead, [servMessagesNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (servMessagesNode && servMessagesNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = servMessagesNode->attribute(QOpcUa::NodeAttribute::Value);
          if (value.canConvert<QVariantList>())
          {
            std::bitset< vtkMRMLSiemensPlcOpcUaNode::MESSAGES_SIZE > servFlags;
            QVariantList servList = value.toList(); // Get the attribute from the cache
            if (servList.size() == vtkMRMLSiemensPlcOpcUaNode::MESSAGES_SIZE)
            {
              int i = 0;
              for (const QVariant& servFlag : servList)
              {
                bool value = servFlag.toBool();
                servFlags.set(i, value);
                ++i;
              }
              uint64_t servValue = servFlags.to_ullong();
              mrmlNode->SetServiceMessages(servValue);
              qDebug() << Q_FUNC_INFO << "Service messages value:" << servValue;
            }
          }
        }
      }
    }
  );
  // Subscribe to data changes
  servMessagesNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(1000));

  // Misc messages
  if (!miscMessagesNode)
  {
    return false;
  }
  QObject::connect(miscMessagesNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert<QVariantList>())
      {
        std::bitset< vtkMRMLSiemensPlcOpcUaNode::MESSAGES_SIZE > miscFlags;
        QVariantList miscList = value.toList(); // Get the attribute from the cache
        if (miscList.size() == vtkMRMLSiemensPlcOpcUaNode::MESSAGES_SIZE)
        {
          int i = 0;
          for (const QVariant& miscFlag : miscList)
          {
            bool value = miscFlag.toBool();
            miscFlags.set(i, value);
            ++i;
          }
          uint64_t miscValue = miscFlags.to_ullong();
          mrmlNode->SetMiscMessages(miscValue);
          qDebug() << Q_FUNC_INFO << "Miscellaneous messages value:" << miscValue;
        }
      }
    }
  );
  QObject::connect(miscMessagesNode,
    &QOpcUaNode::attributeRead, [miscMessagesNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (miscMessagesNode && miscMessagesNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = miscMessagesNode->attribute(QOpcUa::NodeAttribute::Value);
          if (value.canConvert<QVariantList>())
          {
            std::bitset< vtkMRMLSiemensPlcOpcUaNode::MESSAGES_SIZE > miscFlags;
            QVariantList miscList = value.toList(); // Get the attribute from the cache
            if (miscList.size() == vtkMRMLSiemensPlcOpcUaNode::MESSAGES_SIZE)
            {
              int i = 0;
              for (const QVariant& miscFlag : miscList)
              {
                bool value = miscFlag.toBool();
                miscFlags.set(i, value);
                ++i;
              }
              uint64_t miscValue = miscFlags.to_ullong();
              mrmlNode->SetMiscMessages(miscValue);
              qDebug() << Q_FUNC_INFO << "Miscellaneous messages value:" << miscValue;
            }
          }
        }
      }
    }
  );
  // Subscribe to data changes
  miscMessagesNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(1000));
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectAutoManualR1ToLoadNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // AutoManual R1 ToLoad button enabled node
  QOpcUaNode* amR1ToLoadEnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_AM_R1_TOLOAD_ENABLED_NODE_NAME);
  // AutoManual R1 ToLoad button pressed node
  QOpcUaNode* amR1ToLoadPressedNode = this->FindNodeFromFullDisplayName(BUTTONS_AM_R1_TOLOAD_PRESSED_NODE_NAME);

  // AutoManual R1 ToLoad button enabled node
  if (!amR1ToLoadEnabledNode)
  {
    return false;
  }
  QObject::connect(amR1ToLoadEnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool amR1ToLoadCurrentState[2] = { false, false };
        mrmlNode->GetAutoManualR1ToLoad(amR1ToLoadCurrentState);
        amR1ToLoadCurrentState[0] = enabledValue;
        mrmlNode->SetAutoManualR1ToLoad(amR1ToLoadCurrentState);
      }
    }
  );
  QObject::connect(amR1ToLoadEnabledNode,
    &QOpcUaNode::attributeRead, [amR1ToLoadEnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!amR1ToLoadEnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (amR1ToLoadEnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = amR1ToLoadEnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool amR1ToLoadCurrentState[2] = { false, false };
          mrmlNode->GetAutoManualR1ToLoad(amR1ToLoadCurrentState);
          amR1ToLoadCurrentState[0] = enabledValue;
          mrmlNode->SetAutoManualR1ToLoad(amR1ToLoadCurrentState);
        }
      }
    }
  );
  // Subscribe to data changes
  amR1ToLoadEnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(1000));

  // AutoManual R1 ToLoad button pressed node
  if (!amR1ToLoadPressedNode)
  {
    return false;
  }
  QObject::connect(amR1ToLoadPressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool amR1ToLoadCurrentState[2] = { false, false };
        mrmlNode->GetAutoManualR1ToLoad(amR1ToLoadCurrentState);
        amR1ToLoadCurrentState[1] = pressedValue;
        mrmlNode->SetAutoManualR1ToLoad(amR1ToLoadCurrentState);
      }
    }
  );
  QObject::connect(amR1ToLoadPressedNode,
    &QOpcUaNode::attributeRead, [amR1ToLoadPressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!amR1ToLoadPressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (amR1ToLoadPressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = amR1ToLoadPressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool amR1ToLoadCurrentState[2] = { false, false };
          mrmlNode->GetAutoManualR1ToLoad(amR1ToLoadCurrentState);
          amR1ToLoadCurrentState[1] = pressedValue;
          mrmlNode->SetAutoManualR1ToLoad(amR1ToLoadCurrentState);
        }
      }
    }
  );
  // Subscribe to data changes
  amR1ToLoadPressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(1000));
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectAutoManualR1LoadToIsoNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // AutoManual R1 LoadToIso button enabled node
  QOpcUaNode* amR1LoadToIsoEnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_AM_R1_LOADTOISO_ENABLED_NODE_NAME);
  // AutoManual R1 LoadToIso button pressed node
  QOpcUaNode* amR1LoadToIsoPressedNode = this->FindNodeFromFullDisplayName(BUTTONS_AM_R1_LOADTOISO_PRESSED_NODE_NAME);

  // AutoManual R1 LoadToIso button enabled node
  if (!amR1LoadToIsoEnabledNode)
  {
    return false;
  }
  QObject::connect(amR1LoadToIsoEnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool amR1LoadToIsoCurrentState[2] = { false, false };
        mrmlNode->GetAutoManualR1LoadToIso(amR1LoadToIsoCurrentState);
        amR1LoadToIsoCurrentState[0] = enabledValue;
        mrmlNode->SetAutoManualR1LoadToIso(amR1LoadToIsoCurrentState);
      }
    }
  );
  QObject::connect(amR1LoadToIsoEnabledNode,
    &QOpcUaNode::attributeRead, [amR1LoadToIsoEnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!amR1LoadToIsoEnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (amR1LoadToIsoEnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = amR1LoadToIsoEnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool amR1LoadToIsoCurrentState[2] = { false, false };
          mrmlNode->GetAutoManualR1LoadToIso(amR1LoadToIsoCurrentState);
          amR1LoadToIsoCurrentState[0] = enabledValue;
          mrmlNode->SetAutoManualR1LoadToIso(amR1LoadToIsoCurrentState);
        }
      }
    }
  );
  // Subscribe to data changes
  amR1LoadToIsoEnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(1000));

  // AutoManual R1 LoadToIso button pressed node
  if (!amR1LoadToIsoPressedNode)
  {
    return false;
  }
  QObject::connect(amR1LoadToIsoPressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool amR1LoadToIsoCurrentState[2] = { false, false };
        mrmlNode->GetAutoManualR1LoadToIso(amR1LoadToIsoCurrentState);
        amR1LoadToIsoCurrentState[1] = pressedValue;
        mrmlNode->SetAutoManualR1LoadToIso(amR1LoadToIsoCurrentState);
      }
    }
  );
  QObject::connect(amR1LoadToIsoPressedNode,
    &QOpcUaNode::attributeRead, [amR1LoadToIsoPressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!amR1LoadToIsoPressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (amR1LoadToIsoPressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = amR1LoadToIsoPressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool amR1LoadToIsoCurrentState[2] = { false, false };
          mrmlNode->GetAutoManualR1LoadToIso(amR1LoadToIsoCurrentState);
          amR1LoadToIsoCurrentState[1] = pressedValue;
          mrmlNode->SetAutoManualR1LoadToIso(amR1LoadToIsoCurrentState);
        }
      }
    }
  );
  // Subscribe to data changes
  amR1LoadToIsoPressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(1000));
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectResetErrorsNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // ResetErrors button enabled node
  QOpcUaNode* reEnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_RESETERRORS_ENABLED_NODE_NAME);
  // ResetErrors button pressed node
  QOpcUaNode* rePressedNode = this->FindNodeFromFullDisplayName(BUTTONS_RESETERRORS_PRESSED_NODE_NAME);

  // ResetErrors button enabled node
  if (!reEnabledNode)
  {
    return false;
  }
  QObject::connect(reEnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool resetErrorsCurrentState[2] = { false, false };
        mrmlNode->GetResetErrors(resetErrorsCurrentState);
        resetErrorsCurrentState[0] = enabledValue;
        mrmlNode->SetResetErrors(resetErrorsCurrentState);
      }
    }
  );
  QObject::connect(reEnabledNode,
    &QOpcUaNode::attributeRead, [reEnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!reEnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (reEnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = reEnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool resetErrorsCurrentState[2] = { false, false };
          mrmlNode->GetResetErrors(resetErrorsCurrentState);
          resetErrorsCurrentState[0] = enabledValue;
          mrmlNode->SetResetErrors(resetErrorsCurrentState);
        }
      }
    }
  );
  // Subscribe to data changes
  reEnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(1000));

  // ResetErrors button pressed node
  if (!rePressedNode)
  {
    return false;
  }
  QObject::connect(rePressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool resetErrorsCurrentState[2] = { false, false };
        mrmlNode->GetResetErrors(resetErrorsCurrentState);
        resetErrorsCurrentState[1] = pressedValue;
        mrmlNode->SetResetErrors(resetErrorsCurrentState);
      }
    }
  );
  QObject::connect(rePressedNode,
    &QOpcUaNode::attributeRead, [rePressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!rePressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (rePressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = rePressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool resetErrorsCurrentState[2] = { false, false };
          mrmlNode->GetResetErrors(resetErrorsCurrentState);
          resetErrorsCurrentState[1] = pressedValue;
          mrmlNode->SetResetErrors(resetErrorsCurrentState);
        }
      }
    }
  );
  // Subscribe to data changes
  rePressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(1000));
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectMakeXrayNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // MakeXRay button enabled node
  QOpcUaNode* mxEnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_MAKEXRAY_ENABLED_NODE_NAME);
  // MakeXRay button pressed node
  QOpcUaNode* mxPressedNode = this->FindNodeFromFullDisplayName(BUTTONS_MAKEXRAY_PRESSED_NODE_NAME);

  // MakeXRay button enabled node
  if (!mxEnabledNode)
  {
    return false;
  }
  QObject::connect(mxEnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool makeXrayCurrentState[2] = { false, false };
        mrmlNode->GetMakeXray(makeXrayCurrentState);
        makeXrayCurrentState[0] = enabledValue;
        mrmlNode->SetMakeXray(makeXrayCurrentState);
      }
    }
  );
  QObject::connect(mxEnabledNode,
    &QOpcUaNode::attributeRead, [mxEnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!mxEnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (mxEnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = mxEnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool makeXrayCurrentState[2] = { false, false };
          mrmlNode->GetMakeXray(makeXrayCurrentState);
          makeXrayCurrentState[0] = enabledValue;
          mrmlNode->SetMakeXray(makeXrayCurrentState);
        }
      }
    }
  );
  // Subscribe to data changes
  mxEnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(1000));

  // MakeXRay button pressed node
  if (!mxPressedNode)
  {
    return false;
  }
  QObject::connect(mxPressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool makeXrayCurrentState[2] = { false, false };
        mrmlNode->GetMakeXray(makeXrayCurrentState);
        makeXrayCurrentState[1] = pressedValue;
        mrmlNode->SetMakeXray(makeXrayCurrentState);
      }
    }
  );
  QObject::connect(mxPressedNode,
    &QOpcUaNode::attributeRead, [mxPressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!mxPressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (mxPressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = mxPressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool makeXrayCurrentState[2] = { false, false };
          mrmlNode->GetMakeXray(makeXrayCurrentState);
          makeXrayCurrentState[1] = pressedValue;
          mrmlNode->SetMakeXray(makeXrayCurrentState);
        }
      }
    }
  );
  // Subscribe to data changes
  mxPressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(1000));
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectAutoManualR1ToNewCoordsNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // AutoManual R1 ToNewCoords button enabled node
  QOpcUaNode* amR1ToNewCoordsEnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_AM_R1_NEWCOORDS_ENABLED_NODE_NAME);
  // AutoManual R1 ToNewCoords button pressed node
  QOpcUaNode* amR1ToNewCoordsPressedNode = this->FindNodeFromFullDisplayName(BUTTONS_AM_R1_NEWCOORDS_PRESSED_NODE_NAME);

  // AutoManual R1 ToNewCoords button enabled node
  if (!amR1ToNewCoordsEnabledNode)
  {
    return false;
  }
  QObject::connect(amR1ToNewCoordsEnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool amR1ToNewCoordsCurrentState[2] = { false, false };
        mrmlNode->GetAutoManualR1ToNewCoords(amR1ToNewCoordsCurrentState);
        amR1ToNewCoordsCurrentState[0] = enabledValue;
        mrmlNode->SetAutoManualR1ToNewCoords(amR1ToNewCoordsCurrentState);
      }
    }
  );
  QObject::connect(amR1ToNewCoordsEnabledNode,
    &QOpcUaNode::attributeRead, [amR1ToNewCoordsEnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!amR1ToNewCoordsEnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (amR1ToNewCoordsEnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = amR1ToNewCoordsEnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool amR1ToNewCoordsCurrentState[2] = { false, false };
          mrmlNode->GetAutoManualR1ToNewCoords(amR1ToNewCoordsCurrentState);
          amR1ToNewCoordsCurrentState[0] = enabledValue;
          mrmlNode->SetAutoManualR1ToNewCoords(amR1ToNewCoordsCurrentState);
        }
      }
    }
  );
  // Subscribe to data changes
  amR1ToNewCoordsEnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(1000));

  // AutoManual R1 ToNewCoords button pressed node
  if (!amR1ToNewCoordsPressedNode)
  {
    return false;
  }
  QObject::connect(amR1ToNewCoordsPressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool amR1ToNewCoordsCurrentState[2] = { false, false };
        mrmlNode->GetAutoManualR1ToNewCoords(amR1ToNewCoordsCurrentState);
        amR1ToNewCoordsCurrentState[1] = pressedValue;
        mrmlNode->SetAutoManualR1ToNewCoords(amR1ToNewCoordsCurrentState);
      }
    }
  );
  QObject::connect(amR1ToNewCoordsPressedNode,
    &QOpcUaNode::attributeRead, [amR1ToNewCoordsPressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!amR1ToNewCoordsPressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (amR1ToNewCoordsPressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = amR1ToNewCoordsPressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool amR1ToNewCoordsCurrentState[2] = { false, false };
          mrmlNode->GetAutoManualR1ToNewCoords(amR1ToNewCoordsCurrentState);
          amR1ToNewCoordsCurrentState[1] = pressedValue;
          mrmlNode->SetAutoManualR1ToNewCoords(amR1ToNewCoordsCurrentState);
        }
      }
    }
  );
  // Subscribe to data changes
  amR1ToNewCoordsPressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(1000));
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectAutoManualR2ToHomeNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // AutoManual R2 ToHome button enabled node
  QOpcUaNode* amR2ToHomeEnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_AM_R2_TOHOME_ENABLED_NODE_NAME);
  // AutoManual R2 ToHome button pressed node
  QOpcUaNode* amR2ToHomePressedNode = this->FindNodeFromFullDisplayName(BUTTONS_AM_R2_TOHOME_PRESSED_NODE_NAME);

  // AutoManual R2 ToHome button enabled node
  if (!amR2ToHomeEnabledNode)
  {
    return false;
  }
  QObject::connect(amR2ToHomeEnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool amR2ToHomeCurrentState[2] = { false, false };
        mrmlNode->GetAutoManualR2ToHome(amR2ToHomeCurrentState);
        amR2ToHomeCurrentState[0] = enabledValue;
        mrmlNode->SetAutoManualR2ToHome(amR2ToHomeCurrentState);
      }
    }
  );
  QObject::connect(amR2ToHomeEnabledNode,
    &QOpcUaNode::attributeRead, [amR2ToHomeEnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!amR2ToHomeEnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (amR2ToHomeEnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = amR2ToHomeEnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool amR2ToHomeCurrentState[2] = { false, false };
          mrmlNode->GetAutoManualR2ToHome(amR2ToHomeCurrentState);
          amR2ToHomeCurrentState[0] = enabledValue;
          mrmlNode->SetAutoManualR2ToHome(amR2ToHomeCurrentState);
        }
      }
    }
  );
  // Subscribe to data changes
  amR2ToHomeEnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(1000));

  // AutoManual R2 ToHome button pressed node
  if (!amR2ToHomePressedNode)
  {
    return false;
  }
  QObject::connect(amR2ToHomePressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool amR2ToHomeCurrentState[2] = { false, false };
        mrmlNode->GetAutoManualR2ToHome(amR2ToHomeCurrentState);
        amR2ToHomeCurrentState[1] = pressedValue;
        mrmlNode->SetAutoManualR2ToHome(amR2ToHomeCurrentState);
      }
    }
  );
  QObject::connect(amR2ToHomePressedNode,
    &QOpcUaNode::attributeRead, [amR2ToHomePressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!amR2ToHomePressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (amR2ToHomePressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = amR2ToHomePressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool amR2ToHomeCurrentState[2] = { false, false };
          mrmlNode->GetAutoManualR2ToHome(amR2ToHomeCurrentState);
          amR2ToHomeCurrentState[1] = pressedValue;
          mrmlNode->SetAutoManualR2ToHome(amR2ToHomeCurrentState);
        }
      }
    }
  );
  // Subscribe to data changes
  amR2ToHomePressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(1000));
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectModeAndStatusNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Mode node
  QOpcUaNode* modeNode = this->FindNodeFromFullDisplayName(ASU_MODE_NODE_NAME);
  // Status RTK node
  QOpcUaNode* statusRtkNode = this->FindNodeFromFullDisplayName(ASU_STATUS_RTK_NODE_NAME);
  // Status R1 (TableTop) node
  QOpcUaNode* statusR1Node = this->FindNodeFromFullDisplayName(ASU_STATUS_R1_TABLE_NODE_NAME);
  // Status R2 (C-Arm) node
  QOpcUaNode* statusR2Node = this->FindNodeFromFullDisplayName(ASU_STATUS_R2_CARM_NODE_NAME);

  // Connect signal handlers for subscribed values
  // Mode node
  if (!modeNode)
  {
    return false;
  }
  QObject::connect(modeNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int modeValue = value.toInt(&ok);
        if (ok)
        {
          mrmlNode->SetMode(static_cast< vtkMRMLSiemensPlcOpcUaNode::ModeType >(modeValue));
          qDebug() << Q_FUNC_INFO << "Mode value changed:" << modeValue;
        }
      }
    }
  );
  QObject::connect(modeNode,
    &QOpcUaNode::attributeRead, [modeNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!modeNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (modeNode && modeNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = modeNode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int modeValue = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            mrmlNode->SetMode(static_cast< vtkMRMLSiemensPlcOpcUaNode::ModeType >(modeValue));
            qDebug() << Q_FUNC_INFO << "Mode value read:" << modeValue;
          }
        }
      }
    }
  );
  // Subscribe to data changes
  modeNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(1000));

  // Status RTK node
  if (!statusRtkNode)
  {
    return false;
  }
  QObject::connect(statusRtkNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        uint16_t statusRtkValue = value.toInt(&ok);
        if (ok)
        {
          mrmlNode->SetStateRtk(statusRtkValue);
          qDebug() << Q_FUNC_INFO << "Status RTK value changed:" << statusRtkValue;
        }
      }
    }
  );
  QObject::connect(statusRtkNode,
    &QOpcUaNode::attributeRead, [statusRtkNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!statusRtkNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (statusRtkNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = statusRtkNode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          uint16_t statusRtkValue = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            mrmlNode->SetStateRtk(statusRtkValue);
            qDebug() << Q_FUNC_INFO << "Status RTK value read:" << statusRtkValue;
          }
        }
      }
    }
  );
  // Subscribe to data changes
  statusRtkNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(1000));

  // Status R1 (TableTop) node
  if (!statusR1Node)
  {
    return false;
  }
  QObject::connect(statusR1Node,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        uint16_t statusR1Value = value.toInt(&ok);
        if (ok)
        {
          mrmlNode->SetStateR1(statusR1Value);
          qDebug() << Q_FUNC_INFO << "Status R1 value changed:" << statusR1Value;
        }
      }
    }
  );
  QObject::connect(statusR1Node,
    &QOpcUaNode::attributeRead, [statusR1Node, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!statusR1Node || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (statusR1Node->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = statusR1Node->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          uint16_t statusR1Value = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            mrmlNode->SetStateR1(statusR1Value);
            qDebug() << Q_FUNC_INFO << "Status R1 value read:" << statusR1Value;
          }
        }
      }
    }
  );
  // Subscribe to data changes
  statusR1Node->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(1000));

  // Status R2 (C-Arm) node
  if (!statusR2Node)
  {
    return false;
  }
  QObject::connect(statusR2Node,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        uint16_t statusR2Value = value.toInt(&ok);
        if (ok)
        {
          mrmlNode->SetStateR2(statusR2Value);
          qDebug() << Q_FUNC_INFO << "Status R2 value changed:" << statusR2Value;
        }
      }
    }
  );
  QObject::connect(statusR2Node,
    &QOpcUaNode::attributeRead, [statusR2Node, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!statusR2Node || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (statusR2Node->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = statusR2Node->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          uint16_t statusR2Value = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            mrmlNode->SetStateR2(statusR2Value);
            qDebug() << Q_FUNC_INFO << "Status R2 value read:" << statusR2Value;
          }
        }
      }
    }
  );
  // Subscribe to data changes
  statusR2Node->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(1000));

  return true;
}

//-----------------------------------------------------------------------------
// qSlicerSiemensPlcOpcUaWidget methods

//-----------------------------------------------------------------------------
qSlicerSiemensPlcOpcUaWidget::qSlicerSiemensPlcOpcUaWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerSiemensPlcOpcUaWidgetPrivate(*this) )
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  d->setupUi(this);

  // AutoManual R1 ToLoad button
  QObject::connect(d->PushButton_AMR1ToLoad, SIGNAL(pressed()),
    this, SLOT(onAutoManualR1ToLoadPressed()));
  QObject::connect(d->PushButton_AMR1ToLoad, SIGNAL(released()),
    this, SLOT(onAutoManualR1ToLoadReleased()));
  // AutoManual R1 LoadToIso button
  QObject::connect(d->PushButton_AMR1LoadToIso, SIGNAL(pressed()),
    this, SLOT(onAutoManualR1LoadToIsoPressed()));
  QObject::connect(d->PushButton_AMR1LoadToIso, SIGNAL(released()),
    this, SLOT(onAutoManualR1LoadToIsoReleased()));
  // AutoManual R1 ToNewCoords button
  QObject::connect(d->PushButton_AMR1ToNewCoords, SIGNAL(pressed()),
    this, SLOT(onAutoManualR1ToNewCoordsPressed()));
  QObject::connect(d->PushButton_AMR1ToNewCoords, SIGNAL(released()),
    this, SLOT(onAutoManualR1ToNewCoordsReleased()));

  // AutoManual R2 ToHome button
  QObject::connect(d->PushButton_AMR2ToHome, SIGNAL(pressed()),
    this, SLOT(onAutoManualR2ToHomePressed()));
  QObject::connect(d->PushButton_AMR2ToHome, SIGNAL(released()),
    this, SLOT(onAutoManualR2ToHomeReleased()));
  // AutoManual R2 ToPlane1 button
  QObject::connect(d->PushButton_AMR2ToPlane1, SIGNAL(pressed()),
    this, SLOT(onAutoManualR2ToPlane1Pressed()));
  QObject::connect(d->PushButton_AMR2ToPlane1, SIGNAL(released()),
    this, SLOT(onAutoManualR2ToPlane1Released()));
  // AutoManual R2 ToPlane2 button
  QObject::connect(d->PushButton_AMR2ToPlane2, SIGNAL(pressed()),
    this, SLOT(onAutoManualR2ToPlane2Pressed()));
  QObject::connect(d->PushButton_AMR2ToPlane2, SIGNAL(released()),
    this, SLOT(onAutoManualR2ToPlane2Released()));

  // Reset Errors button
  QObject::connect(d->PushButton_ResetErrors, SIGNAL(pressed()),
    this, SLOT(onResetErrorsPressed()));
  QObject::connect(d->PushButton_ResetErrors, SIGNAL(released()),
    this, SLOT(onResetErrorsReleased()));

  // MakeXRay button
  QObject::connect(d->PushButton_MakeXRay, SIGNAL(pressed()),
    this, SLOT(onMakeXrayPressed()));
  QObject::connect(d->PushButton_MakeXRay, SIGNAL(released()),
    this, SLOT(onMakeXrayReleased()));
}

//-----------------------------------------------------------------------------
qSlicerSiemensPlcOpcUaWidget::~qSlicerSiemensPlcOpcUaWidget()
{
}

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::setSiemensPlcOpcUaClient(const QSharedPointer< QOpcUaClient >& sharedClient)
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);

  d->OpcUaClient = sharedClient;
}

//-----------------------------------------------------------------------------
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
    d->CollapsibleButton_AutomaticManualMovement->setEnabled(true);
    d->CollapsibleButton_AutomaticManualMovement->setCollapsed(false);
    d->CollapsibleButton_ServiceMovement->setEnabled(false);
    d->CollapsibleButton_ServiceMovement->setCollapsed(true);
    d->CollapsibleButton_KukaControllersMovement->setEnabled(false);
    d->CollapsibleButton_KukaControllersMovement->setCollapsed(true);
    break;
  case vtkMRMLSiemensPlcOpcUaNode::SERVICE:
    d->CollapsibleButton_AutomaticManualMovement->setEnabled(false);
    d->CollapsibleButton_AutomaticManualMovement->setCollapsed(true);
    d->CollapsibleButton_ServiceMovement->setEnabled(true);
    d->CollapsibleButton_ServiceMovement->setCollapsed(false);
    d->CollapsibleButton_KukaControllersMovement->setEnabled(false);
    d->CollapsibleButton_KukaControllersMovement->setCollapsed(true);
    break;
  case vtkMRMLSiemensPlcOpcUaNode::KUKA_CONTROLLERS:
    d->CollapsibleButton_AutomaticManualMovement->setEnabled(false);
    d->CollapsibleButton_AutomaticManualMovement->setCollapsed(true);
    d->CollapsibleButton_ServiceMovement->setEnabled(false);
    d->CollapsibleButton_ServiceMovement->setCollapsed(true);
    d->CollapsibleButton_KukaControllersMovement->setEnabled(true);
    d->CollapsibleButton_KukaControllersMovement->setCollapsed(false);
    break;
  case vtkMRMLSiemensPlcOpcUaNode::UNKNOWN:
    d->CollapsibleButton_AutomaticManualMovement->setEnabled(false);
    d->CollapsibleButton_AutomaticManualMovement->setCollapsed(true);
    d->CollapsibleButton_ServiceMovement->setEnabled(false);
    d->CollapsibleButton_ServiceMovement->setCollapsed(true);
    d->CollapsibleButton_KukaControllersMovement->setEnabled(false);
    d->CollapsibleButton_KukaControllersMovement->setCollapsed(true);
  default:
    break;
  }
  
  bool buttonState[2] = { false, false };
  d->ParameterNode->GetAutoManualR1ToLoad(buttonState);
  d->PushButton_AMR1ToLoad->setEnabled(buttonState[0]);

  d->ParameterNode->GetAutoManualR1LoadToIso(buttonState);
  d->PushButton_AMR1LoadToIso->setEnabled(buttonState[0]);

  d->ParameterNode->GetAutoManualR1ToNewCoords(buttonState);
  d->PushButton_AMR1ToNewCoords->setEnabled(buttonState[0]);

  d->ParameterNode->GetAutoManualR2ToHome(buttonState);
  d->PushButton_AMR2ToHome->setEnabled(buttonState[0]);

  d->ParameterNode->GetResetErrors(buttonState);
  d->PushButton_ResetErrors->setEnabled(buttonState[0]);

  d->ParameterNode->GetMakeXray(buttonState);
  d->PushButton_MakeXRay->setEnabled(buttonState[0]);

  qDebug() << Q_FUNC_INFO << "Update SiemensPlcOpcUa buttons";
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onOpcUaClientConnected()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  
  QSharedPointer< QOpcUaClient > sharedClient = d->OpcUaClient.toStrongRef();
  if (!sharedClient)
  {
    return;
  }

  d->SiemensPlcOpcUaModel->setOpcUaClient(sharedClient.data());
  d->TreeView_OpcUaModel->header()->setSectionResizeMode(1 /* Value column*/, QHeaderView::Interactive);

  d->CurrentTimeNode.reset(sharedClient->node(d->SIEMENS_PLC_CURRENT_TIME_NODE_ID));
  if (d->CurrentTimeNode)
  {
    qDebug() << Q_FUNC_INFO << "Current time node is OK";
    // Subscribe to data changes
    d->CurrentTimeNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(1000));
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onParseServerInterfacesClicked()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  
  QSharedPointer< QOpcUaClient > sharedClient = d->OpcUaClient.toStrongRef();
  if (!sharedClient)
  {
    return;
  }

  d->ServerInterfacesNode.reset(sharedClient->node(OpcUaTreeItem::SIEMENS_PLC_SERVER_INTERFACES_NODE_ID));
  if (d->ServerInterfacesNode)
  {
    qDebug() << Q_FUNC_INFO << "ServerInterfaces node is OK";
    QObject::connect(d->ServerInterfacesNode.data(), &QOpcUaNode::attributeRead, this, &qSlicerSiemensPlcOpcUaWidget::onServerInterfacesRead);
    d->ServerInterfacesNode->readAttributes(QOpcUa::NodeAttribute::DisplayName);
  }
  bool resMsg = d->ConnectMessagesNodes();
  bool resStatus = d->ConnectModeAndStatusNodes();
  if (resMsg && resStatus)
  {
    qDebug() << Q_FUNC_INFO << "Messages and Status nodes are OK";
  }
  bool amR1Buttons = true;
  amR1Buttons &= d->ConnectAutoManualR1ToLoadNodes();
  amR1Buttons &= d->ConnectAutoManualR1LoadToIsoNodes();
  amR1Buttons &= d->ConnectAutoManualR1ToNewCoordsNodes();

  bool amR2Buttons = true;
  amR2Buttons &= d->ConnectAutoManualR2ToHomeNodes();

  bool resetErrors = d->ConnectResetErrorsNodes();
  bool makeXray = d->ConnectMakeXrayNodes();

  if (amR1Buttons)
  {
    qDebug() << Q_FUNC_INFO << "AutomaticManual R1 nodes are OK";
  }

  if (resetErrors)
  {
    qDebug() << Q_FUNC_INFO << "ResetErrors nodes are OK";
  }
  if (makeXray)
  {
    qDebug() << Q_FUNC_INFO << "MakeXRay nodes are OK";
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onOpcUaClientDisconnected()
{
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onOpcUaModeChanged(vtkMRMLSiemensPlcOpcUaNode::ModeType mode)
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Write Mode value
  QOpcUaNode* modeNode = d->FindNodeFromFullDisplayName(ASU_MODE_NODE_NAME);
  if (modeNode)
  {
    qDebug() << Q_FUNC_INFO << ": Set new mode:" << mode;
    modeNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(mode), QOpcUa::Types::Byte);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR1LoadToIsoPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutoManual R1 LoadToIso button pressed node
  QOpcUaNode* amR1LoadToIsoPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_R1_LOADTOISO_PRESSED_NODE_NAME);
  if (amR1LoadToIsoPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 LoadToIso pressed";
    amR1LoadToIsoPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR1LoadToIsoReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutoManual R1 LoadToIso button pressed node
  QOpcUaNode* amR1LoadToIsoPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_R1_LOADTOISO_PRESSED_NODE_NAME);
  if (amR1LoadToIsoPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 LoadToIso released";
    amR1LoadToIsoPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR1ToLoadPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutoManual R1 ToLoad button pressed node
  QOpcUaNode* amR1ToLoadPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_R1_TOLOAD_PRESSED_NODE_NAME);
  if (amR1ToLoadPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 ToLoad pressed";
    amR1ToLoadPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR1ToLoadReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutoManual R1 ToLoad button pressed node
  QOpcUaNode* amR1ToLoadPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_R1_TOLOAD_PRESSED_NODE_NAME);
  if (amR1ToLoadPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 ToLoad released";
    amR1ToLoadPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR1ToNewCoordsPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutomaticManual R1 NewCoords button pressed node
  QOpcUaNode* amR1NewCoordsPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_R1_NEWCOORDS_PRESSED_NODE_NAME);
  if (amR1NewCoordsPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 NewCoord pressed";
    amR1NewCoordsPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR1ToNewCoordsReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutomaticManual R1 NewCoords button pressed node
  QOpcUaNode* amR1NewCoordsPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_R1_NEWCOORDS_PRESSED_NODE_NAME);
  if (amR1NewCoordsPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 NewCoord released";
    amR1NewCoordsPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR2ToHomePressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutomaticManual R2 ToHome button pressed node
  QOpcUaNode* amR2ToHomePressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_R2_TOHOME_PRESSED_NODE_NAME);
  if (amR2ToHomePressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 ToHome button pressed";
    amR2ToHomePressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR2ToHomeReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutomaticManual R2 ToHome button pressed node
  QOpcUaNode* amR2ToHomePressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_R2_TOHOME_PRESSED_NODE_NAME);
  if (amR2ToHomePressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 ToHome button released";
    amR2ToHomePressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR2ToPlane1Pressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutomaticManual R2 ToPlane1 button pressed node
  QOpcUaNode* amR2ToPlane1PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_R2_TOPLANE1_PRESSED_NODE_NAME);
  if (amR2ToPlane1PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 ToPlane1 button pressed";
    amR2ToPlane1PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR2ToPlane1Released()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutomaticManual R2 ToPlane1 button pressed node
  QOpcUaNode* amR2ToPlane1PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_R2_TOPLANE1_PRESSED_NODE_NAME);
  if (amR2ToPlane1PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 ToPlane1 button released";
    amR2ToPlane1PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR2ToPlane2Pressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutomaticManual R2 ToPlane2 button pressed node
  QOpcUaNode* amR2ToPlane2PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_R2_TOPLANE2_PRESSED_NODE_NAME);
  if (amR2ToPlane2PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 ToPlane2 button pressed";
    amR2ToPlane2PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR2ToPlane2Released()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutomaticManual R2 ToPlane2 button pressed node
  QOpcUaNode* amR2ToPlane2PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_R2_TOPLANE2_PRESSED_NODE_NAME);
  if (amR2ToPlane2PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 ToPlane2 button released";
    amR2ToPlane2PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR1R2EmerEvacPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutomaticManual R1R2 Emergency Evacuation button pressed node
  QOpcUaNode* amR1R2EmEvacPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_R1R2_EMEVAC_PRESSED_NODE_NAME);
  if (amR1R2EmEvacPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1R2 Emergency Evacuation button pressed";
    amR1R2EmEvacPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR1R2EmerEvacReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutomaticManual R1R2 Emergency Evacuation button pressed node
  QOpcUaNode* amR1R2EmEvacPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_R1R2_EMEVAC_PRESSED_NODE_NAME);
  if (amR1R2EmEvacPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1R2 Emergency Evacuation button released";
    amR1R2EmEvacPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualApplyTablePositionPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutomaticManual ApplyTableTopPosition button pressed node
  QOpcUaNode* amApplyTableTopPosPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_APPLYTABLEPOSITION_PRESSED_NODE_NAME);
  if (amApplyTableTopPosPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": ApplyTableTopPosition button pressed";
    amApplyTableTopPosPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualApplyTablePositionReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutomaticManual ApplyTableTopPosition button pressed node
  QOpcUaNode* amApplyTableTopPosPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_APPLYTABLEPOSITION_PRESSED_NODE_NAME);
  if (amApplyTableTopPosPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": ApplyTableTopPosition button pressed";
    amApplyTableTopPosPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1BreakTestPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 BreakTest button pressed node
  QOpcUaNode* smR1BeakTestPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_BREAKTEST_PRESSED_NODE_NAME);
  if (smR1BeakTestPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 BreakTest button pressed";
    smR1BeakTestPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1BreakTestReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 BreakTest button pressed node
  QOpcUaNode* smR1BeakTestPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_BREAKTEST_PRESSED_NODE_NAME);
  if (smR1BeakTestPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 BreakTest button released";
    smR1BeakTestPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1MasterReferenceTestPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 MasterReferenceTest button pressed node
  QOpcUaNode* smR1MasRefTestPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_MASREFTEST_PRESSED_NODE_NAME);
  if (smR1MasRefTestPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 MasterReferenceTest button pressed";
    smR1MasRefTestPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1MasterReferenceTestReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 MasterReferenceTest button pressed node
  QOpcUaNode* smR1MasRefTestPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_MASREFTEST_PRESSED_NODE_NAME);
  if (smR1MasRefTestPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 MasterReferenceTest button released";
    smR1MasRefTestPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1LoadPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 Load button pressed node
  QOpcUaNode* smR1LoadPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_LOAD_PRESSED_NODE_NAME);
  if (smR1LoadPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 Load button pressed";
    smR1LoadPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1LoadReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 Load button pressed node
  QOpcUaNode* smR1LoadPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_LOAD_PRESSED_NODE_NAME);
  if (smR1LoadPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 Load button released";
    smR1LoadPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1ServicePos1Pressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 ServicePosition1 button pressed node
  QOpcUaNode* smR1ServicePos1PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_SERVICEPOS1_PRESSED_NODE_NAME);
  if (smR1ServicePos1PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 ServicePosition1 button pressed";
    smR1ServicePos1PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1ServicePos1Released()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 ServicePosition1 button pressed node
  QOpcUaNode* smR1ServicePos1PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_SERVICEPOS1_PRESSED_NODE_NAME);
  if (smR1ServicePos1PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 ServicePosition1 button released";
    smR1ServicePos1PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1ServicePos2Pressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 ServicePosition2 button pressed node
  QOpcUaNode* smR1ServicePos2PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_SERVICEPOS2_PRESSED_NODE_NAME);
  if (smR1ServicePos2PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 ServicePosition2 button pressed";
    smR1ServicePos2PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1ServicePos2Released()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 ServicePosition2 button pressed node
  QOpcUaNode* smR1ServicePos2PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_SERVICEPOS2_PRESSED_NODE_NAME);
  if (smR1ServicePos2PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 ServicePosition2 button released";
    smR1ServicePos2PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1ServicePos3Pressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 ServicePosition3 button pressed node
  QOpcUaNode* smR1ServicePos3PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_SERVICEPOS3_PRESSED_NODE_NAME);
  if (smR1ServicePos3PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 ServicePosition3 button pressed";
    smR1ServicePos3PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1ServicePos3Released()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 ServicePosition3 button pressed node
  QOpcUaNode* smR1ServicePos3PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_SERVICEPOS3_PRESSED_NODE_NAME);
  if (smR1ServicePos3PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 ServicePosition3 button released";
    smR1ServicePos3PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2BreakTestPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 BreakTest button pressed node
  QOpcUaNode* smR2BeakTestPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_BREAKTEST_PRESSED_NODE_NAME);
  if (smR2BeakTestPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 BreakTest button pressed";
    smR2BeakTestPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2BreakTestReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 BreakTest button pressed node
  QOpcUaNode* smR2BeakTestPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_BREAKTEST_PRESSED_NODE_NAME);
  if (smR2BeakTestPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 BreakTest button released";
    smR2BeakTestPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2MasterReferenceTestPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 MasterReferenceTest button pressed node
  QOpcUaNode* smR2MasRefTestPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_MASREFTEST_PRESSED_NODE_NAME);
  if (smR2MasRefTestPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 MasterReferenceTest button pressed";
    smR2MasRefTestPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2MasterReferenceTestReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 MasterReferenceTest button pressed node
  QOpcUaNode* smR2MasRefTestPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_MASREFTEST_PRESSED_NODE_NAME);
  if (smR2MasRefTestPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 MasterReferenceTest button released";
    smR2MasRefTestPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2HomePressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 Home button pressed node
  QOpcUaNode* smR2HomePressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_HOME_PRESSED_NODE_NAME);
  if (smR2HomePressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 Home button pressed";
    smR2HomePressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2HomeReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 Load button pressed node
  QOpcUaNode* smR2HomePressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_HOME_PRESSED_NODE_NAME);
  if (smR2HomePressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 Home button released";
    smR2HomePressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2ServicePos1Pressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 ServicePosition1 button pressed node
  QOpcUaNode* smR2ServicePos1PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_SERVICEPOS1_PRESSED_NODE_NAME);
  if (smR2ServicePos1PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 ServicePosition1 button pressed";
    smR2ServicePos1PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2ServicePos1Released()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 ServicePosition1 button pressed node
  QOpcUaNode* smR2ServicePos1PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_SERVICEPOS1_PRESSED_NODE_NAME);
  if (smR2ServicePos1PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 ServicePosition1 button released";
    smR2ServicePos1PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2ServicePos2Pressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 ServicePosition2 button pressed node
  QOpcUaNode* smR2ServicePos2PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_SERVICEPOS2_PRESSED_NODE_NAME);
  if (smR2ServicePos2PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 ServicePosition2 button pressed";
    smR2ServicePos2PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2ServicePos2Released()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 ServicePosition2 button pressed node
  QOpcUaNode* smR2ServicePos2PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_SERVICEPOS2_PRESSED_NODE_NAME);
  if (smR2ServicePos2PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 ServicePosition2 button released";
    smR2ServicePos2PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2ServicePos3Pressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 ServicePosition3 button pressed node
  QOpcUaNode* smR2ServicePos3PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_SERVICEPOS3_PRESSED_NODE_NAME);
  if (smR2ServicePos3PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 ServicePosition3 button pressed";
    smR2ServicePos3PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2ServicePos3Released()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 ServicePosition3 button pressed node
  QOpcUaNode* smR2ServicePos3PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_SERVICEPOS3_PRESSED_NODE_NAME);
  if (smR2ServicePos3PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 ServicePosition3 button released";
    smR2ServicePos3PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceRestartSmPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service RestartServiceMode button pressed node
  QOpcUaNode* smRestartSmPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_RESTARTSM_PRESSED_NODE_NAME);
  if (smRestartSmPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": RestartServiceMode button pressed";
    smRestartSmPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceRestartSmReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service RestartServiceMode button pressed node
  QOpcUaNode* smRestartSmPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_RESTARTSM_PRESSED_NODE_NAME);
  if (smRestartSmPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": RestartServiceMode button released";
    smRestartSmPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onKukaAllowT1Pressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // KUKA Controllers AllowT1 button pressed node
  QOpcUaNode* kcmAllowT1PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_KCM_ALLOWT1_PRESSED_NODE_NAME);
  if (kcmAllowT1PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": AllowT1 button pressed";
    kcmAllowT1PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onKukaAllowT1Released()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // KUKA Controllers AllowT1 button pressed node
  QOpcUaNode* kcmAllowT1PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_KCM_ALLOWT1_PRESSED_NODE_NAME);
  if (kcmAllowT1PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": AllowT1 button released";
    kcmAllowT1PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onKukaAllowXrayPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // KUKA Controllers AllowXRay button pressed node
  QOpcUaNode* kcmAllowXrayPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_KCM_ALLOWXRAY_PRESSED_NODE_NAME);
  if (kcmAllowXrayPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": AllowXRay button pressed";
    kcmAllowXrayPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onKukaAllowXrayReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // KUKA Controllers AllowXRay button pressed node
  QOpcUaNode* kcmAllowXrayPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_KCM_ALLOWXRAY_PRESSED_NODE_NAME);
  if (kcmAllowXrayPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": AllowXRay button released";
    kcmAllowXrayPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

void qSlicerSiemensPlcOpcUaWidget::onKukaAllowBeamPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // KUKA Controllers AllowBeam button pressed node
  QOpcUaNode* kcmAllowBeamPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_KCM_ALLOWBEAM_PRESSED_NODE_NAME);
  if (kcmAllowBeamPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": AllowBeam button pressed";
    kcmAllowBeamPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onKukaAllowBeamReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // KUKA Controllers AllowXRay button pressed node
  QOpcUaNode* kcmAllowBeamPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_KCM_ALLOWBEAM_PRESSED_NODE_NAME);
  if (kcmAllowBeamPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": AllowBeam button released";
    kcmAllowBeamPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onResetErrorsPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Reset errors button pressed node
  QOpcUaNode* resetErrorsPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_RESETERRORS_PRESSED_NODE_NAME);
  if (resetErrorsPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": ResetErrors pressed";
    resetErrorsPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onResetErrorsReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutoManual R1 ToLoad button pressed node
  QOpcUaNode* resetErrorsPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_RESETERRORS_PRESSED_NODE_NAME);
  if (resetErrorsPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": ResetErrors released";
    resetErrorsPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onMakeXrayPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // MakeXRay button pressed node
  QOpcUaNode* makeXrayPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_MAKEXRAY_PRESSED_NODE_NAME);
  if (makeXrayPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": MakeXRay pressed";
    makeXrayPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onMakeXrayReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // MakeXRay button pressed node
  QOpcUaNode* makeXrayPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_MAKEXRAY_PRESSED_NODE_NAME);
  if (makeXrayPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": MakeXRay released";
    makeXrayPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

void qSlicerSiemensPlcOpcUaWidget::onServerInterfacesRead(QOpcUa::NodeAttributes attr)
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  
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
          qDebug() << Q_FUNC_INFO << "Node ID:" << nodeData.getNodeId() \
            << ", Name:" << nodeData.getNodeName() \
            << ", Description:" << nodeData.getNodeDescription() << Qt::endl;
        }
      }
    }
  }
}
