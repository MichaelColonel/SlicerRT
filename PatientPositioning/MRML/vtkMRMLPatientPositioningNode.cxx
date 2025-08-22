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
#include <vtkTransform.h>

#include "vtkMRMLPatientPositioningNode.h"
#include "vtkMRMLChannel26GeometryNode.h"
#include "vtkMRMLRTCarmBeamNode.h"
#include "vtkMRMLRTChannel26Cabin3BeamNode.h"
#include "vtkMRMLDrrImageComputationNode.h"

//------------------------------------------------------------------------------
namespace
{

const char* DRR_REFERENCE_ROLE = "drrRef";
const char* DRR_IMAGE_REFERENCE_ROLE = "drrImageRef";
const char* CARM_XRAY_IMAGE_REFERENCE_ROLE = "carmXrayImageRef";
const char* FIXED_BEAM_AXIS_REFERENCE_ROLE = "fixedBeamAxisRef";
const char* FIXED_ISOCENTER_REFERENCE_ROLE = "fixedIsocenterRef";
const char* CHANNEL26_GEOMETRY_REFERENCE_ROLE = "channel26GeometryRef";
const char* FIXED_ION_BEAM_REFERENCE_ROLE = "fixedIonBeamRef";
const char* CARM_XRAY_BEAM_REFERENCE_ROLE = "carmXrayBeamRef";
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
vtkMRMLChannel26GeometryNode* vtkMRMLPatientPositioningNode::GetChannel26GeometryNode()
{
  return vtkMRMLChannel26GeometryNode::SafeDownCast( this->GetNodeReference(CHANNEL26_GEOMETRY_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLPatientPositioningNode::SetAndObserveChannel26GeometryNode(vtkMRMLChannel26GeometryNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("SetAndObserveCabin26AGeometryNode: Cannot set reference, the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(CHANNEL26_GEOMETRY_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsLineNode* vtkMRMLPatientPositioningNode::GetCabin3BeamAxisLineNode()
{
  return vtkMRMLMarkupsLineNode::SafeDownCast( this->GetNodeReference(FIXED_BEAM_AXIS_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLPatientPositioningNode::SetAndObserveCabin3BeamAxisLineNode(vtkMRMLMarkupsLineNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("SetAndObserveCabin3BeamAxisLineNode: Cannot set reference, the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(FIXED_BEAM_AXIS_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialNode* vtkMRMLPatientPositioningNode::GetCabin3IsocenterFiducialNode()
{
  return vtkMRMLMarkupsFiducialNode::SafeDownCast( this->GetNodeReference(FIXED_ISOCENTER_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLPatientPositioningNode::SetAndObserveCabin3IsocenterFiducialNode(vtkMRMLMarkupsFiducialNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("SetAndObserveCabin3IsocenterFiducialNode: Cannot set reference, the referenced and referencing node are not in the same scene");
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
vtkMRMLRTChannel26Cabin3BeamNode* vtkMRMLPatientPositioningNode::GetFixedReferenceBeamNode()
{
  return vtkMRMLRTChannel26Cabin3BeamNode::SafeDownCast( this->GetNodeReference(FIXED_ION_BEAM_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLPatientPositioningNode::SetAndObserveFixedReferenceBeamNode(vtkMRMLRTChannel26Cabin3BeamNode* node)
{
  if (node && this->Scene != node->GetScene())
    {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return;
    }

  this->SetNodeReferenceID(FIXED_ION_BEAM_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

//----------------------------------------------------------------------------
vtkMRMLRTBeamNode* vtkMRMLPatientPositioningNode::GetCarmXrayBeamNode()
{
  return vtkMRMLRTBeamNode::SafeDownCast( this->GetNodeReference(CARM_XRAY_BEAM_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLPatientPositioningNode::SetAndObserveCarmXrayBeamNode(vtkMRMLRTBeamNode* node)
{
  if (node && this->Scene != node->GetScene())
    {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return;
    }

  this->SetNodeReferenceID(CARM_XRAY_BEAM_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

//----------------------------------------------------------------------------
vtkMRMLDrrImageComputationNode* vtkMRMLPatientPositioningNode::GetDrrComputationNode()
{
  return vtkMRMLDrrImageComputationNode::SafeDownCast( this->GetNodeReference(DRR_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLPatientPositioningNode::SetAndObserveDrrComputationNode(vtkMRMLDrrImageComputationNode* node)
{
  if (node && this->Scene != node->GetScene())
    {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return;
    }

  this->SetNodeReferenceID(DRR_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

//----------------------------------------------------------------------------
vtkTransform* vtkMRMLPatientPositioningNode::GetRegistrationTransform(CarmProjectionOrientation proj)
{
  return this->OrientationTransformMatrixMap[proj];
}

//----------------------------------------------------------------------------
void vtkMRMLPatientPositioningNode::SetRegistrationTransform(CarmProjectionOrientation proj,
  vtkTransform* transform)
{
  this->OrientationTransformMatrixMap[proj] = transform;
}

//----------------------------------------------------------------------------
bool vtkMRMLPatientPositioningNode::GetRegistrationImages(CarmProjectionOrientation proj,
  vtkMRMLScalarVolumeNode* staticImage, vtkMRMLScalarVolumeNode* movedImage)
{
  auto pair = this->OrientationImagesMap[proj];
  staticImage = pair.first;
  movedImage = pair.second;
  return false;
}

//----------------------------------------------------------------------------
void vtkMRMLPatientPositioningNode::SetRegistrationImages(CarmProjectionOrientation proj,
  vtkMRMLScalarVolumeNode* staticImage, vtkMRMLScalarVolumeNode* moveImage)
{
  this->OrientationImagesMap[proj] = { staticImage, moveImage };
}
