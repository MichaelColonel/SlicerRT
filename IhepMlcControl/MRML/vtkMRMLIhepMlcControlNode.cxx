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
  vtkMRMLWriteXMLBooleanMacro(debugMode, DebugMode);
  vtkMRMLWriteXMLEnumMacro(orientation, Orientation);
  vtkMRMLWriteXMLEnumMacro(layers, Layers);
  vtkMRMLWriteXMLIntMacro(numberOfLeafPairs, NumberOfLeafPairs);
  vtkMRMLWriteXMLFloatMacro(pairOfLeavesSize, PairOfLeavesSize);
  vtkMRMLWriteXMLFloatMacro(isocenterOffset, IsocenterOffset);
  vtkMRMLWriteXMLFloatMacro(distanceBetweenTwoLayers, DistanceBetweenTwoLayers);
  vtkMRMLWriteXMLFloatMacro(offsetBetweenTwoLayers, OffsetBetweenTwoLayers);
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
  vtkMRMLReadXMLBooleanMacro(debugMode, DebugMode);
  vtkMRMLReadXMLEnumMacro(orientation, Orientation);
  vtkMRMLReadXMLEnumMacro(layers, Layers);
  vtkMRMLReadXMLIntMacro(numberOfLeafPairs, NumberOfLeafPairs);
  vtkMRMLReadXMLFloatMacro(pairOfLeavesSize, PairOfLeavesSize);
  vtkMRMLReadXMLFloatMacro(isocenterOffset, IsocenterOffset);
  vtkMRMLReadXMLFloatMacro(distanceBetweenTwoLayers, DistanceBetweenTwoLayers);
  vtkMRMLReadXMLFloatMacro(offsetBetweenTwoLayers, OffsetBetweenTwoLayers);

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
  vtkMRMLCopyBooleanMacro(DebugMode);
  vtkMRMLCopyEnumMacro(Orientation);
  vtkMRMLCopyEnumMacro(Layers);
  vtkMRMLCopyIntMacro(NumberOfLeafPairs);
  vtkMRMLCopyFloatMacro(PairOfLeavesSize);
  vtkMRMLCopyFloatMacro(IsocenterOffset);
  vtkMRMLCopyFloatMacro(DistanceBetweenTwoLayers);
  vtkMRMLCopyFloatMacro(OffsetBetweenTwoLayers);
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
  vtkMRMLCopyBooleanMacro(DebugMode);
  vtkMRMLCopyBooleanMacro(ParallelBeam);
  vtkMRMLCopyEnumMacro(Orientation);
  vtkMRMLCopyEnumMacro(Layers);
  vtkMRMLCopyIntMacro(NumberOfLeafPairs);
  vtkMRMLCopyFloatMacro(PairOfLeavesSize);
  vtkMRMLCopyFloatMacro(IsocenterOffset);
  vtkMRMLCopyFloatMacro(DistanceBetweenTwoLayers);
  vtkMRMLCopyFloatMacro(OffsetBetweenTwoLayers);
  this->LeavesDataMap = node->GetPairOfLeavesMap();

  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLIhepMlcControlNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);
  // add new parameters here
  vtkMRMLPrintBooleanMacro(ParallelBeam);
  vtkMRMLPrintBooleanMacro(DebugMode);
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
  else
  {
    vtkWarningMacro("SetPredefinedPosition: MLC table is invalid");
    return;
  }

  double axisWidth = this->PairOfLeavesSize * this->NumberOfLeafPairs;
  double tanAngle = IHEP_SIDE_OPENING / axisWidth;
  vtkMRMLIhepMlcControlNode::LeafData leafData;
  switch (predef)
  {
  case vtkMRMLIhepMlcControlNode::Side1Edge:
    {
      axisWidth = this->PairOfLeavesSize * this->NumberOfLeafPairs * 0.8;
      tanAngle = IHEP_SIDE_OPENING / axisWidth;
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
            table->SetValue(i, 1, -1. * IHEP_TOTAL_DISTANCE + leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "1" for layer-1
            break;
          case vtkMRMLIhepMlcControlNode::Layer2:
            table->SetValue(i, 4, -1. * IHEP_TOTAL_DISTANCE + leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "1" for layer-2
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
      axisWidth = this->PairOfLeavesSize * this->NumberOfLeafPairs * 0.8;
      tanAngle = IHEP_SIDE_OPENING / axisWidth;
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
int vtkMRMLIhepMlcControlNode::GetLeafOffsetLayerByAddress(int address, int& key,
  SideType& side, LayerType& layer)
{
  for (auto iter = LeavesDataMap.begin(); iter != LeavesDataMap.end(); ++iter)
  {
    int leavesPairKey = (*iter).first;
    const PairOfLeavesData& leavesPair = (*iter).second;
    const vtkMRMLIhepMlcControlNode::LeafData& leafSide1 = leavesPair.first;
    const vtkMRMLIhepMlcControlNode::LeafData& leafSide2 = leavesPair.second;
    if (leafSide1.Address == address)
    {
      side = vtkMRMLIhepMlcControlNode::Side1;//leafSide1.Side;
      layer = leafSide1.Layer;
      key = leavesPairKey;
      return leavesPairKey - IHEP_PAIR_OF_LEAVES_PER_LAYER * static_cast<int>(leafSide1.Layer);
    }
    if (leafSide2.Address == address)
    {
      side = vtkMRMLIhepMlcControlNode::Side2;//leafSide2.Side;
      layer = leafSide2.Layer;
      key = leavesPairKey;
      return leavesPairKey - IHEP_PAIR_OF_LEAVES_PER_LAYER * static_cast<int>(leafSide2.Layer);
    }
  }
  layer = vtkMRMLIhepMlcControlNode::Layer_Last;
  side = vtkMRMLIhepMlcControlNode::Side_Last;
  key = -1;
  return -1;
}

//----------------------------------------------------------------------------
int vtkMRMLIhepMlcControlNode::GetLeafOffsetByAddressInLayer(int address, int& key,
  SideType& side, LayerType layer)
{
  for (auto iter = LeavesDataMap.begin(); iter != LeavesDataMap.end(); ++iter)
  {
    int leavesPairKey = (*iter).first;
    const PairOfLeavesData& leavesPair = (*iter).second;
    const vtkMRMLIhepMlcControlNode::LeafData& leafSide1 = leavesPair.first;
    const vtkMRMLIhepMlcControlNode::LeafData& leafSide2 = leavesPair.second;
    if (leafSide1.Address == address && leafSide1.Layer == layer)
    {
      side = vtkMRMLIhepMlcControlNode::Side1;
      key = leavesPairKey;
      return leavesPairKey - IHEP_PAIR_OF_LEAVES_PER_LAYER * static_cast<int>(leafSide1.Layer);
    }
    if (leafSide2.Address == address && leafSide2.Layer == layer)
    {
      side = vtkMRMLIhepMlcControlNode::Side2;
      key = leavesPairKey;
      return leavesPairKey - IHEP_PAIR_OF_LEAVES_PER_LAYER * static_cast<int>(leafSide2.Layer);
    }
  }
  side = vtkMRMLIhepMlcControlNode::Side_Last;
  key = -1;
  return -1;
}

//----------------------------------------------------------------------------
bool vtkMRMLIhepMlcControlNode::GetLeafDataByAddress(LeafData& leafData, int address)
{
  int key;
  int offset = -1;
  vtkMRMLIhepMlcControlNode::SideType side = Side_Last;
  vtkMRMLIhepMlcControlNode::LayerType layer = Layer_Last;
  if ((offset = this->GetLeafOffsetLayerByAddress(address, key, side, layer)) != -1)
  {
    return this->GetLeafData(leafData, offset, side, layer);
  }
  return false;
}

//----------------------------------------------------------------------------
bool vtkMRMLIhepMlcControlNode::GetLeafDataByAddressInLayer(LeafData& leafData, int address, vtkMRMLIhepMlcControlNode::LayerType layer)
{
  int key;
  int offset = -1;
  vtkMRMLIhepMlcControlNode::SideType side = Side_Last;
  if ((offset = this->GetLeafOffsetByAddressInLayer(address, key, side, layer)) != -1)
  {
    return this->GetLeafData(leafData, offset, side, layer);
  }
  return false;
}

//----------------------------------------------------------------------------
bool vtkMRMLIhepMlcControlNode::SetLeafDataByAddress(const LeafData& leafData, int address)
{
  int key;
  int offset = -1;
  vtkMRMLIhepMlcControlNode::SideType side = Side_Last;
  vtkMRMLIhepMlcControlNode::LayerType layer = Layer_Last;
  if ((offset = this->GetLeafOffsetLayerByAddress(address, key, side, layer)) != -1)
  {
    return this->SetLeafData(leafData, offset, side, layer);
  }
  return false;
}

//----------------------------------------------------------------------------
bool vtkMRMLIhepMlcControlNode::SetLeafDataState(const LeafData& leafData)
{
  int key;
  int offset = -1;
  vtkMRMLIhepMlcControlNode::SideType side = Side_Last;
  vtkMRMLIhepMlcControlNode::LayerType layer = Layer_Last;
  vtkMRMLIhepMlcControlNode::LeafData currentLeafData;
  if (!this->GetLeafDataByAddress(currentLeafData, leafData.Address))
  {
    return false;
  }
  currentLeafData.StateEnabled = leafData.StateEnabled;
  currentLeafData.StateReset = leafData.StateReset;
  currentLeafData.StateDirection = leafData.StateDirection;
  currentLeafData.StateStepMode = leafData.StateStepMode;
  currentLeafData.SwitchState = leafData.SwitchState;
  currentLeafData.StepsLeft = leafData.StepsLeft;
  currentLeafData.EncoderCounts = leafData.EncoderCounts;
  currentLeafData.CurrentPosition = leafData.CurrentPosition;
//  currentLeafData.RequiredPosition = currentLeafData.Steps;
  if ((offset = this->GetLeafOffsetLayerByAddress(leafData.Address, key, side, layer)) != -1)
  {
    return this->SetLeafData(currentLeafData, offset, side, layer);
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
//  if (leafData.SwitchState)
//  {
//    leafData.CurrentPosition = 0;
//  }
  leafData.StateReset = !stateBits.test(2); // internal motor reset
  leafData.StateStepMode = stateBits.test(3); // internal motor steps mode
  leafData.StateDirection = stateBits.test(4); // internal motor direction
  leafData.StateEnabled = stateBits.test(5); // internal motor enabled
  
  leafData.StepsLeft = (buf[3] << CHAR_BIT) | buf[4];
  leafData.EncoderCounts = ((buf[5] << CHAR_BIT) | buf[6]);
  leafData.CurrentPosition = ((buf[7] << CHAR_BIT) | buf[8]);

  std::cout << "ProcessCommandBufferToLeafData: State Reset: " << leafData.StateReset
   << ", StateStepMode: " << leafData.StateStepMode << ", StateDirection: " << leafData.StateDirection
   << ", StateEnabled: " << leafData.StateEnabled << ", StepsLeft: " << leafData.StepsLeft
   << ", EncoderCounts: " << leafData.EncoderCounts << ", current position: " << leafData.CurrentPosition << std::endl;

//  leafData.EncoderCounts = static_cast<int32_t>((buf[5] << CHAR_BIT) | buf[6]);
//  leafData.Frequency = buf[8]; // do not use Frequency, the value is dummy
}

//-----------------------------------------------------------------------------
void vtkMRMLIhepMlcControlNode::GetAddressesByLayerSide(std::vector<int>& addresses,
  vtkMRMLIhepMlcControlNode::SideType side, vtkMRMLIhepMlcControlNode::LayerType layer)
{
  addresses.clear();
  for (auto iter = LeavesDataMap.begin(); iter != LeavesDataMap.end(); ++iter)
  {
//    int leavesPairKey = (*iter).first;
    const PairOfLeavesData& leavesPair = (*iter).second;
    const vtkMRMLIhepMlcControlNode::LeafData& leafSide1 = leavesPair.first;
    const vtkMRMLIhepMlcControlNode::LeafData& leafSide2 = leavesPair.second;
    if (side == leafSide1.Side && layer == leafSide1.Layer)
    {
      addresses.push_back(leafSide1.Address);
    }
    if (side == leafSide2.Side && layer == leafSide2.Layer)
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
//    int leavesPairKey = (*iter).first;
    const PairOfLeavesData& leavesPair = (*iter).second;
    const vtkMRMLIhepMlcControlNode::LeafData& leafSide1 = leavesPair.first;
    const vtkMRMLIhepMlcControlNode::LeafData& leafSide2 = leavesPair.second;
    if (layer == leafSide1.Layer)
    {
      addresses.push_back(leafSide1.Address);
    }
    if (layer == leafSide2.Layer)
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
//    int leavesPairKey = (*iter).first;
    const PairOfLeavesData& leavesPair = (*iter).second;
    const vtkMRMLIhepMlcControlNode::LeafData& leafSide1 = leavesPair.first;
    const vtkMRMLIhepMlcControlNode::LeafData& leafSide2 = leavesPair.second;
    addresses.push_back(leafSide1.Address);
    addresses.push_back(leafSide2.Address);
  }
}

//-----------------------------------------------------------------------------
/*
int vtkMRMLIhepMlcControlNode::GetCurrentPositionByAddress(int address)
{
  vtkMRMLIhepMlcControlNode::SideType side = vtkMRMLIhepMlcControlNode::Side_Last;
  vtkMRMLIhepMlcControlNode::LayerType layer = vtkMRMLIhepMlcControlNode::Layer_Last;
  int key = -1;
  int res = this->GetLeafOffsetLayerByAddress(address, key, side, layer);

  vtkMRMLIhepMlcControlNode::LeafData data;
  if (this->GetLeafDataByAddress(data, address) && res != -1)
  {
    int currentPosition = -1;

    if (data.SwitchState)
    {
      currentPosition = 0;
    }
    else
    {
      if (side == vtkMRMLIhepMlcControlNode::Side1)
      {
        if (data.CurrentPosition > 0)
        {
          currentPosition = data.CurrentPosition;
        }
        else
        {
          currentPosition = 0;
        }
        if (data.isMovingToTheSwitch())
        {
          currentPosition = data.CurrentPosition - 2 * data.EncoderCounts;
        }
        else if (data.isMovingFromTheSwitch())
        {
          currentPosition = data.CurrentPosition + 2 * data.EncoderCounts;
        }
        else if (data.isStopped())
        {
          currentPosition = data.CurrentPosition;
        }
      }
      else if (side == vtkMRMLIhepMlcControlNode::Side2 && side == data.Side)
      {
        if (data.CurrentPosition > 0)
        {
          currentPosition = data.CurrentPosition;
        }
        else
        {
          currentPosition = 0;
        }
        
        if (data.isMovingToTheSwitch())
        {
          currentPosition = data.CurrentPosition - 2 * data.EncoderCounts;
        }
        else if (data.isMovingFromTheSwitch())
        {
          currentPosition = data.CurrentPosition + 2 * data.EncoderCounts;
        }
        else if (data.isStopped())
        {
          currentPosition = data.CurrentPosition;
        }
      }
    }
    return currentPosition;
  }
  return -1;
}
*/

//-----------------------------------------------------------------------------
int vtkMRMLIhepMlcControlNode::GetRelativeMovementByAddress(int address)
{
  vtkMRMLIhepMlcControlNode::SideType side = vtkMRMLIhepMlcControlNode::Side_Last;
  vtkMRMLIhepMlcControlNode::LayerType layer = vtkMRMLIhepMlcControlNode::Layer_Last;
  int key = -1;
  int res = this->GetLeafOffsetLayerByAddress(address, key, side, layer);

  vtkMRMLIhepMlcControlNode::LeafData data;
  if (this->GetLeafDataByAddress(data, address) && res != -1)
  {
    return data.GetRelativeMovement();
/*
    int currentPosition = 0;

    if (data.SwitchState)
    {
      currentPosition = 0;
    }
    else
    {
      if (side == vtkMRMLIhepMlcControlNode::Side1 && side == data.Side)
      {
        if (data.CurrentPosition > 0)
        {
          currentPosition = data.CurrentPosition;
        }
        else
        {
          currentPosition = 0;
        }
        if (data.isMovingToTheSwitch())
        {
          currentPosition = data.CurrentPosition - 2 * data.EncoderCounts;
        }
        else if (data.isMovingFromTheSwitch())
        {
          currentPosition = data.CurrentPosition + 2 * data.EncoderCounts;
        }
        else if (data.isStopped())
        {
          currentPosition = data.CurrentPosition;
        }
      }
      else if (side == vtkMRMLIhepMlcControlNode::Side2 && side == data.Side)
      {
        if (data.CurrentPosition > 0)
        {
          currentPosition = data.CurrentPosition;
        }
        else
        {
          currentPosition = 0;
        }
        
        if (data.isMovingToTheSwitch())
        {
          currentPosition = data.CurrentPosition - 2 * data.EncoderCounts;
        }
        else if (data.isMovingFromTheSwitch())
        {
          currentPosition = data.CurrentPosition + 2 * data.EncoderCounts;
        }
        else if (data.isStopped())
        {
          currentPosition = data.CurrentPosition;
        }
      }
    }
//    return leafData.RequiredPosition - leafData.CurrentPosition;
    return data.RequiredPosition - currentPosition;
*/
  }
  return 0;
}

//-----------------------------------------------------------------------------
int vtkMRMLIhepMlcControlNode::GetStepsFromMlcTableByAddress(int address)
{
  vtkMRMLTableNode* mlcTableNode = nullptr;
  if (this->GetBeamNode())
  {
    mlcTableNode = this->GetBeamNode()->GetMultiLeafCollimatorTableNode();
  }

  return this->GetStepsFromMlcTableByAddress(mlcTableNode, address);
}

//-----------------------------------------------------------------------------
int vtkMRMLIhepMlcControlNode::GetStepsFromMlcTableByAddress(vtkMRMLTableNode* mlcTableNode, int address)
{
  vtkTable* table = nullptr;
  if (mlcTableNode && mlcTableNode->GetTable())
  {
    table = mlcTableNode->GetTable();
  }
  else
  {
    vtkWarningMacro("GetStepsFromMlcTableByAddress: MLC table is invalid");
    return 0;
  }

  int key = -1;
  SideType side = Side_Last;
  LayerType layer = Layer_Last;
  int offset = this->GetLeafOffsetLayerByAddress( address, key, side, layer);
  double movement = 0.;
  if (offset != -1)
  {
    if (side == vtkMRMLIhepMlcControlNode::Side1 && layer == vtkMRMLIhepMlcControlNode::Layer1)
    {
      movement = IHEP_SIDE_OPENING + table->GetValue(offset, 1).ToDouble();
    }
    else if (side == vtkMRMLIhepMlcControlNode::Side2 && layer == vtkMRMLIhepMlcControlNode::Layer1)
    {
      movement = IHEP_SIDE_OPENING - table->GetValue(offset, 2).ToDouble();
    }
    else if (side == vtkMRMLIhepMlcControlNode::Side1 && layer == vtkMRMLIhepMlcControlNode::Layer2)
    {
      movement = IHEP_SIDE_OPENING + table->GetValue(offset, 4).ToDouble();
    }
    else if (side == vtkMRMLIhepMlcControlNode::Side2 && layer == vtkMRMLIhepMlcControlNode::Layer2)
    {
      movement = IHEP_SIDE_OPENING - table->GetValue(offset, 5).ToDouble();
    }
    else
    {
      movement = 0.;
    }
  }
  return static_cast<int>(DistanceToInternalCounterValue(movement));
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

//----------------------------------------------------------------------------
double vtkMRMLIhepMlcControlNode::ExternalCounterValueToMlcPosition(int extCounterValue, vtkMRMLIhepMlcControlNode::SideType side)
{
  double res = 0.;
  switch (side)
  {
  case vtkMRMLIhepMlcControlNode::Side1:
    res = vtkMRMLIhepMlcControlNode::ExternalCounterValueToDistance(extCounterValue) - vtkMRMLIhepMlcControlNode::IHEP_TOTAL_DISTANCE;
    break;
  case vtkMRMLIhepMlcControlNode::Side2:
    res = vtkMRMLIhepMlcControlNode::IHEP_TOTAL_DISTANCE - vtkMRMLIhepMlcControlNode::ExternalCounterValueToDistance(extCounterValue);
    break;
  default:
    break;
  }
  return res;
}

//----------------------------------------------------------------------------
double vtkMRMLIhepMlcControlNode::InternalCounterValueToMlcPosition(int intCounterValue, vtkMRMLIhepMlcControlNode::SideType side)
{
  double res = 0.;
  switch (side)
  {
  case vtkMRMLIhepMlcControlNode::Side1:
    res = vtkMRMLIhepMlcControlNode::InternalCounterValueToDistance(intCounterValue) - vtkMRMLIhepMlcControlNode::IHEP_TOTAL_DISTANCE;
    break;
  case vtkMRMLIhepMlcControlNode::Side2:
    res = vtkMRMLIhepMlcControlNode::IHEP_TOTAL_DISTANCE - vtkMRMLIhepMlcControlNode::InternalCounterValueToDistance(intCounterValue);
    break;
  default:
    break;
  }
  return res;
}

//---------------------------------------------------------------------------
void vtkMRMLIhepMlcControlNode::SetOrientation(int id)
{
  switch (id)
  {
  case 0:
    this->SetOrientation(vtkMRMLIhepMlcControlNode::X);
    break;
  case 1:
    this->SetOrientation(vtkMRMLIhepMlcControlNode::Y);
    break;
  default:
    this->SetOrientation(vtkMRMLIhepMlcControlNode::Orientation_Last);
    break;
  }
}

//---------------------------------------------------------------------------
const char* vtkMRMLIhepMlcControlNode::GetOrientationAsString(int id)
{
  switch (id)
  {
  case vtkMRMLIhepMlcControlNode::X:
    return "X";
  case vtkMRMLIhepMlcControlNode::Y:
    return "Y";
  default:
    return "Orientation_Last";
  }
}

//---------------------------------------------------------------------------
int vtkMRMLIhepMlcControlNode::GetOrientationFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int i = 0; i < vtkMRMLIhepMlcControlNode::Orientation_Last; i++)
    {
    if (std::strcmp(name, vtkMRMLIhepMlcControlNode::GetOrientationAsString(i)) == 0)
      {
      // found a matching name
      return i;
      }
    }
  // unknown name
  return -1;
}

//---------------------------------------------------------------------------
void vtkMRMLIhepMlcControlNode::SetLayers(int id)
{
  switch (id)
  {
  case 0:
    this->SetOrientation(vtkMRMLIhepMlcControlNode::OneLayer);
    break;
  case 1:
    this->SetOrientation(vtkMRMLIhepMlcControlNode::TwoLayers);
    break;
  default:
    this->SetOrientation(vtkMRMLIhepMlcControlNode::Layers_Last);
    break;
  }
}

//---------------------------------------------------------------------------
const char* vtkMRMLIhepMlcControlNode::GetLayersAsString(int id)
{
  switch (id)
  {
  case vtkMRMLIhepMlcControlNode::OneLayer:
    return "OneLayer";
  case vtkMRMLIhepMlcControlNode::TwoLayers:
    return "TwoLayers";
  default:
    return "Layers_Last";
  }
}

//---------------------------------------------------------------------------
int vtkMRMLIhepMlcControlNode::GetLayersFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int i = 0; i < vtkMRMLIhepMlcControlNode::Layers_Last; i++)
    {
    if (std::strcmp(name, vtkMRMLIhepMlcControlNode::GetLayersAsString(i)) == 0)
      {
      // found a matching name
      return i;
      }
    }
  // unknown name
  return -1;
}

//-----------------------------------------------------------------------------
int vtkMRMLIhepMlcControlNode::LeafData::GetActualCurrentPosition() const
{
  std::cout << "GetActualCurrentPosition: Current position: " << this->CurrentPosition
    << ", Required position: " << this->RequiredPosition << ", EncoderCounts: " << this->EncoderCounts << " ";
  if (this->Side != vtkMRMLIhepMlcControlNode::Side_Last && this->Layer != vtkMRMLIhepMlcControlNode::Layer_Last)
  {
    if (this->SwitchState)
    {
      std::cout << "GetActualCurrentPosition: Switch is pressed, ";
      return 0;
    }
    else
    {
      int currentPosition = -1;
      if (this->Side == vtkMRMLIhepMlcControlNode::Side1)
      {
        std::cout << "Side1, ";
        if (this->CurrentPosition > 0)
        {
          currentPosition = this->CurrentPosition;
        }
        else
        {
          currentPosition = 0;
        }

        if (this->isStopped())
        {
          std::cout << "Stopped." << std::endl;
          return (currentPosition);
        }
        else if (this->isMovingToTheSwitch())
        {
          std::cout << "Moving to switch." << std::endl;
          return (currentPosition - 2 * this->EncoderCounts);
        }
        else if (this->isMovingFromTheSwitch())
        {
          std::cout << "Moving from switch." << std::endl;
          return (currentPosition + 2 * this->EncoderCounts);
        }
        else
        {
          std::cout << "Impossible." << std::endl;
          return -1;
        }
      }
      else if (this->Side == vtkMRMLIhepMlcControlNode::Side2)
      {
        std::cout << "Side2, ";
        if (this->CurrentPosition > 0)
        {
          currentPosition = this->CurrentPosition;
        }
        else
        {
          currentPosition = 0;
        }
        
        if (this->isStopped())
        {
          std::cout << "Stopped." << std::endl;
          return currentPosition;
        }
        else if (this->isMovingToTheSwitch())
        {
          std::cout << "Moving to switch." << std::endl;
          return (currentPosition - 2 * this->EncoderCounts);
        }
        else if (this->isMovingFromTheSwitch())
        {
          std::cout << "Moving from switch." << std::endl;
          return (currentPosition + 2 * this->EncoderCounts);
        }
        else
        {
          std::cout << "Impossible." << std::endl;
          return -1;
        }
      }
    }
  }
  return -1;
}

//-----------------------------------------------------------------------------
int vtkMRMLIhepMlcControlNode::LeafData::GetRelativeMovement() const
{
  int pos = this->GetActualCurrentPosition();
  std::cout << "GetActualCurrentPosition: Actual current position: " << pos << std::endl;
  if (pos != -1)
  {
    return this->RequiredPosition - pos;
  }
  return 0;
}
