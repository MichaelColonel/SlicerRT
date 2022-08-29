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

#ifndef __vtkMRMLRTIhepIonBeamNode_h
#define __vtkMRMLRTIhepIonBeamNode_h

// Beams includes
#include "vtkSlicerBeamsModuleMRMLExport.h"

// MRML includes
#include "vtkMRMLRTIonBeamNode.h"

/// \ingroup SlicerRt_QtModules_Beams
class VTK_SLICER_BEAMS_MODULE_MRML_EXPORT vtkMRMLRTIhepIonBeamNode : public vtkMRMLRTIonBeamNode
{

public:
  static vtkMRMLRTIhepIonBeamNode *New();
  vtkTypeMacro(vtkMRMLRTIhepIonBeamNode,vtkMRMLRTIonBeamNode);
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
  vtkMRMLCopyContentMacro(vtkMRMLRTIonBeamNode);

  /// Make sure display node and transform node are present and valid
  void SetScene(vtkMRMLScene* scene) override;

  /// Get unique node XML tag name (like Volume, Model) 
  const char* GetNodeTagName() override { return "RTIhepIonBeam"; };

public:
  /// Get Isocenter to MLC layer-1 distance
  vtkGetMacro( IsocenterToMlcLayer1Distance, double);
  /// Set Isocenter to MLC layer-1 distance
  void SetIsocenterToMlcLayer1Distance(double);
  /// Get Isocenter to MLC layer-2 distance
  vtkGetMacro( IsocenterToMlcLayer2Distance, double);
  /// Set Isocenter to MLC layer-2 distance
  void SetIsocenterToMlcLayer2Distance(double);

protected:
  vtkMRMLRTIhepIonBeamNode();
  ~vtkMRMLRTIhepIonBeamNode();
  vtkMRMLRTIhepIonBeamNode(const vtkMRMLRTIhepIonBeamNode&);
  void operator=(const vtkMRMLRTIhepIonBeamNode&);

// Beam properties
protected:
  /// distance from isocenter to beam limiting device MLCX, MLCY
  // using IsocenterToMultiLeafCollimatorDistance to store value from vtkMRMLRTIonBeamNode
  double& IsocenterToMlcLayer1Distance{ IsocenterToMultiLeafCollimatorDistance };
  double IsocenterToMlcLayer2Distance;

};

#endif // __vtkMRMLRTIhepIonBeamNode_h
