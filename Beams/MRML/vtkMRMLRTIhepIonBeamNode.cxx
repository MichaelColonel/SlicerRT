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
#include "vtkMRMLRTIhepIonBeamNode.h"
#include "vtkMRMLRTPlanNode.h"

// SlicerRT includes
#include "vtkSlicerRtCommon.h"

// MRML includes
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLTableNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLSubjectHierarchyNode.h>
#include <vtkMRMLSubjectHierarchyConstants.h>

// VTK includes
#include <vtkCommand.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkIntArray.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkDoubleArray.h>
#include <vtkTable.h>
#include <vtkCellArray.h>
#include <vtkAppendPolyData.h>
//------------------------------------------------------------------------------
namespace
{

const char* const SCANSPOT_REFERENCE_ROLE = "ScanSpotRef";
double FWHM_TO_SIGMA = 1. / (2. * sqrt(2. * log(2.)));

} // namespace

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLRTIhepIonBeamNode);

//----------------------------------------------------------------------------
vtkMRMLRTIhepIonBeamNode::vtkMRMLRTIhepIonBeamNode()
  :
  Superclass(),
  IsocenterToMlcLayer1Distance(vtkMRMLRTIonBeamNode::IsocenterToMultiLeafCollimatorDistance),
  IsocenterToMlcLayer2Distance(IsocenterToMlcLayer1Distance)
{
  this->IsocenterToMlcLayer1Distance = 2500.;
  this->IsocenterToMlcLayer2Distance = 2500.;
}

//----------------------------------------------------------------------------
vtkMRMLRTIhepIonBeamNode::~vtkMRMLRTIhepIonBeamNode()
{
  this->SetBeamDescription(nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLRTIhepIonBeamNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  // Write all MRML node attributes into output stream
  vtkMRMLWriteXMLFloatMacro( isocenterToMlcLayer1Distance, IsocenterToMlcLayer1Distance);
  vtkMRMLWriteXMLFloatMacro( isocenterToMlcLayer2Distance, IsocenterToMlcLayer2Distance);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLRTIhepIonBeamNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
  
  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLFloatMacro( isocenterToMlcLayer1Distance, IsocenterToMlcLayer1Distance);
  vtkMRMLReadXMLFloatMacro( isocenterToMlcLayer2Distance, IsocenterToMlcLayer2Distance);
  vtkMRMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLRTIhepIonBeamNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  // Do not call Copy function of the direct model base class, as it copies the poly data too,
  // which is undesired for beams, as they generate their own poly data from their properties.
  vtkMRMLDisplayableNode::Copy(anode);

  vtkMRMLRTIhepIonBeamNode* node = vtkMRMLRTIhepIonBeamNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  // Create transform node for beam
  this->CreateNewBeamTransformNode();

  // Add beam in the same plan if beam nodes are in the same scene
  vtkMRMLRTPlanNode* planNode = node->GetParentPlanNode();
  if (planNode && node->GetScene() == this->Scene)
  {
    planNode->AddBeam(this);
  }

  // Copy beam parameters
  this->DisableModifiedEventOn();

  vtkMRMLCopyBeginMacro(node);
  vtkMRMLCopyIntMacro(BeamNumber);
  vtkMRMLCopyStringMacro(BeamDescription);
  vtkMRMLCopyFloatMacro(BeamWeight);
  vtkMRMLCopyFloatMacro(X1Jaw);
  vtkMRMLCopyFloatMacro(X2Jaw);
  vtkMRMLCopyFloatMacro(Y1Jaw);
  vtkMRMLCopyFloatMacro(Y2Jaw);
  vtkMRMLCopyFloatMacro(VSADx);
  vtkMRMLCopyFloatMacro(VSADy);
  vtkMRMLCopyFloatMacro(IsocenterToJawsDistanceX);
  vtkMRMLCopyFloatMacro(IsocenterToJawsDistanceY);
  vtkMRMLCopyFloatMacro(IsocenterToMlcLayer1Distance);
  vtkMRMLCopyFloatMacro(IsocenterToMlcLayer2Distance);
  vtkMRMLCopyFloatMacro(IsocenterToRangeShifterDistance);
  vtkMRMLCopyVectorMacro(ScanningSpotSize, float, 2);
  vtkMRMLCopyFloatMacro(GantryAngle);
  vtkMRMLCopyFloatMacro(CollimatorAngle);
  vtkMRMLCopyFloatMacro(CouchAngle);
  vtkMRMLCopyEndMacro();

  this->EndModify(disabledModify);
  
  this->InvokePendingModifiedEvent();
}

//----------------------------------------------------------------------------
void vtkMRMLRTIhepIonBeamNode::CopyContent(vtkMRMLNode *anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent( anode, deepCopy);

  vtkMRMLRTIhepIonBeamNode* node = vtkMRMLRTIhepIonBeamNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  vtkMRMLCopyBeginMacro(node);
  vtkMRMLCopyIntMacro(BeamNumber);
  vtkMRMLCopyStringMacro(BeamDescription);
  vtkMRMLCopyFloatMacro(BeamWeight);
  vtkMRMLCopyFloatMacro(X1Jaw);
  vtkMRMLCopyFloatMacro(X2Jaw);
  vtkMRMLCopyFloatMacro(Y1Jaw);
  vtkMRMLCopyFloatMacro(Y2Jaw);
  vtkMRMLCopyFloatMacro(VSADx);
  vtkMRMLCopyFloatMacro(VSADy);
  vtkMRMLCopyFloatMacro(IsocenterToJawsDistanceX);
  vtkMRMLCopyFloatMacro(IsocenterToJawsDistanceY);
  vtkMRMLCopyFloatMacro(IsocenterToMlcLayer1Distance);
  vtkMRMLCopyFloatMacro(IsocenterToMlcLayer2Distance);
  vtkMRMLCopyFloatMacro(IsocenterToRangeShifterDistance);
  vtkMRMLCopyVectorMacro(ScanningSpotSize, float, 2);
  vtkMRMLCopyFloatMacro(GantryAngle);
  vtkMRMLCopyFloatMacro(CollimatorAngle);
  vtkMRMLCopyFloatMacro(CouchAngle);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLRTIhepIonBeamNode::SetScene(vtkMRMLScene* scene)
{
  Superclass::SetScene(scene);
}

//----------------------------------------------------------------------------
void vtkMRMLRTIhepIonBeamNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintFloatMacro(IsocenterToMlcLayer1Distance);
  vtkMRMLPrintFloatMacro(IsocenterToMlcLayer2Distance);
  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLRTIhepIonBeamNode::SetIsocenterToMlcLayer1Distance(double distance)
{
  this->IsocenterToMlcLayer1Distance = distance;
  this->Modified();
  this->InvokeCustomModifiedEvent(vtkMRMLRTBeamNode::BeamGeometryModified);
}

//----------------------------------------------------------------------------
void vtkMRMLRTIhepIonBeamNode::SetIsocenterToMlcLayer2Distance(double distance)
{
  this->IsocenterToMlcLayer2Distance = distance;
  this->Modified();
  this->InvokeCustomModifiedEvent(vtkMRMLRTBeamNode::BeamGeometryModified);
}
