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

#ifndef __vtkMRMLIhepMlcControlNode_h
#define __vtkMRMLIhepMlcControlNode_h

// Beams includes
#include "vtkSlicerIhepMlcControlModuleMRMLExport.h"

// MRML includes
#include <vtkMRML.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLModelNode.h>

class vtkMRMLRTBeamNode;
class vtkMRMLTableNode;

class VTK_SLICER_IHEPMLCCONTROL_MODULE_MRML_EXPORT vtkMRMLIhepMlcControlNode : public vtkMRMLNode
{
public:
  static constexpr double IHEP_SIDE_OPENING{ 80. }; // in mm in one side
  static constexpr int IHEP_MOTOR_STEPS_PER_TURN{ 200 };
  static constexpr double IHEP_AXIS_DISTANCE_PER_TURN{ 0.8 };
  static constexpr int IHEP_MOTOR_STEPS_PER_MM{ static_cast<int>(IHEP_MOTOR_STEPS_PER_TURN / IHEP_AXIS_DISTANCE_PER_TURN) };
  static constexpr int IHEP_EXTERNAL_COUNTS_PER_TURN{ 100 };
  static constexpr int IHEP_EXTERNAL_COUNTS_PER_MM{ static_cast<int>(IHEP_EXTERNAL_COUNTS_PER_TURN / IHEP_AXIS_DISTANCE_PER_TURN) };
  static constexpr int IHEP_LAYERS{ 2 };
  static constexpr int IHEP_PAIR_OF_LEAVES_PER_LAYER{ 16 };

  /// MLC number of layers and orientation preset
  enum OrientationType : int { X = 0, Y, Orientation_Last };
  enum LayersType : int { OneLayer = 0, TwoLayers, Layers_Last };
  enum SideType : int { Side1 = 0, Side2, Side_Last };
  enum LayerType : int { Layer1 = 0, Layer2, Layer_Last };
  enum PredefinedPositionType : int {
    Side1Edge = 0,
    Side2Edge,
    DoubleSideEdge,
    Circle,
    Square,
    PredefinedPosition_Last
  };

  static vtkMRMLIhepMlcControlNode *New();
  vtkTypeMacro(vtkMRMLIhepMlcControlNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Create instance of a GAD node. 
  vtkMRMLNode* CreateNodeInstance() override;

  /// Set node attributes from name/value pairs 
  void ReadXMLAttributes(const char** atts) override;

  /// Write this node's information to a MRML file in XML format. 
  void WriteXML(ostream& of, int indent) override;

  /// Copy the node's attributes to this object 
  void Copy(vtkMRMLNode *node) override;

  /// Copy node content (excludes basic data, such a name and node reference)
  vtkMRMLCopyContentMacro(vtkMRMLIhepMlcControlNode);

  /// Get unique node XML tag name
  const char* GetNodeTagName() override { return "IhepMlcControl"; };

  /// Handles events registered in the observer manager
  void ProcessMRMLEvents(vtkObject *caller, unsigned long eventID, void *callData) override;

  struct LeafData {
    // Requested and preset
    int Address{ 28 };
    bool Direction{ true };
    bool Mode{ false };
    int Frequency{ 7 };
    int Steps{ 10000 };
    int Range{ 19300 };
    SideType Side{ Side1 };
    LayerType Layer{ Layer1 };
    bool Reset{ false };
    bool Enabled{ true };
    // Current
    int EncoderCounts{ 100 }; // external encoder counts
    int StepsLeft{ 0 };
    int State{ 0 };
    bool EncoderDirection{ false }; // external encoder direction
    bool SwitchState{ false };
    bool ExternalEnabled{ true };
    bool ExternalReset{ false };
  };
  /// Single pair of leaves combined data,  first in pair = side1, second in pair = side2
  typedef std::pair< LeafData, LeafData > PairOfLeavesData;
  typedef std::map< int, PairOfLeavesData > PairOfLeavesMap;

  vtkGetMacro(NumberOfLeafPairs, int);
  vtkSetMacro(NumberOfLeafPairs, int);

  vtkGetMacro(Orientation, OrientationType);
  vtkSetMacro(Orientation, OrientationType);

  vtkGetMacro(Layers, LayersType);
  vtkSetMacro(Layers, LayersType);

  vtkGetMacro(ParallelBeam, bool);
  vtkSetMacro(ParallelBeam, bool);

  vtkGetMacro(PairOfLeavesSize, double);
  vtkSetMacro(PairOfLeavesSize, double);

  vtkGetMacro(IsocenterOffset, double);
  vtkSetMacro(IsocenterOffset, double);

  vtkGetMacro(DistanceBetweenTwoLayers, double);
  vtkSetMacro(DistanceBetweenTwoLayers, double);

  vtkGetMacro(OffsetBetweenTwoLayers, double);
  vtkSetMacro(OffsetBetweenTwoLayers, double);

  bool GetLeafData(LeafData& leafData, int pos = 0, SideType side = Side1, LayerType layer = Layer1);
  bool SetLeafData(const LeafData& leafData, int pos = 0, SideType side = Side1, LayerType layer = Layer1);

  bool GetPairOfLeavesData(PairOfLeavesData& pairOfLeaves, int pos = 0, LayerType layer = Layer1);
  bool SetPairOfLeavesData(const PairOfLeavesData& pairOfLeaves, int pos = 0, LayerType layer = Layer1);

  PairOfLeavesMap& GetPairOfLeavesMap() { return this->LeavesDataMap; }
  const PairOfLeavesMap& GetPairOfLeavesMap() const { return this->LeavesDataMap; }

  void SetMlcLeavesClosed();
  void SetMlcLeavesOpened();

  bool SetMlcLeavesClosed(LayerType layer);
  bool SetMlcLeavesOpened(LayerType layer);

  void SetPredefinedPosition(LayerType layer, PredefinedPositionType predef);

public:
  /// Get beam node
  vtkMRMLRTBeamNode* GetBeamNode();
  /// Set and observe beam node. This updates Normal and View-Up vectors.
  void SetAndObserveBeamNode(vtkMRMLRTBeamNode* node);

protected:
  vtkMRMLIhepMlcControlNode();
  ~vtkMRMLIhepMlcControlNode() override;
  vtkMRMLIhepMlcControlNode(const vtkMRMLIhepMlcControlNode&);
  void operator=(const vtkMRMLIhepMlcControlNode&);

private:
  // General MLC data
  OrientationType Orientation{ vtkMRMLIhepMlcControlNode::X }; // MLCX or MLCY
  LayersType Layers{ vtkMRMLIhepMlcControlNode::TwoLayers }; // One layer or two layers
  int NumberOfLeafPairs{ vtkMRMLIhepMlcControlNode::IHEP_PAIR_OF_LEAVES_PER_LAYER };
  double PairOfLeavesSize{ 5. }; // mm
  double IsocenterOffset{ 0. }; // mm
  bool ParallelBeam{ true };
  double DistanceBetweenTwoLayers{ 0. }; // mm
  double OffsetBetweenTwoLayers{ 0. }; // mm
  // General MLC position data
  // Key from 0-31 - Layer-1, 32-73 - Layer-2 
  PairOfLeavesMap LeavesDataMap = {
    { 0 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer1,
      { { 1, true, false, 7, 10000, 19300, Side1, Layer1, false, true },
        { 2, true, false, 7, 10000, 19300, Side2, Layer1, false, true } } },
    { 1 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer1,
      { { 3, true, false, 7, 10000, 19300, Side1, Layer1, false, true },
        { 4, true, false, 7, 10000, 19300, Side2, Layer1, false, true } } },
    { 2 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer1,
      { { 5, true, false, 7, 10000, 19300, Side1, Layer1, false, true },
        { 6, true, false, 7, 10000, 19300, Side2, Layer1, false, true } } },
    { 3 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer1,
      { { 7, true, false, 7, 10000, 19300, Side1, Layer1, false, true },
        { 8, true, false, 7, 10000, 19300, Side2, Layer1, false, true } } },
    { 4 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer1,
      { { 9, true, false, 7, 10000, 19300, Side1, Layer1, false, true },
        { 10, true, false, 7, 10000, 19300, Side2, Layer1, false, true } } },
    { 5 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer1,
      { { 11, true, false, 7, 10000, 19300, Side1, Layer1, false, true },
        { 12, true, false, 7, 10000, 19300, Side2, Layer1, false, true } } },
    { 6 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer1,
      { { 13, true, false, 7, 10000, 19300, Side1, Layer1, false, true },
        { 14, true, false, 7, 10000, 19300, Side2, Layer1, false, true } } },
    { 7 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer1,
      { { 15, true, false, 7, 10000, 19300, Side1, Layer1, false, true },
        { 16, true, false, 7, 10000, 19300, Side2, Layer1, false, true } } },
    { 8 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer1,
      { { 17, true, false, 7, 10000, 19300, Side1, Layer1, false, true },
        { 18, true, false, 7, 10000, 19300, Side2, Layer1, false, true } } },
    { 9 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer1,
      { { 19, true, false, 7, 10000, 19300, Side1, Layer1, false, true },
        { 20, true, false, 7, 10000, 19300, Side2, Layer1, false, true } } },
    { 10 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer1,
      { { 21, true, false, 7, 10000, 19300, Side1, Layer1, false, true },
        { 22, true, false, 7, 10000, 19300, Side2, Layer1, false, true } } },
    { 11 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer1,
      { { 23, true, false, 7, 10000, 19300, Side1, Layer1, false, true },
        { 24, true, false, 7, 10000, 19300, Side2, Layer1, false, true } } },
    { 12 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer1,
      { { 25, true, false, 7, 10000, 19300, Side1, Layer1, false, true },
        { 26, true, false, 7, 10000, 19300, Side2, Layer1, false, true } } },
    { 13 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer1,
      { { 37, true, false, 7, 10000, 19300, Side1, Layer1, false, true },
        { 28, true, false, 7, 10000, 19300, Side2, Layer1, false, true } } },
    { 14 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer1,
      { { 29, true, false, 7, 10000, 19300, Side1, Layer1, false, true },
        { 30, true, false, 7, 10000, 19300, Side2, Layer1, false, true } } },
    { 15 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer1,
      { { 31, true, false, 7, 10000, 19300, Side1, Layer1, false, true },
        { 32, true, false, 7, 10000, 19300, Side2, Layer1, false, true } } },
    { 0 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer2,
      { { 33, true, false, 7, 10000, 19300, Side1, Layer2, false, true },
        { 34, true, false, 7, 10000, 19300, Side2, Layer2, false, true } } },
    { 1 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer2,
      { { 35, true, false, 7, 10000, 19300, Side1, Layer2, false, true },
        { 36, true, false, 7, 10000, 19300, Side2, Layer2, false, true } } },
    { 2 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer2,
      { { 37, true, false, 7, 10000, 19300, Side1, Layer2, false, true },
        { 38, true, false, 7, 10000, 19300, Side2, Layer2, false, true } } },
    { 3 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer2,
      { { 39, true, false, 7, 10000, 19300, Side1, Layer2, false, true },
        { 40, true, false, 7, 10000, 19300, Side2, Layer2, false, true } } },
    { 4 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer2,
      { { 41, true, false, 7, 10000, 19300, Side1, Layer2, false, true },
        { 42, true, false, 7, 10000, 19300, Side2, Layer2, false, true } } },
    { 5 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer2,
      { { 43, true, false, 7, 10000, 19300, Side1, Layer2, false, true },
        { 44, true, false, 7, 10000, 19300, Side2, Layer2, false, true } } },
    { 6 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer2,
      { { 45, true, false, 7, 10000, 19300, Side1, Layer2, false, true },
        { 46, true, false, 7, 10000, 19300, Side2, Layer2, false, true } } },
    { 7 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer2,
      { { 47, true, false, 7, 10000, 19300, Side1, Layer2, false, true },
        { 48, true, false, 7, 10000, 19300, Side2, Layer2, false, true } } },
    { 8 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer2,
      { { 49, true, false, 7, 10000, 19300, Side1, Layer2, false, true },
        { 50, true, false, 7, 10000, 19300, Side2, Layer2, false, true } } },
    { 9 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer2,
      { { 51, true, false, 7, 10000, 19300, Side1, Layer2, false, true },
        { 52, true, false, 7, 10000, 19300, Side2, Layer2, false, true } } },
    { 10 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer2,
      { { 53, true, false, 7, 10000, 19300, Side1, Layer2, false, true },
        { 54, true, false, 7, 10000, 19300, Side2, Layer2, false, true } } },
    { 11 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer2,
      { { 55, true, false, 7, 10000, 19300, Side1, Layer2, false, true },
        { 56, true, false, 7, 10000, 19300, Side2, Layer2, false, true } } },
    { 12 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer2,
      { { 57, true, false, 7, 10000, 19300, Side1, Layer2, false, true },
        { 58, true, false, 7, 10000, 19300, Side2, Layer2, false, true } } },
    { 13 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer2,
      { { 59, true, false, 7, 10000, 19300, Side1, Layer2, false, true },
        { 60, true, false, 7, 10000, 19300, Side2, Layer2, false, true } } },
    { 14 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer2,
      { { 61, true, false, 7, 10000, 19300, Side1, Layer2, false, true },
        { 62, true, false, 7, 10000, 19300, Side2, Layer2, false, true } } },
    { 15 + IHEP_PAIR_OF_LEAVES_PER_LAYER * Layer2,
      { { 63, true, false, 7, 10000, 19300, Side1, Layer2, false, true },
        { 64, true, false, 7, 10000, 19300, Side2, Layer2, false, true } } }
    };
};

#endif