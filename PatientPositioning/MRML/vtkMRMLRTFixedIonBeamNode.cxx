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

// Beams includes
#include "vtkMRMLRTFixedIonBeamNode.h"

// SlicerRT includes
#include "vtkSlicerRtCommon.h"

// MRML includes
#include <vtkMRMLModelDisplayNode.h>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLRTFixedIonBeamNode);

//----------------------------------------------------------------------------
vtkMRMLRTFixedIonBeamNode::vtkMRMLRTFixedIonBeamNode()
  :
  Superclass()
{
  this->SetGantryAngle(270.);
  this->SetCollimatorAngle(90.);
}

//----------------------------------------------------------------------------
vtkMRMLRTFixedIonBeamNode::~vtkMRMLRTFixedIonBeamNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLRTFixedIonBeamNode::CreateDefaultDisplayNodes()
{
  // Create default model display node
  this->vtkMRMLModelNode::CreateDefaultDisplayNodes();

  // Set beam-specific parameters
  vtkMRMLModelDisplayNode* displayNode = vtkMRMLModelDisplayNode::SafeDownCast(this->GetDisplayNode());
  if (!displayNode)
  {
    vtkErrorMacro("CreateDefaultDisplayNodes: Failed to create default display node");
    return;
  }

  displayNode->SetColor(0.3, 0.1, 1.0);
  displayNode->SetOpacity(0.3);
  displayNode->SetBackfaceCulling(0); // Disable backface culling to make the back side of the contour visible as well
  displayNode->VisibilityOn();
  displayNode->Visibility2DOn();
}
