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


// Beams includes
#include <vtkMRMLRTBeamNode.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLLinearTransformNode.h>

#include <vtkMRMLSegmentationNode.h>
#include <vtkMRMLScalarVolumeNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>

#include "vtkMRMLIhepStandGeometryNode.h"

//------------------------------------------------------------------------------
namespace
{

const char* BEAM_REFERENCE_ROLE = "beamRef";
const char* PATIENT_BODY_SEGMENTATION_REFERENCE_ROLE = "patientBodySegmentationRef";
const char* PATIENT_BODY_VOLUME_REFERENCE_ROLE = "patientBodyVolumeRef";

} // namespace

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLIhepStandGeometryNode);

//----------------------------------------------------------------------------
vtkMRMLIhepStandGeometryNode::vtkMRMLIhepStandGeometryNode()
  :
  PatientBodySegmentID(nullptr),
  TreatmentMachineType(nullptr),
  PatientSupportRotationAngle(0.),
  TableTopVerticalPosition(0.),
  TableTopVerticalPositionOrigin(0.),
  TableTopVerticalPositionMirror(0.),
  TableTopVerticalPositionMiddle(0.),
  TableTopLongitudinalPosition(0.),
  TableTopLateralPosition(0.),
  TableTopLongitudinalAngle(0.),
  TableTopLateralAngle(0.),
  PatientToTableTopTranslation(),
  UseStandCoordinateSystem(false),
  PatientHeadFeetRotation(false)
{
}

//----------------------------------------------------------------------------
vtkMRMLIhepStandGeometryNode::~vtkMRMLIhepStandGeometryNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLIhepStandGeometryNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream
  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLFloatMacro(PatientSupportRotationAngle, PatientSupportRotationAngle);
  vtkMRMLWriteXMLFloatMacro(TableTopLongitudinalPosition, TableTopLongitudinalPosition);
  vtkMRMLWriteXMLFloatMacro(TableTopLateralPosition, TableTopLateralPosition);
  vtkMRMLWriteXMLFloatMacro(TableTopVerticalPosition, TableTopVerticalPosition);
  vtkMRMLWriteXMLFloatMacro(TableTopVerticalPositionOrigin, TableTopVerticalPositionOrigin);
  vtkMRMLWriteXMLFloatMacro(TableTopVerticalPositionMirror, TableTopVerticalPositionMirror);
  vtkMRMLWriteXMLFloatMacro(TableTopVerticalPositionMiddle, TableTopVerticalPositionMiddle);
  vtkMRMLWriteXMLFloatMacro(TableTopLongitudinalAngle, TableTopLongitudinalAngle);
  vtkMRMLWriteXMLFloatMacro(TableTopLateralAngle, TableTopLateralAngle);
  vtkMRMLWriteXMLVectorMacro(PatientToTableTopTranslation, PatientToTableTopTranslation, double, 3);
  vtkMRMLWriteXMLBooleanMacro(UseStandCoordinateSystem, UseStandCoordinateSystem);
  vtkMRMLWriteXMLBooleanMacro(PatientHeadFeetRotation, PatientHeadFeetRotation);
  // add new parameters here
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLIhepStandGeometryNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  vtkMRMLNode::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLFloatMacro(PatientSupportRotationAngle, PatientSupportRotationAngle);
  vtkMRMLReadXMLFloatMacro(TableTopLongitudinalPosition, TableTopLongitudinalPosition);
  vtkMRMLReadXMLFloatMacro(TableTopLateralPosition, TableTopLateralPosition);
  vtkMRMLReadXMLFloatMacro(TableTopVerticalPosition, TableTopVerticalPosition);
  vtkMRMLReadXMLFloatMacro(TableTopVerticalPositionOrigin, TableTopVerticalPositionOrigin);
  vtkMRMLReadXMLFloatMacro(TableTopVerticalPositionMirror, TableTopVerticalPositionMirror);
  vtkMRMLReadXMLFloatMacro(TableTopVerticalPositionMiddle, TableTopVerticalPositionMiddle);
  vtkMRMLReadXMLFloatMacro(TableTopLongitudinalAngle, TableTopLongitudinalAngle);
  vtkMRMLReadXMLFloatMacro(TableTopLateralAngle, TableTopLateralAngle);
  vtkMRMLReadXMLVectorMacro(PatientToTableTopTranslation, PatientToTableTopTranslation, double, 3);
  vtkMRMLReadXMLBooleanMacro(UseStandCoordinateSystem, UseStandCoordinateSystem);  
  vtkMRMLReadXMLBooleanMacro(PatientHeadFeetRotation, PatientHeadFeetRotation);  
  // add new parameters here
  vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);

  // Note: ReportString is not read from XML, it is a strictly temporary value
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
void vtkMRMLIhepStandGeometryNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  vtkMRMLIhepStandGeometryNode* node = vtkMRMLIhepStandGeometryNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  // Copy parameters
  this->DisableModifiedEventOn();
  vtkMRMLCopyBeginMacro(node);
  vtkMRMLCopyFloatMacro(PatientSupportRotationAngle);
  vtkMRMLCopyFloatMacro(TableTopLongitudinalPosition);
  vtkMRMLCopyFloatMacro(TableTopLateralPosition);
  vtkMRMLCopyFloatMacro(TableTopVerticalPosition);
  vtkMRMLCopyFloatMacro(TableTopVerticalPositionOrigin);
  vtkMRMLCopyFloatMacro(TableTopVerticalPositionMirror);
  vtkMRMLCopyFloatMacro(TableTopVerticalPositionMiddle);
  vtkMRMLCopyFloatMacro(TableTopLongitudinalAngle);
  vtkMRMLCopyFloatMacro(TableTopLateralAngle);
  vtkMRMLCopyVectorMacro(PatientToTableTopTranslation, double, 3);
  vtkMRMLCopyBooleanMacro(UseStandCoordinateSystem);
  vtkMRMLCopyBooleanMacro(PatientHeadFeetRotation);
  // add new parameters here
  vtkMRMLCopyEndMacro(); 

  this->EndModify(disabledModify);

  this->InvokePendingModifiedEvent();
}

//----------------------------------------------------------------------------
void vtkMRMLIhepStandGeometryNode::CopyContent(vtkMRMLNode *anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLIhepStandGeometryNode* node = vtkMRMLIhepStandGeometryNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  vtkMRMLCopyBeginMacro(node);
  vtkMRMLCopyFloatMacro(PatientSupportRotationAngle);
  vtkMRMLCopyFloatMacro(TableTopLongitudinalPosition);
  vtkMRMLCopyFloatMacro(TableTopLateralPosition);
  vtkMRMLCopyFloatMacro(TableTopVerticalPosition);
  vtkMRMLCopyFloatMacro(TableTopVerticalPositionOrigin);
  vtkMRMLCopyFloatMacro(TableTopVerticalPositionMirror);
  vtkMRMLCopyFloatMacro(TableTopVerticalPositionMiddle);
  vtkMRMLCopyFloatMacro(TableTopLongitudinalAngle);
  vtkMRMLCopyFloatMacro(TableTopLateralAngle);
  vtkMRMLCopyVectorMacro(PatientToTableTopTranslation, double, 3);
  vtkMRMLCopyBooleanMacro(UseStandCoordinateSystem);
  vtkMRMLCopyBooleanMacro(PatientHeadFeetRotation);
  // add new parameters here
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLIhepStandGeometryNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintFloatMacro(PatientSupportRotationAngle);
  vtkMRMLPrintFloatMacro(TableTopLongitudinalPosition);
  vtkMRMLPrintFloatMacro(TableTopLateralPosition);
  vtkMRMLPrintFloatMacro(TableTopVerticalPosition);
  vtkMRMLPrintFloatMacro(TableTopVerticalPositionOrigin);
  vtkMRMLPrintFloatMacro(TableTopVerticalPositionMirror);
  vtkMRMLPrintFloatMacro(TableTopVerticalPositionMiddle);
  vtkMRMLPrintFloatMacro(TableTopLongitudinalAngle);
  vtkMRMLPrintFloatMacro(TableTopLateralAngle);
  vtkMRMLPrintVectorMacro(PatientToTableTopTranslation, double, 3);
  vtkMRMLPrintBooleanMacro(UseStandCoordinateSystem);
  vtkMRMLPrintBooleanMacro(PatientHeadFeetRotation);
  // add new parameters here
  vtkMRMLPrintEndMacro(); 
}

void vtkMRMLIhepStandGeometryNode::ResetModelsToInitialPositions()
{
  PatientSupportRotationAngle = 0.;
  TableTopVerticalPosition = 0.;
  TableTopVerticalPositionOrigin = 0.;
  TableTopVerticalPositionMirror = 0.;
  TableTopVerticalPositionMiddle = 0.;
  TableTopLongitudinalPosition = 0.;
  TableTopLateralPosition = 0.;
  TableTopLongitudinalAngle = 0.;
  TableTopLateralAngle = 0.;
  PatientToTableTopTranslation[0] = 0.,
  PatientToTableTopTranslation[1] = 0.,
  PatientToTableTopTranslation[2] = 0.,
  UseStandCoordinateSystem = false;
  PatientHeadFeetRotation = false;
}

//----------------------------------------------------------------------------
vtkMRMLRTBeamNode* vtkMRMLIhepStandGeometryNode::GetBeamNode()
{
  return vtkMRMLRTBeamNode::SafeDownCast( this->GetNodeReference(BEAM_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLIhepStandGeometryNode::SetAndObserveBeamNode(vtkMRMLRTBeamNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("SetAndObserveBeamNode: Cannot set reference, the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(BEAM_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

//----------------------------------------------------------------------------
vtkMRMLSegmentationNode* vtkMRMLIhepStandGeometryNode::GetPatientBodySegmentationNode()
{
  return vtkMRMLSegmentationNode::SafeDownCast( this->GetNodeReference(PATIENT_BODY_SEGMENTATION_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLIhepStandGeometryNode::SetAndObservePatientBodySegmentationNode(vtkMRMLSegmentationNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(PATIENT_BODY_SEGMENTATION_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkMRMLIhepStandGeometryNode::GetReferenceVolumeNode()
{
  return vtkMRMLScalarVolumeNode::SafeDownCast( this->GetNodeReference(PATIENT_BODY_VOLUME_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLIhepStandGeometryNode::SetAndObserveReferenceVolumeNode(vtkMRMLScalarVolumeNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(PATIENT_BODY_VOLUME_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}
