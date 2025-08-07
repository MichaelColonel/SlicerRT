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

#ifndef __vtkMRMLRTCarmBeamNode_h
#define __vtkMRMLRTCarmBeamNode_h

#include "vtkSlicerPatientPositioningModuleMRMLExport.h"

// MRML includes
#include "vtkMRMLRTBeamNode.h"

/// \ingroup SlicerRt_QtModules_PatientPositioning
class VTK_SLICER_PATIENTPOSITIONING_MODULE_MRML_EXPORT vtkMRMLRTCarmBeamNode : public vtkMRMLRTBeamNode
{
public:
  static vtkMRMLRTCarmBeamNode *New();
  vtkTypeMacro(vtkMRMLRTCarmBeamNode,vtkMRMLRTBeamNode);
  /// Create instance of a GAD node. 
  vtkMRMLNode* CreateNodeInstance() override;

  /// Get unique node XML tag name (like Volume, Model) 
  const char* GetNodeTagName() override { return "RTCarmBeam"; };

  /// Create and observe default display node
  void CreateDefaultDisplayNodes() override;

  bool GetPlanIsocenterPositionWorld(double isocenter[3]);

protected:
  vtkMRMLRTCarmBeamNode();
  ~vtkMRMLRTCarmBeamNode() override;
  vtkMRMLRTCarmBeamNode(const vtkMRMLRTCarmBeamNode&);
  void operator=(const vtkMRMLRTCarmBeamNode&);
};

#endif // __vtkMRMLRTCarmBeamNode_h
