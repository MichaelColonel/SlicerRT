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
#include "vtkSlicerU70RoomScadaOpcUaNodesNames.h"

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
    vtkWarningMacro("ServerConnected: key Full Name: " << nodeFullName.toStdString() << ", node value id: " << i.value().toStdString());
    
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
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::WritePlanFileName(const std::string& textString)
{
  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::WritePatientID(const std::string& textString)
{
  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::WritePlanOpen(bool)
{
  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::WriteSysTimeModPos(uint64_t val)
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
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::WriteStateModPos(int state)
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
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::WriteEventString(const std::string& text)
{
  std::string nodeName = vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosEventString();
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeName));
  if (!node)
  {
    return false;
  }
  return node->writeValueAttribute(QVariant(QString::fromStdString(text)), QOpcUa::Types::String);
}
