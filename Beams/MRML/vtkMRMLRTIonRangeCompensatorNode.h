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
  std::string ID;
  std::string Description;
  DivergenceType Divergence{ vtkMRMLRTIonRangeCompensatorNode::Absent };
  MountingPositionType MountingPosition{ vtkMRMLRTIonRangeCompensatorNode::PatientSide };
  double MountingPositionCoordinate[2] = { 0., 0. };
  double PixelSpacing[2] = { 0., 0. };
  double Position[2] = { 0., 0. };
  std::vector< double > ThicknessData;
  double MillingToolDiameter{ 0. };
};

#endif // __vtkMRMLRTIonRangeCompensatorNode_h
