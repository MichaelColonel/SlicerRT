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

#ifndef __vtkMRMLRTFixedBeamNode_h
#define __vtkMRMLRTFixedBeamNode_h

// Beams includes
#include "vtkSlicerBeamsModuleMRMLExport.h"

// MRML includes
#include "vtkMRMLRTBeamNode.h"

/// \ingroup SlicerRt_QtModules_Beams
class VTK_SLICER_BEAMS_MODULE_MRML_EXPORT vtkMRMLRTFixedBeamNode : public vtkMRMLRTBeamNode
{
public:
  static vtkMRMLRTFixedBeamNode *New();
  vtkTypeMacro(vtkMRMLRTFixedBeamNode,vtkMRMLRTBeamNode);
  /// Create instance of a GAD node. 
  vtkMRMLNode* CreateNodeInstance() override;

  /// Get unique node XML tag name (like Volume, Model) 
  const char* GetNodeTagName() override { return "RTFixedBeam"; };

  /// Create and observe default display node
  void CreateDefaultDisplayNodes() override;

protected:
  vtkMRMLRTFixedBeamNode();
  ~vtkMRMLRTFixedBeamNode() override;
  vtkMRMLRTFixedBeamNode(const vtkMRMLRTFixedBeamNode&);
  void operator=(const vtkMRMLRTFixedBeamNode&);
};

#endif // __vtkMRMLRTFixedBeamNode_h
