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

#include "vtkMRMLU70RoomGeoNode.h"
#include "vtkMRMLChannel26GeometryNode.h"

//------------------------------------------------------------------------------
namespace
{

const char* FIXED_BEAM_AXIS_REFERENCE_ROLE = "fixedBeamAxisRef";
const char* FIXED_ISOCENTER_REFERENCE_ROLE = "fixedIsocenterRef";
const char* CHANNEL26_GEOMETRY_REFERENCE_ROLE = "channel26GeometryRef";
const char* PATIENT_BODY_SEGMENTATION_REFERENCE_ROLE = "patientBodySegmentationRef";

} // namespace

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLU70RoomGeoNode);

//----------------------------------------------------------------------------
vtkMRMLU70RoomGeoNode::vtkMRMLU70RoomGeoNode()
  :
  TreatmentMachineDescriptorFilePath(nullptr),
  TreatmentMachineType(nullptr)
{
}

//----------------------------------------------------------------------------
vtkMRMLU70RoomGeoNode::~vtkMRMLU70RoomGeoNode()
{
  this->SetTreatmentMachineDescriptorFilePath(nullptr);
  this->SetTreatmentMachineType(nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLU70RoomGeoNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream
  vtkMRMLWriteXMLBeginMacro(of);

  vtkMRMLWriteXMLStringMacro(treatmentMachineType, TreatmentMachineType);
  vtkMRMLWriteXMLStringMacro(treatmentMachineDescriptorFilePath, TreatmentMachineDescriptorFilePath);
  vtkMRMLWriteXMLStringMacro(patientBodySegmentID, PatientBodySegmentID);
  vtkMRMLWriteXMLEnumMacro(u70RoomGeo, U70RoomGeo);

  // add new parameters here
  vtkMRMLWriteXMLEndMacro(); 
}

//----------------------------------------------------------------------------
void vtkMRMLU70RoomGeoNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  vtkMRMLNode::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);

  vtkMRMLReadXMLStringMacro(treatmentMachineType, TreatmentMachineType);
  vtkMRMLReadXMLStringMacro(treatmentMachineDescriptorFilePath, TreatmentMachineDescriptorFilePath);
  vtkMRMLReadXMLStringMacro(patientBodySegmentID, PatientBodySegmentID);
  vtkMRMLReadXMLEnumMacro(u70RoomGeo, U70RoomGeo);

  // add new parameters here
  vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);

  // Note: ReportString is not read from XML, it is a strictly temporary value
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
void vtkMRMLU70RoomGeoNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  vtkMRMLU70RoomGeoNode* node = vtkMRMLU70RoomGeoNode::SafeDownCast(anode);
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
  vtkMRMLCopyEnumMacro(U70RoomGeo);
  // add new parameters here
  vtkMRMLCopyEndMacro(); 

  this->EndModify(disabledModify);

  this->InvokePendingModifiedEvent();
}

//----------------------------------------------------------------------------
void vtkMRMLU70RoomGeoNode::CopyContent(vtkMRMLNode *anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLU70RoomGeoNode* node = vtkMRMLU70RoomGeoNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  vtkMRMLCopyBeginMacro(node);

  vtkMRMLCopyStringMacro(TreatmentMachineType);
  vtkMRMLCopyStringMacro(TreatmentMachineDescriptorFilePath);
  vtkMRMLCopyStringMacro(PatientBodySegmentID);
  vtkMRMLCopyEnumMacro(U70RoomGeo);

  // add new parameters here
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLU70RoomGeoNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);

  vtkMRMLPrintStringMacro(TreatmentMachineType);
  vtkMRMLPrintStringMacro(TreatmentMachineDescriptorFilePath);
  vtkMRMLPrintStringMacro(PatientBodySegmentID);
  vtkMRMLPrintEnumMacro(U70RoomGeo);

  // add new parameters here
  vtkMRMLPrintEndMacro(); 
}

//----------------------------------------------------------------------------
void vtkMRMLU70RoomGeoNode::ProcessMRMLEvents(vtkObject *caller, unsigned long eventID, void *callData)
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
}

//----------------------------------------------------------------------------
vtkMRMLChannel26GeometryNode* vtkMRMLU70RoomGeoNode::GetChannel26GeometryNode()
{
  return vtkMRMLChannel26GeometryNode::SafeDownCast( this->GetNodeReference(CHANNEL26_GEOMETRY_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLU70RoomGeoNode::SetAndObserveChannel26GeometryNode(vtkMRMLChannel26GeometryNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("SetAndObserveCabin26AGeometryNode: Cannot set reference, the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(CHANNEL26_GEOMETRY_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsLineNode* vtkMRMLU70RoomGeoNode::GetBeamAxisLineNode()
{
  return vtkMRMLMarkupsLineNode::SafeDownCast( this->GetNodeReference(FIXED_BEAM_AXIS_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLU70RoomGeoNode::SetAndObserveBeamAxisLineNode(vtkMRMLMarkupsLineNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("SetAndObserveCabin3BeamAxisLineNode: Cannot set reference, the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(FIXED_BEAM_AXIS_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialNode* vtkMRMLU70RoomGeoNode::GetIsocenterFiducialNode()
{
  return vtkMRMLMarkupsFiducialNode::SafeDownCast( this->GetNodeReference(FIXED_ISOCENTER_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLU70RoomGeoNode::SetAndObserveIsocenterFiducialNode(vtkMRMLMarkupsFiducialNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("SetAndObserveCabin3IsocenterFiducialNode: Cannot set reference, the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(FIXED_ISOCENTER_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

//----------------------------------------------------------------------------
vtkMRMLSegmentationNode* vtkMRMLU70RoomGeoNode::GetPatientBodySegmentationNode()
{
  return vtkMRMLSegmentationNode::SafeDownCast( this->GetNodeReference(PATIENT_BODY_SEGMENTATION_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLU70RoomGeoNode::SetAndObservePatientBodySegmentationNode(vtkMRMLSegmentationNode* node)
{
  if (node && this->Scene != node->GetScene())
    {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return;
    }

  this->SetNodeReferenceID(PATIENT_BODY_SEGMENTATION_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

//---------------------------------------------------------------------------
void vtkMRMLU70RoomGeoNode::SetU70RoomGeo(int id)
{
  switch (id)
  {
    case 0: this->SetU70RoomGeo(vtkMRMLU70RoomGeoNode::CABIN1); break;
    case 1: this->SetU70RoomGeo(vtkMRMLU70RoomGeoNode::CABIN2); break;
    case 2:
    default: this->SetU70RoomGeo(vtkMRMLU70RoomGeoNode::CABIN3); break;
  }
}

//---------------------------------------------------------------------------
const char* vtkMRMLU70RoomGeoNode::GetU70RoomGeoAsString(int id)
{
  switch (id)
  {
    case vtkMRMLU70RoomGeoNode::CABIN1: return "CABIN1";
    case vtkMRMLU70RoomGeoNode::CABIN2: return "CABIN2";
    case vtkMRMLU70RoomGeoNode::CABIN3:
    default: return "CABIN3";
  }
}

//---------------------------------------------------------------------------
int vtkMRMLU70RoomGeoNode::GetU70RoomGeoFromString(const char* name)
{
  if (name == nullptr)
  {
    // invalid name
    return -1;
  }
  for (int i = 0; i < vtkMRMLU70RoomGeoNode::U70RoomGeoType_Last; i++)
  {
    if (std::strcmp(name, vtkMRMLU70RoomGeoNode::GetU70RoomGeoAsString(i)) == 0)
    {
      // found a matching name
      return i;
    }
  }
  // unknown name
  return -1;
}
