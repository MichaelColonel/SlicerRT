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
vtkStandardNewMacro(vtkSlicerU70RoomScadaOpcUaLogic);


//---------------------------------------------------------------------------
class vtkSlicerU70RoomScadaOpcUaLogic::vtkInternal
{
public:
  vtkInternal(vtkSlicerU70RoomScadaOpcUaLogic* external);
  ~vtkInternal();
   QOpcUaNode* node(const QString& fullNodeName);

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
void vtkSlicerU70RoomScadaOpcUaLogic::ServerConnected(const QSharedPointer< QOpcUaClient >& sharedClient, const QHash< QString, QString >& hashNodesFullNamesIds)
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
    vtkWarningMacro("ServerConnected: key Full Name: " << nodeFullName.toStdString() << ", node value id: " << i.value().toStdString());
    
    vtkInternal::Entry scadaOpcUaNodeEntry;
    scadaOpcUaNodeEntry.Node = sharedClient->node(i.value());
    if (!scadaOpcUaNodeEntry.Node)
    {
      continue;
    }
    
    QOpcUaNode* node = scadaOpcUaNodeEntry.Node.data();

    node->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(1000));

    scadaOpcUaNodeEntry.ConnChanged = QObject::connect(node,
      &QOpcUaNode::dataChangeOccurred, [this, nodeFullName](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr != QOpcUa::NodeAttribute::Value)
      {
        return;
      }
      this->Internal->Nodes[nodeFullName].Cache = value; // map types as needed
    });

    scadaOpcUaNodeEntry.ConnRead = QObject::connect(node,
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
    this->Internal->Nodes.insert(nodeFullName, scadaOpcUaNodeEntry);
  }
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
bool vtkSlicerU70RoomScadaOpcUaLogic::WritePlanFileName(const std::string& textString)
{
  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomScadaOpcUaLogic::WritePatientID(const std::string& textString)
{
  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomScadaOpcUaLogic::WritePlanOpen(bool)
{
  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomScadaOpcUaLogic::WriteSysTimeModPos(uint64_t val)
{
  std::string nodeName = vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosSysTime();
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeName));
  if (!node)
  {
    return false;
  }
  return node->writeValueAttribute(QVariant(static_cast< qulonglong >(val)), QOpcUa::Types::UInt64);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomScadaOpcUaLogic::WriteStateModPos(int state)
{
  std::string nodeName = vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosSt();
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeName));
  if (!node)
  {
    return false;
  }
  return node->writeValueAttribute(QVariant(state), QOpcUa::Types::UInt16);
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
