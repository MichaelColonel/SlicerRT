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

#include "vtkMRMLChannel26GeometryNode.h"

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
vtkMRMLNodeNewMacro(vtkMRMLChannel26GeometryNode);

//----------------------------------------------------------------------------
vtkMRMLChannel26GeometryNode::vtkMRMLChannel26GeometryNode()
  : PatientBodySegmentID(nullptr)
{
}

//----------------------------------------------------------------------------
vtkMRMLChannel26GeometryNode::~vtkMRMLChannel26GeometryNode()
{
  this->SetPatientBodySegmentID(nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLChannel26GeometryNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream
  vtkMRMLWriteXMLBeginMacro(of);

  vtkMRMLWriteXMLVectorMacro(carmRobotAngles, CarmRobotAngles, double, 6);
  vtkMRMLWriteXMLVectorMacro(tableRobotAngles, TableRobotAngles, double, 6);
  vtkMRMLWriteXMLVectorMacro(tableBaseFixedToFixedReferenceTranslation, TableBaseFixedToFixedReferenceTranslation, double, 3);
  vtkMRMLWriteXMLVectorMacro(carmBaseFixedToTableTopBaseFixedOffset, CarmBaseFixedToTableBaseFixedOffset, double, 3);
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
void vtkMRMLChannel26GeometryNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  vtkMRMLNode::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);

  vtkMRMLReadXMLVectorMacro(carmRobotAngles, CarmRobotAngles, double, 6);
  vtkMRMLReadXMLVectorMacro(tableRobotAngles, TableRobotAngles, double, 6);
  vtkMRMLReadXMLVectorMacro(tableBaseFixedToFixedReferenceTranslation, TableBaseFixedToFixedReferenceTranslation, double, 3);
  vtkMRMLReadXMLVectorMacro(carmBaseFixedToTableBaseFixedOffset, CarmBaseFixedToTableBaseFixedOffset, double, 3);
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
void vtkMRMLChannel26GeometryNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  vtkMRMLChannel26GeometryNode* node = vtkMRMLChannel26GeometryNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  // Copy beam parameters
  this->DisableModifiedEventOn();

  vtkMRMLCopyBeginMacro(node);

  vtkMRMLCopyVectorMacro(CarmRobotAngles, double, 6);
  vtkMRMLCopyVectorMacro(TableRobotAngles, double, 6);
  vtkMRMLCopyVectorMacro(TableBaseFixedToFixedReferenceTranslation, double, 6);
  vtkMRMLCopyVectorMacro(CarmBaseFixedToTableBaseFixedOffset, double, 6);
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
void vtkMRMLChannel26GeometryNode::CopyContent(vtkMRMLNode *anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLChannel26GeometryNode* node = vtkMRMLChannel26GeometryNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  vtkMRMLCopyBeginMacro(node);

  vtkMRMLCopyVectorMacro(CarmRobotAngles, double, 6);
  vtkMRMLCopyVectorMacro(TableRobotAngles, double, 6);
  vtkMRMLCopyVectorMacro(TableBaseFixedToFixedReferenceTranslation, double, 6);
  vtkMRMLCopyVectorMacro(CarmBaseFixedToTableBaseFixedOffset, double, 6);
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
void vtkMRMLChannel26GeometryNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);

  vtkMRMLPrintVectorMacro(CarmRobotAngles, double, 6);
  vtkMRMLPrintVectorMacro(TableRobotAngles, double, 6);
  vtkMRMLPrintVectorMacro(TableBaseFixedToFixedReferenceTranslation, double, 6);
  vtkMRMLPrintVectorMacro(CarmBaseFixedToTableBaseFixedOffset, double, 6);
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
void vtkMRMLChannel26GeometryNode::ProcessMRMLEvents(vtkObject *caller, unsigned long eventID, void *callData)
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

vtkMRMLLinearTransformNode* vtkMRMLChannel26GeometryNode::GetBaseFixedToFixedReferenceTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(BASEFIXED_TO_FIXEDREFERENCE_TRANSFORM_NODE_REFERENCE_ROLE));
}

void vtkMRMLChannel26GeometryNode::SetAndObserveBaseFixedToFixedReferenceTransformNode(vtkMRMLLinearTransformNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(BASEFIXED_TO_FIXEDREFERENCE_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

vtkMRMLLinearTransformNode* vtkMRMLChannel26GeometryNode::GetBaseRotationToBaseFixedTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(BASEROTATION_TO_BASEFIXED_TRANSFORM_NODE_REFERENCE_ROLE));
}

void vtkMRMLChannel26GeometryNode::SetAndObserveBaseRotationToBaseFixedTransformNode(vtkMRMLLinearTransformNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(BASEROTATION_TO_BASEFIXED_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

vtkMRMLLinearTransformNode* vtkMRMLChannel26GeometryNode::GetShoulderToBaseRotationTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(SHOULDER_TO_BASEROTATION_TRANSFORM_NODE_REFERENCE_ROLE));
}

void vtkMRMLChannel26GeometryNode::SetAndObserveShoulderToBaseRotationTransformNode(vtkMRMLLinearTransformNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(SHOULDER_TO_BASEROTATION_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

vtkMRMLLinearTransformNode* vtkMRMLChannel26GeometryNode::GetElbowToShoulderTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(ELBOW_TO_SHOULDER_TRANSFORM_NODE_REFERENCE_ROLE));
}

void vtkMRMLChannel26GeometryNode::SetAndObserveElbowToShoulderTransformNode(vtkMRMLLinearTransformNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(ELBOW_TO_SHOULDER_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

vtkMRMLLinearTransformNode* vtkMRMLChannel26GeometryNode::GetWristToElbowTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(WRIST_TO_ELBOW_TRANSFORM_NODE_REFERENCE_ROLE));
}

void vtkMRMLChannel26GeometryNode::SetAndObserveWristToElbowTransformNode(vtkMRMLLinearTransformNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(WRIST_TO_ELBOW_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

vtkMRMLLinearTransformNode* vtkMRMLChannel26GeometryNode::GetTableTopToWristTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(TABLETOP_TO_WRIST_TRANSFORM_NODE_REFERENCE_ROLE));
}

void vtkMRMLChannel26GeometryNode::SetAndObserveTableTopToWristTransformNode(vtkMRMLLinearTransformNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(TABLETOP_TO_WRIST_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

vtkMRMLLinearTransformNode* vtkMRMLChannel26GeometryNode::GetPatientToTableTopTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(PATIENT_TO_TABLETOP_TRANSFORM_NODE_REFERENCE_ROLE));
}

void vtkMRMLChannel26GeometryNode::SetAndObservePatientToTableTopTransformNode(vtkMRMLLinearTransformNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(PATIENT_TO_TABLETOP_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}
