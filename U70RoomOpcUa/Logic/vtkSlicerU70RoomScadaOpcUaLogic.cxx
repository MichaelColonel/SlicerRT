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

// U70RoomOpcUa Logic includes
#include "vtkSlicerU70RoomScadaOpcUaLogic.h"
#include "vtkSlicerU70RoomScadaOpcUaNodesNames.h"

// MRML includes
#include <vtkMRMLScene.h>

// U70RoomOpcUa MRML includes
#include "vtkMRMLScadaOpcUaNode.h"

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cstring>

// QtOpcUa includes
#include <QOpcUaClient>
#include <QWeakPointer>
#include <QPointer>
#include <QVariant>
#include <QSharedPointer>
#include <QScopedPointer>
#include <QOpcUaNode>
#include <QOpcUaMonitoringParameters>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerU70RoomScadaOpcUaLogic);

//---------------------------------------------------------------------------
class vtkSlicerU70RoomScadaOpcUaLogic::vtkInternal
{
public:
  vtkInternal(vtkSlicerU70RoomScadaOpcUaLogic* external);
  ~vtkInternal();
   QOpcUaNode* node(const QString& fullNodeName);
   uint64_t getUint64(const QString& fullNodeName, bool& res);
   uint32_t getUint32(const QString& fullNodeName, bool& res);
   int32_t getInt32(const QString& fullNodeName, bool& res);
   uint16_t getUint16(const QString& fullNodeName, bool& res);
   bool getBool(const QString& fullNodeName, bool& res);
   int8_t getInt8(const QString& fullNodeName, bool& res);
   uint8_t getUint8(const QString& fullNodeName, bool& res);
   std::string getString(const QString& fullNodeName, bool& res);

  vtkSlicerU70RoomScadaOpcUaLogic* External{ nullptr };

  struct Entry
  {
    QPointer< QOpcUaNode > Node;        // becomes null automatically if node is deleted
    QMetaObject::Connection ConnChanged;
    QMetaObject::Connection ConnRead;
    QVariant Cache;
  };

  QWeakPointer< QOpcUaClient > Client;
  QHash< QString, Entry > Nodes;
  bool InitialReadValuesFlag{ false };
};

//---------------------------------------------------------------------------
QOpcUaNode* vtkSlicerU70RoomScadaOpcUaLogic::vtkInternal::node(const QString& fullNodeName)
{
  QSharedPointer< QOpcUaClient > sharedClient = this->Client.toStrongRef();
  if (!sharedClient)
  {
    vtkWarningWithObjectMacro( this->External, "node: Scada OPC UA client is invalid");
    return nullptr;
  }

  if (sharedClient->state() != QOpcUaClient::Connected)
  {
    vtkWarningWithObjectMacro( this->External, "node: Scada OPC UA client is created, but isn't connected");
    return nullptr;
  }

  return this->Nodes.value(fullNodeName).Node;
}

//---------------------------------------------------------------------------
uint64_t vtkSlicerU70RoomScadaOpcUaLogic::vtkInternal::getUint64(const QString& fullNodeName, bool& res)
{
  QSharedPointer< QOpcUaClient > sharedClient = this->Client.toStrongRef();
  if (!sharedClient)
  {
    vtkWarningWithObjectMacro(this->External, "getUint64: Scada OPC UA client is invalid");
    res = false;
  }

  if (sharedClient->state() != QOpcUaClient::Connected)
  {
    vtkWarningWithObjectMacro(this->External, "getUint64: Scada OPC UA client is created, but isn't connected");
    res = false;
  }

  const QVariant& var = this->Nodes.value(fullNodeName).Cache;
  if (var.canConvert< qulonglong >())
  {
    bool resBool;
    qulonglong v = var.toULongLong(&resBool);
    if (resBool)
    {
      res = true;
      return static_cast< uint64_t >(v);
    }
  }
  return 0;
}


//---------------------------------------------------------------------------
uint32_t vtkSlicerU70RoomScadaOpcUaLogic::vtkInternal::getUint32(const QString& fullNodeName, bool& res)
{
  QSharedPointer< QOpcUaClient > sharedClient = this->Client.toStrongRef();
  if (!sharedClient)
  {
    vtkWarningWithObjectMacro(this->External, "getUint32: Scada OPC UA client is invalid");
    res = false;
  }

  if (sharedClient->state() != QOpcUaClient::Connected)
  {
    vtkWarningWithObjectMacro(this->External, "getUint32: Scada OPC UA client is created, but isn't connected");
    res = false;
  }

  const QVariant& var = this->Nodes.value(fullNodeName).Cache;
  if (var.canConvert< uint >())
  {
    bool resBool;
    uint v = var.toUInt(&resBool);
    if (resBool)
    {
      res = true;
      return static_cast< uint32_t >(v);
    }
  }
  return 0;
}

//---------------------------------------------------------------------------
int32_t vtkSlicerU70RoomScadaOpcUaLogic::vtkInternal::getInt32(const QString& fullNodeName, bool& res)
{
  QSharedPointer< QOpcUaClient > sharedClient = this->Client.toStrongRef();
  if (!sharedClient)
  {
    vtkWarningWithObjectMacro(this->External, "getInt32: Scada OPC UA client is invalid");
    res = false;
  }

  if (sharedClient->state() != QOpcUaClient::Connected)
  {
    vtkWarningWithObjectMacro(this->External, "getInt32: Scada OPC UA client is created, but isn't connected");
    res = false;
  }

  const QVariant& var = this->Nodes.value(fullNodeName).Cache;
  if (var.canConvert< int32_t >())
  {
    bool resBool;
    int v = var.toInt(&resBool);
    if (resBool)
    {
      res = true;
      return static_cast< int32_t >(v);
    }
  }
  return 0;
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomScadaOpcUaLogic::vtkInternal::getBool(const QString& fullNodeName, bool& res)
{
  QSharedPointer< QOpcUaClient > sharedClient = this->Client.toStrongRef();
  if (!sharedClient)
  {
    vtkWarningWithObjectMacro(this->External, "getInt32: Scada OPC UA client is invalid");
    res = false;
  }

  if (sharedClient->state() != QOpcUaClient::Connected)
  {
    vtkWarningWithObjectMacro(this->External, "getInt32: Scada OPC UA client is created, but isn't connected");
    res = false;
  }

  const QVariant& var = this->Nodes.value(fullNodeName).Cache;
  if (var.canConvert< bool >())
  {
    return var.toBool();
  }
  return false;
}

//---------------------------------------------------------------------------
int8_t vtkSlicerU70RoomScadaOpcUaLogic::vtkInternal::getInt8(const QString& fullNodeName, bool& res)
{
  QSharedPointer< QOpcUaClient > sharedClient = this->Client.toStrongRef();
  if (!sharedClient)
  {
    vtkWarningWithObjectMacro(this->External, "getInt8: Scada OPC UA client is invalid");
    res = false;
  }

  if (sharedClient->state() != QOpcUaClient::Connected)
  {
    vtkWarningWithObjectMacro(this->External, "getInt8: Scada OPC UA client is created, but isn't connected");
    res = false;
  }

  const QVariant& var = this->Nodes.value(fullNodeName).Cache;
  if (var.canConvert< int8_t >())
  {
    bool resBool;
    int v = var.toInt(&resBool);
    if (resBool)
    {
      res = true;
      return static_cast< int8_t >(v);
    }
  }
  return 0;
}

//---------------------------------------------------------------------------
uint8_t vtkSlicerU70RoomScadaOpcUaLogic::vtkInternal::getUint8(const QString& fullNodeName, bool& res)
{
  QSharedPointer< QOpcUaClient > sharedClient = this->Client.toStrongRef();
  if (!sharedClient)
  {
    vtkWarningWithObjectMacro(this->External, "getInt32: Scada OPC UA client is invalid");
    res = false;
  }

  if (sharedClient->state() != QOpcUaClient::Connected)
  {
    vtkWarningWithObjectMacro(this->External, "getInt32: Scada OPC UA client is created, but isn't connected");
    res = false;
  }

  const QVariant& var = this->Nodes.value(fullNodeName).Cache;
  if (var.canConvert< uint8_t >())
  {
    bool resBool;
    int v = var.toInt(&resBool);
    if (resBool)
    {
      res = true;
      return static_cast< uint8_t >(v);
    }
  }
  return 0;
}

//---------------------------------------------------------------------------
uint16_t vtkSlicerU70RoomScadaOpcUaLogic::vtkInternal::getUint16(const QString& fullNodeName, bool& res)
{
  QSharedPointer< QOpcUaClient > sharedClient = this->Client.toStrongRef();
  if (!sharedClient)
  {
    vtkWarningWithObjectMacro(this->External, "getInt32: Scada OPC UA client is invalid");
    res = false;
  }

  if (sharedClient->state() != QOpcUaClient::Connected)
  {
    vtkWarningWithObjectMacro(this->External, "getInt32: Scada OPC UA client is created, but isn't connected");
    res = false;
  }

  const QVariant& var = this->Nodes.value(fullNodeName).Cache;
  if (var.canConvert< int >())
  {
    bool resBool;
    int v = var.toInt(&resBool);
    if (resBool)
    {
      res = true;
      return static_cast< uint16_t >(v);
    }
  }
  return 0;
}


//---------------------------------------------------------------------------
std::string vtkSlicerU70RoomScadaOpcUaLogic::vtkInternal::getString(const QString& fullNodeName, bool& res)
{
  QSharedPointer< QOpcUaClient > sharedClient = this->Client.toStrongRef();
  if (!sharedClient)
  {
    vtkWarningWithObjectMacro(this->External, "getUint64: Scada OPC UA client is invalid");
    res = false;
  }

  if (sharedClient->state() != QOpcUaClient::Connected)
  {
    vtkWarningWithObjectMacro(this->External, "getUint64: Scada OPC UA client is created, but isn't connected");
    res = false;
  }

  const QVariant& var = this->Nodes.value(fullNodeName).Cache;
  if (var.canConvert< QString >())
  {
    QString str = var.toString();
    res = true;
    return str.toStdString();
  }
  return std::string();
}

//---------------------------------------------------------------------------
vtkSlicerU70RoomScadaOpcUaLogic::vtkInternal::vtkInternal(vtkSlicerU70RoomScadaOpcUaLogic* external)
{
  this->External = external;
}

//---------------------------------------------------------------------------
vtkSlicerU70RoomScadaOpcUaLogic::vtkInternal::~vtkInternal()
{
}

//----------------------------------------------------------------------------
vtkSlicerU70RoomScadaOpcUaLogic::vtkSlicerU70RoomScadaOpcUaLogic()
{
  this->Internal = new vtkInternal(this); 
}

//----------------------------------------------------------------------------
vtkSlicerU70RoomScadaOpcUaLogic::~vtkSlicerU70RoomScadaOpcUaLogic()
{
  if (this->Internal)
  {
    delete this->Internal;
    this->Internal = nullptr;
  }
}

//----------------------------------------------------------------------------
void vtkSlicerU70RoomScadaOpcUaLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomScadaOpcUaLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerU70RoomScadaOpcUaLogic::RegisterNodes()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    return;
  }

  if (!scene->IsNodeClassRegistered("vtkMRMLScadaOpcUaNode"))
  {
    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLScadaOpcUaNode>::New());
  }
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomScadaOpcUaLogic::UpdateFromMRMLScene()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    return;
  }
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomScadaOpcUaLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomScadaOpcUaLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
std::list< QOpcUaNode* > vtkSlicerU70RoomScadaOpcUaLogic::ServerConnected(const QSharedPointer< QOpcUaClient >& sharedClient, const QHash< QString, QString >& hashNodesFullNamesIds)
{
  std::list< QOpcUaNode* > connectedNodesList;

  this->UnregisterAllOpcUaNodes();

  this->Internal->Client = sharedClient;

  if (!sharedClient)
  {
    return connectedNodesList;
  }

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    return connectedNodesList;
  }

  vtkMRMLScadaOpcUaNode* scadaOpcUaNode = nullptr;
  if (vtkMRMLNode* node = scene->GetFirstNodeByClass("vtkMRMLScadaOpcUaNode"))
  {
    scadaOpcUaNode = vtkMRMLScadaOpcUaNode::SafeDownCast(node);
  }

  QHashIterator< QString, QString > i(hashNodesFullNamesIds);
  std::list< std::string > fullNameNodesList = vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaFullNodeNamesList();

  while (i.hasNext())
  {
    i.next();
    QString nodeFullName = i.key();

    vtkInternal::Entry scadaOpcUaNodeEntry;
    scadaOpcUaNodeEntry.Node = sharedClient->node(i.value());
    if (!scadaOpcUaNodeEntry.Node)
    {
      continue;
    }
    
    bool flag = false;
    for (const auto& builtinFullNodeName : fullNameNodesList)
    {
      if (nodeFullName == QString::fromStdString(builtinFullNodeName))
      {
        flag = true;
        break;
      }
    }
    if (!flag)
    {
      continue;
    }

    QOpcUaNode* node = scadaOpcUaNodeEntry.Node.data();

    connectedNodesList.push_back(node);

    node->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));

    scadaOpcUaNodeEntry.ConnChanged = QObject::connect(node,
      &QOpcUaNode::dataChangeOccurred, [this, nodeFullName, scadaOpcUaNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr != QOpcUa::NodeAttribute::Value)
      {
        return;
      }
      this->Internal->Nodes[nodeFullName].Cache = value; // map types as needed
      std::string stdNodeName = nodeFullName.toStdString();
      if (scadaOpcUaNode && !nodeFullName.isEmpty())
      {
        char* ptr = new char[stdNodeName.size() + 1];
        std::memset(ptr, 0, stdNodeName.size() + 1);
        stdNodeName.copy(ptr, stdNodeName.size());
        scadaOpcUaNode->InvokeCustomModifiedEvent(vtkMRMLScadaOpcUaNode::ValueChangedEvent, static_cast< void* >(ptr));
      }
    });

    scadaOpcUaNodeEntry.ConnRead = QObject::connect(node,
      &QOpcUaNode::attributeRead, [this, nodeFullName, node, scadaOpcUaNode](QOpcUa::NodeAttributes attr)
    {
      if (!node)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (node->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = node->attribute(QOpcUa::NodeAttribute::Value);
          this->Internal->Nodes[nodeFullName].Cache = value; // map types as needed
          std::string stdNodeName = nodeFullName.toStdString();
          if (scadaOpcUaNode && !nodeFullName.isEmpty())
          {
            char* ptr = new char[stdNodeName.size() + 1];
            std::memset(ptr, 0, stdNodeName.size() + 1);
            stdNodeName.copy(ptr, stdNodeName.size());
            scadaOpcUaNode->InvokeCustomModifiedEvent(vtkMRMLScadaOpcUaNode::ValueReadEvent, static_cast< void* >(ptr));
          }
        }
      }
    });
    this->Internal->Nodes.insert(nodeFullName, scadaOpcUaNodeEntry);
  }
  return connectedNodesList;
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomScadaOpcUaLogic::UnregisterOpcUaNode(const std::string& nodeFullName)
{
  auto it = this->Internal->Nodes.find(QString::fromStdString(nodeFullName));
  if (it == this->Internal->Nodes.end())
  {
    return;
  }
  QObject::disconnect(it.value().ConnChanged);
  QObject::disconnect(it.value().ConnRead);
  this->Internal->Nodes.erase(it);
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomScadaOpcUaLogic::UnregisterAllOpcUaNodes()
{
  for (auto& kv : this->Internal->Nodes)
  {
    QObject::disconnect(kv.ConnChanged);
    QObject::disconnect(kv.ConnRead);
  }
  this->Internal->Nodes.clear();
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomScadaOpcUaLogic::WritePlanFileName(const std::string& planFilename)
{
  std::string nodeName = vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosPlanStr();
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeName));
  if (!node)
  {
    return false;
  }
  return node->writeValueAttribute(QVariant(QString::fromStdString(planFilename)), QOpcUa::Types::String);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomScadaOpcUaLogic::WritePatientID(const std::string& patientId)
{
  std::string nodeName = vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosIdp();
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeName));
  if (!node)
  {
    return false;
  }
  return node->writeValueAttribute(QVariant(QString::fromStdString(patientId)), QOpcUa::Types::String);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomScadaOpcUaLogic::WritePlanOpenFlag(bool flagValue)
{
  std::string nodeName = vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosPlanOpen();
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeName));
  if (!node)
  {
    return false;
  }
  return node->writeValueAttribute(QVariant(flagValue), QOpcUa::Types::Boolean);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomScadaOpcUaLogic::WriteSysTimeModPos(int32_t val)
{
  std::string nodeName = vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosSysTime();
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeName));
  if (!node)
  {
    return false;
  }
  return node->writeValueAttribute(QVariant(val), QOpcUa::Types::Int32);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomScadaOpcUaLogic::WriteStateModPos(int32_t state)
{
  std::string nodeName = vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosSt();
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeName));
  if (!node)
  {
    return false;
  }
  return node->writeValueAttribute(QVariant(state), QOpcUa::Types::Int32);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomScadaOpcUaLogic::WriteEventString(const std::string& text)
{
  std::string nodeName = vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosEventStr();
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeName));
  if (!node)
  {
    return false;
  }
  return node->writeValueAttribute(QVariant(QString::fromStdString(text)), QOpcUa::Types::String);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomScadaOpcUaLogic::UpdateMRMLNodeFromOpcUaNodeValue(vtkMRMLScadaOpcUaNode* mrmlScadaNode, const std::string& fullNodeName)
{
  bool res = false;
  if (!mrmlScadaNode)
  {
    return false;
  }

  if (fullNodeName == vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaSysTime())
  {
    int32_t scadaSysTime = this->Internal->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker blocker(mrmlScadaNode);
      mrmlScadaNode->SetSysTimeScada(scadaSysTime);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosSt())
  {
    int32_t modPosState = this->Internal->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker blocker(mrmlScadaNode);
      mrmlScadaNode->SetSt(static_cast< vtkMRMLScadaOpcUaNode::StateType >(modPosState));
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosSysTime())
  {
    int32_t modPosSysTime = this->Internal->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker blocker(mrmlScadaNode);
      mrmlScadaNode->SetSysTimeModPos(modPosSysTime);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosPlanStr())
  {
    std::string planFileName = this->Internal->getString(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker blocker(mrmlScadaNode);
      mrmlScadaNode->SetPlanStr(planFileName);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosIdp())
  {
    std::string patientID = this->Internal->getString(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker blocker(mrmlScadaNode);
      mrmlScadaNode->SetIDP(patientID);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosEcp())
  {
    bool ecpPlanIsPresent = this->Internal->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker blocker(mrmlScadaNode);
      mrmlScadaNode->SetECP(ecpPlanIsPresent);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosImmob())
  {
    std::string immobName = this->Internal->getString(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker blocker(mrmlScadaNode);
      mrmlScadaNode->SetImmob(immobName);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosPlanOpen())
  {
    bool planIsOpen = this->Internal->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker blocker(mrmlScadaNode);
      mrmlScadaNode->SetPlanOpen(planIsOpen);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosPlanNpos())
  {
    int32_t npos = this->Internal->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker blocker(mrmlScadaNode);
      mrmlScadaNode->SetNPos(npos);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaRemoteCtrl())
  {
    int32_t remoteCtrl = this->Internal->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker blocker(mrmlScadaNode);
      mrmlScadaNode->SetRemoteCtrl(static_cast< vtkMRMLScadaOpcUaNode::RemoteControlType >(remoteCtrl));
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaRemoteStateMachine())
  {
    int32_t remoteCtrlStateMachine = this->Internal->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker blocker(mrmlScadaNode);
      mrmlScadaNode->SetRemoteCtrl(static_cast< vtkMRMLScadaOpcUaNode::RemoteStateMachineType >(remoteCtrlStateMachine));
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaRemoteCmd())
  {
    bool remoteCmd = this->Internal->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker blocker(mrmlScadaNode);
      mrmlScadaNode->SetRemoteCmd(remoteCmd);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaRemotePause())
  {
    bool remotePause = this->Internal->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker blocker(mrmlScadaNode);
      mrmlScadaNode->SetRemotePause(remotePause);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaRemoteAvr())
  {
    bool remoteMalfunction = this->Internal->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker blocker(mrmlScadaNode);
      mrmlScadaNode->SetRemoteAvr(remoteMalfunction);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaDoorBlock())
  {
    bool doorBlock = this->Internal->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker blocker(mrmlScadaNode);
      mrmlScadaNode->SetDoorBlock(doorBlock);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaReadyEq())
  {
    bool readyEq = this->Internal->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker blocker(mrmlScadaNode);
      mrmlScadaNode->SetReadyEq(readyEq);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaReadyPlan())
  {
    bool readyPlan = this->Internal->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker blocker(mrmlScadaNode);
      mrmlScadaNode->SetReadyPlan(readyPlan);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaReadyProc())
  {
    bool readyProc = this->Internal->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker blocker(mrmlScadaNode);
      mrmlScadaNode->SetReadyProc(readyProc);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaInQueue())
  {
    bool inQueue = this->Internal->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker blocker(mrmlScadaNode);
      mrmlScadaNode->SetInQueue(inQueue);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosReadySt())
  {
    bool readySt = this->Internal->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker blocker(mrmlScadaNode);
      mrmlScadaNode->SetReadySt(readySt);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaBlock())
  {
    bool readySt = this->Internal->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker blocker(mrmlScadaNode);
      mrmlScadaNode->SetReadySt(readySt);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaCurrentLayer())
  {
    int32_t currLayer = this->Internal->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker blocker(mrmlScadaNode);
      mrmlScadaNode->SetCurrentLayer(currLayer);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaMaxLayer())
  {
    int32_t maxLayer = this->Internal->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker blocker(mrmlScadaNode);
      mrmlScadaNode->SetMaxLayer(maxLayer);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaProcLayer())
  {
    int32_t procLayer = this->Internal->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker blocker(mrmlScadaNode);
      mrmlScadaNode->SetProcLayer(procLayer);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaProcEnd())
  {
    bool procEnd = this->Internal->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker blocker(mrmlScadaNode);
      mrmlScadaNode->SetProcEnd(procEnd);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosProcNote())
  {
    std::string procNote = this->Internal->getString(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker blocker(mrmlScadaNode);
      mrmlScadaNode->SetProcNote(procNote);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosRecClose())
  {
    bool recClose = this->Internal->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker blocker(mrmlScadaNode);
      mrmlScadaNode->SetRecClose(recClose);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosErrStr())
  {
    std::string errStr = this->Internal->getString(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker blocker(mrmlScadaNode);
      mrmlScadaNode->SetErrStr(errStr);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosEventStr())
  {
    std::string eventStr = this->Internal->getString(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker blocker(mrmlScadaNode);
      mrmlScadaNode->SetEventStr(eventStr);
    }
  }
  else
  {
    vtkErrorMacro("UpdateMRMLNodeFromOpcUaNodeValue: Unknown SCADA node, name: " << fullNodeName);
    res = false;
  }
  return res;
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomScadaOpcUaLogic::SetInitialReadValuesFlag(bool flag)
{
  this->Internal->InitialReadValuesFlag = flag;
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomScadaOpcUaLogic::ReadNodesValues(const std::list< std::string >& fullNamesList)
{
  bool result = true;
  for (const auto& nodefullName : fullNamesList)
  {
    QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodefullName));
    if (!node)
    {
      result = false;
      continue;
    }
    result &= node->readAttributes(QOpcUa::NodeAttribute::Value);
  }
  return result;
}

//---------------------------------------------------------------------------
std::vector< std::string > vtkSlicerU70RoomScadaOpcUaLogic::GetRegisteredNodeIds()
{
  std::vector< std::string > ids;
  for (const auto& nodeEntry : this->Internal->Nodes)
  {
    if (nodeEntry.Node)
    {
      QString nodeID = nodeEntry.Node->nodeId();
      ids.push_back(nodeID.toStdString());
    }
  }
  return ids;
}
