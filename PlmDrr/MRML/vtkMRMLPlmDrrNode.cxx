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
{
  ImagerCenterOffset[0] = 0.;
  ImagerCenterOffset[1] = 0.;
  ImageDimention[0] = 2048; // columns = x
  ImageDimention[1] = 2048; // rows = y

  ImageCenter[0] = ImageDimention[0] / 2; // columns = x
  ImageCenter[1] = ImageDimention[1] / 2; // rows = y

  ImageSpacing[0] = 0.25; // 250 um
  ImageSpacing[1] = 0.25; // 250 um

  ImageWindow[0] = 0; // c1 = x0 (start column) 
  ImageWindow[1] = 0; // r1 = y0 (start row)
  ImageWindow[2] = 1024; // c2 = x1 (end column)
  ImageWindow[3] = 768; // r2 = y1 (end row)

  AlgorithmReconstuction = EXACT;
  HUConversion = PREPROCESS;
  Threading = CPU;
  ExponentialMappingFlag = true;
  AutoscaleFlag = false;
  AutoscaleRange[0] = 0;
  AutoscaleRange[1] = 255;

  IsocenterImagerDistance = 300.;
  RotateX = 0.;
  RotateY = 0.;
  RotateZ = 0.;

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
  vtkMRMLWriteXMLFloatMacro(IsocenterImagerDistance, IsocenterImagerDistance);
  vtkMRMLWriteXMLVectorMacro(ImagerCenterOffset, ImagerCenterOffset, double, 2);
  vtkMRMLWriteXMLVectorMacro(ImageDimention, ImageDimention, int, 2);
  vtkMRMLWriteXMLVectorMacro(ImageSpacing, ImageSpacing, double, 2);
  vtkMRMLWriteXMLVectorMacro(ImageCenter, ImageCenter, int, 2);
  vtkMRMLWriteXMLVectorMacro(ImageWindow, ImageWindow, int, 4);
  vtkMRMLWriteXMLFloatMacro(RotateX, RotateX);
  vtkMRMLWriteXMLFloatMacro(RotateY, RotateY);
  vtkMRMLWriteXMLFloatMacro(RotateZ, RotateZ);
  // add new parameters here
  vtkMRMLWriteXMLEndMacro(); 
}

//----------------------------------------------------------------------------
void vtkMRMLPlmDrrNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  vtkMRMLNode::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLFloatMacro(IsocenterImagerDistance, IsocenterImagerDistance);
  vtkMRMLReadXMLVectorMacro(ImagerCenterOffset, ImagerCenterOffset, double, 2);
  vtkMRMLReadXMLVectorMacro(ImageDimention, ImageDimention, int, 2);
  vtkMRMLReadXMLVectorMacro(ImageSpacing, ImageSpacing, double, 2);
  vtkMRMLReadXMLVectorMacro(ImageCenter, ImageCenter, int, 2);
  vtkMRMLReadXMLVectorMacro(ImageWindow, ImageWindow, int, 4);
  vtkMRMLReadXMLFloatMacro(RotateX, RotateX);
  vtkMRMLReadXMLFloatMacro(RotateY, RotateY);
  vtkMRMLReadXMLFloatMacro(RotateZ, RotateZ);
  // add new parameters here
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
/*
  this->SetIsocenterImagerDistance(node->GetIsocenterImagerDistance());
  this->SetImagerCenterOffset(node->GetImagerCenterOffset());
  this->SetImageDimention(node->GetImageDimention());
  this->SetImageSpacing(node->GetImageSpacing());
  this->SetImageWindow(node->GetImageWindow());
  this->SetRotateX(node->GetRotateX());
  this->SetRotateY(node->GetRotateY());
  this->SetRotateZ(node->GetRotateZ());

  this->EndModify(disabledModify);
*/

  vtkMRMLCopyBeginMacro(node);
  vtkMRMLCopyFloatMacro(IsocenterImagerDistance);
  vtkMRMLCopyVectorMacro(ImagerCenterOffset, double, 2);
  vtkMRMLCopyVectorMacro(ImageDimention, int, 2);
  vtkMRMLCopyVectorMacro(ImageSpacing, double, 2);
  vtkMRMLCopyVectorMacro(ImageCenter, int, 2);
  vtkMRMLCopyVectorMacro(ImageWindow, int, 4);
  vtkMRMLCopyFloatMacro(RotateX);
  vtkMRMLCopyFloatMacro(RotateY);
  vtkMRMLCopyFloatMacro(RotateZ);
  // add new parameters here
  vtkMRMLCopyEndMacro(); 

  this->EndModify(disabledModify);

  this->InvokePendingModifiedEvent();
}

//----------------------------------------------------------------------------
void vtkMRMLPlmDrrNode::CopyContent(vtkMRMLNode *anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLPlmDrrNode* node = vtkMRMLPlmDrrNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  vtkMRMLCopyBeginMacro(node);
  vtkMRMLCopyFloatMacro(IsocenterImagerDistance);
  vtkMRMLCopyVectorMacro(ImagerCenterOffset, double, 2);
  vtkMRMLCopyVectorMacro(ImageDimention, int, 2);
  vtkMRMLCopyVectorMacro(ImageSpacing, double, 2);
  vtkMRMLCopyVectorMacro(ImageCenter, int, 2);
  vtkMRMLCopyVectorMacro(ImageWindow, int, 4);
  vtkMRMLCopyFloatMacro(RotateX);
  vtkMRMLCopyFloatMacro(RotateY);
  vtkMRMLCopyFloatMacro(RotateZ);
  // add new parameters here
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLPlmDrrNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintFloatMacro(IsocenterImagerDistance);
  vtkMRMLPrintVectorMacro(ImagerCenterOffset, double, 2);
  vtkMRMLPrintVectorMacro(ImageDimention, int, 2);
  vtkMRMLPrintVectorMacro(ImageSpacing, double, 2);
  vtkMRMLPrintVectorMacro(ImageCenter, int, 2);
  vtkMRMLPrintVectorMacro(ImageWindow, int, 4);
  vtkMRMLPrintFloatMacro(RotateX);
  vtkMRMLPrintFloatMacro(RotateY);
  vtkMRMLPrintFloatMacro(RotateZ);
  // add new parameters here
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
    vtkErrorMacro("SetAndObserveBeamNode: Cannot set reference, the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(BEAM_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

//----------------------------------------------------------------------------
void vtkMRMLPlmDrrNode::GetRTImagePosition(double position[2])
{
  position[0] = -1. * ImageSpacing[0] * ImageDimention[0] / 2.; // columns (X)
  position[1] = ImageSpacing[1] * ImageDimention[1] / 2.; // rows (Y)
}
