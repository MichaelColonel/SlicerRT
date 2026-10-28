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
#include <QSharedPointer>
#include <QScopedPointer>
#include <QOpcUaNode>

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
  QWeakPointer< QOpcUaClient > Client;
  QHash< QString, QOpcUaNode* > Nodes;
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
void vtkSlicerU70RoomOpcUaLogic::ServerConnected(const QSharedPointer< QOpcUaClient >& sharedClient, const QStringList& listOfNodes)
{
  this->Internal->Client = sharedClient;

  for (const QString& nodeId : listOfNodes)
  {
    this->Internal->node(nodeId);
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
