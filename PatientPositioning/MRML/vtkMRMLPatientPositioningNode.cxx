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

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

#include "vtkMRMLPatientPositioningNode.h"

//------------------------------------------------------------------------------
namespace
{

const char* DRR_REFERENCE_ROLE = "drrRef";
const char* XRAY_IMAGE_REFERENCE_ROLE = "xrayImageRef";

} // namespace

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLPatientPositioningNode);

//----------------------------------------------------------------------------
vtkMRMLPatientPositioningNode::vtkMRMLPatientPositioningNode()
{
  // Observe RTBeam node events (like change of transform or geometry)
  vtkNew<vtkIntArray> nodeEvents;
  nodeEvents->InsertNextValue(vtkCommand::ModifiedEvent);
//  nodeEvents->InsertNextValue(vtkMRMLRTBeamNode::BeamGeometryModified);
//  nodeEvents->InsertNextValue(vtkMRMLRTBeamNode::BeamTransformModified);
  this->AddNodeReferenceRole(DRR_REFERENCE_ROLE, nullptr, nodeEvents);
  this->AddNodeReferenceRole(XRAY_IMAGE_REFERENCE_ROLE, nullptr, nodeEvents);
}

//----------------------------------------------------------------------------
vtkMRMLPatientPositioningNode::~vtkMRMLPatientPositioningNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLPatientPositioningNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream
  vtkMRMLWriteXMLBeginMacro(of);
/*
  vtkMRMLWriteXMLVectorMacro(NormalVector, NormalVector, double, 3);
  vtkMRMLWriteXMLVectorMacro(ViewUpVector, ViewUpVector, double, 3);
  vtkMRMLWriteXMLFloatMacro(IsocenterImagerDistance, IsocenterImagerDistance);
  vtkMRMLWriteXMLVectorMacro(ImagerCenterOffset, ImagerCenterOffset, double, 2);
  vtkMRMLWriteXMLVectorMacro(ImagerResolution, ImagerResolution, int, 2);
  vtkMRMLWriteXMLVectorMacro(ImagerSpacing, ImagerSpacing, double, 2);
  vtkMRMLWriteXMLVectorMacro(ImageCenter, ImageCenter, int, 2);
  vtkMRMLWriteXMLBooleanMacro(ImageWindowFlag, ImageWindowFlag);
  vtkMRMLWriteXMLVectorMacro(ImageWindow, ImageWindow, int, 4);
  vtkMRMLWriteXMLBooleanMacro(ExponentialMappingFlag, ExponentialMappingFlag);
  vtkMRMLWriteXMLBooleanMacro(AutoscaleFlag, AutoscaleFlag);
  vtkMRMLWriteXMLBooleanMacro(InvertIntensityFlag, InvertIntensityFlag);
  vtkMRMLWriteXMLVectorMacro(AutoscaleRange, AutoscaleRange, float, 2); 
  vtkMRMLWriteXMLIntMacro(AlgorithmReconstuction, AlgorithmReconstuction);
  vtkMRMLWriteXMLIntMacro(HUConversion, HUConversion);
  vtkMRMLWriteXMLIntMacro(HUThresholdBelow, HUThresholdBelow);
  vtkMRMLWriteXMLIntMacro(Threading, Threading);
*/
  // add new parameters here
  vtkMRMLWriteXMLEndMacro(); 
}

//----------------------------------------------------------------------------
void vtkMRMLPatientPositioningNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  vtkMRMLNode::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
/*
  vtkMRMLReadXMLVectorMacro(NormalVector, NormalVector, double, 3);
  vtkMRMLReadXMLVectorMacro(ViewUpVector, ViewUpVector, double, 3);
  vtkMRMLReadXMLFloatMacro(IsocenterImagerDistance, IsocenterImagerDistance);
  vtkMRMLReadXMLVectorMacro(ImagerCenterOffset, ImagerCenterOffset, double, 2);
  vtkMRMLReadXMLVectorMacro(ImagerResolution, ImagerResolution, int, 2);
  vtkMRMLReadXMLVectorMacro(ImagerSpacing, ImagerSpacing, double, 2);
  vtkMRMLReadXMLVectorMacro(ImageCenter, ImageCenter, int, 2);
  vtkMRMLReadXMLBooleanMacro(ImageWindowFlag, ImageWindowFlag);
  vtkMRMLReadXMLVectorMacro(ImageWindow, ImageWindow, int, 4);
  vtkMRMLReadXMLBooleanMacro(ExponentialMappingFlag, ExponentialMappingFlag);
  vtkMRMLReadXMLBooleanMacro(AutoscaleFlag, AutoscaleFlag);
  vtkMRMLReadXMLBooleanMacro(InvertIntensityFlag, InvertIntensityFlag);
  vtkMRMLReadXMLVectorMacro(AutoscaleRange, AutoscaleRange, float, 2);
  vtkMRMLReadXMLIntMacro(AlgorithmReconstuction, AlgorithmReconstuction);
  vtkMRMLReadXMLIntMacro(HUConversion, HUConversion);
  vtkMRMLReadXMLIntMacro(HUThresholdBelow, HUThresholdBelow);
  vtkMRMLReadXMLIntMacro(Threading, Threading);
*/
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
/*
  vtkMRMLCopyVectorMacro(NormalVector, double, 3);
  vtkMRMLCopyVectorMacro(ViewUpVector, double, 3);
  vtkMRMLCopyFloatMacro(IsocenterImagerDistance);
  vtkMRMLCopyVectorMacro(ImagerCenterOffset, double, 2);
  vtkMRMLCopyVectorMacro(ImagerResolution, int, 2);
  vtkMRMLCopyVectorMacro(ImagerSpacing, double, 2);
  vtkMRMLCopyVectorMacro(ImageCenter, int, 2);
  vtkMRMLCopyBooleanMacro(ImageWindowFlag);
  vtkMRMLCopyVectorMacro(ImageWindow, int, 4);
  vtkMRMLCopyBooleanMacro(ExponentialMappingFlag);
  vtkMRMLCopyBooleanMacro(AutoscaleFlag);
  vtkMRMLCopyBooleanMacro(InvertIntensityFlag);
  vtkMRMLCopyVectorMacro(AutoscaleRange, float, 2);
  vtkMRMLCopyIntMacro(AlgorithmReconstuction);
  vtkMRMLCopyIntMacro(HUConversion);
  vtkMRMLCopyIntMacro(HUThresholdBelow);
  vtkMRMLCopyIntMacro(Threading);
*/
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
/*
  vtkMRMLCopyVectorMacro(NormalVector, double, 3);
  vtkMRMLCopyVectorMacro(ViewUpVector, double, 3);
  vtkMRMLCopyFloatMacro(IsocenterImagerDistance);
  vtkMRMLCopyVectorMacro(ImagerCenterOffset, double, 2);
  vtkMRMLCopyVectorMacro(ImagerResolution, int, 2);
  vtkMRMLCopyVectorMacro(ImagerSpacing, double, 2);
  vtkMRMLCopyVectorMacro(ImageCenter, int, 2);
  vtkMRMLCopyBooleanMacro(ImageWindowFlag);
  vtkMRMLCopyVectorMacro(ImageWindow, int, 4);
  vtkMRMLCopyBooleanMacro(ExponentialMappingFlag);
  vtkMRMLCopyBooleanMacro(AutoscaleFlag);
  vtkMRMLCopyBooleanMacro(InvertIntensityFlag);
  vtkMRMLCopyVectorMacro(AutoscaleRange, float, 2);
  vtkMRMLCopyIntMacro(AlgorithmReconstuction);
  vtkMRMLCopyIntMacro(HUConversion);
  vtkMRMLCopyIntMacro(HUThresholdBelow);
  vtkMRMLCopyIntMacro(Threading);
*/
  // add new parameters here
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLPatientPositioningNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);
/*
  vtkMRMLPrintVectorMacro(NormalVector, double, 3);
  vtkMRMLPrintVectorMacro(ViewUpVector, double, 3);
  vtkMRMLPrintFloatMacro(IsocenterImagerDistance);
  vtkMRMLPrintVectorMacro(ImagerCenterOffset, double, 2);
  vtkMRMLPrintVectorMacro(ImagerResolution, int, 2);
  vtkMRMLPrintVectorMacro(ImagerSpacing, double, 2);
  vtkMRMLPrintVectorMacro(ImageCenter, int, 2);
  vtkMRMLPrintBooleanMacro(ImageWindowFlag);
  vtkMRMLPrintVectorMacro(ImageWindow, int, 4);
  vtkMRMLPrintBooleanMacro(ExponentialMappingFlag);
  vtkMRMLPrintBooleanMacro(AutoscaleFlag);
  vtkMRMLPrintBooleanMacro(InvertIntensityFlag);
  vtkMRMLPrintVectorMacro(AutoscaleRange, float, 2);
  vtkMRMLPrintIntMacro(AlgorithmReconstuction);
  vtkMRMLPrintIntMacro(HUConversion);
  vtkMRMLPrintIntMacro(HUThresholdBelow);
  vtkMRMLPrintIntMacro(Threading);
*/
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
vtkMRMLScalarVolumeNode* vtkMRMLPatientPositioningNode::GetDrrNode()
{
  return vtkMRMLScalarVolumeNode::SafeDownCast( this->GetNodeReference(DRR_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLPatientPositioningNode::SetAndObserveDrrNode(vtkMRMLScalarVolumeNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("SetAndObserveDrrNode: Cannot set reference, the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(DRR_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkMRMLPatientPositioningNode::GetXrayImageNode()
{
  return vtkMRMLScalarVolumeNode::SafeDownCast( this->GetNodeReference(XRAY_IMAGE_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLPatientPositioningNode::SetAndObserveXrayImageNode(vtkMRMLScalarVolumeNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("SetAndObserveXrayImageNode: Cannot set reference, the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(XRAY_IMAGE_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

//----------------------------------------------------------------------------
void vtkMRMLPatientPositioningNode::SetXrayImageNode(vtkMRMLScalarVolumeNode* node, XrayProjectionType projectionType)
{
  if (!node || projectionType == vtkMRMLPatientPositioningNode::XrayProjectionType_Last)
  {
    return;
  }

  DrrXrayImagePair& imagesPair = ImagesMap[projectionType];
  imagesPair.second = node;
}

//----------------------------------------------------------------------------
void vtkMRMLPatientPositioningNode::SetDrrNode(vtkMRMLScalarVolumeNode* node, XrayProjectionType projectionType)
{
  if (!node || projectionType == vtkMRMLPatientPositioningNode::XrayProjectionType_Last)
  {
    return;
  }

  DrrXrayImagePair& imagesPair = ImagesMap[projectionType];
  imagesPair.first = node;
}

