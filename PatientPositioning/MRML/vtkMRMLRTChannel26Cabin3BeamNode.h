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

#ifndef __vtkMRMLRTChannel26Cabin3BeamNode_h
#define __vtkMRMLRTChannel26Cabin3BeamNode_h

#include "vtkSlicerPatientPositioningModuleMRMLExport.h"

// Beams MRML includes
#include <vtkMRMLRTBeamNode.h>

// MRML includes
#include "vtkMRMLRTIonBeamNode.h"

/// \ingroup SlicerRt_QtModules_PatientPositioning
class VTK_SLICER_PATIENTPOSITIONING_MODULE_MRML_EXPORT vtkMRMLRTChannel26Cabin3BeamNode : public vtkMRMLRTIonBeamNode
{

public:
  static vtkMRMLRTChannel26Cabin3BeamNode *New();
  vtkTypeMacro(vtkMRMLRTChannel26Cabin3BeamNode, vtkMRMLRTIonBeamNode);
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
  vtkMRMLCopyContentMacro(vtkMRMLRTChannel26Cabin3BeamNode);

  /// Make sure display node and transform node are present and valid
  void SetScene(vtkMRMLScene* scene) override;

  /// Get unique node XML tag name (like Volume, Model) 
  const char* GetNodeTagName() override { return "RTChannel26Cabin3BeamNode"; };

  /// Create and observe default display node
  void CreateDefaultDisplayNodes() override;

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
  vtkMRMLRTChannel26Cabin3BeamNode();
  ~vtkMRMLRTChannel26Cabin3BeamNode() override;
  vtkMRMLRTChannel26Cabin3BeamNode(const vtkMRMLRTChannel26Cabin3BeamNode&);
  void operator=(const vtkMRMLRTChannel26Cabin3BeamNode&);

// Beam properties
protected:
  /// distance from isocenter to beam limiting device MLCX, MLCY
  // using IsocenterToMultiLeafCollimatorDistance to store value from vtkMRMLRTIonBeamNode
  double& IsocenterToMlcLayer1Distance{ IsocenterToMultiLeafCollimatorDistance };
  double IsocenterToMlcLayer2Distance;

};

#endif // __vtkMRMLRTChannel26Cabin3BeamNode_h
