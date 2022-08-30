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

// IhepMlcControl Logic includes
#include "vtkSlicerIhepMlcControlLogic.h"

// SlicerRT Beams MRML includes
#include <vtkMRMLRTBeamNode.h>

// SlicerRT IhepMlcControl MRML includes
#include <vtkMRMLIhepMlcControlNode.h>

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerIhepMlcControlLogic);

//----------------------------------------------------------------------------
vtkSlicerIhepMlcControlLogic::vtkSlicerIhepMlcControlLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerIhepMlcControlLogic::~vtkSlicerIhepMlcControlLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerIhepMlcControlLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerIhepMlcControlLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerIhepMlcControlLogic::RegisterNodes()
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("RegisterNodes: Invalid MRML scene");
    return;
  }
//  if (!scene->IsNodeClassRegistered("vtkMRMLRTPlanNode"))
//  {
//    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLRTPlanNode>::New());
//  }
//  if (!scene->IsNodeClassRegistered("vtkMRMLRTBeamNode"))
//  {
//    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLRTBeamNode>::New());
//  }
  if (!scene->IsNodeClassRegistered("vtkMRMLIhepMlcControlNode"))
  {
    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLIhepMlcControlNode>::New());
  }
}

//---------------------------------------------------------------------------
void vtkSlicerIhepMlcControlLogic::UpdateFromMRMLScene()
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("UpdateFromMRMLScene: Invalid MRML scene");
    return;
  }
}

//---------------------------------------------------------------------------
void vtkSlicerIhepMlcControlLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (!node || !this->GetMRMLScene())
  {
    vtkErrorMacro("OnMRMLSceneNodeAdded: Invalid MRML scene or input node");
    return;
  }

//  if (node->IsA("vtkMRMLRTBeamNode"))
//  {
//    // Observe beam events
//    vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
//    events->InsertNextValue(vtkMRMLRTBeamNode::BeamGeometryModified);
//    events->InsertNextValue(vtkMRMLRTBeamNode::BeamTransformModified);
//    vtkObserveMRMLNodeEventsMacro(node, events);
//  }
  if (node->IsA("vtkMRMLIhepMlcControlNode"))
  {
    vtkNew<vtkIntArray> events;
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    vtkObserveMRMLNodeEventsMacro(node, events);
  }
}

//---------------------------------------------------------------------------
void vtkSlicerIhepMlcControlLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
  if (!this->GetMRMLScene())
  {
    vtkErrorMacro("OnMRMLSceneEndImport: Invalid MRML scene");
    return;
  }
}

//----------------------------------------------------------------------------
void vtkSlicerIhepMlcControlLogic::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
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

//  if (caller->IsA("vtkMRMLRTBeamNode"))
//  {
//    vtkMRMLRTBeamNode* beamNode = vtkMRMLRTBeamNode::SafeDownCast(caller);
//    if (event == vtkMRMLRTBeamNode::BeamTransformModified)
//    {
//    }
//    else if (event == vtkMRMLRTBeamNode::BeamGeometryModified)
//    {
//    }
//  }
  if (caller->IsA("vtkMRMLIhepMlcControlNode"))
  {
    vtkMRMLIhepMlcControlNode* parameterNode = vtkMRMLIhepMlcControlNode::SafeDownCast(caller);

    if (event == vtkCommand::ModifiedEvent)
    {
      // Update parameters using beam node data and create/update markups transform if they weren't created/updated
      vtkWarningMacro("ProcessMRMLNodesEvents: Process IhepMLC events in logic");
    }
  }
}
