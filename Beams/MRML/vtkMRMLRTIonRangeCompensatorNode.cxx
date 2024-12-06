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
#include "vtkMRMLRTIonRangeCompensatorNode.h"
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
vtkMRMLNodeNewMacro(vtkMRMLRTIonRangeCompensatorNode);

//----------------------------------------------------------------------------
vtkMRMLRTIonRangeCompensatorNode::vtkMRMLRTIonRangeCompensatorNode()
  :
  Superclass(),
  Rows(-1),
  Columns(-1)
{
}

//----------------------------------------------------------------------------
vtkMRMLRTIonRangeCompensatorNode::~vtkMRMLRTIonRangeCompensatorNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLRTIonRangeCompensatorNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  // Write all MRML node attributes into output stream
  vtkMRMLWriteXMLIntMacro( rows, Rows);
  vtkMRMLWriteXMLIntMacro( rows, Columns);
  vtkMRMLWriteXMLEnumMacro( divergence, Divergence);
  vtkMRMLWriteXMLEnumMacro( mountingPosition, MountingPosition);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLRTIonRangeCompensatorNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
  
  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLIntMacro( rows, Rows);
  vtkMRMLReadXMLIntMacro( rows, Columns);
  vtkMRMLReadXMLEnumMacro( divergence, Divergence);
  vtkMRMLReadXMLEnumMacro( mountingPosition, MountingPosition);
  vtkMRMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLRTIonRangeCompensatorNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  // Do not call Copy function of the direct model base class, as it copies the poly data too,
  // which is undesired for beams, as they generate their own poly data from their properties.
  vtkMRMLDisplayableNode::Copy(anode);

  vtkMRMLRTIonRangeCompensatorNode* node = vtkMRMLRTIonRangeCompensatorNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }
/*
  // Create transform node for beam
  this->CreateNewBeamTransformNode();

  // Add beam in the same plan if beam nodes are in the same scene
  vtkMRMLRTPlanNode* planNode = node->GetParentPlanNode();
  if (planNode && node->GetScene() == this->Scene)
  {
    planNode->AddBeam(this);
  }
*/
  // Copy beam parameters
  this->DisableModifiedEventOn();

  vtkMRMLCopyBeginMacro(node);
  vtkMRMLCopyIntMacro(Rows);
  vtkMRMLCopyIntMacro(Columns);
  vtkMRMLCopyEnumMacro(Divergence);
  vtkMRMLCopyEnumMacro(MountingPosition);
  vtkMRMLCopyEndMacro();

  this->EndModify(disabledModify);
  
  this->InvokePendingModifiedEvent();
}

//----------------------------------------------------------------------------
void vtkMRMLRTIonRangeCompensatorNode::CopyContent(vtkMRMLNode *anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent( anode, deepCopy);

  vtkMRMLRTIonRangeCompensatorNode* node = vtkMRMLRTIonRangeCompensatorNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  vtkMRMLCopyBeginMacro(node);
  vtkMRMLCopyIntMacro(Rows);
  vtkMRMLCopyIntMacro(Columns);
  vtkMRMLCopyEnumMacro(Divergence);
  vtkMRMLCopyEnumMacro(MountingPosition);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLRTIonRangeCompensatorNode::SetScene(vtkMRMLScene* scene)
{
  Superclass::SetScene(scene);
}

//----------------------------------------------------------------------------
void vtkMRMLRTIonRangeCompensatorNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintIntMacro(Rows);
  vtkMRMLPrintIntMacro(Columns);
  vtkMRMLPrintEnumMacro(Divergence);
  vtkMRMLPrintEnumMacro(MountingPosition);
  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLRTIonRangeCompensatorNode::CreateDefaultDisplayNodes()
{
  // Create default model display node
  Superclass::CreateDefaultDisplayNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLRTIonRangeCompensatorNode::CreateCompensatorPolyData(vtkPolyData* compensatorModelPolyData/*=nullptr*/)
{
  if (!compensatorModelPolyData)
  {
    compensatorModelPolyData = this->GetPolyData();
  }
  if (!compensatorModelPolyData)
  {
    vtkErrorMacro("CreateCompensatorPolyData: Invalid compensator node");
    return;
  }

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> cellArray = vtkSmartPointer<vtkCellArray>::New();

  compensatorModelPolyData->SetPoints(points);
  compensatorModelPolyData->SetPolys(cellArray);
}

//---------------------------------------------------------------------------
const char* vtkMRMLRTIonRangeCompensatorNode::GetDivergenceAsString(int id)
{
  switch (id)
  {
  case vtkMRMLRTIonRangeCompensatorNode::Present:
    return "Present";
  case vtkMRMLRTIonRangeCompensatorNode::Absent:
    return "Absent";
  default:
    return "Divergence_Last";
  }
}

//---------------------------------------------------------------------------
int vtkMRMLRTIonRangeCompensatorNode::GetDivergenceFromString(const char* name)
{
  if (name == nullptr)
  {
    // invalid name
    return -1;
  }
  for (int i = 0; i < vtkMRMLRTIonRangeCompensatorNode::Divergence_Last; i++)
  {
    if (std::strcmp(name, vtkMRMLRTIonRangeCompensatorNode::GetDivergenceAsString(i)) == 0)
    {
      // found a matching name
      return i;
    }
  }
  // unknown name
  return -1;
}

//---------------------------------------------------------------------------
void vtkMRMLRTIonRangeCompensatorNode::SetDivergence(int id)
{
  switch (id)
  {
  case 0:
    this->SetDivergence(vtkMRMLRTIonRangeCompensatorNode::Present);
    break;
  case 1:
    this->SetDivergence(vtkMRMLRTIonRangeCompensatorNode::Absent);
    break;
  default:
    this->SetDivergence(vtkMRMLRTIonRangeCompensatorNode::Divergence_Last);
    break;
  }
}

//---------------------------------------------------------------------------
const char* vtkMRMLRTIonRangeCompensatorNode::GetMountingPositionAsString(int id)
{
  switch (id)
  {
  case vtkMRMLRTIonRangeCompensatorNode::PatientSide:
    return "PatientSide";
  case vtkMRMLRTIonRangeCompensatorNode::SourceSide:
    return "SourceSide";
  case vtkMRMLRTIonRangeCompensatorNode::DoubleSided:
    return "DoubleSided";
  default:
    return "MountingPosition_Last";
  }
}

//---------------------------------------------------------------------------
int vtkMRMLRTIonRangeCompensatorNode::GetMountingPositionFromString(const char* name)
{
  if (name == nullptr)
  {
    // invalid name
    return -1;
  }
  for (int i = 0; i < vtkMRMLRTIonRangeCompensatorNode::MountingPosition_Last; i++)
  {
    if (std::strcmp(name, vtkMRMLRTIonRangeCompensatorNode::GetMountingPositionAsString(i)) == 0)
    {
      // found a matching name
      return i;
    }
  }
  // unknown name
  return -1;
}

//---------------------------------------------------------------------------
void vtkMRMLRTIonRangeCompensatorNode::SetMountingPosition(int id)
{
  switch (id)
  {
  case 0:
    this->SetMountingPosition(vtkMRMLRTIonRangeCompensatorNode::PatientSide);
    break;
  case 1:
    this->SetMountingPosition(vtkMRMLRTIonRangeCompensatorNode::SourceSide);
    break;
  case 2:
    this->SetMountingPosition(vtkMRMLRTIonRangeCompensatorNode::DoubleSided);
    break;
  default:
    this->SetMountingPosition(vtkMRMLRTIonRangeCompensatorNode::MountingPosition_Last);
    break;
  }
}
