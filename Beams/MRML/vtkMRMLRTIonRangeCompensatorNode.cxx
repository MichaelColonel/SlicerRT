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
#include <vtkCubeSource.h>
#include <vtkAppendPolyData.h>
#include <vtkTriangleFilter.h>
#include <vtkBooleanOperationPolyDataFilter.h>
#include <vtkPolyDataNormals.h>

namespace
{
const char* PARENT_ION_BEAM_NODE_REFERENCE_ROLE = "parentIonBeamRef";
}

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
  vtkMRMLWriteXMLFloatMacro( isocenterToCompensatorTrayDistance, IsocenterToCompensatorTrayDistance);
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
  vtkMRMLReadXMLFloatMacro( isocenterToCompensatorTrayDistance, IsocenterToCompensatorTrayDistance);
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
  vtkMRMLCopyFloatMacro(IsocenterToCompensatorTrayDistance);
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
  vtkMRMLCopyFloatMacro(IsocenterToCompensatorTrayDistance);
  vtkMRMLCopyEnumMacro(Divergence);
  vtkMRMLCopyEnumMacro(MountingPosition);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLRTIonRangeCompensatorNode::SetScene(vtkMRMLScene* scene)
{
  Superclass::SetScene(scene);

  if (!this->GetPolyData())
  {
    // Create compensator model
    vtkSmartPointer<vtkPolyData> beamModelPolyData = vtkSmartPointer<vtkPolyData>::New();
    this->SetAndObservePolyData(beamModelPolyData);
  }

  this->CreateCompensatorPolyData();
}

//----------------------------------------------------------------------------
void vtkMRMLRTIonRangeCompensatorNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintIntMacro(Rows);
  vtkMRMLPrintIntMacro(Columns);
  vtkMRMLPrintFloatMacro(IsocenterToCompensatorTrayDistance);
  vtkMRMLPrintEnumMacro(Divergence);
  vtkMRMLPrintEnumMacro(MountingPosition);
  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLRTIonRangeCompensatorNode::CreateDefaultDisplayNodes()
{
  // Create default model display node
  Superclass::CreateDefaultDisplayNodes();

  // Set beam-specific parameters when first created
  vtkMRMLModelDisplayNode* displayNode = vtkMRMLModelDisplayNode::SafeDownCast(this->GetDisplayNode());
  if (displayNode != nullptr)
  {
    displayNode->SetColor(0.0, 1.0, 0.2);
    displayNode->SetOpacity(0.3);
    displayNode->SetBackfaceCulling(0); // Disable backface culling to make the back side of the contour visible as well
    displayNode->VisibilityOn();
    displayNode->Visibility2DOff();
  }
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

  double sizeX = this->Rows * this->PixelSpacing[0];
  double sizeY = this->Columns * this->PixelSpacing[1];

  vtkNew<vtkAppendPolyData> appendFilter;
  for (int i = 0; i < this->Rows; ++i)
  {
    for (int j = 0; j < this->Columns; ++j)
    {
      vtkNew< vtkCubeSource > voxel;
      size_t pixelIndex = i * this->Columns + j;
      double pixelThickness = this->ThicknessData[pixelIndex];
      double minX = (-1. * sizeX / 2.) + i * this->PixelSpacing[0];
      double maxX = minX + this->PixelSpacing[0];
      double minY = (-1. * sizeY / 2.) + j * this->PixelSpacing[1];
      double maxY = minY + this->PixelSpacing[1];

      switch (this->MountingPosition)
      {
      case vtkMRMLRTIonRangeCompensatorNode::SourceSide:
        voxel->SetBounds( minX, maxX, minY, maxY,
          -1 * pixelThickness - this->IsocenterToCompensatorTrayDistance,
          -1. * this->IsocenterToCompensatorTrayDistance);
        break;
      case vtkMRMLRTIonRangeCompensatorNode::PatientSide:
        voxel->SetBounds( minX, maxX, minY, maxY,
          -1 * this->IsocenterToCompensatorTrayDistance,
          pixelThickness - this->IsocenterToCompensatorTrayDistance);
        break;
      default:
      break;
      }
      voxel->Update();
      appendFilter->AddInputData(voxel->GetOutput());
    }
  }
  appendFilter->Update();
  compensatorModelPolyData->DeepCopy(appendFilter->GetOutput());
}

//----------------------------------------------------------------------------
vtkMRMLRTIonBeamNode* vtkMRMLRTIonRangeCompensatorNode::GetBeamNode()
{
  return vtkMRMLRTIonBeamNode::SafeDownCast( this->GetNodeReference(PARENT_ION_BEAM_NODE_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLRTIonRangeCompensatorNode::SetAndObserveBeamNode(vtkMRMLRTIonBeamNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(PARENT_ION_BEAM_NODE_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
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

//---------------------------------------------------------------------------
void vtkMRMLRTIonRangeCompensatorNode::SetThicknessDataVector(const std::vector< double >& data)
{
  if (data.size() != static_cast< size_t >(this->Rows * this->Columns))
  {
    vtkErrorMacro("SetThicknessDataVector: A new thickness data size != rows*columns");
    return;
  }
  this->ThicknessData.resize(data.size());
  std::copy( std::begin(data), std::end(data), std::begin(this->ThicknessData));
  this->Modified();
}
