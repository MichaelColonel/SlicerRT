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
#include "vtkSlicerU70RoomScadaOpcUaLogic.h"
#include "vtkSlicerU70RoomSiemensPlcOpcUaLogic.h"

// MRML includes
#include <vtkMRMLScene.h>

// U70RoomOpcUa MRML includes
#include "vtkMRMLScadaOpcUaNode.h"
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

//----------------------------------------------------------------------------
vtkSlicerU70RoomOpcUaLogic::vtkSlicerU70RoomOpcUaLogic()
{
  this->Internal = new vtkInternal(this);
  this->ScadaOpcUaLogic = vtkSlicerU70RoomScadaOpcUaLogic::New();
  this->SiemensPlcOpcUaLogic = vtkSlicerU70RoomSiemensPlcOpcUaLogic::New();
}

//----------------------------------------------------------------------------
vtkSlicerU70RoomOpcUaLogic::~vtkSlicerU70RoomOpcUaLogic()
{
  if (this->ScadaOpcUaLogic)
  {
    this->ScadaOpcUaLogic->Delete();
    this->ScadaOpcUaLogic = nullptr;
  }

  if (this->SiemensPlcOpcUaLogic)
  {
    this->SiemensPlcOpcUaLogic->Delete();
    this->SiemensPlcOpcUaLogic = nullptr;
  }

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

  if (this->ScadaOpcUaLogic)
  {
    this->ScadaOpcUaLogic->SetMRMLScene(newScene);
  }
  if (this->SiemensPlcOpcUaLogic)
  {
    this->SiemensPlcOpcUaLogic->SetMRMLScene(newScene);
  }
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
  if (!scene->IsNodeClassRegistered("vtkMRMLSiemensPlcOpcUaNode"))
  {
    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLSiemensPlcOpcUaNode>::New());
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
void vtkSlicerU70RoomOpcUaLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    return;
  }

  if (node->IsA("vtkMRMLScadaOpcUaNode"))
  {
    // Observe custom OPC UA node events
    vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
    events->InsertNextValue(vtkMRMLScadaOpcUaNode::ValueChangedEvent);
    events->InsertNextValue(vtkMRMLScadaOpcUaNode::ValueReadEvent);
    vtkObserveMRMLNodeEventsMacro(node, events);
  }
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomOpcUaLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}


//----------------------------------------------------------------------------
void vtkSlicerU70RoomOpcUaLogic::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  Superclass::ProcessMRMLNodesEvents(caller, event, callData);
  vtkMRMLScene* mrmlScene = this->GetMRMLScene();
  if (!mrmlScene)
  {
    vtkErrorMacro("ProcessMRMLNodesEvents: Invalid MRML scene");
    return;
  }
  if (mrmlScene->IsBatchProcessing())
  {
    return;
  }

  if (caller->IsA("vtkMRMLScadaOpcUaNode"))
  {
    vtkMRMLScadaOpcUaNode* opcUaNode = vtkMRMLScadaOpcUaNode::SafeDownCast(caller);
    if (!opcUaNode)
    {
      vtkErrorMacro("ProcessMRMLNodesEvents: Scada OPC UA node is invalid");
      return;
    }
    if (event == vtkMRMLScadaOpcUaNode::ValueChangedEvent)
    {
      char* str = reinterpret_cast< char* >(callData);
      std::string string(str);
      delete [] str;
    }
    else if (event == vtkMRMLScadaOpcUaNode::ValueReadEvent)
    {
      char* str = reinterpret_cast< char* >(callData);
      std::string string(str);
      delete [] str;
    }
  }
  if (caller->IsA("vtkMRMLSiemensPlcOpcUaNode"))
  {
    vtkMRMLSiemensPlcOpcUaNode* opcUaNode = vtkMRMLSiemensPlcOpcUaNode::SafeDownCast(caller);
    if (!opcUaNode)
    {
      vtkErrorMacro("ProcessMRMLNodesEvents: Scada OPC UA node is invalid");
      return;
    }
    if (event == vtkMRMLSiemensPlcOpcUaNode::ValueChangedEvent)
    {
      char* str = reinterpret_cast< char* >(callData);
      std::string string(str);
      delete [] str;
    }
    else if (event == vtkMRMLSiemensPlcOpcUaNode::ValueReadEvent)
    {
      char* str = reinterpret_cast< char* >(callData);
      std::string string(str);
      delete [] str;
    }
  }
}

vtkSlicerU70RoomScadaOpcUaLogic* vtkSlicerU70RoomOpcUaLogic::GetScadaOpcUaLogic() const
{
  return this->ScadaOpcUaLogic;
}

vtkSlicerU70RoomSiemensPlcOpcUaLogic* vtkSlicerU70RoomOpcUaLogic::GetSiemensPlcOpcUaLogic() const
{
  return this->SiemensPlcOpcUaLogic;
}
