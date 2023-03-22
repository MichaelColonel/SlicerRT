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
#include <climits>
#include <bitset>

//------------------------------------------------------------------------------
namespace
{

const char* BEAM_REFERENCE_ROLE = "beamRef";

unsigned short updateCrc16(unsigned short crc, unsigned char a)
{
  crc ^= a;
  for (int i = 0; i < CHAR_BIT; ++i)
  {
    if (crc & 1)
    {
      crc = (crc >> 1) ^ 0xA001;
    }
    else
    {
      crc = (crc >> 1);
    }
  }

  return crc;
}

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
int vtkMRMLIhepMlcControlNode::GetLeafPositionLayerByAddress(int address, int& key,
  SideType& side, LayerType& layer)
{
//  vtkMRMLIhepMlcControlNode::SideType side_ = Side_Last;
//  vtkMRMLIhepMlcControlNode::LayerType layer_ = Layer_Last;
  for (auto iter = LeavesDataMap.begin(); iter != LeavesDataMap.end(); ++iter)
  {
    int leavesPairKey = (*iter).first;
    const PairOfLeavesData& leavesPair = (*iter).second;
    const vtkMRMLIhepMlcControlNode::LeafData& leafSide1 = leavesPair.first;
    const vtkMRMLIhepMlcControlNode::LeafData& leafSide2 = leavesPair.second;
    if (leafSide1.Address == address)
    {
//      side_ = leafSide1.Side;
      side = leafSide1.Side;
//      layer_ = leafSide1.Layer;
      layer = leafSide1.Layer;
    }
    else if (leafSide2.Address == address)
    {
//      side_ = leafSide2.Side;
      side = leafSide2.Side;
//      layer_ = leafSide2.Layer;
      layer = leafSide2.Layer;
    }
//    if (layer_ != vtkMRMLIhepMlcControlNode::Layer_Last)
    if (layer != vtkMRMLIhepMlcControlNode::Layer_Last)
    {
      key = leavesPairKey;
//      vtkWarningMacro("Address: " << address << " side: " << side_ << " layer: " << layer_);
//      side = side_;
//      layer = layer_;
      return leavesPairKey - IHEP_PAIR_OF_LEAVES_PER_LAYER * static_cast<int>(layer);
    }
  }
  key = -1;
  return -1;
}

//----------------------------------------------------------------------------
bool vtkMRMLIhepMlcControlNode::GetLeafDataByAddress(LeafData& leafData, int address)
{
  int key;
  int pos = -1;
  vtkMRMLIhepMlcControlNode::SideType side = Side_Last;
  vtkMRMLIhepMlcControlNode::LayerType layer = Layer_Last;
  if ((pos = this->GetLeafPositionLayerByAddress(address, key, side, layer)) != -1)
  {
    return this->GetLeafData(leafData, pos, side, layer);
  }
  return false;
}

//----------------------------------------------------------------------------
bool vtkMRMLIhepMlcControlNode::SetLeafDataByAddress(const LeafData& leafData, int address)
{
  int key;
  int pos = -1;
  vtkMRMLIhepMlcControlNode::SideType side = Side_Last;
  vtkMRMLIhepMlcControlNode::LayerType layer = Layer_Last;
  if ((pos = this->GetLeafPositionLayerByAddress(address, key, side, layer)) != -1)
  {
    return this->SetLeafData(leafData, pos, side, layer);
  }
  return false;
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

//----------------------------------------------------------------------------
double vtkMRMLIhepMlcControlNode::ExternalCounterValueToDistance(int extCounterValue)
{
  return (extCounterValue / vtkMRMLIhepMlcControlNode::IHEP_EXTERNAL_COUNTS_PER_MM);
}

//----------------------------------------------------------------------------
double vtkMRMLIhepMlcControlNode::InternalCounterValueToDistance(int intCounterValue)
{
  return (intCounterValue / vtkMRMLIhepMlcControlNode::IHEP_MOTOR_STEPS_PER_MM);
}

//----------------------------------------------------------------------------
int vtkMRMLIhepMlcControlNode::DistanceToExternalCounterValue(double distance)
{
  return (distance * vtkMRMLIhepMlcControlNode::IHEP_EXTERNAL_COUNTS_PER_MM);
}

//----------------------------------------------------------------------------
int vtkMRMLIhepMlcControlNode::DistanceToInternalCounterValue(double distance)
{
  return (distance * vtkMRMLIhepMlcControlNode::IHEP_MOTOR_STEPS_PER_MM);
}

//-----------------------------------------------------------------------------
unsigned short vtkMRMLIhepMlcControlNode::CommandCalculateCrc16(const vtkMRMLIhepMlcControlNode::CommandBufferType& buf)
{
  unsigned short crc = 0xFFFF;
  for (size_t i = 0; i < vtkMRMLIhepMlcControlNode::COMMAND_DATA; ++i)
  {
    crc = updateCrc16(crc, buf[i]);
  }

  return crc;
}

//-----------------------------------------------------------------------------
bool vtkMRMLIhepMlcControlNode::CommandCheckCrc16(const vtkMRMLIhepMlcControlNode::CommandBufferType& buf)
{
  unsigned short dataCRC16 = CommandCalculateCrc16(buf); // calculated CRC16 check sum for buffer data
  const size_t& lsb = vtkMRMLIhepMlcControlNode::COMMAND_CRC16_LSB;
  const size_t& msb = vtkMRMLIhepMlcControlNode::COMMAND_CRC16_MSB;
  unsigned short comCRC16 = (buf[msb] << CHAR_BIT) | buf[lsb]; // check sum from buffer
  return (dataCRC16 == comCRC16);
}

//-----------------------------------------------------------------------------
void vtkMRMLIhepMlcControlNode::ProcessCommandBufferToLeafData(const vtkMRMLIhepMlcControlNode::CommandBufferType& buf,
  vtkMRMLIhepMlcControlNode::LeafData& leafData)
{
  leafData.Address = buf[0];
  leafData.State = buf[1];

  std::bitset<CHAR_BIT> stateBits(buf[2]);
  leafData.EncoderDirection = !stateBits.test(0); // external encoder direction (to switch == true, from switch == false)
  leafData.SwitchState = !stateBits.test(1); // external switch state
  leafData.Reset = !stateBits.test(2); // internal motor reset
  leafData.Mode = stateBits.test(3); // internal motor steps mode
  leafData.Direction = stateBits.test(4); // internal motor direction
  leafData.Enabled = !stateBits.test(5); // internal motor enabled
  
  leafData.StepsLeft = (buf[3] << CHAR_BIT) | buf[4];
  leafData.EncoderCounts = ((buf[5] << CHAR_BIT) | buf[6]) + (USHRT_MAX + 1) * buf[7];
  leafData.Frequency = buf[8];
}

//-----------------------------------------------------------------------------
void vtkMRMLIhepMlcControlNode::GetAddressesByLayerSide(std::vector<int>& addresses,
  vtkMRMLIhepMlcControlNode::SideType side, vtkMRMLIhepMlcControlNode::LayerType layer)
{
  addresses.clear();
  for (auto iter = LeavesDataMap.begin(); iter != LeavesDataMap.end(); ++iter)
  {
    int leavesPairKey = (*iter).first;
    const PairOfLeavesData& leavesPair = (*iter).second;
    const vtkMRMLIhepMlcControlNode::LeafData& leafSide1 = leavesPair.first;
    const vtkMRMLIhepMlcControlNode::LeafData& leafSide2 = leavesPair.second;
    if (side == leafSide1.Side && layer == leafSide1.Layer)
    {
      addresses.push_back(leafSide1.Address);
    }
    else if (side == leafSide2.Side && layer == leafSide2.Layer)
    {
      addresses.push_back(leafSide2.Address);
    }
  }
}

//-----------------------------------------------------------------------------
void vtkMRMLIhepMlcControlNode::GetAddressesByLayer(std::vector<int>& addresses,
  vtkMRMLIhepMlcControlNode::LayerType layer)
{
  addresses.clear();
  for (auto iter = LeavesDataMap.begin(); iter != LeavesDataMap.end(); ++iter)
  {
    int leavesPairKey = (*iter).first;
    const PairOfLeavesData& leavesPair = (*iter).second;
    const vtkMRMLIhepMlcControlNode::LeafData& leafSide1 = leavesPair.first;
    const vtkMRMLIhepMlcControlNode::LeafData& leafSide2 = leavesPair.second;
    if (layer == leafSide1.Layer)
    {
      addresses.push_back(leafSide1.Address);
    }
    else if (layer == leafSide2.Layer)
    {
      addresses.push_back(leafSide2.Address);
    }
  }
}

//-----------------------------------------------------------------------------
void vtkMRMLIhepMlcControlNode::GetAddresses(std::vector<int>& addresses)
{
  addresses.clear();
  for (auto iter = LeavesDataMap.begin(); iter != LeavesDataMap.end(); ++iter)
  {
    int leavesPairKey = (*iter).first;
    const PairOfLeavesData& leavesPair = (*iter).second;
    const vtkMRMLIhepMlcControlNode::LeafData& leafSide1 = leavesPair.first;
    const vtkMRMLIhepMlcControlNode::LeafData& leafSide2 = leavesPair.second;
    addresses.push_back(leafSide1.Address);
    addresses.push_back(leafSide2.Address);
  }
}

//-----------------------------------------------------------------------------
int vtkMRMLIhepMlcControlNode::GetRelativeMovementByAddress(int address)
{
  vtkMRMLIhepMlcControlNode::LeafData leafData;
  if (this->GetLeafDataByAddress(leafData, address))
  {
    return leafData.RequiredPosition - leafData.CurrentPosition;
  }
  return 0;
}
