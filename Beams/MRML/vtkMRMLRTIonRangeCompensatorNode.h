/*==============================================================================

  Copyright (c) Radiation Medicine Program, University Health Network,
  Princess Margaret Hospital, Toronto, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kevin Wang, Princess Margaret Cancer Centre 
  and was supported by Cancer Care Ontario (CCO)'s ACRU program 
  with funds provided by the Ontario Ministry of Health and Long-Term Care
  and Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO).

==============================================================================*/

#ifndef __vtkMRMLRTIonRangeCompensatorNode_h
#define __vtkMRMLRTIonRangeCompensatorNode_h

// Beams includes
#include "vtkSlicerBeamsModuleMRMLExport.h"
#include "vtkMRMLRTIonBeamNode.h"

// MRML includes
#include <vtkMRMLModelNode.h>

/// \ingroup SlicerRt_QtModules_Beams
class VTK_SLICER_BEAMS_MODULE_MRML_EXPORT vtkMRMLRTIonRangeCompensatorNode : public vtkMRMLModelNode
{

public:
  enum DivergenceType : int {
    Present,
    Absent,
    Divergence_Last,
  };
  enum MountingPositionType : int {
    PatientSide,
    SourceSide,
    DoubleSided,
    MountingPosition_Last,
  };
  
  static vtkMRMLRTIonRangeCompensatorNode *New();
  vtkTypeMacro(vtkMRMLRTIonRangeCompensatorNode,vtkMRMLModelNode);
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
  vtkMRMLCopyContentMacro(vtkMRMLRTIonRangeCompensatorNode);

  /// Make sure display node and transform node are present and valid
  void SetScene(vtkMRMLScene* scene) override;

  /// Get unique node XML tag name (like Volume, Model) 
  const char* GetNodeTagName() override { return "RTIonRangeCompensator"; };

  /// Create and observe default display node
  void CreateDefaultDisplayNodes() override;

public:
  /// Get compensator rows
  vtkGetMacro(Rows, int);
  /// Set compensator rows
  vtkSetMacro(Rows, int);

  /// Get compensator columns
  vtkGetMacro(Columns, int);
  /// Set compensator columns
  vtkSetMacro(Columns, int);

  /// Get divergence type
  vtkGetMacro(Divergence, DivergenceType);
  /// Set divergence type
  vtkSetMacro(Divergence, DivergenceType);

  /// Get mounting position type
  vtkGetMacro(MountingPosition, MountingPositionType);
  /// Set mounting position type
  vtkSetMacro(MountingPosition, MountingPositionType);

  /// Get isocenter to compensator tray distance
  vtkGetMacro(IsocenterToCompensatorTrayDistance, double);
  /// Set isocenter to compensator tray distance
  vtkSetMacro(IsocenterToCompensatorTrayDistance, double);

  /// Get parent ion beam node
  vtkMRMLRTIonBeamNode* GetBeamNode();
  /// Set and observe parent ion beam node
  void SetAndObserveBeamNode(vtkMRMLRTIonBeamNode* node);

  vtkGetMacro(MaterialID, std::string);
  vtkSetMacro(MaterialID, std::string);
  vtkGetMacro(CompensatorID, std::string);
  vtkSetMacro(CompensatorID, std::string);
  vtkGetMacro(AccessoryCode, std::string);
  vtkSetMacro(AccessoryCode, std::string);
  vtkGetMacro(CompensatorDescription, std::string);
  vtkSetMacro(CompensatorDescription, std::string);

  vtkGetVector2Macro(MountingPositionCoordinate, double);
  vtkSetVector2Macro(MountingPositionCoordinate, double);
  vtkGetVector2Macro(PixelSpacing, double);
  vtkSetVector2Macro(PixelSpacing, double);
  vtkGetVector2Macro(Position, double);
  vtkSetVector2Macro(Position, double);
  vtkGetMacro(MillingToolDiameter, double);
  vtkSetMacro(MillingToolDiameter, double);

  void SetThicknessDataVector(const std::vector< double >&);

protected:
  /// Create compensator model from compensator parameters
  /// \param compensatorModelPolyData Output polydata. If none given then the compensator node's own polydata is used
  virtual void CreateCompensatorPolyData(vtkPolyData* compensatorModelPolyData=nullptr);

protected:
  vtkMRMLRTIonRangeCompensatorNode();
  ~vtkMRMLRTIonRangeCompensatorNode();
  vtkMRMLRTIonRangeCompensatorNode(const vtkMRMLRTIonRangeCompensatorNode&);
  void operator=(const vtkMRMLRTIonRangeCompensatorNode&);

  static const char* GetDivergenceAsString(int id);
  static int GetDivergenceFromString(const char* name);
  void SetDivergence(int id);

  static const char* GetMountingPositionAsString(int id);
  static int GetMountingPositionFromString(const char* name);
  void SetMountingPosition(int id);

// Compensator properties
  int Rows;
  int Columns;
  std::string MaterialID;
  std::string AccessoryCode;
  std::string CompensatorID;
  std::string CompensatorDescription;
  double IsocenterToCompensatorTrayDistance;
  DivergenceType Divergence{ vtkMRMLRTIonRangeCompensatorNode::Divergence_Last };
  MountingPositionType MountingPosition{ vtkMRMLRTIonRangeCompensatorNode::MountingPosition_Last };
  double MountingPositionCoordinate[2] = { 0., 0. };
  double PixelSpacing[2] = { -1., -1. };
  double Position[2] = { 0., 0. };
  std::vector< double > ThicknessData;
  double MillingToolDiameter{ -1. };
};

#endif // __vtkMRMLRTIonRangeCompensatorNode_h
