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


// Beams includes
#include <vtkMRMLRTBeamNode.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLTableNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

#include "vtkMRMLIhepMlcControlNode.h"

// STD include
#include <cstring>

//------------------------------------------------------------------------------
namespace
{

const char* BEAM_REFERENCE_ROLE = "beamRef";

} // namespace

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLIhepMlcControlNode);

//----------------------------------------------------------------------------
vtkMRMLIhepMlcControlNode::vtkMRMLIhepMlcControlNode()
{
  // Observe RTBeam node events (like change of transform or geometry)
  vtkNew<vtkIntArray> nodeEvents;
  nodeEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  nodeEvents->InsertNextValue(vtkMRMLRTBeamNode::BeamGeometryModified);
  nodeEvents->InsertNextValue(vtkMRMLRTBeamNode::BeamTransformModified);
  this->AddNodeReferenceRole(BEAM_REFERENCE_ROLE, nullptr, nodeEvents);
}

//----------------------------------------------------------------------------
vtkMRMLIhepMlcControlNode::~vtkMRMLIhepMlcControlNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLIhepMlcControlNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream
  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLBooleanMacro(parallelBeam, ParallelBeam);
  // add new parameters here
  vtkMRMLWriteXMLEndMacro(); 
}

//----------------------------------------------------------------------------
void vtkMRMLIhepMlcControlNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  vtkMRMLNode::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLBooleanMacro(parallelBeam, ParallelBeam);
  // add new parameters here
  vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);

  // Note: ReportString is not read from XML, it is a strictly temporary value
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
void vtkMRMLIhepMlcControlNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  vtkMRMLIhepMlcControlNode* node = vtkMRMLIhepMlcControlNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  // Copy beam parameters
  this->DisableModifiedEventOn();
  vtkMRMLCopyBeginMacro(node);
  // add new parameters here
  vtkMRMLCopyBooleanMacro(ParallelBeam);
  vtkMRMLCopyEndMacro(); 

  this->EndModify(disabledModify);

  this->InvokePendingModifiedEvent();
}

//----------------------------------------------------------------------------
void vtkMRMLIhepMlcControlNode::CopyContent(vtkMRMLNode *anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLIhepMlcControlNode* node = vtkMRMLIhepMlcControlNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  vtkMRMLCopyBeginMacro(node);
  // add new parameters here
  vtkMRMLCopyBooleanMacro(ParallelBeam);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLIhepMlcControlNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);
  // add new parameters here
  vtkMRMLPrintBooleanMacro(ParallelBeam);
  vtkMRMLPrintEndMacro(); 
}

//----------------------------------------------------------------------------
void vtkMRMLIhepMlcControlNode::ProcessMRMLEvents(vtkObject *caller, unsigned long eventID, void *callData)
{
  Superclass::ProcessMRMLEvents(caller, eventID, callData);

  if (!this->Scene)
  {
    vtkErrorMacro("ProcessMRMLEvents: Invalid MRML scene");
    return;
  }
  if (this->Scene->IsBatchProcessing())
  {
    return;
  }

  // Update the DRR View-Up and normal vectors, if beam geometry or transform was changed
  switch (eventID)
  {
  case vtkMRMLRTBeamNode::BeamGeometryModified:
  case vtkMRMLRTBeamNode::BeamTransformModified:
    this->Modified();
    break;
  default:
    break;
  }
}

//----------------------------------------------------------------------------
vtkMRMLRTBeamNode* vtkMRMLIhepMlcControlNode::GetBeamNode()
{
  return vtkMRMLRTBeamNode::SafeDownCast( this->GetNodeReference(BEAM_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLIhepMlcControlNode::SetAndObserveBeamNode(vtkMRMLRTBeamNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("SetAndObserveBeamNode: Cannot set reference, the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(BEAM_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

//----------------------------------------------------------------------------
bool vtkMRMLIhepMlcControlNode::GetPairOfLeavesData(vtkMRMLIhepMlcControlNode::PairOfLeavesData& pairOfLeaves,
  int index, vtkMRMLIhepMlcControlNode::LayerType layer)
{
  int key = index + this->NumberOfLeafPairs * static_cast<int>(layer);
  auto it = this->LeavesDataMap.find(key);
  if (it != this->LeavesDataMap.end())
  {
    pairOfLeaves = it->second;
    return true;
  }
  return false;
}

//----------------------------------------------------------------------------
bool vtkMRMLIhepMlcControlNode::SetPairOfLeavesData(const vtkMRMLIhepMlcControlNode::PairOfLeavesData& pairOfLeaves,
  int index, vtkMRMLIhepMlcControlNode::LayerType layer)
{
  int key = index + this->NumberOfLeafPairs * static_cast<int>(layer);
  auto it = this->LeavesDataMap.find(key);
  if (it != this->LeavesDataMap.end())
  {
    it->second = pairOfLeaves;
    this->Modified();
    return true;
  }
  return false;
}

//----------------------------------------------------------------------------
bool vtkMRMLIhepMlcControlNode::GetLeafData(vtkMRMLIhepMlcControlNode::LeafData& leafData,
  int index, vtkMRMLIhepMlcControlNode::SideType side, vtkMRMLIhepMlcControlNode::LayerType layer)
{
  int key = index + this->NumberOfLeafPairs * static_cast<int>(layer);
  auto it = this->LeavesDataMap.find(key);
  if (it != this->LeavesDataMap.end())
  {
    bool res = true;
    switch (side)
    {
    case vtkMRMLIhepMlcControlNode::Side1:
      leafData = it->second.first;
      break;
    case vtkMRMLIhepMlcControlNode::Side2:
      leafData = it->second.second;
      break;
    default:
      res = false;
      break;
    }
    return res;
  }
  return false;
}

//----------------------------------------------------------------------------
bool vtkMRMLIhepMlcControlNode::SetLeafData(const vtkMRMLIhepMlcControlNode::LeafData& leafData, int index,
  vtkMRMLIhepMlcControlNode::SideType side, vtkMRMLIhepMlcControlNode::LayerType layer)
{
  int key = index + this->NumberOfLeafPairs * static_cast<int>(layer);
  auto it = this->LeavesDataMap.find(key);
  if (it != this->LeavesDataMap.end())
  {
    bool res = true;
    switch (side)
    {
    case vtkMRMLIhepMlcControlNode::Side1:
      it->second.first = leafData;
      break;
    case vtkMRMLIhepMlcControlNode::Side2:
      it->second.second = leafData;
      break;
    default:
      res = false;
      break;
    }
    return res;
  }
  return false;
}

//----------------------------------------------------------------------------
void vtkMRMLIhepMlcControlNode::SetPredefinedPosition(vtkMRMLIhepMlcControlNode::LayerType layer,
  vtkMRMLIhepMlcControlNode::PredefinedPositionType predef)
{
  double axisWidth = this->PairOfLeavesSize * this->NumberOfLeafPairs;
  double tanAngle = IHEP_SIDE_OPENING / axisWidth;

  vtkMRMLIhepMlcControlNode::LeafData leafData;
  switch (predef)
  {
  case Side1Edge:
    {
      for (int i = 0; i < this->NumberOfLeafPairs; ++i)
      {
        this->GetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side1, layer);
        leafData.Steps = IHEP_MOTOR_STEPS_PER_MM * (i * this->PairOfLeavesSize) * tanAngle + 400;
        this->SetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side1, layer);
      }
    }
    break;
  case Side2Edge:
    {
      for (int i = 0; i < this->NumberOfLeafPairs; ++i)
      {
        this->GetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side2, layer);
        leafData.Steps = IHEP_MOTOR_STEPS_PER_MM * (i * this->PairOfLeavesSize) * tanAngle + 400;
        this->SetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side2, layer);
      }
    }
    break;
  default:
    break;
  }
}

//----------------------------------------------------------------------------
void vtkMRMLIhepMlcControlNode::SetMlcLeavesClosed()
{
}

//----------------------------------------------------------------------------
void vtkMRMLIhepMlcControlNode::SetMlcLeavesOpened()
{
}

//----------------------------------------------------------------------------
bool vtkMRMLIhepMlcControlNode::SetMlcLeavesClosed(vtkMRMLIhepMlcControlNode::LayerType)
{
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLIhepMlcControlNode::SetMlcLeavesOpened(vtkMRMLIhepMlcControlNode::LayerType)
{
  return true;
}