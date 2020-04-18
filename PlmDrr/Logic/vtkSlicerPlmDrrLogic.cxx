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

// LoadableModuleTemplate Logic includes
#include "vtkSlicerPlmDrrLogic.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLRTBeamNode.h>
#include <vtkMRMLRTPlanNode.h>

#include <vtkMRMLMarkupsClosedCurveNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLMarkupsLineNode.h>

#include "vtkMRMLPlmDrrNode.h"

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

// Plastimatch reconstruct module
#include <drr.h>
#include <drr_options.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerPlmDrrLogic);

//----------------------------------------------------------------------------
vtkSlicerPlmDrrLogic::vtkSlicerPlmDrrLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerPlmDrrLogic::~vtkSlicerPlmDrrLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerPlmDrrLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerPlmDrrLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerPlmDrrLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("RegisterNodes: Invalid MRML scene");
    return;
  }
  if (!scene->IsNodeClassRegistered("vtkMRMLRTPlanNode"))
  {
    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLRTPlanNode>::New());
  }
  if (!scene->IsNodeClassRegistered("vtkMRMLRTBeamNode"))
  {
    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLRTBeamNode>::New());
  }
}

//---------------------------------------------------------------------------
void vtkSlicerPlmDrrLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerPlmDrrLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (!node || !this->GetMRMLScene())
  {
    vtkErrorMacro("OnMRMLSceneNodeAdded: Invalid MRML scene or input node");
    return;
  }

  if (node->IsA("vtkMRMLRTBeamNode"))
  {
    // Observe beam events
    vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
    events->InsertNextValue(vtkMRMLRTBeamNode::BeamTransformModified);
    vtkObserveMRMLNodeEventsMacro(node, events);
  }
  else if (node->IsA("vtkMRMLRTPlanNode"))
  {
    vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
    events->InsertNextValue(vtkMRMLRTPlanNode::IsocenterModifiedEvent);
    vtkObserveMRMLNodeEventsMacro(node, events);
  }
}

//---------------------------------------------------------------------------
void vtkSlicerPlmDrrLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

bool vtkSlicerPlmDrrLogic::SaveVolumeNode( const vtkMRMLVolumeNode* volumeNode, std::string& vtkNotUsed(filename))
{
  if (!volumeNode)
  {
    return false;
  }
  
  return false;
}

bool vtkSlicerPlmDrrLogic::ComputeDRR(Drr_options* opts)
{
  if (!opts)
  {
    return false;
  }
  return true;
}

bool vtkSlicerPlmDrrLogic::LoadDRR( vtkMRMLVolumeNode* volumeNode, const std::string& vtkNotUsed(filename))
{
  if (!volumeNode)
  {
    return false;
  }
  return true;
}

vtkMRMLMarkupsLineNode* vtkSlicerPlmDrrLogic::CreateDetectorNormal(vtkMRMLPlmDrrNode* node)
{
  return nullptr;
}

vtkMRMLMarkupsClosedCurveNode* vtkSlicerPlmDrrLogic::CreateDetectorBoundary(vtkMRMLPlmDrrNode* node)
{
  return nullptr;
}

vtkMRMLMarkupsClosedCurveNode* vtkSlicerPlmDrrLogic::CreateImageBoundary(vtkMRMLPlmDrrNode* node)
{
  return nullptr;
}

vtkMRMLMarkupsFiducialNode* vtkSlicerPlmDrrLogic::CreateImageFirstRowColumn(vtkMRMLPlmDrrNode* node)
{
  return nullptr;
}
