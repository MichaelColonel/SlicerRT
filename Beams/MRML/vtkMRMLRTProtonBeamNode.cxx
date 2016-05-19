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

// SlicerRt includes
#include "SlicerRtCommon.h"
#include "vtkMRMLRTBeamNode.h"
#include "vtkMRMLRTPlanNode.h"
#include "vtkMRMLRTProtonBeamNode.h"
#include "vtkMRMLSegmentationNode.h"

// MRML includes
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLLabelMapVolumeDisplayNode.h>
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelHierarchyNode.h>
#include <vtkMRMLDoubleArrayNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLSubjectHierarchyNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkIntArray.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkImageResample.h>
#include <vtkGeneralTransform.h>
#include <vtkCollection.h>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLRTProtonBeamNode);

//----------------------------------------------------------------------------
vtkMRMLRTProtonBeamNode::vtkMRMLRTProtonBeamNode()
{
  this->ProximalMargin = 0.0;
  this->DistalMargin = 0.0;

  this->BeamLineType = true;

  this->ManualEnergyLimits = false;
  this->MaximumEnergy = 0;
  this->MinimumEnergy = 0;

  this->EnergyResolution = 2.0;
  this->EnergySpread = 1.0;
  this->StepLength = 1.0;
  this->PencilBeamResolution = 1.0;
  this->RangeCompensatorSmearingRadius = 0.0;
  this->Algorithm = RayTracer;

  this->ApertureOffset = 1500.0;
  this->ApertureSpacing[0] = 1;
  this->ApertureSpacing[1] = 1;
  this->ApertureOrigin[0] = -1;
  this->ApertureOrigin[1] = -1;
  this->ApertureDim[0] = 1;
  this->ApertureDim[1] = 1;

  this->SourceSize = 0.0;
  this->SetRadiationType(Proton);

  this->LateralSpreadHomoApprox = false;
  this->RangeCompensatorHighland = false;
}

//----------------------------------------------------------------------------
vtkMRMLRTProtonBeamNode::~vtkMRMLRTProtonBeamNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLRTProtonBeamNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream
  vtkIndent indent(nIndent);

  // GCS FIX TODO *** Add all members ***
  of << indent << " RangeCompensatorSmearingRadius=\"" << this->RangeCompensatorSmearingRadius << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLRTProtonBeamNode::ReadXMLAttributes(const char** atts)
{
  vtkMRMLNode::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;

  while (*atts != NULL) 
  {
    attName = *(atts++);
    attValue = *(atts++);

    // GCS FIX TODO *** Add all members ***
    if (!strcmp(attName, "RangeCompensatorSmearingRadius")) 
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->RangeCompensatorSmearingRadius;
    }
  }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLRTProtonBeamNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  this->DisableModifiedEventOn();

  vtkMRMLRTProtonBeamNode *node = (vtkMRMLRTProtonBeamNode *) anode;

  // GCS FIX TODO *** Add all members ***
  
  this->SetProximalMargin(node->GetProximalMargin());
  this->SetDistalMargin(node->GetDistalMargin());

  this->SetBeamLineType(node->GetBeamLineType());

  this->SetManualEnergyLimits(node->GetManualEnergyLimits());
  this->SetMaximumEnergy(node->GetMaximumEnergy());
  this->SetMinimumEnergy(node->GetMinimumEnergy());

  this->SetEnergyResolution(node->GetEnergyResolution());
  this->SetEnergySpread(node->GetEnergySpread());
  this->SetStepLength(node->GetStepLength());
  this->SetAlgorithm(node->GetAlgorithm());
  this->SetPencilBeamResolution(node->GetPencilBeamResolution());
  this->SetRangeCompensatorSmearingRadius(node->GetRangeCompensatorSmearingRadius());

  this->SetApertureOffset(node->GetApertureOffset());

  this->SetSourceSize(node->GetSourceSize());
  this->SetRadiationType(node->GetRadiationType());

  this->SetLateralSpreadHomoApprox(node->GetLateralSpreadHomoApprox());
  this->SetRangeCompensatorHighland(node->GetRangeCompensatorHighland());

  this->DisableModifiedEventOff();
  this->InvokePendingModifiedEvent();
}

//----------------------------------------------------------------------------
void vtkMRMLRTProtonBeamNode::UpdateReferences()
{
  Superclass::UpdateReferences();
}

//----------------------------------------------------------------------------
void vtkMRMLRTProtonBeamNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  Superclass::UpdateReferenceID(oldID, newID);
}

//----------------------------------------------------------------------------
void vtkMRMLRTProtonBeamNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
}

//----------------------------------------------------------------------------
void vtkMRMLRTProtonBeamNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  // GCS FIX TODO *** Add all members ***
  os << indent << "RangeCompensatorSmearingRadius:   " << this->RangeCompensatorSmearingRadius << "\n";
}

//----------------------------------------------------------------------------
const double* vtkMRMLRTProtonBeamNode::GetApertureSpacing ()
{
  return this->ApertureSpacing;
}

//----------------------------------------------------------------------------
double vtkMRMLRTProtonBeamNode::GetApertureSpacing (int dim)
{
  return this->ApertureSpacing[dim];
}

//----------------------------------------------------------------------------
void vtkMRMLRTProtonBeamNode::SetApertureSpacing (const float* spacing)
{
  for (int d = 0; d < 2; d++) 
  {
    this->ApertureSpacing[d] = spacing[d];
  }
}

//----------------------------------------------------------------------------
void vtkMRMLRTProtonBeamNode::SetApertureSpacing (const double* spacing)
{
  for (int d = 0; d < 2; d++) 
  {
    this->ApertureSpacing[d] = spacing[d];
  }
}

//----------------------------------------------------------------------------
const double* vtkMRMLRTProtonBeamNode::GetApertureOrigin ()
{
  return this->ApertureOrigin;
}

//----------------------------------------------------------------------------
double vtkMRMLRTProtonBeamNode::GetApertureOrigin (int dim)
{
  return this->ApertureOrigin[dim];
}

//----------------------------------------------------------------------------
void vtkMRMLRTProtonBeamNode::SetApertureOrigin (const float* position)
{
  for (int d = 0; d < 2; d++) 
  {
    this->ApertureOrigin[d] = position[d];
  }
}

//----------------------------------------------------------------------------
void vtkMRMLRTProtonBeamNode::SetApertureOrigin (const double* position)
{
  for (int d = 0; d < 2; d++) 
  {
    this->ApertureOrigin[d] = position[d];
  }
}

//----------------------------------------------------------------------------
const int* vtkMRMLRTProtonBeamNode::GetApertureDim ()
{
  return this->ApertureDim;
}

//----------------------------------------------------------------------------
int vtkMRMLRTProtonBeamNode::GetApertureDim (int dim)
{
  return this->ApertureDim[dim];
}

//----------------------------------------------------------------------------
void vtkMRMLRTProtonBeamNode::SetApertureDim (const int* dim)
{
  for (int d = 0; d < 2; d++) 
  {
    this->ApertureDim[d] = dim[d];
  }
}

//----------------------------------------------------------------------------
void vtkMRMLRTProtonBeamNode::UpdateApertureParameters()
{
  if (this->SAD < 0 || this->SAD < this->ApertureOffset)
  {
    vtkErrorMacro("UpdateApertureParameters: SAD (=" << this->SAD << ") must be positive and greater than Aperture offset (" << this->ApertureOffset << ")");
    printf("SAD = %lg, Aperture offset = %lg\n", this->SAD, this->ApertureOffset);
    return;
  }
  double origin[2] = {this->X1Jaw * this->ApertureOffset / this->SAD , this->Y1Jaw * this->ApertureOffset / this->SAD };
  this->SetApertureOrigin(origin);

  double spacing_at_aperture[2] = {1/ this->PencilBeamResolution * this->ApertureOffset / this->SAD, 1 / this->PencilBeamResolution * this->ApertureOffset / this->SAD};
  this->SetApertureSpacing(spacing_at_aperture);

  int dim[2] = { (int) ((this->X2Jaw - this->X1Jaw) / this->PencilBeamResolution + 1 ), (int) ((this->Y2Jaw - this->Y1Jaw) / this->PencilBeamResolution + 1 )};
  this->SetApertureDim(dim);
}