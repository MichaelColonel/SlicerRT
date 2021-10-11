/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Vinith Suriyakumar and Csaba Pinter,
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Beams includes
#include "vtkSlicerIhepStandGeometryTransformLogic.h"

#include "vtkMRMLRTBeamNode.h"
#include "vtkMRMLRTPlanNode.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLLinearTransformNode.h>

// IHEP stand geometry MRML node
#include <vtkMRMLIhepStandGeometryNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkGeneralTransform.h>
#include <vtkTransform.h>

// STD includes
#include <array>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerIhepStandGeometryTransformLogic);

//-----------------------------------------------------------------------------
vtkSlicerIhepStandGeometryTransformLogic::vtkSlicerIhepStandGeometryTransformLogic()
{
  using IHEP = CoordinateSystemIdentifier;
  // Setup coordinate system ID to name map
  this->CoordinateSystemsMap.clear();
  this->CoordinateSystemsMap[IHEP::RAS] = "Ras";
  this->CoordinateSystemsMap[IHEP::FixedReference] = "FixedReference";
  this->CoordinateSystemsMap[IHEP::Collimator] = "Collimator";
  this->CoordinateSystemsMap[IHEP::PatientSupportRotation] = "PatientSupportRotation";
  this->CoordinateSystemsMap[IHEP::TableLongitudinalMovement] = "TableLongitudinalMovement";
  this->CoordinateSystemsMap[IHEP::TableLateralMovement] = "TableLateralMovement";
  this->CoordinateSystemsMap[IHEP::TableOriginVerticalMovement] = "TableOriginVerticalMovement";
  this->CoordinateSystemsMap[IHEP::TableMiddleVerticalMovement] = "TableMiddleVerticalMovement";
  this->CoordinateSystemsMap[IHEP::TableMirrorVerticalMovement] = "TableMirrorVerticalMovement";
  this->CoordinateSystemsMap[IHEP::TableTop] = "TableTop";
  this->CoordinateSystemsMap[IHEP::Patient] = "Patient";

  this->IhepTransforms.clear();
  this->IhepTransforms.push_back(std::make_pair(IHEP::FixedReference, IHEP::RAS));
  this->IhepTransforms.push_back(std::make_pair(IHEP::Collimator, IHEP::FixedReference)); // Collimator in canyon system
  this->IhepTransforms.push_back(std::make_pair(IHEP::PatientSupportRotation, IHEP::FixedReference)); // Rotation of patient support platform
  this->IhepTransforms.push_back(std::make_pair(IHEP::TableLongitudinalMovement, IHEP::PatientSupportRotation)); // Longitudinal movement along Y-axis of the table top
  this->IhepTransforms.push_back(std::make_pair(IHEP::TableLateralMovement, IHEP::TableLongitudinalMovement)); // Lateral movement along X-axis of the table top
  this->IhepTransforms.push_back(std::make_pair(IHEP::TableOriginVerticalMovement, IHEP::TableLateralMovement)); // Vertical movement of table top origin
  this->IhepTransforms.push_back(std::make_pair(IHEP::TableMiddleVerticalMovement, IHEP::TableLateralMovement)); // Vertical movement of table top middle
  this->IhepTransforms.push_back(std::make_pair(IHEP::TableMirrorVerticalMovement, IHEP::TableLateralMovement)); // Vertical movement of table top mirror
  this->IhepTransforms.push_back(std::make_pair(IHEP::TableTop, IHEP::TableLateralMovement)); // Movement and rotation of table top
  this->IhepTransforms.push_back(std::make_pair(IHEP::Patient, IHEP::TableTop));
  this->IhepTransforms.push_back(std::make_pair(IHEP::RAS, IHEP::Patient));

  this->CoordinateSystemsHierarchy.clear();
  // key - parent, value - children
  this->CoordinateSystemsHierarchy[IHEP::FixedReference] = { IHEP::Collimator, IHEP::PatientSupportRotation };
  this->CoordinateSystemsHierarchy[IHEP::PatientSupportRotation] = { IHEP::TableLongitudinalMovement };
  this->CoordinateSystemsHierarchy[IHEP::TableLongitudinalMovement] = { IHEP::TableLateralMovement };
  this->CoordinateSystemsHierarchy[IHEP::TableLateralMovement] = { IHEP::TableTop, IHEP::TableOriginVerticalMovement, IHEP::TableMiddleVerticalMovement, IHEP::TableMirrorVerticalMovement };
  this->CoordinateSystemsHierarchy[IHEP::TableTop] = { IHEP::Patient };
  this->CoordinateSystemsHierarchy[IHEP::Patient] = { IHEP::RAS };
}

//-----------------------------------------------------------------------------
vtkSlicerIhepStandGeometryTransformLogic::~vtkSlicerIhepStandGeometryTransformLogic()
{
  this->CoordinateSystemsMap.clear();
  this->IhepTransforms.clear();
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryTransformLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  // Transforms
  os << indent << "Transforms:" << std::endl;
  vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
  for ( auto& transformPair : this->IhepTransforms)
  {
    std::string transformNodeName = this->GetTransformNodeNameBetween( transformPair.first, transformPair.second);
    vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(
      this->GetMRMLScene()->GetFirstNodeByName(transformNodeName.c_str()) );

    os << indent.GetNextIndent() << transformNodeName << std::endl;
    transformNode->GetMatrixTransformToParent(matrix);
    for (int i = 0; i < 4; i++)
    {
      os << indent.GetNextIndent() << indent.GetNextIndent();
      for (int j = 0; j < 4; j++)
      {
        os << matrix->GetElement(i,j) << " ";
      }
      os << std::endl;
    }
  }
}

//---------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryTransformLogic::BuildIHEPTransformHierarchy()
{
  if (!this->GetMRMLScene())
  {
    vtkErrorMacro("BuildIHEPTransformHierarchy: Invalid MRML scene");
    return;
  }

  // Create transform nodes if they do not exist
  for (auto& transformPair : this->IhepTransforms)
  {
    std::string transformNodeName = this->GetTransformNodeNameBetween( transformPair.first, transformPair.second);
    
    if (!this->GetMRMLScene()->GetFirstNodeByName(transformNodeName.c_str()))
    {
      vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      transformNode->SetName(transformNodeName.c_str());
//      transformNode->SetHideFromEditors(1);
      std::string singletonTag = std::string("IHEP_") + transformNodeName;
      transformNode->SetSingletonTag(singletonTag.c_str());
      this->GetMRMLScene()->AddNode(transformNode);
    }
  }

  using IHEP = CoordinateSystemIdentifier;

  // Organize transforms into hierarchy based on IHEP RBS geometry

  // FixedReference parent
  this->GetTransformNodeBetween(IHEP::Collimator, IHEP::FixedReference)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(IHEP::FixedReference, IHEP::RAS)->GetID() );
  // FixedReference parent, rotation of patient support
  this->GetTransformNodeBetween(IHEP::PatientSupportRotation, IHEP::FixedReference)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(IHEP::FixedReference, IHEP::RAS)->GetID() );

  // PatientSupport parent, longitudinal movement of table top
  this->GetTransformNodeBetween(IHEP::TableLongitudinalMovement, IHEP::PatientSupportRotation)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(IHEP::PatientSupportRotation, IHEP::FixedReference)->GetID() );

  // TableLongitudinalMovement parent, lateral movement of table top
  this->GetTransformNodeBetween(IHEP::TableLateralMovement, IHEP::TableLongitudinalMovement)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(IHEP::TableLongitudinalMovement, IHEP::PatientSupportRotation)->GetID() );

  // TableLateralMovement parent, inverse movement of the table top 
  this->GetTransformNodeBetween(IHEP::TableTop, IHEP::TableLateralMovement)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(IHEP::TableLateralMovement, IHEP::TableLongitudinalMovement)->GetID() );

  // TableLateralMovement parent, table origin vertical movement
  this->GetTransformNodeBetween(IHEP::TableOriginVerticalMovement, IHEP::TableLateralMovement)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(IHEP::TableLateralMovement, IHEP::TableLongitudinalMovement)->GetID() );
  // TableLateralMovement parent, table mirror vertical movement
  this->GetTransformNodeBetween(IHEP::TableMirrorVerticalMovement, IHEP::TableLateralMovement)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(IHEP::TableLateralMovement, IHEP::TableLongitudinalMovement)->GetID() );
  // TableLateralMovement parent, table middle vertical movement
  this->GetTransformNodeBetween(IHEP::TableMiddleVerticalMovement, IHEP::TableLateralMovement)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(IHEP::TableLateralMovement, IHEP::TableLongitudinalMovement)->GetID() );

  // TableTop parent, patient movement
  this->GetTransformNodeBetween( IHEP::Patient, IHEP::TableTop)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(IHEP::TableTop, IHEP::TableLateralMovement)->GetID() );

  // Patient parent, transform to RAS
  this->GetTransformNodeBetween( IHEP::RAS, IHEP::Patient)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween( IHEP::Patient, IHEP::TableTop)->GetID() );
}

//-----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryTransformLogic::ResetRasToPatientIsocenterTranslate()
{
  using IHEP = CoordinateSystemIdentifier;
  // Update IEC Patient to RAS transform based on the isocenter defined in the beam's parent plan
  vtkMRMLLinearTransformNode* rasToPatientReferenceTransformNode =
    this->GetTransformNodeBetween( IHEP::RAS, IHEP::Patient);
  vtkTransform* rasToPatientReferenceTransform = vtkTransform::SafeDownCast(rasToPatientReferenceTransformNode->GetTransformToParent());
  rasToPatientReferenceTransform->Identity();
  rasToPatientReferenceTransform->RotateX(-90.);
  rasToPatientReferenceTransform->RotateZ(180.);
  rasToPatientReferenceTransform->Modified();
}

//-----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryTransformLogic::RestoreRasToPatientIsocenterTranslate(double isocenter[3])
{
  using IHEP = CoordinateSystemIdentifier;
  // Update IEC Patient to RAS transform based on the isocenter defined in the beam's parent plan
  vtkMRMLLinearTransformNode* rasToPatientReferenceTransformNode =
    this->GetTransformNodeBetween( IHEP::RAS, IHEP::Patient);
  vtkTransform* rasToPatientReferenceTransform = vtkTransform::SafeDownCast(rasToPatientReferenceTransformNode->GetTransformToParent());
  rasToPatientReferenceTransform->Identity();
  rasToPatientReferenceTransform->Translate(isocenter[0], isocenter[1], isocenter[2]);
  rasToPatientReferenceTransform->RotateX(-90.);
  rasToPatientReferenceTransform->RotateZ(180.);
  rasToPatientReferenceTransform->Modified();
}

//-----------------------------------------------------------------------------
std::string vtkSlicerIhepStandGeometryTransformLogic::GetTransformNodeNameBetween(
  CoordinateSystemIdentifier fromFrame, CoordinateSystemIdentifier toFrame)
{
  return this->CoordinateSystemsMap[fromFrame] + "To" + this->CoordinateSystemsMap[toFrame] + "Transform";
}

//-----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerIhepStandGeometryTransformLogic::GetTransformNodeBetween(
  CoordinateSystemIdentifier fromFrame, CoordinateSystemIdentifier toFrame )
{
  if (!this->GetMRMLScene())
  {
    vtkErrorMacro("GetTransformNodeBetween: Invalid MRML scene");
    return nullptr;
  }

  return vtkMRMLLinearTransformNode::SafeDownCast(
    this->GetMRMLScene()->GetFirstNodeByName( this->GetTransformNodeNameBetween(fromFrame, toFrame).c_str() ) );
}

//-----------------------------------------------------------------------------
bool vtkSlicerIhepStandGeometryTransformLogic::GetTransformBetween(
  CoordinateSystemIdentifier fromFrame, CoordinateSystemIdentifier toFrame, 
  vtkGeneralTransform* outputTransform, bool transformForBeam)
{
  if (!outputTransform)
  {
    vtkErrorMacro("GetTransformBetween: Invalid output transform node");
    return false;
  }
  if (!this->GetMRMLScene())
  {
    vtkErrorMacro("GetTransformBetween: Invalid MRML scene");
    return false;
  }

  CoordinateSystemsList fromFramePath, toFramePath;
  if (this->GetPathToRoot( fromFrame, fromFramePath) && this->GetPathFromRoot( toFrame, toFramePath))
  {
    std::vector< CoordinateSystemIdentifier > toFrameVector(toFramePath.size());
    std::vector< CoordinateSystemIdentifier > fromFrameVector(fromFramePath.size());

    std::copy( toFramePath.begin(), toFramePath.end(), toFrameVector.begin());
    std::copy( fromFramePath.begin(), fromFramePath.end(), fromFrameVector.begin());

    outputTransform->Identity();
    outputTransform->PostMultiply();
    for ( size_t i = 0; i < fromFrameVector.size() - 1; ++i)
    {
      CoordinateSystemIdentifier parent, child;
      child = fromFrameVector[i];
      parent = fromFrameVector[i + 1];

      if (child == parent)
      {
        continue;
      }

      vtkMRMLLinearTransformNode* fromTransform = this->GetTransformNodeBetween( child, parent);
      if (fromTransform)
      {
        vtkNew<vtkMatrix4x4> mat;
        fromTransform->GetMatrixTransformToParent(mat);
        outputTransform->Concatenate(mat);

        vtkDebugMacro("GetTransformBetween: Transform node \"" << fromTransform->GetName() << "\" is valid");
      }
      else
      {
        vtkErrorMacro("GetTransformBetween: Transform node \"" << fromTransform->GetName() << "\" is invalid");
        return false;
      }
    }

    for ( size_t i = 0; i < toFrameVector.size() - 1; ++i)
    {
      CoordinateSystemIdentifier parent, child;
      parent = toFrameVector[i];
      child = toFrameVector[i + 1];

      if (child == parent)
      {
        continue;
      }

      vtkMRMLLinearTransformNode* toTransform = this->GetTransformNodeBetween( child, parent);
      if (toTransform)
      {
        vtkNew<vtkMatrix4x4> mat;
        if (transformForBeam) // calculation for beam transformation
        {
          toTransform->GetMatrixTransformFromParent(mat);
        }
        else // calculation for a treatment room models transformations
        {
          toTransform->GetMatrixTransformToParent(mat);
        }
        mat->Invert();
        outputTransform->Concatenate(mat);

        vtkDebugMacro("GetTransformBetween: Transform node \"" << toTransform->GetName() << "\" is valid");
      }
      else
      {
        vtkErrorMacro("GetTransformBetween: Transform node \"" << toTransform->GetName() << "\" is invalid");
        return false;
      }
    }

    outputTransform->Modified();
    return true;
  }

  vtkErrorMacro("GetTransformBetween: Failed to get transform " << this->GetTransformNodeNameBetween(fromFrame, toFrame));
  return false;
}

//-----------------------------------------------------------------------------
bool vtkSlicerIhepStandGeometryTransformLogic::GetPathToRoot( CoordinateSystemIdentifier frame, 
  CoordinateSystemsList& path)
{
  using IHEP = CoordinateSystemIdentifier;
  if (frame == IHEP::FixedReference)
  {
    path.push_back(IHEP::FixedReference);
    return true;
  }

  bool found = false;
  do
  {
    for (auto& pair : this->CoordinateSystemsHierarchy)
    {
      CoordinateSystemIdentifier parent = pair.first;

      auto& children = pair.second;
      auto iter = std::find( children.begin(), children.end(), frame);
      if (iter != children.end())
      {
        CoordinateSystemIdentifier id = *iter;

        vtkDebugMacro("GetPathToRoot: Checking affine transformation " 
          << "\"" << this->CoordinateSystemsMap[id] << "\" -> " 
          << "\"" << this->CoordinateSystemsMap[parent] << "\"");

        frame = parent;
        path.push_back(id);
        if (frame != IHEP::FixedReference)
        {
          found = true;
          break;
        }
        else
        {
          path.push_back(IHEP::FixedReference);
        }
      }
      else
      {
        found = false;
      }
    }
  }
  while (found);

  return (path.size() > 0);
}

//-----------------------------------------------------------------------------
bool vtkSlicerIhepStandGeometryTransformLogic::GetPathFromRoot( CoordinateSystemIdentifier frame, 
  CoordinateSystemsList& path)
{
  if (this->GetPathToRoot( frame, path))
  {
    std::reverse( path.begin(), path.end());
    return true;
  }
  else
  {
    return false;
  }
}
