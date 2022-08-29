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

// STD includes
#include <initializer_list>

class vtkMRMLRTBeamNode;
class vtkMRMLTableNode;

class VTK_SLICER_IHEPMLCCONTROL_MODULE_MRML_EXPORT vtkMRMLIhepMlcControlNode : public vtkMRMLNode
{
public:
  static constexpr int IHEP_LAYERS{ 2 };
  static constexpr int IHEP_PAIR_OF_LEAVES_PER_LAYER{ 2 };

  /// MLC number of layers and orientation preset
  enum OrientationType : int { X = 0, Y, Orientation_Last };
  enum LayersType : int { OneLayer = 0, TwoLayers, Layers_Last };
  enum SideType : int { Side1 = 0, Side2, Side_Last };
  enum LayerType : int { Layer1 = 0, Layer2, Layer_Last };

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

  vtkGetMacro(Layers, LayersType);
  vtkSetMacro(Layers, LayersType);

  vtkGetMacro(ParallelBeam, bool);
  vtkSetMacro(ParallelBeam, bool);

  bool GetLeafData(LeafData& leafData, int pos = 0, SideType side = Side1, LayerType layer = Layer1);
  bool GetPairOfLeavesData(PairOfLeavesData& pairOfLeaves, int pos = 0, LayerType layer = Layer1);
  bool SetPairOfLeavesData(const PairOfLeavesData& pairOfLeaves, int pos = 0, LayerType layer = Layer1);

  PairOfLeavesMap& GetPairOfLeavesMap() { return this->LeavesDataMap; }
  const PairOfLeavesMap& GetPairOfLeavesMap() const { return this->LeavesDataMap; }

public:
  /// Get beam node
  vtkMRMLRTBeamNode* GetBeamNode();
  /// Set and observe beam node. This updates Normal and View-Up vectors.
  void SetAndObserveBeamNode(vtkMRMLRTBeamNode* node);

  /// Get table node
  vtkMRMLTableNode* GetTableNode();
  /// Set and observe table node.   
  void SetAndObserveTableNode(vtkMRMLTableNode* node);

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
  double SizeOfLeafPair{ 5. }; // mm
  double IsocenterOffset{ 0. }; // mm
  bool ParallelBeam{ true };
  double DistanceBetweenTwoLayers{ 0. }; // mm
  double OffsetBetweenTwoLayers{ 0. }; // mm
  // General MLC position data
  // Key from 0-31 - Layer-1, 32-73 - Layer-2 
  std::map< int, PairOfLeavesData > LeavesDataMap = {
    { 0 + IHEP_LAYERS * Layer1,
      { { 29, true, false, 7, 10000, 19300, Side1, Layer1, false, true },
        { 28, true, false, 7, 10000, 19300, Side2, Layer1, false, true } } },
    { 1 + IHEP_LAYERS * Layer1,
      { { 30, true, false, 7, 10000, 19300, Side1, Layer1, false, true },
        { 31, true, false, 7, 10000, 19300, Side2, Layer1, false, true } } },
    { 0 + IHEP_LAYERS * Layer2,
      { { 28, true, false, 7, 10000, 19300, Side1, Layer2, false, true },
        { 29, true, false, 7, 10000, 19300, Side2, Layer2, false, true } } },
    { 1 + IHEP_LAYERS * Layer2,
      { { 30, true, false, 7, 10000, 19300, Side1, Layer2, false, true },
        { 31, true, false, 7, 10000, 19300, Side2, Layer2, false, true } } }
    };
};

#endif
