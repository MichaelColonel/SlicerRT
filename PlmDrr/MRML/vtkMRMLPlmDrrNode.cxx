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

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// STD includes
#include <sstream>

#include "vtkMRMLPlmDrrNode.h"

//------------------------------------------------------------------------------
namespace
{

  const char* BEAM_REFERENCE_ROLE = "beamRef";

} // namespace

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLPlmDrrNode);

//----------------------------------------------------------------------------
vtkMRMLPlmDrrNode::vtkMRMLPlmDrrNode()
  :
  IsocenterDetectorDistance(300.),
  RotateX(0.),
  RotateY(0.),
  RotateZ(0.)
{
  DetectorCenterOffset[0] = 0.;
  DetectorCenterOffset[1] = 0.;
  ImageDimention[0] = 1024;
  ImageDimention[1] = 768;
  ImageSpacing[0] = 1.;
  ImageSpacing[1] = 1.;
  this->SetSingletonTag("DRR");
}

//----------------------------------------------------------------------------
vtkMRMLPlmDrrNode::~vtkMRMLPlmDrrNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLPlmDrrNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream
  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLFloatMacro( IsocenterDetectorDistance, IsocenterDetectorDistance);
  vtkMRMLWriteXMLVectorMacro( DetectorCenterOffset, DetectorCenterOffset, double, 2);
  vtkMRMLWriteXMLVectorMacro( ImageDimention, ImageDimention, int, 2);
  vtkMRMLWriteXMLVectorMacro( ImageSpacing, ImageSpacing, double, 2);
  vtkMRMLWriteXMLFloatMacro( RotateX, RotateX);
  vtkMRMLWriteXMLFloatMacro( RotateY, RotateY);
  vtkMRMLWriteXMLFloatMacro( RotateZ, RotateZ);
  vtkMRMLWriteXMLEndMacro(); 
}

//----------------------------------------------------------------------------
void vtkMRMLPlmDrrNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  vtkMRMLNode::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLFloatMacro( IsocenterDetectorDistance, IsocenterDetectorDistance);
  vtkMRMLReadXMLVectorMacro( DetectorCenterOffset, DetectorCenterOffset, double, 2);
  vtkMRMLReadXMLVectorMacro( ImageDimention, ImageDimention, int, 2);
  vtkMRMLReadXMLVectorMacro( ImageSpacing, ImageSpacing, double, 2);
  vtkMRMLReadXMLFloatMacro( RotateX, RotateX);
  vtkMRMLReadXMLFloatMacro( RotateY, RotateY);
  vtkMRMLReadXMLFloatMacro( RotateZ, RotateZ);
  vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);

  // Note: ReportString is not read from XML, it is a strictly temporary value
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
void vtkMRMLPlmDrrNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  vtkMRMLPlmDrrNode* node = vtkMRMLPlmDrrNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  // Copy beam parameters
  this->DisableModifiedEventOn();

  this->SetIsocenterDetectorDistance(node->GetIsocenterDetectorDistance());
  this->SetDetectorCenterOffset(node->GetDetectorCenterOffset());
  this->SetImageDimention(node->GetImageDimention());
  this->SetImageSpacing(node->GetImageSpacing());
  this->SetRotateX(node->GetRotateX());
  this->SetRotateY(node->GetRotateY());
  this->SetRotateZ(node->GetRotateZ());

  this->EndModify(disabledModify);
  
  this->InvokePendingModifiedEvent();
/*
  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyFloatMacro(IsocenterDetectorDistance);
  vtkMRMLCopyVectorMacro( DetectorCenterOffset, double, 2);
  vtkMRMLCopyVectorMacro( ImageDimention, int, 2);
  vtkMRMLCopyVectorMacro( ImageSpacing, double, 2);
  vtkMRMLCopyFloatMacro(RotateX);
  vtkMRMLCopyFloatMacro(RotateY);
  vtkMRMLCopyFloatMacro(RotateZ);
  vtkMRMLCopyEndMacro(); 

  this->EndModify(disabledModify);
*/
}

//----------------------------------------------------------------------------
void vtkMRMLPlmDrrNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintFloatMacro(IsocenterDetectorDistance);
  vtkMRMLPrintVectorMacro( DetectorCenterOffset, double, 2);
  vtkMRMLPrintVectorMacro( ImageDimention, int, 2);
  vtkMRMLPrintVectorMacro( ImageSpacing, double, 2);
  vtkMRMLPrintFloatMacro(RotateX);
  vtkMRMLPrintFloatMacro(RotateY);
  vtkMRMLPrintFloatMacro(RotateZ);
  vtkMRMLPrintEndMacro(); 
}

//----------------------------------------------------------------------------
vtkMRMLRTBeamNode* vtkMRMLPlmDrrNode::GetBeamNode()
{
  return vtkMRMLRTBeamNode::SafeDownCast( this->GetNodeReference(BEAM_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLPlmDrrNode::SetAndObserveBeamNode(vtkMRMLRTBeamNode* node)
{
  if (node && this->Scene != node->GetScene())
    {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return;
    }

  this->SetNodeReferenceID(BEAM_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}
