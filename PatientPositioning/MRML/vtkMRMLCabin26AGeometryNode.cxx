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
#include <vtkMRMLLinearTransformNode.h>

// Beams includes
#include <vtkMRMLRTBeamNode.h>

// Segmentations includes
#include <vtkMRMLSegmentationNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

#include "vtkMRMLCabin26AGeometryNode.h"

namespace {

const char* BASEFIXED_TO_FIXEDREFERENCE_TRANSFORM_NODE_REFERENCE_ROLE = "baseFixedToFixedReferenceTransformRef";
const char* BASEROTATION_TO_BASEFIXED_TRANSFORM_NODE_REFERENCE_ROLE = "baseRotationToBaseFixedTransformRef";
const char* SHOULDER_TO_BASEROTATION_TRANSFORM_NODE_REFERENCE_ROLE = "shoulderToBaseRotationTransformRef";
const char* ELBOW_TO_SHOULDER_TRANSFORM_NODE_REFERENCE_ROLE = "elbowToShoulderTransformRef";
const char* WRIST_TO_ELBOW_TRANSFORM_NODE_REFERENCE_ROLE = "wristToElbowTransformRef";
const char* TABLETOP_TO_WRIST_TRANSFORM_NODE_REFERENCE_ROLE = "tableTopToWristTransformRef";
const char* PATIENT_TO_TABLETOP_TRANSFORM_NODE_REFERENCE_ROLE = "patientToTableTopTransformRef";


}
//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLCabin26AGeometryNode);

//----------------------------------------------------------------------------
vtkMRMLCabin26AGeometryNode::vtkMRMLCabin26AGeometryNode()
  : PatientBodySegmentID(nullptr)
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
vtkMRMLCabin26AGeometryNode::~vtkMRMLCabin26AGeometryNode()
{
  this->SetPatientBodySegmentID(nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLCabin26AGeometryNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream
  vtkMRMLWriteXMLBeginMacro(of);

  vtkMRMLWriteXMLVectorMacro(cArmRobotAngles, CArmRobotAngles, double, 6);
  vtkMRMLWriteXMLVectorMacro(tableTopRobotAngles, TableTopRobotAngles, double, 6);
  vtkMRMLWriteXMLVectorMacro(baseFixedToFixedReferenceTranslation, BaseFixedToFixedReferenceTranslation, double, 3);
  vtkMRMLWriteXMLVectorMacro(cArmBaseFixedToTableTopBaseFixedOffset, CArmBaseFixedToTableTopBaseFixedOffset, double, 3);
  vtkMRMLWriteXMLVectorMacro(patientToTableTopTranslation, PatientToTableTopTranslation, double, 3);
  vtkMRMLWriteXMLFloatMacro(tableTopLateralAngle, TableTopLateralAngle);
  vtkMRMLWriteXMLFloatMacro(tableTopLongitudinalAngle, TableTopLongitudinalAngle);
  vtkMRMLWriteXMLFloatMacro(tableTopVerticalAngle, TableTopVerticalAngle);
  vtkMRMLWriteXMLBooleanMacro(collisionDetectionEnabled, CollisionDetectionEnabled);
  vtkMRMLWriteXMLBooleanMacro(patientHeadFeetRotation, PatientHeadFeetRotation);
  vtkMRMLWriteXMLStringMacro(patientBodySegmentID, PatientBodySegmentID);

  // add new parameters here
  vtkMRMLWriteXMLEndMacro();

}

//----------------------------------------------------------------------------
void vtkMRMLCabin26AGeometryNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  vtkMRMLNode::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);

  vtkMRMLReadXMLVectorMacro(cArmRobotAngles, CArmRobotAngles, double, 6);
  vtkMRMLReadXMLVectorMacro(tableTopRobotAngles, TableTopRobotAngles, double, 6);
  vtkMRMLReadXMLVectorMacro(baseFixedToFixedReferenceTranslation, BaseFixedToFixedReferenceTranslation, double, 3);
  vtkMRMLReadXMLVectorMacro(cArmBaseFixedToTableTopBaseFixedOffset, CArmBaseFixedToTableTopBaseFixedOffset, double, 3);
  vtkMRMLReadXMLVectorMacro(patientToTableTopTranslation, PatientToTableTopTranslation, double, 3);
  vtkMRMLReadXMLFloatMacro(tableTopLateralAngle, TableTopLateralAngle);
  vtkMRMLReadXMLFloatMacro(tableTopLongitudinalAngle, TableTopLongitudinalAngle);
  vtkMRMLReadXMLFloatMacro(tableTopVerticalAngle, TableTopVerticalAngle);
  vtkMRMLReadXMLBooleanMacro(collisionDetectionEnabled, CollisionDetectionEnabled);
  vtkMRMLReadXMLBooleanMacro(patientHeadFeetRotation, PatientHeadFeetRotation);
  vtkMRMLReadXMLStringMacro(patientBodySegmentID, PatientBodySegmentID);

  // add new parameters here
  vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);

  // Note: ReportString is not read from XML, it is a strictly temporary value
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
void vtkMRMLCabin26AGeometryNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  vtkMRMLCabin26AGeometryNode* node = vtkMRMLCabin26AGeometryNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  // Copy beam parameters
  this->DisableModifiedEventOn();

  vtkMRMLCopyBeginMacro(node);

  vtkMRMLCopyVectorMacro(CArmRobotAngles, double, 6);
  vtkMRMLCopyVectorMacro(TableTopRobotAngles, double, 6);
  vtkMRMLCopyVectorMacro(BaseFixedToFixedReferenceTranslation, double, 6);
  vtkMRMLCopyVectorMacro(CArmBaseFixedToTableTopBaseFixedOffset, double, 6);
  vtkMRMLCopyVectorMacro(PatientToTableTopTranslation, double, 3);
  vtkMRMLCopyFloatMacro(TableTopLateralAngle);
  vtkMRMLCopyFloatMacro(TableTopLongitudinalAngle);
  vtkMRMLCopyFloatMacro(TableTopVerticalAngle);
  vtkMRMLCopyBooleanMacro(CollisionDetectionEnabled);
  vtkMRMLCopyBooleanMacro(PatientHeadFeetRotation);
  vtkMRMLCopyStringMacro(PatientBodySegmentID);

  // add new parameters here
  vtkMRMLCopyEndMacro(); 

  this->EndModify(disabledModify);

  this->InvokePendingModifiedEvent();
}

//----------------------------------------------------------------------------
void vtkMRMLCabin26AGeometryNode::CopyContent(vtkMRMLNode *anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLCabin26AGeometryNode* node = vtkMRMLCabin26AGeometryNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  vtkMRMLCopyBeginMacro(node);

  vtkMRMLCopyVectorMacro(CArmRobotAngles, double, 6);
  vtkMRMLCopyVectorMacro(TableTopRobotAngles, double, 6);
  vtkMRMLCopyVectorMacro(BaseFixedToFixedReferenceTranslation, double, 6);
  vtkMRMLCopyVectorMacro(CArmBaseFixedToTableTopBaseFixedOffset, double, 6);
  vtkMRMLCopyVectorMacro(PatientToTableTopTranslation, double, 3);
  vtkMRMLCopyFloatMacro(TableTopLateralAngle);
  vtkMRMLCopyFloatMacro(TableTopLongitudinalAngle);
  vtkMRMLCopyFloatMacro(TableTopVerticalAngle);
  vtkMRMLCopyBooleanMacro(CollisionDetectionEnabled);
  vtkMRMLCopyBooleanMacro(PatientHeadFeetRotation);
  vtkMRMLCopyStringMacro(PatientBodySegmentID);

  // add new parameters here
  vtkMRMLCopyEndMacro();

}

//----------------------------------------------------------------------------
void vtkMRMLCabin26AGeometryNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);

  vtkMRMLPrintVectorMacro(CArmRobotAngles, double, 6);
  vtkMRMLPrintVectorMacro(TableTopRobotAngles, double, 6);
  vtkMRMLPrintVectorMacro(BaseFixedToFixedReferenceTranslation, double, 6);
  vtkMRMLPrintVectorMacro(CArmBaseFixedToTableTopBaseFixedOffset, double, 6);
  vtkMRMLPrintVectorMacro(PatientToTableTopTranslation, double, 3);
  vtkMRMLPrintFloatMacro(TableTopLateralAngle);
  vtkMRMLPrintFloatMacro(TableTopLongitudinalAngle);
  vtkMRMLPrintFloatMacro(TableTopVerticalAngle);
  vtkMRMLPrintBooleanMacro(CollisionDetectionEnabled);
  vtkMRMLPrintBooleanMacro(PatientHeadFeetRotation);
  vtkMRMLPrintStringMacro(PatientBodySegmentID);

  // add new parameters here
  vtkMRMLPrintEndMacro(); 

}

//----------------------------------------------------------------------------
void vtkMRMLCabin26AGeometryNode::ProcessMRMLEvents(vtkObject *caller, unsigned long eventID, void *callData)
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

  // Update the geomtry if beam geometry or transform was changed
  switch (eventID)
  {
  case vtkMRMLRTBeamNode::BeamGeometryModified:
  case vtkMRMLRTBeamNode::BeamTransformModified:
    break;
  default:
    break;
  }
}

vtkMRMLLinearTransformNode* vtkMRMLCabin26AGeometryNode::GetBaseFixedToFixedReferenceTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(BASEFIXED_TO_FIXEDREFERENCE_TRANSFORM_NODE_REFERENCE_ROLE));
}

void vtkMRMLCabin26AGeometryNode::SetAndObserveBaseFixedToFixedReferenceTransformNode(vtkMRMLLinearTransformNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(BASEFIXED_TO_FIXEDREFERENCE_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

vtkMRMLLinearTransformNode* vtkMRMLCabin26AGeometryNode::GetBaseRotationToBaseFixedTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(BASEROTATION_TO_BASEFIXED_TRANSFORM_NODE_REFERENCE_ROLE));
}

void vtkMRMLCabin26AGeometryNode::SetAndObserveBaseRotationToBaseFixedTransformNode(vtkMRMLLinearTransformNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(BASEROTATION_TO_BASEFIXED_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

vtkMRMLLinearTransformNode* vtkMRMLCabin26AGeometryNode::GetShoulderToBaseRotationTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(SHOULDER_TO_BASEROTATION_TRANSFORM_NODE_REFERENCE_ROLE));
}

void vtkMRMLCabin26AGeometryNode::SetAndObserveShoulderToBaseRotationTransformNode(vtkMRMLLinearTransformNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(SHOULDER_TO_BASEROTATION_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

vtkMRMLLinearTransformNode* vtkMRMLCabin26AGeometryNode::GetElbowToShoulderTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(ELBOW_TO_SHOULDER_TRANSFORM_NODE_REFERENCE_ROLE));
}

void vtkMRMLCabin26AGeometryNode::SetAndObserveElbowToShoulderTransformNode(vtkMRMLLinearTransformNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(ELBOW_TO_SHOULDER_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

vtkMRMLLinearTransformNode* vtkMRMLCabin26AGeometryNode::GetWristToElbowTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(WRIST_TO_ELBOW_TRANSFORM_NODE_REFERENCE_ROLE));
}

void vtkMRMLCabin26AGeometryNode::SetAndObserveWristToElbowTransformNode(vtkMRMLLinearTransformNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(WRIST_TO_ELBOW_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

vtkMRMLLinearTransformNode* vtkMRMLCabin26AGeometryNode::GetTableTopToWristTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(TABLETOP_TO_WRIST_TRANSFORM_NODE_REFERENCE_ROLE));
}

void vtkMRMLCabin26AGeometryNode::SetAndObserveTableTopToWristTransformNode(vtkMRMLLinearTransformNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(TABLETOP_TO_WRIST_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

vtkMRMLLinearTransformNode* vtkMRMLCabin26AGeometryNode::GetPatientToTableTopTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(PATIENT_TO_TABLETOP_TRANSFORM_NODE_REFERENCE_ROLE));
}

void vtkMRMLCabin26AGeometryNode::SetAndObservePatientToTableTopTransformNode(vtkMRMLLinearTransformNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(PATIENT_TO_TABLETOP_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}
