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
#include "vtkSlicerU70RoomSiemensPlcOpcUaLogic.h"
#include "vtkSlicerU70RoomSiemensPlcOpcUaNodesNames.h"

// MRML includes
#include <vtkMRMLScene.h>

// U70RoomOpcUa MRML includes
#include "vtkMRMLSiemensPlcOpcUaNode.h"

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

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
vtkStandardNewMacro(vtkSlicerU70RoomSiemensPlcOpcUaLogic);

//---------------------------------------------------------------------------
class vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal
{
public:
  vtkInternal(vtkSlicerU70RoomSiemensPlcOpcUaLogic* external);
  ~vtkInternal();
   QOpcUaNode* node(const QString& fullNodeName);
   double getDouble(const QString& fullNodeName, bool& res);
   uint64_t getUint64(const QString& fullNodeName, bool& res);
   uint32_t getUint32(const QString& fullNodeName, bool& res);
   int32_t getInt32(const QString& fullNodeName, bool& res);
   uint16_t getUint16(const QString& fullNodeName, bool& res);
   bool getBool(const QString& fullNodeName, bool& res);
   int8_t getInt8(const QString& fullNodeName, bool& res);
   uint8_t getUint8(const QString& fullNodeName, bool& res);

  vtkSlicerU70RoomSiemensPlcOpcUaLogic* External{ nullptr };

  struct Entry
  {
    QPointer< QOpcUaNode > Node;        // becomes null automatically if node is deleted
    QMetaObject::Connection ConnChanged;
    QMetaObject::Connection ConnRead;
    QVariant Cache;
  };

  QWeakPointer< QOpcUaClient > Client;
  QHash< QString, Entry > Nodes;

};

//---------------------------------------------------------------------------
QOpcUaNode* vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::node(const QString& fullNodeName)
{
  QSharedPointer< QOpcUaClient > sharedClient = this->Client.toStrongRef();
  if (!sharedClient)
  {
    vtkWarningWithObjectMacro(this->External, "node: Scada OPC UA client is invalid");
    return nullptr;
  }

  if (sharedClient->state() != QOpcUaClient::Connected)
  {
    vtkWarningWithObjectMacro(this->External, "node: Scada OPC UA client is created, but isn't connected");
    return nullptr;
  }

  return this->Nodes.value(fullNodeName).Node;
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::getDouble(const QString& fullNodeName, bool& res)
{
  int32_t v = this->getInt32(fullNodeName, res);
  return double(v) / 1000.;
}

//---------------------------------------------------------------------------
uint64_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::getUint64(const QString& fullNodeName, bool& res)
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
uint32_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::getUint32(const QString& fullNodeName, bool& res)
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
int32_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::getInt32(const QString& fullNodeName, bool& res)
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
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::getBool(const QString& fullNodeName, bool& res)
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
int8_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::getInt8(const QString& fullNodeName, bool& res)
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
uint8_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::getUint8(const QString& fullNodeName, bool& res)
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
uint16_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::getUint16(const QString& fullNodeName, bool& res)
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
vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::vtkInternal(vtkSlicerU70RoomSiemensPlcOpcUaLogic* external)
{
  this->External = external;
}

//---------------------------------------------------------------------------
vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::~vtkInternal()
{
}

//----------------------------------------------------------------------------
vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkSlicerU70RoomSiemensPlcOpcUaLogic()
{
  this->Internal = new vtkInternal(this); 
}

//----------------------------------------------------------------------------
vtkSlicerU70RoomSiemensPlcOpcUaLogic::~vtkSlicerU70RoomSiemensPlcOpcUaLogic()
{
  if (this->Internal)
  {
    delete this->Internal;
    this->Internal = nullptr;
  }
}

//----------------------------------------------------------------------------
void vtkSlicerU70RoomSiemensPlcOpcUaLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomSiemensPlcOpcUaLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerU70RoomSiemensPlcOpcUaLogic::RegisterNodes()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    return;
  }

  if (!scene->IsNodeClassRegistered("vtkMRMLSiemensPlcOpcUaNode"))
  {
    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLSiemensPlcOpcUaNode>::New());
  }
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomSiemensPlcOpcUaLogic::UpdateFromMRMLScene()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    return;
  }
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomSiemensPlcOpcUaLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomSiemensPlcOpcUaLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomSiemensPlcOpcUaLogic::ServerConnected(const QSharedPointer< QOpcUaClient >& sharedClient, const QHash< QString, QString >& hashNodesFullNamesIds)
{
  this->UnregisterAllOpcUaNodes();

  this->Internal->Client = sharedClient;

  if (!sharedClient)
  {
    return;
  }
  QHashIterator< QString, QString > i(hashNodesFullNamesIds);
  while (i.hasNext())
  {
    i.next();
    QString nodeFullName = i.key();
//    vtkWarningMacro("ServerConnected: key Full Name: " << nodeFullName.toStdString() << ", node value id: " << i.value().toStdString());
    
    vtkInternal::Entry siemensPlcOpcUaNodeEntry;
    siemensPlcOpcUaNodeEntry.Node = sharedClient->node(i.value());
    if (!siemensPlcOpcUaNodeEntry.Node)
    {
      continue;
    }
    
    QOpcUaNode* node = siemensPlcOpcUaNodeEntry.Node.data();

    node->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(1000));

    siemensPlcOpcUaNodeEntry.ConnChanged = QObject::connect(node,
      &QOpcUaNode::dataChangeOccurred, [this, nodeFullName](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr != QOpcUa::NodeAttribute::Value)
      {
        return;
      }
      this->Internal->Nodes[nodeFullName].Cache = value; // map types as needed
    });

    siemensPlcOpcUaNodeEntry.ConnRead = QObject::connect(node,
      &QOpcUaNode::attributeRead, [this, nodeFullName, node](QOpcUa::NodeAttributes attr)
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
        }
      }
    });
    this->Internal->Nodes.insert(nodeFullName, siemensPlcOpcUaNodeEntry);
  }
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomSiemensPlcOpcUaLogic::UnregisterOpcUaNode(const std::string& nodeFullName)
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
void vtkSlicerU70RoomSiemensPlcOpcUaLogic::UnregisterAllOpcUaNodes()
{
  for (auto& kv : this->Internal->Nodes)
  {
    QObject::disconnect(kv.ConnChanged);
    QObject::disconnect(kv.ConnRead);
  }
  this->Internal->Nodes.clear();
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR1A1(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A1();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR1A2(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A2();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR1A3(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A3();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR1A4(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A4();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR1A5(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A5();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR1A6(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A6();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR2A1(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A1();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR2A2(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A2();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR2A3(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A3();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR2A4(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A4();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR2A5(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A5();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR2A6(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A6();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
uint8_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadMode(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetMode();
  return this->Internal->getUint8(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
uint16_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadStatusRtk(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetStatusRtk();
  return this->Internal->getUint16(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
uint16_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadStatusR1(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetMode();
  return this->Internal->getUint16(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
uint16_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadStatusR2(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetMode();
  return this->Internal->getUint16(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomSiemensPlcOpcUaLogic::UpdateMRMLNodeFromOpcUaNodeValue(vtkMRMLSiemensPlcOpcUaNode* mrmlSiemensPlcNode, const std::string& fullNodeName)
{
  bool res = false;
  if (!mrmlSiemensPlcNode)
  {
    return;
  }
  if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A1())
  {
    int32_t r1a1 = this->Internal->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t r1axis[6] = {};
      mrmlSiemensPlcNode->GetCurrentAxisCoordsR1(r1axis);
      r1axis[0] = r1a1;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetCurrentAxisCoordsR1(r1axis);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetErrorsFlags())
  {
    uint64_t errMsgFlags = this->Internal->getUint64(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      mrmlSiemensPlcNode->SetErrorMessages(errMsgFlags);
    }
  }
}
