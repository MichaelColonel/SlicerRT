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
#include "vtkSlicerU70RoomOpcUaLogic.h"

// MRML includes
#include <vtkMRMLScene.h>

// U70RoomOpcUa MRML includes
#include "vtkMRMLScadaOpcUaNode.h"

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
#include <QSharedPointer>
#include <QScopedPointer>
#include <QOpcUaNode>

#include "vtkSlicerU70RoomScadaOpcUaNodesNames.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerU70RoomOpcUaLogic);

//---------------------------------------------------------------------------
class vtkSlicerU70RoomOpcUaLogic::vtkInternal
{
public:
  vtkInternal(vtkSlicerU70RoomOpcUaLogic* external);
  ~vtkInternal();
  QOpcUaNode* node(const QString& id);

  vtkSlicerU70RoomOpcUaLogic* External{ nullptr };

  struct Entry
  {
    QPointer< QOpcUaNode > Node;       // becomes null automatically if node is deleted
    QMetaObject::Connection Conn;
    vtkVariant Cache;
  };

  QWeakPointer< QOpcUaClient > Client;
  QHash< QString, QPointer< QOpcUaNode > > Nodes;
};

//---------------------------------------------------------------------------
vtkSlicerU70RoomOpcUaLogic::vtkInternal::vtkInternal(vtkSlicerU70RoomOpcUaLogic* external)
{
  this->External = external;
}

//---------------------------------------------------------------------------
vtkSlicerU70RoomOpcUaLogic::vtkInternal::~vtkInternal()
{
}

//---------------------------------------------------------------------------
QOpcUaNode* vtkSlicerU70RoomOpcUaLogic::vtkInternal::node(const QString& id)
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

  if (!this->Nodes.contains(id))
  {
    this->Nodes.insert(id, sharedClient->node(id));
  }
  return this->Nodes.value(id);
}

//----------------------------------------------------------------------------
vtkSlicerU70RoomOpcUaLogic::vtkSlicerU70RoomOpcUaLogic()
{
  this->Internal = new vtkInternal(this); 
}

//----------------------------------------------------------------------------
vtkSlicerU70RoomOpcUaLogic::~vtkSlicerU70RoomOpcUaLogic()
{
  if (this->Internal)
  {
    delete this->Internal;
    this->Internal = nullptr;
  }
}

//----------------------------------------------------------------------------
void vtkSlicerU70RoomOpcUaLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomOpcUaLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerU70RoomOpcUaLogic::RegisterNodes()
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
void vtkSlicerU70RoomOpcUaLogic::UpdateFromMRMLScene()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    return;
  }
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomOpcUaLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomOpcUaLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomOpcUaLogic::ServerConnected(const QSharedPointer< QOpcUaClient >& sharedClient, const QHash< QString, QPointer< QOpcUaNode > >& hashNodes)
{
  this->Internal->Client = sharedClient;
  this->Internal->Nodes = hashNodes;

  QHashIterator< QString, QPointer< QOpcUaNode > > i(this->Internal->Nodes);
  while (i.hasNext())
  {
    i.next();
    vtkWarningMacro("ServerConnected: key: " << i.key().toStdString() << ", node id: " << i.value()->nodeId().toStdString());
  }
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomOpcUaLogic::WriteBool(const std::string& nodeId, bool value)
{
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeId));
  if (!node)
  {
    return false;
  }
  return node->writeValueAttribute(QVariant(value), QOpcUa::Types::Boolean);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomOpcUaLogic::WriteByte(const std::string& nodeId, uint8_t value)
{
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeId));
  if (!node)
  {
    return false;
  }
  return node->writeValueAttribute(QVariant(value), QOpcUa::Types::Byte);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomOpcUaLogic::WriteUint32(const std::string& nodeId, uint32_t value)
{
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeId));
  if (!node)
  {
    return false;
  }
  return node->writeValueAttribute(QVariant(value), QOpcUa::Types::UInt32);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomOpcUaLogic::WriteString(const std::string& nodeId, const std::string& textString)
{
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeId));
  if (!node)
  {
    return false;
  }
  return node->writeValueAttribute(QVariant(QString::fromStdString(textString)), QOpcUa::Types::String);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomOpcUaLogic::WriteSysTimeModPos(uint64_t value)
{
  std::string nodeName = vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosSysTime();
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeName));
  if (!node)
  {
    return false;
  }
  return node->writeValueAttribute(QVariant(static_cast< qulonglong >(value)), QOpcUa::Types::UInt64);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomOpcUaLogic::WriteStateModPos(int state)
{
  std::string nodeName = vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosState();
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeName));
  if (!node)
  {
    return false;
  }
  return node->writeValueAttribute(QVariant(state), QOpcUa::Types::UInt16);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomOpcUaLogic:: WriteEventString(const std::string& text)
{
  std::string nodeName = vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosEventString();
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeName));
  if (!node)
  {
    return false;
  }
  return node->writeValueAttribute(QVariant(QString::fromStdString(text)), QOpcUa::Types::String);
}

//---------------------------------------------------------------------------
uint64_t vtkSlicerU70RoomOpcUaLogic::ReadSysTimeScada()
{
  return 0;
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomOpcUaLogic::WritePlanFileName(const std::string& textString)
{
  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomOpcUaLogic::WritePatientID(const std::string& textString)
{
  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomOpcUaLogic::WritePlanOpen(bool)
{
  return true;
}
