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
#include <vtkTable.h>

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
  vtkMRMLTableNode* mlcTableNode = nullptr;
  if (this->GetBeamNode())
  {
    mlcTableNode = this->GetBeamNode()->GetMultiLeafCollimatorTableNode();
  }

  vtkTable* table = nullptr;
  if (mlcTableNode && mlcTableNode->GetTable())
  {
    table = mlcTableNode->GetTable();
  }

  if (table)
  {
    vtkWarningMacro("SetPairOfLeavesData: table is valid, layer number: " << layer << ", index " << index);
  }

  int key = index + this->NumberOfLeafPairs * static_cast<int>(layer);
  auto it = this->LeavesDataMap.find(key);
  if (it != this->LeavesDataMap.end())
  {
    it->second = pairOfLeaves;
    this->Modified();
    if (table)
    {
      switch (layer)
      {
      case vtkMRMLIhepMlcControlNode::Layer1:
        table->SetValue(index, 1, -IHEP_TOTAL_DISTANCE + pairOfLeaves.first.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "1" for layer-1
        table->SetValue(index, 2, IHEP_TOTAL_DISTANCE - pairOfLeaves.second.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "2" for layer-1
        table->Modified();
        break;
      case vtkMRMLIhepMlcControlNode::Layer2:
        table->SetValue(index, 4, -IHEP_TOTAL_DISTANCE + pairOfLeaves.first.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "1" for layer-2
        table->SetValue(index, 5, IHEP_TOTAL_DISTANCE - pairOfLeaves.second.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "2" for layer-2
        table->Modified();
        break;
      default:
        break;
      }
    }
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
  vtkMRMLTableNode* mlcTableNode = nullptr;
  if (this->GetBeamNode())
  {
    mlcTableNode = this->GetBeamNode()->GetMultiLeafCollimatorTableNode();
  }

  vtkTable* table = nullptr;
  if (mlcTableNode && mlcTableNode->GetTable())
  {
    table = mlcTableNode->GetTable();
  }

  double axisWidth = this->PairOfLeavesSize * this->NumberOfLeafPairs;
  double tanAngle = IHEP_SIDE_OPENING / axisWidth;
  double totalDistange = IHEP_SIDE_OPENING_STEPS / IHEP_MOTOR_STEPS_PER_MM;
  vtkMRMLIhepMlcControlNode::LeafData leafData;
  switch (predef)
  {
  case vtkMRMLIhepMlcControlNode::Side1Edge:
    {
      for (int i = 0; i < this->NumberOfLeafPairs; ++i)
      {
        this->GetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side1, layer);
        leafData.Steps = IHEP_MOTOR_STEPS_PER_MM * (i * this->PairOfLeavesSize) * tanAngle + 400;
        this->SetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side1, layer);
        if (table)
        {
          switch (layer)
          {
          case vtkMRMLIhepMlcControlNode::Layer1:
            table->SetValue(i, 1, -IHEP_TOTAL_DISTANCE + leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "1" for layer-1
            break;
          case vtkMRMLIhepMlcControlNode::Layer2:
            table->SetValue(i, 4, -IHEP_TOTAL_DISTANCE + leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "1" for layer-2
            break;
          default:
            break;
          }
        }
      }
    }
    break;
  case vtkMRMLIhepMlcControlNode::Side2Edge:
    {
      for (int i = 0; i < this->NumberOfLeafPairs; ++i)
      {
        this->GetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side2, layer);
        leafData.Steps = IHEP_MOTOR_STEPS_PER_MM * (i * this->PairOfLeavesSize) * tanAngle + 400;
        this->SetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side2, layer);
        if (table)
        {
          switch (layer)
          {
          case vtkMRMLIhepMlcControlNode::Layer1:
            table->SetValue(i, 2, IHEP_TOTAL_DISTANCE - leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "2" for layer-1
            break;
          case vtkMRMLIhepMlcControlNode::Layer2:
            table->SetValue(i, 5, IHEP_TOTAL_DISTANCE - leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "2" for layer-2
            break;
          default:
            break;
          }
        }
      }
    }
    break;
  case vtkMRMLIhepMlcControlNode::DoubleSidedEdge:
    {
      for (int i = 0; i < this->NumberOfLeafPairs; ++i)
      {
        int pos = IHEP_MOTOR_STEPS_PER_MM * (i * this->PairOfLeavesSize) * tanAngle + 400;
        this->GetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side1, layer);
        leafData.Steps = pos;
        this->SetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side1, layer);
        if (table)
        {
          switch (layer)
          {
          case vtkMRMLIhepMlcControlNode::Layer1:
            table->SetValue(i, 1, -IHEP_TOTAL_DISTANCE + leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "1" for layer-1
            break;
          case vtkMRMLIhepMlcControlNode::Layer2:
            table->SetValue(i, 4, -IHEP_TOTAL_DISTANCE + leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "1" for layer-2
            break;
          default:
            break;
          }
        }
        this->GetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side2, layer);
        leafData.Steps = pos;
        this->SetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side2, layer);
        if (table)
        {
          switch (layer)
          {
          case vtkMRMLIhepMlcControlNode::Layer1:
            table->SetValue(i, 2, IHEP_TOTAL_DISTANCE - leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "2" for layer-1
            break;
          case vtkMRMLIhepMlcControlNode::Layer2:
            table->SetValue(i, 5, IHEP_TOTAL_DISTANCE - leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "2" for layer-2
            break;
          default:
            break;
          }
        }
      }
    }
    break;
  case vtkMRMLIhepMlcControlNode::Square:
    {
      for (int i = 0; i < this->NumberOfLeafPairs; ++i)
      {
        int pos = 19300;
        if (i > 3 && i < 12)
        {
          pos = (vtkMRMLIhepMlcControlNode::IHEP_MOTOR_STEPS_PER_MM) * 12 * this->PairOfLeavesSize;
        }

        this->GetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side1, layer);
        leafData.Steps = pos;
        this->SetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side1, layer);
        if (table)
        {
          switch (layer)
          {
          case vtkMRMLIhepMlcControlNode::Layer1:
            table->SetValue(i, 1, -IHEP_TOTAL_DISTANCE + leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "1" for layer-1
            break;
          case vtkMRMLIhepMlcControlNode::Layer2:
            table->SetValue(i, 4, -IHEP_TOTAL_DISTANCE + leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "1" for layer-2
            break;
          default:
            break;
          }
        }
        this->GetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side2, layer);
        leafData.Steps = pos;
        this->SetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side2, layer);
        if (table)
        {
          switch (layer)
          {
          case vtkMRMLIhepMlcControlNode::Layer1:
            table->SetValue(i, 2, IHEP_TOTAL_DISTANCE - leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "2" for layer-1
            break;
          case vtkMRMLIhepMlcControlNode::Layer2:
            table->SetValue(i, 5, IHEP_TOTAL_DISTANCE - leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "2" for layer-2
            break;
          default:
            break;
          }
        }
      }
    }
    break;
  case vtkMRMLIhepMlcControlNode::Circle:
    {
      constexpr double radius = 40.; // mm
      double centerOffset = this->PairOfLeavesSize * this->NumberOfLeafPairs / 2.;
      for (int i = 0; i < this->NumberOfLeafPairs; ++i)
      {
        double centerPos = this->PairOfLeavesSize * (i + 0.5) - centerOffset;
        int pos = 19300;
        if (std::fabs(centerPos) < radius)
        {
          pos = 19300 - static_cast<int>(sqrt(radius * radius - centerPos * centerPos) * vtkMRMLIhepMlcControlNode::IHEP_MOTOR_STEPS_PER_MM);
        }
        
        this->GetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side1, layer);
        leafData.Steps = pos;
        this->SetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side1, layer);
        if (table)
        {
          switch (layer)
          {
          case vtkMRMLIhepMlcControlNode::Layer1:
            table->SetValue(i, 1, -IHEP_TOTAL_DISTANCE + leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "1" for layer-1
            break;
          case vtkMRMLIhepMlcControlNode::Layer2:
            table->SetValue(i, 4, -IHEP_TOTAL_DISTANCE + leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "1" for layer-2
            break;
          default:
            break;
          }
        }

        this->GetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side2, layer);
        leafData.Steps = pos;
        this->SetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side2, layer);
        if (table)
        {
          switch (layer)
          {
          case vtkMRMLIhepMlcControlNode::Layer1:
            table->SetValue(i, 2, IHEP_TOTAL_DISTANCE - leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "2" for layer-1
            break;
          case vtkMRMLIhepMlcControlNode::Layer2:
            table->SetValue(i, 5, IHEP_TOTAL_DISTANCE - leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "2" for layer-2
            break;
          default:
            break;
          }
        }
      }
    }
    break;
  case vtkMRMLIhepMlcControlNode::Open:
    {
      for (int i = 0; i < this->NumberOfLeafPairs; ++i)
      {
        this->GetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side1, layer);
        leafData.Steps = 100;
        this->SetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side1, layer);
        if (table)
        {
          switch (layer)
          {
          case vtkMRMLIhepMlcControlNode::Layer1:
            table->SetValue(i, 1, -IHEP_TOTAL_DISTANCE + leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "1" for layer-1
            break;
          case vtkMRMLIhepMlcControlNode::Layer2:
            table->SetValue(i, 4, -IHEP_TOTAL_DISTANCE + leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "1" for layer-2
            break;
          default:
            break;
          }
        }
        this->GetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side2, layer);
        leafData.Steps = 100;
        this->SetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side2, layer);
        if (table)
        {
          switch (layer)
          {
          case vtkMRMLIhepMlcControlNode::Layer1:
            table->SetValue(i, 2, IHEP_TOTAL_DISTANCE - leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "2" for layer-1
            break;
          case vtkMRMLIhepMlcControlNode::Layer2:
            table->SetValue(i, 5, IHEP_TOTAL_DISTANCE - leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "2" for layer-2
            break;
          default:
            break;
          }
        }
      }
    }
    break;
  case vtkMRMLIhepMlcControlNode::Close:
    {
      for (int i = 0; i < this->NumberOfLeafPairs; ++i)
      {
        this->GetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side1, layer);
        leafData.Steps = 19300;
        this->SetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side1, layer);
        if (table)
        {
          switch (layer)
          {
          case vtkMRMLIhepMlcControlNode::Layer1:
            table->SetValue(i, 1, -IHEP_TOTAL_DISTANCE + leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "1" for layer-1
            break;
          case vtkMRMLIhepMlcControlNode::Layer2:
            table->SetValue(i, 4, -IHEP_TOTAL_DISTANCE + leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "1" for layer-2
            break;
          default:
            break;
          }
        }
        this->GetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side2, layer);
        leafData.Steps = 19300;
        this->SetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side2, layer);
        if (table)
        {
          switch (layer)
          {
          case vtkMRMLIhepMlcControlNode::Layer1:
            table->SetValue(i, 2, IHEP_TOTAL_DISTANCE - leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "2" for layer-1
            break;
          case vtkMRMLIhepMlcControlNode::Layer2:
            table->SetValue(i, 5, IHEP_TOTAL_DISTANCE - leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "2" for layer-2
            break;
          default:
            break;
          }
        }
      }
    }
    break;
  default:
    break;
  }
  mlcTableNode->Modified();
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
