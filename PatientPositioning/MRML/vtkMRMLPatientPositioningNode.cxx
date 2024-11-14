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

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLMarkupsLineNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLSegmentationNode.h>

// Beams includes
#include <vtkMRMLRTBeamNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

#include "vtkMRMLPatientPositioningNode.h"
#include "vtkMRMLCabin26AGeometryNode.h"
#include "vtkMRMLRTFixedBeamNode.h"
#include "vtkMRMLRTCabin26AIonBeamNode.h"

//------------------------------------------------------------------------------
namespace
{

const char* DRR_REFERENCE_ROLE = "drrRef";
const char* XRAY_IMAGE_REFERENCE_ROLE = "xrayImageRef";
const char* FIXED_BEAM_AXIS_REFERENCE_ROLE = "fixedBeamAxisRef";
const char* FIXED_ISOCENTER_REFERENCE_ROLE = "fixedIsocenterRef";
const char* CABIN26A_GEOMETRY_REFERENCE_ROLE = "cabin26AGeometryRef";
const char* FIXED_ION_BEAM_REFERENCE_ROLE = "fixedIonBeamRef";
const char* EXTERNAL_XRAY_BEAM_REFERENCE_ROLE = "externalXrayBeamRef";
const char* BEAM_REFERENCE_ROLE = "beamRef";
const char* PATIENT_BODY_SEGMENTATION_REFERENCE_ROLE = "patientBodySegmentationRef";

} // namespace

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLPatientPositioningNode);

//----------------------------------------------------------------------------
vtkMRMLPatientPositioningNode::vtkMRMLPatientPositioningNode()
  :
  TreatmentMachineDescriptorFilePath(nullptr)
  , TreatmentMachineType(nullptr)
{
  // Observe RTBeam node events (like change of transform or geometry)
///  vtkNew<vtkIntArray> nodeEvents;
///  nodeEvents->InsertNextValue(vtkCommand::ModifiedEvent);
//  nodeEvents->InsertNextValue(vtkMRMLRTBeamNode::BeamGeometryModified);
//  nodeEvents->InsertNextValue(vtkMRMLRTBeamNode::BeamTransformModified);
///  this->AddNodeReferenceRole(DRR_REFERENCE_ROLE, nullptr, nodeEvents);
///  this->AddNodeReferenceRole(XRAY_IMAGE_REFERENCE_ROLE, nullptr, nodeEvents);
}

//----------------------------------------------------------------------------
vtkMRMLPatientPositioningNode::~vtkMRMLPatientPositioningNode()
{
  this->SetTreatmentMachineDescriptorFilePath(nullptr);
  this->SetTreatmentMachineType(nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLPatientPositioningNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream
  vtkMRMLWriteXMLBeginMacro(of);

  vtkMRMLWriteXMLStringMacro(treatmentMachineType, TreatmentMachineType);
  vtkMRMLWriteXMLStringMacro(treatmentMachineDescriptorFilePath, TreatmentMachineDescriptorFilePath);
  vtkMRMLWriteXMLStringMacro(patientBodySegmentID, PatientBodySegmentID);

  // add new parameters here
  vtkMRMLWriteXMLEndMacro(); 
}

//----------------------------------------------------------------------------
void vtkMRMLPatientPositioningNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  vtkMRMLNode::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);

  vtkMRMLReadXMLStringMacro(treatmentMachineType, TreatmentMachineType);
  vtkMRMLReadXMLStringMacro(treatmentMachineDescriptorFilePath, TreatmentMachineDescriptorFilePath);
  vtkMRMLReadXMLStringMacro(patientBodySegmentID, PatientBodySegmentID);

  // add new parameters here
  vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);

  // Note: ReportString is not read from XML, it is a strictly temporary value
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
void vtkMRMLPatientPositioningNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  vtkMRMLPatientPositioningNode* node = vtkMRMLPatientPositioningNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  // Copy beam parameters
  this->DisableModifiedEventOn();

  vtkMRMLCopyBeginMacro(node);
  vtkMRMLCopyStringMacro(TreatmentMachineType);
  vtkMRMLCopyStringMacro(TreatmentMachineDescriptorFilePath);
  vtkMRMLCopyStringMacro(PatientBodySegmentID);
  // add new parameters here
  vtkMRMLCopyEndMacro(); 

  this->EndModify(disabledModify);

  this->InvokePendingModifiedEvent();
}

//----------------------------------------------------------------------------
void vtkMRMLPatientPositioningNode::CopyContent(vtkMRMLNode *anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLPatientPositioningNode* node = vtkMRMLPatientPositioningNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  vtkMRMLCopyBeginMacro(node);

  vtkMRMLCopyStringMacro(TreatmentMachineType);
  vtkMRMLCopyStringMacro(TreatmentMachineDescriptorFilePath);
  vtkMRMLCopyStringMacro(PatientBodySegmentID);

  // add new parameters here
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLPatientPositioningNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);

  vtkMRMLPrintStringMacro(TreatmentMachineType);
  vtkMRMLPrintStringMacro(TreatmentMachineDescriptorFilePath);
  vtkMRMLPrintStringMacro(PatientBodySegmentID);

  // add new parameters here
  vtkMRMLPrintEndMacro(); 
}

//----------------------------------------------------------------------------
void vtkMRMLPatientPositioningNode::ProcessMRMLEvents(vtkObject *caller, unsigned long eventID, void *callData)
{
  Superclass::ProcessMRMLEvents(caller, eventID, callData);

  if (!this->Scene)
  {
    vtkErrorMacro("ProcessMRMLEvents: Invalid MRML scene");
    return;
  }
  if (this->Scene->IsBatchProcessing())
  {
    return;
  }

  // Update the DRR View-Up and normal vectors, if beam geometry or transform was changed
  switch (eventID)
  {
//  case vtkMRMLRTBeamNode::BeamGeometryModified:
//  case vtkMRMLRTBeamNode::BeamTransformModified:
//    this->Modified();
//    break;
  default:
    break;
  }
}

//----------------------------------------------------------------------------
vtkMRMLCabin26AGeometryNode* vtkMRMLPatientPositioningNode::GetCabin26AGeometryNode()
{
  return vtkMRMLCabin26AGeometryNode::SafeDownCast( this->GetNodeReference(CABIN26A_GEOMETRY_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLPatientPositioningNode::SetAndObserveCabin26AGeometryNode(vtkMRMLCabin26AGeometryNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("SetAndObserveCabin26AGeometryNode: Cannot set reference, the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(CABIN26A_GEOMETRY_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsLineNode* vtkMRMLPatientPositioningNode::GetFixedBeamAxisLineNode()
{
  return vtkMRMLMarkupsLineNode::SafeDownCast( this->GetNodeReference(FIXED_BEAM_AXIS_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLPatientPositioningNode::SetAndObserveFixedBeamAxisLineNode(vtkMRMLMarkupsLineNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("SetAndObserveFixedBeamAxisLineNode: Cannot set reference, the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(FIXED_BEAM_AXIS_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialNode* vtkMRMLPatientPositioningNode::GetFixedIsocenterFiducialNode()
{
  return vtkMRMLMarkupsFiducialNode::SafeDownCast( this->GetNodeReference(FIXED_ISOCENTER_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLPatientPositioningNode::SetAndObserveFixedIsocenterFiducialNode(vtkMRMLMarkupsFiducialNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("SetAndObserveFixedBeamAxisLineNode: Cannot set reference, the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(FIXED_ISOCENTER_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}


//----------------------------------------------------------------------------
vtkMRMLSegmentationNode* vtkMRMLPatientPositioningNode::GetPatientBodySegmentationNode()
{
  return vtkMRMLSegmentationNode::SafeDownCast( this->GetNodeReference(PATIENT_BODY_SEGMENTATION_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLPatientPositioningNode::SetAndObservePatientBodySegmentationNode(vtkMRMLSegmentationNode* node)
{
  if (node && this->Scene != node->GetScene())
    {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return;
    }

  this->SetNodeReferenceID(PATIENT_BODY_SEGMENTATION_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

//----------------------------------------------------------------------------
vtkMRMLRTBeamNode* vtkMRMLPatientPositioningNode::GetBeamNode()
{
  return vtkMRMLRTBeamNode::SafeDownCast( this->GetNodeReference(BEAM_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLPatientPositioningNode::SetAndObserveBeamNode(vtkMRMLRTBeamNode* node)
{
  if (node && this->Scene != node->GetScene())
    {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return;
    }

  this->SetNodeReferenceID(BEAM_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

//----------------------------------------------------------------------------
vtkMRMLRTCabin26AIonBeamNode* vtkMRMLPatientPositioningNode::GetFixedReferenceBeamNode()
{
  return vtkMRMLRTCabin26AIonBeamNode::SafeDownCast( this->GetNodeReference(FIXED_ION_BEAM_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLPatientPositioningNode::SetAndObserveFixedReferenceBeamNode(vtkMRMLRTCabin26AIonBeamNode* node)
{
  if (node && this->Scene != node->GetScene())
    {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return;
    }

  this->SetNodeReferenceID(FIXED_ION_BEAM_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

//----------------------------------------------------------------------------
vtkMRMLRTFixedBeamNode* vtkMRMLPatientPositioningNode::GetExternalXrayBeamNode()
{
  return vtkMRMLRTFixedBeamNode::SafeDownCast( this->GetNodeReference(EXTERNAL_XRAY_BEAM_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLPatientPositioningNode::SetAndObserveExternalXrayBeamNode(vtkMRMLRTFixedBeamNode* node)
{
  if (node && this->Scene != node->GetScene())
    {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return;
    }

  this->SetNodeReferenceID(EXTERNAL_XRAY_BEAM_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}
