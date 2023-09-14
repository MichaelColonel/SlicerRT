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
#include <cmath>

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

  int key = index + this->NumberOfLeafPairs * static_cast<int>(layer);

  int range = this->GetCalibrationRangeInLayer(layer);
  double rangeHalfDistance = InternalCounterValueToDistance(range) / 2.;

  auto it = this->LeavesDataMap.find(key);
  if (it != this->LeavesDataMap.end())
  {
    it->second = pairOfLeaves;
//    this->Modified();
    if (table)
    {
      switch (layer)
      {
      case vtkMRMLIhepMlcControlNode::Layer1:
        table->SetValue(index, 1, -rangeHalfDistance + InternalCounterValueToDistance(pairOfLeaves.first.Steps)); // default meaningful value for side "1" for layer-1
        table->SetValue(index, 2, rangeHalfDistance - InternalCounterValueToDistance(pairOfLeaves.second.Steps)); // default meaningful value for side "2" for layer-1
        table->Modified();
        break;
      case vtkMRMLIhepMlcControlNode::Layer2:
        table->SetValue(index, 4, -rangeHalfDistance + InternalCounterValueToDistance(pairOfLeaves.first.Steps)); // default meaningful value for side "1" for layer-2
        table->SetValue(index, 5, rangeHalfDistance - InternalCounterValueToDistance(pairOfLeaves.second.Steps)); // default meaningful value for side "2" for layer-2
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

  int layerMinStepsRange = this->GetCalibrationRangeInLayer(layer);
  double layerMinStepsHalfRange = layerMinStepsRange / IHEP_MOTOR_STEPS_PER_MM / 2.;
  double axisWidth = this->PairOfLeavesSize * this->NumberOfLeafPairs / 2;
  double tanAngle = layerMinStepsHalfRange / axisWidth;
  vtkMRMLIhepMlcControlNode::LeafData leafData;
  switch (predef)
  {
  case vtkMRMLIhepMlcControlNode::Side1Edge:
    {
//      axisWidth = this->PairOfLeavesSize * this->NumberOfLeafPairs * 0.8;
//      tanAngle = IHEP_SIDE_OPENING / axisWidth;
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
            table->SetValue(i, 1, -1. * layerMinStepsHalfRange + leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "1" for layer-1
            break;
          case vtkMRMLIhepMlcControlNode::Layer2:
            table->SetValue(i, 4, -1. * layerMinStepsHalfRange + leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "1" for layer-2
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
//      axisWidth = this->PairOfLeavesSize * this->NumberOfLeafPairs * 0.8;
//      tanAngle = IHEP_SIDE_OPENING / axisWidth;
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
            table->SetValue(i, 2, layerMinStepsHalfRange - leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "2" for layer-1
            break;
          case vtkMRMLIhepMlcControlNode::Layer2:
            table->SetValue(i, 5, layerMinStepsHalfRange - leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "2" for layer-2
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
        int pos = IHEP_MOTOR_STEPS_PER_MM * (i * this->PairOfLeavesSize / 2.) * tanAngle + 400;
        this->GetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side1, layer);
        leafData.Steps = pos;
        this->SetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side1, layer);
        if (table)
        {
          switch (layer)
          {
          case vtkMRMLIhepMlcControlNode::Layer1:
            table->SetValue(i, 1, -1. * layerMinStepsHalfRange + leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "1" for layer-1
            break;
          case vtkMRMLIhepMlcControlNode::Layer2:
            table->SetValue(i, 4, -1. * layerMinStepsHalfRange + leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "1" for layer-2
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
            table->SetValue(i, 2, layerMinStepsHalfRange - leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "2" for layer-1
            break;
          case vtkMRMLIhepMlcControlNode::Layer2:
            table->SetValue(i, 5, layerMinStepsHalfRange - leafData.Steps / IHEP_MOTOR_STEPS_PER_MM); // default meaningful value for side "2" for layer-2
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
        int pos = layerMinStepsRange / 2;
        if (i > 3 && i < 12)
        {
          pos = (vtkMRMLIhepMlcControlNode::IHEP_MOTOR_STEPS_PER_MM) * 4 * this->PairOfLeavesSize;
        }

        this->GetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side1, layer);
        leafData.Steps = pos;
        this->SetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side1, layer);

        if (table)
        {
          switch (layer)
          {
          case vtkMRMLIhepMlcControlNode::Layer1:
            {
              int range = this->GetCalibrationRangeInLayer(vtkMRMLIhepMlcControlNode::Layer1);
              double rangeHalfDistance = InternalCounterValueToDistance(range) / 2.;
              double value = InternalCounterValueToDistance(leafData.Steps) - rangeHalfDistance;
              table->SetValue(i, 1, value); // default meaningful value for side "1" for layer-1
            }
            break;
          case vtkMRMLIhepMlcControlNode::Layer2:
            {
              int range = this->GetCalibrationRangeInLayer(vtkMRMLIhepMlcControlNode::Layer2);
              double rangeHalfDistance = InternalCounterValueToDistance(range) / 2.;
              double value = InternalCounterValueToDistance(leafData.Steps) - rangeHalfDistance;
              table->SetValue(i, 4, value); // default meaningful value for side "1" for layer-2
            }
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
            {
              int range = this->GetCalibrationRangeInLayer(vtkMRMLIhepMlcControlNode::Layer1);
              double rangeHalfDistance = InternalCounterValueToDistance(range) / 2.;
              double value = rangeHalfDistance - InternalCounterValueToDistance(leafData.Steps);
              table->SetValue(i, 2, value); // default meaningful value for side "2" for layer-1
            }
            break;
          case vtkMRMLIhepMlcControlNode::Layer2:
            {
              int range = this->GetCalibrationRangeInLayer(vtkMRMLIhepMlcControlNode::Layer2);
              double rangeHalfDistance = InternalCounterValueToDistance(range) / 2.;
              double value = rangeHalfDistance - InternalCounterValueToDistance(leafData.Steps);
              table->SetValue(i, 5, value); // default meaningful value for side "2" for layer-2
            }
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

        switch (layer)
        {
        case vtkMRMLIhepMlcControlNode::Layer1:
          pos = this->GetCalibrationRangeInLayer(vtkMRMLIhepMlcControlNode::Layer1) / 2;
          break;
        case vtkMRMLIhepMlcControlNode::Layer2:
          pos = this->GetCalibrationRangeInLayer(vtkMRMLIhepMlcControlNode::Layer2) / 2;
          break;
        default:
          break;
        }
        int center = pos;
        if (std::fabs(centerPos) < radius)
        {
          pos = center - static_cast<int>(sqrt(radius * radius - centerPos * centerPos) * vtkMRMLIhepMlcControlNode::IHEP_MOTOR_STEPS_PER_MM);
        }
        
        this->GetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side1, layer);
        leafData.Steps = pos;
        this->SetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side1, layer);
        if (table)
        {
          switch (layer)
          {
          case vtkMRMLIhepMlcControlNode::Layer1:
            {
              int range = this->GetCalibrationRangeInLayer(vtkMRMLIhepMlcControlNode::Layer1);
              double rangeHalfDistance = InternalCounterValueToDistance(range) / 2.;
              double value = InternalCounterValueToDistance(leafData.Steps) - rangeHalfDistance;
              table->SetValue(i, 1, value); // default meaningful value for side "1" for layer-1
            }
            break;
          case vtkMRMLIhepMlcControlNode::Layer2:
            {
              int range = this->GetCalibrationRangeInLayer(vtkMRMLIhepMlcControlNode::Layer2);
              double rangeHalfDistance = InternalCounterValueToDistance(range) / 2.;
              double value = InternalCounterValueToDistance(leafData.Steps) - rangeHalfDistance;
              table->SetValue(i, 4, value); // default meaningful value for side "1" for layer-2
            }
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
            {
              int range = this->GetCalibrationRangeInLayer(vtkMRMLIhepMlcControlNode::Layer1);
              double rangeHalfDistance = InternalCounterValueToDistance(range) / 2.;
              double value = rangeHalfDistance - InternalCounterValueToDistance(leafData.Steps);
              table->SetValue(i, 2, value); // default meaningful value for side "2" for layer-1
            }
            break;
          case vtkMRMLIhepMlcControlNode::Layer2:
            {
              int range = this->GetCalibrationRangeInLayer(vtkMRMLIhepMlcControlNode::Layer2);
              double rangeHalfDistance = InternalCounterValueToDistance(range) / 2.;
              double value = rangeHalfDistance - InternalCounterValueToDistance(leafData.Steps);
              table->SetValue(i, 5, value); // default meaningful value for side "2" for layer-2
            }
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
        leafData.Steps = 0;
        this->SetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side1, layer);
        if (table)
        {
          switch (layer)
          {
          case vtkMRMLIhepMlcControlNode::Layer1:
            {
              int range = this->GetCalibrationRangeInLayer(vtkMRMLIhepMlcControlNode::Layer1);
              double rangeHalfDistance = InternalCounterValueToDistance(range) / 2.;
              double value = InternalCounterValueToDistance(leafData.Steps) - rangeHalfDistance;
              table->SetValue(i, 1, value); // default meaningful value for side "1" for layer-1
            }
            break;
          case vtkMRMLIhepMlcControlNode::Layer2:
            {
              int range = this->GetCalibrationRangeInLayer(vtkMRMLIhepMlcControlNode::Layer2);
              double rangeHalfDistance = InternalCounterValueToDistance(range) / 2.;
              double value = InternalCounterValueToDistance(leafData.Steps) - rangeHalfDistance;
              table->SetValue(i, 4, value); // default meaningful value for side "1" for layer-2
            }
            break;
          default:
            break;
          }
        }
        this->GetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side2, layer);
        leafData.Steps = 0;
        this->SetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side2, layer);
        if (table)
        {
          switch (layer)
          {
          case vtkMRMLIhepMlcControlNode::Layer1:
            {
              int range = this->GetCalibrationRangeInLayer(vtkMRMLIhepMlcControlNode::Layer1);
              double rangeHalfDistance = InternalCounterValueToDistance(range) / 2.;
              double value = rangeHalfDistance - InternalCounterValueToDistance(leafData.Steps);
              table->SetValue(i, 2, value); // default meaningful value for side "2" for layer-1
            }
            break;
          case vtkMRMLIhepMlcControlNode::Layer2:
            {
              int range = this->GetCalibrationRangeInLayer(vtkMRMLIhepMlcControlNode::Layer2);
              double rangeHalfDistance = InternalCounterValueToDistance(range) / 2.;
              double value = rangeHalfDistance - InternalCounterValueToDistance(leafData.Steps);
              table->SetValue(i, 5, value); // default meaningful value for side "2" for layer-2
            }
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
      int range = this->GetCalibrationRangeInLayer(layer);
      for (int i = 0; i < this->NumberOfLeafPairs; ++i)
      {
        this->GetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side1, layer);
        leafData.Steps = range / 2;
        this->SetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side1, layer);
        if (table)
        {
          switch (layer)
          {
          case vtkMRMLIhepMlcControlNode::Layer1:
            table->SetValue(i, 1, 0.); // default meaningful value for side "1" for layer-1
            break;
          case vtkMRMLIhepMlcControlNode::Layer2:
            table->SetValue(i, 4, 0.); // default meaningful value for side "1" for layer-2
            break;
          default:
            break;
          }
        }
        this->GetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side2, layer);
        leafData.Steps = range / 2;
        this->SetLeafData(leafData, i, vtkMRMLIhepMlcControlNode::Side2, layer);
        if (table)
        {
          switch (layer)
          {
          case vtkMRMLIhepMlcControlNode::Layer1:
            table->SetValue(i, 2, 0.); // default meaningful value for side "2" for layer-1
            break;
          case vtkMRMLIhepMlcControlNode::Layer2:
            table->SetValue(i, 5, 0.); // default meaningful value for side "2" for layer-2
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
bool vtkMRMLIhepMlcControlNode::SetLeafDataByAddressInLayer(const LeafData& leafData, int address, vtkMRMLIhepMlcControlNode::LayerType layer)
{
  int key;
  int offset = -1;
  vtkMRMLIhepMlcControlNode::SideType side = Side_Last;
  if ((offset = this->GetLeafOffsetByAddressInLayer(address, key, side, layer)) != -1)
  {
    return this->SetLeafData(leafData, offset, side, layer);
  }
  return false;
}

//----------------------------------------------------------------------------
bool vtkMRMLIhepMlcControlNode::SetLeafDataState(const LeafData& leafDataState)
{
  int key;
  int offset = -1;
  vtkMRMLIhepMlcControlNode::SideType side = vtkMRMLIhepMlcControlNode::Side_Last;
  vtkMRMLIhepMlcControlNode::LeafData currentLeafData;
  if (!this->GetLeafDataByAddressInLayer(currentLeafData, leafDataState.Address, leafDataState.Layer))
  {
    return false;
  }
  currentLeafData.StateEnabled = leafDataState.StateEnabled;
  currentLeafData.StateReset = leafDataState.StateReset;
  currentLeafData.StateDirection = leafDataState.StateDirection;
  currentLeafData.StateStepMode = leafDataState.StateStepMode;
  currentLeafData.SwitchState = leafDataState.SwitchState;
  currentLeafData.StepsLeft = leafDataState.StepsLeft;
  currentLeafData.EncoderCounts = leafDataState.EncoderCounts;
  currentLeafData.CurrentPosition = leafDataState.CurrentPosition;
//  currentLeafData.RequiredPosition = currentLeafData.Steps;

  if (!leafDataState.SwitchState && !leafDataState.isPositionUnknown())
  {
    int calibrationCorrection = 0;
    int minCalibrationSteps = this->GetMinCalibrationStepsBySideInLayer(currentLeafData.Side, leafDataState.Layer);
    if (minCalibrationSteps != -1)
    {
      calibrationCorrection = currentLeafData.CalibrationSteps - minCalibrationSteps;
    }

///    if (leafData.CurrentPosition > 0)
///    {
///      currentLeafData.CurrentPosition = leafDataState.CurrentPosition - calibrationCorrection;
///    }
///    else
///    {
///      currentLeafData.CurrentPosition = 0;
///    }
    if (leafDataState.isMovingToTheSwitch())
    {
      currentLeafData.CurrentPosition = leafDataState.CurrentPosition - 2 * leafDataState.EncoderCounts - calibrationCorrection;
    }
    else if (leafDataState.isMovingFromTheSwitch())
    {
      currentLeafData.CurrentPosition = leafDataState.CurrentPosition + 2 * leafDataState.EncoderCounts - calibrationCorrection;
    }
    else if (leafDataState.isStopped())
    {
      currentLeafData.CurrentPosition = leafDataState.CurrentPosition - calibrationCorrection;
    }
    else
    {
      currentLeafData.CurrentPosition = 0;
    }
  }
  else if (leafDataState.SwitchState)
  {
    currentLeafData.CurrentPosition = 0;
  }
  else if (leafDataState.isPositionUnknown())
  {
    currentLeafData.CurrentPosition = USHRT_MAX;
  }
  else
  {
    ;
  }

  if ((offset = this->GetLeafOffsetByAddressInLayer(leafDataState.Address, key, side, leafDataState.Layer)) != -1 && side == leafDataState.Side)
  {
    return this->SetLeafData(currentLeafData, offset, side, leafDataState.Layer);
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
int vtkMRMLIhepMlcControlNode::GetRelativeMovementByAddressInLayer(int address, LayerType layer)
{
  vtkMRMLIhepMlcControlNode::LeafData data;
  if (this->GetLeafDataByAddressInLayer( data, address, layer))
  {
    if (!data.SwitchState)
    {
      return data.GetRelativeMovement();
    }
    else
    {
      int minCalibrationSteps = this->GetMinCalibrationStepsBySideInLayer(data.Side, layer);
      if (minCalibrationSteps != -1)
      {
        return (data.GetRelativeMovement() + (data.CalibrationSteps - minCalibrationSteps));
      }
    }
  }
  return 0;
}

//-----------------------------------------------------------------------------
double vtkMRMLIhepMlcControlNode::GetPositionGapByAddressInLayer(int address, LayerType layer)
{
  vtkMRMLIhepMlcControlNode::LeafData data;
  if (this->GetLeafDataByAddressInLayer( data, address, layer))
  {
    if (!data.SwitchState)
    {
      return std::abs(this->InternalCounterValueToDistance(data.GetRelativeMovement()));
    }
    else
    {
      int minCalibrationSteps = this->GetMinCalibrationStepsBySideInLayer(data.Side, layer);
      if (minCalibrationSteps != -1)
      {
        return std::abs(this->InternalCounterValueToDistance(data.GetRelativeMovement() + (data.CalibrationSteps - minCalibrationSteps)));
      }
    }
  }
  return -1.;
}

//----------------------------------------------------------------------------
double vtkMRMLIhepMlcControlNode::ExternalCounterValueToDistance(int extCounterValue)
{
  return (double(extCounterValue) / double(vtkMRMLIhepMlcControlNode::IHEP_EXTERNAL_COUNTS_PER_MM));
}

//----------------------------------------------------------------------------
double vtkMRMLIhepMlcControlNode::InternalCounterValueToDistance(int intCounterValue)
{
  return (double(intCounterValue) / double(vtkMRMLIhepMlcControlNode::IHEP_MOTOR_STEPS_PER_MM));
}

//----------------------------------------------------------------------------
int vtkMRMLIhepMlcControlNode::DistanceToExternalCounterValue(double distance)
{
  return static_cast<int>(distance * vtkMRMLIhepMlcControlNode::IHEP_EXTERNAL_COUNTS_PER_MM);
}

//----------------------------------------------------------------------------
int vtkMRMLIhepMlcControlNode::DistanceToInternalCounterValue(double distance)
{
  return static_cast<int>(distance * vtkMRMLIhepMlcControlNode::IHEP_MOTOR_STEPS_PER_MM);
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
  if (this->Side != vtkMRMLIhepMlcControlNode::Side_Last && this->Layer != vtkMRMLIhepMlcControlNode::Layer_Last)
  {
    if (this->SwitchState)
    {
      // Switch is pressed.
      return 0;
    }
    else
    {
      int currentPosition = -1;
      if (this->Side == vtkMRMLIhepMlcControlNode::Side1)
      {
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
          // Stopped.
          return (currentPosition);
        }
        else if (this->isMovingToTheSwitch())
        {
          // Moving to switch.
          return (currentPosition - 2 * this->EncoderCounts);
        }
        else if (this->isMovingFromTheSwitch())
        {
          // Moving from switch.
          return (currentPosition + 2 * this->EncoderCounts);
        }
        else
        {
          // Impossible.
          return -1;
        }
      }
      else if (this->Side == vtkMRMLIhepMlcControlNode::Side2)
      {
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
          // Stopped.
          return currentPosition;
        }
        else if (this->isMovingToTheSwitch())
        {
          // Moving to switch.
          return (currentPosition - 2 * this->EncoderCounts);
        }
        else if (this->isMovingFromTheSwitch())
        {
          // Moving from switch.
          return (currentPosition + 2 * this->EncoderCounts);
        }
        else
        {
          // Impossible.
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
  if (pos != -1)
  {
    return (this->RequiredPosition - pos);
  }
  return 0;
}

//-----------------------------------------------------------------------------
int vtkMRMLIhepMlcControlNode::GetPairOfLeavesCalibrationRangeByAddressInLayer(int address, LayerType layer)
{
  int range = -1;
  for (auto iter = LeavesDataMap.begin(); iter != LeavesDataMap.end(); ++iter)
  {
    const PairOfLeavesData& leavesPair = (*iter).second;
    const vtkMRMLIhepMlcControlNode::LeafData& leafSide1 = leavesPair.first;
    const vtkMRMLIhepMlcControlNode::LeafData& leafSide2 = leavesPair.second;
    if ((layer == leafSide1.Layer && leafSide1.Address == address) || (layer == leafSide2.Layer && leafSide2.Address == address))
    {
      range = leafSide1.CalibrationSteps + leafSide2.CalibrationSteps;
    }
  }
  return range;
}

//-----------------------------------------------------------------------------
int vtkMRMLIhepMlcControlNode::GetMinCalibrationStepsBySideInLayer(SideType side, LayerType layer)
{
  std::vector<int> steps;
  int pos = -1;
  for (auto iter = LeavesDataMap.begin(); iter != LeavesDataMap.end(); ++iter)
  {
    const PairOfLeavesData& leavesPair = (*iter).second;
    const vtkMRMLIhepMlcControlNode::LeafData& leafSide1 = leavesPair.first;
    const vtkMRMLIhepMlcControlNode::LeafData& leafSide2 = leavesPair.second;
    if (leafSide1.Layer == layer && leafSide1.Side == side)
    {
      steps.push_back(leafSide1.CalibrationSteps);
    }
    else if (leafSide2.Layer == layer && leafSide2.Side == side)
    {
      steps.push_back(leafSide2.CalibrationSteps);
    }
  }
  if (steps.size() == static_cast< size_t >(this->NumberOfLeafPairs))
  {
    pos = *std::min_element(std::begin(steps), std::end(steps));
  }
  return pos;
}

//-----------------------------------------------------------------------------
int vtkMRMLIhepMlcControlNode::GetMaxCalibrationStepsBySideInLayer(SideType side, LayerType layer)
{
  std::vector<int> steps;
  int pos = -1;
  for (auto iter = LeavesDataMap.begin(); iter != LeavesDataMap.end(); ++iter)
  {
    const PairOfLeavesData& leavesPair = (*iter).second;
    const vtkMRMLIhepMlcControlNode::LeafData& leafSide1 = leavesPair.first;
    const vtkMRMLIhepMlcControlNode::LeafData& leafSide2 = leavesPair.second;
    if (leafSide1.Layer == layer && leafSide1.Side == side)
    {
      steps.push_back(leafSide1.CalibrationSteps);
    }
    else if (leafSide2.Layer == layer && leafSide2.Side == side)
    {
      steps.push_back(leafSide2.CalibrationSteps);
    }
  }
  if (steps.size() == static_cast< size_t >(this->NumberOfLeafPairs))
  {
    pos = *std::max_element(std::begin(steps), std::end(steps));
  }
  return pos;
}

//-----------------------------------------------------------------------------
int vtkMRMLIhepMlcControlNode::GetOppositeSideAddressByAddressInLayer(int address, LayerType layer)
{
  int key = -1;
  SideType side = Side_Last;
  int offset = this->GetLeafOffsetByAddressInLayer( address, key, side, layer);
  if (offset != -1)
  {
    vtkMRMLIhepMlcControlNode::PairOfLeavesData pairOfLeaves;
    if (this->GetPairOfLeavesData( pairOfLeaves, offset, layer))
    {
      vtkMRMLIhepMlcControlNode::LeafData& side1 = pairOfLeaves.first;
      vtkMRMLIhepMlcControlNode::LeafData& side2 = pairOfLeaves.second;
      if (side1.Address != address)
      {
        return side1.Address;
      }
      else if (side2.Address != address)
      {
        return side2.Address;
      }
    }
  }
  return -1;
}

//-----------------------------------------------------------------------------
int vtkMRMLIhepMlcControlNode::GetLeafRangeByAddressInLayer(int address, LayerType layer)
{
  int minRangeSide1 = this->GetMinCalibrationStepsBySideInLayer(vtkMRMLIhepMlcControlNode::Side1, layer);
  int minRangeSide2 = this->GetMinCalibrationStepsBySideInLayer(vtkMRMLIhepMlcControlNode::Side2, layer);
  int oppositeAddress = this->GetOppositeSideAddressByAddressInLayer(address, layer);
  if (oppositeAddress != -1)
  {
    vtkMRMLIhepMlcControlNode::LeafData leafData;
    if (this->GetLeafDataByAddressInLayer(leafData, oppositeAddress, layer))
    {
      return (minRangeSide1 + minRangeSide2) - leafData.Steps;
    }
  }
  return -1;
}

//-----------------------------------------------------------------------------
int vtkMRMLIhepMlcControlNode::GetCalibrationRangeInLayer(LayerType layer)
{
  int minRangeSide1 = this->GetMinCalibrationStepsBySideInLayer(vtkMRMLIhepMlcControlNode::Side1, layer);
  int minRangeSide2 = this->GetMinCalibrationStepsBySideInLayer(vtkMRMLIhepMlcControlNode::Side2, layer);
  if (minRangeSide1 != -1 && minRangeSide2 != -1)
  {
    return (minRangeSide1 + minRangeSide2);
  }
  return -1;
}

//-----------------------------------------------------------------------------
double vtkMRMLIhepMlcControlNode::GetTotalGapInLayer(LayerType layer, std::vector<int>& errorAddresses)
{
  double gap = 0.;
  errorAddresses.clear();
  for (auto iter = LeavesDataMap.begin(); iter != LeavesDataMap.end(); ++iter)
  {
    const PairOfLeavesData& leavesPair = (*iter).second;
    const vtkMRMLIhepMlcControlNode::LeafData& leafSide1 = leavesPair.first;
    const vtkMRMLIhepMlcControlNode::LeafData& leafSide2 = leavesPair.second;
    if (leafSide1.Layer == layer && leafSide2.Layer == layer)
    {
      double side1Gap = GetPositionGapByAddressInLayer(leafSide1.Address, layer);
      if (side1Gap < 0.)
      {
        errorAddresses.push_back(leafSide1.Address);
        side1Gap = 0.0;
      }
      double side2Gap = GetPositionGapByAddressInLayer(leafSide2.Address, layer);
      if (side2Gap < 0.)
      {
        errorAddresses.push_back(leafSide2.Address);
        side1Gap = 0.0;
      }
      gap += (side1Gap + side1Gap);
    }
  }
  return gap;
}
