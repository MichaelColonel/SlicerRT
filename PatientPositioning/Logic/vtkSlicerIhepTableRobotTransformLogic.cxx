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
#include "vtkSlicerIhepTableRobotTransformLogic.h"

#include "vtkMRMLRTBeamNode.h"
#include "vtkMRMLRTPlanNode.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLLinearTransformNode.h>

// IHEP stand geometry MRML node
//#include <vtkMRMLIhepStandGeometryNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkGeneralTransform.h>
#include <vtkTransform.h>

// STD includes
#include <array>

namespace {

const std::array< double, 3 > A{ 0., 0., 163. };
const std::array< double, 3 > B{ 0., 330., 645. };
const std::array< double, 3 > C{ 0., 330., 645. + 1150. };
const std::array< double, 3 > D{ 0., 330. + 1220, 645. + 1150. + 115. };
const std::array< double, 3 > E{ 0., 330. + 1220 + 240, 645. + 1150. + 115. };

}

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerIhepTableRobotTransformLogic);

//-----------------------------------------------------------------------------
vtkSlicerIhepTableRobotTransformLogic::vtkSlicerIhepTableRobotTransformLogic()
{
  using CoordSys = CoordinateSystemIdentifier;
  // Setup coordinate system ID to name map
  this->CoordinateSystemsMap.clear();
  this->CoordinateSystemsMap[CoordSys::RAS] = "Ras";
  this->CoordinateSystemsMap[CoordSys::FixedReference] = "FixedReference";
  this->CoordinateSystemsMap[CoordSys::BaseFixed] = "RobotBaseFixed";
  this->CoordinateSystemsMap[CoordSys::BaseRotation] = "RobotBaseRotation";
  this->CoordinateSystemsMap[CoordSys::Shoulder] = "RobotShoulder";
  this->CoordinateSystemsMap[CoordSys::Elbow] = "RobotElbow";
  this->CoordinateSystemsMap[CoordSys::Wrist] = "RobotWrist";
  this->CoordinateSystemsMap[CoordSys::TableTop] = "TableTop";
  this->CoordinateSystemsMap[CoordSys::Patient] = "Patient";

  this->IhepTransforms.clear();
  this->IhepTransforms.push_back(std::make_pair(CoordSys::FixedReference, CoordSys::RAS)); // Dummy
  this->IhepTransforms.push_back(std::make_pair(CoordSys::BaseFixed, CoordSys::FixedReference)); // Collimator in canyon system
  this->IhepTransforms.push_back(std::make_pair(CoordSys::BaseRotation, CoordSys::BaseFixed)); // Rotation of patient support platform
  this->IhepTransforms.push_back(std::make_pair(CoordSys::Shoulder, CoordSys::BaseRotation)); // Lateral movement along Y-axis in RAS of the table top
  this->IhepTransforms.push_back(std::make_pair(CoordSys::Elbow, CoordSys::Shoulder)); // Longitudinal movement along X-axis in RAS of the table top
  this->IhepTransforms.push_back(std::make_pair(CoordSys::Wrist, CoordSys::Elbow)); // Vertical movement of table top origin
  this->IhepTransforms.push_back(std::make_pair(CoordSys::TableTop, CoordSys::Wrist)); // Movement and rotation of table top on origin point
  this->IhepTransforms.push_back(std::make_pair(CoordSys::Patient, CoordSys::TableTop));
  this->IhepTransforms.push_back(std::make_pair(CoordSys::RAS, CoordSys::Patient));

  this->CoordinateSystemsHierarchy.clear();
  // key - parent, value - children
  this->CoordinateSystemsHierarchy[CoordSys::FixedReference] = { CoordSys::BaseFixed };
  this->CoordinateSystemsHierarchy[CoordSys::BaseFixed] = { CoordSys::BaseRotation };
  this->CoordinateSystemsHierarchy[CoordSys::BaseRotation] = { CoordSys::Shoulder };
  this->CoordinateSystemsHierarchy[CoordSys::Shoulder] = { CoordSys::Elbow };
  this->CoordinateSystemsHierarchy[CoordSys::Elbow] = { CoordSys::Wrist };
  this->CoordinateSystemsHierarchy[CoordSys::Wrist] = { CoordSys::TableTop };
  this->CoordinateSystemsHierarchy[CoordSys::TableTop] = { CoordSys::Patient };
  this->CoordinateSystemsHierarchy[CoordSys::Patient] = { CoordSys::RAS };
}

//-----------------------------------------------------------------------------
vtkSlicerIhepTableRobotTransformLogic::~vtkSlicerIhepTableRobotTransformLogic()
{
  this->CoordinateSystemsMap.clear();
  this->IhepTransforms.clear();
}

//----------------------------------------------------------------------------
void vtkSlicerIhepTableRobotTransformLogic::PrintSelf(ostream& os, vtkIndent indent)
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
const char* vtkSlicerIhepTableRobotTransformLogic::GetTreatmentMachinePartTypeAsString(CoordinateSystemIdentifier type)
{
  switch (type)
  {
    case FixedReference: return "FixedReference";
    case BaseFixed: return "RobotBaseFixed";
    case TableTop: return "TableTop";
    case BaseRotation: return "RobotBaseRotation";
    case Shoulder: return "RobotShoulder";
    case Elbow: return "RobotElbow";
    case Wrist: return "RobotWrist";
    default:
      // invalid type
      return "";
  }
}

//---------------------------------------------------------------------------
void vtkSlicerIhepTableRobotTransformLogic::BuildTableRobotTransformHierarchy()
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
      transformNode->SetHideFromEditors(1);
      std::string singletonTag = std::string("IHEP_") + transformNodeName;
      transformNode->SetSingletonTag(singletonTag.c_str());
      this->GetMRMLScene()->AddNode(transformNode);
    }
  }

  using CoordSys = CoordinateSystemIdentifier;

  // Organize transforms into hierarchy based on IHEP RBS geometry

  // FixedReference parent, translation of fixed base part of the robot from fixed reference isocenter
  this->GetTransformNodeBetween(CoordSys::BaseFixed, CoordSys::FixedReference)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::FixedReference, CoordSys::RAS)->GetID() );

  // BaseFixed parent, rotation of base part of the robot along Z-axis
  this->GetTransformNodeBetween(CoordSys::BaseRotation, CoordSys::BaseFixed)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::BaseFixed, CoordSys::FixedReference)->GetID() );

  // BaseRotation parent, rotation of shoulder part of the robot along Y-axis
  this->GetTransformNodeBetween(CoordSys::Shoulder, CoordSys::BaseRotation)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::BaseRotation, CoordSys::BaseFixed)->GetID() );

  // Shoulder parent, rotation of elbow part of the robot along Y-axis
  this->GetTransformNodeBetween(CoordSys::Elbow, CoordSys::Shoulder)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::Shoulder, CoordSys::BaseRotation)->GetID() );
  // Elbow parent, rotation of wrist part of the robot along Y-axis
  this->GetTransformNodeBetween(CoordSys::Wrist, CoordSys::Elbow)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::Elbow, CoordSys::Shoulder)->GetID() );

  // Wrist parent, translation of table top center from wrist flange center
  this->GetTransformNodeBetween(CoordSys::TableTop, CoordSys::Wrist)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::Wrist, CoordSys::Elbow)->GetID() );

  // TableTop parent, translation of patient from wrist flange center
  this->GetTransformNodeBetween( CoordSys::Patient, CoordSys::TableTop)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::TableTop, CoordSys::Wrist)->GetID() );

  // Patient parent, transform to RAS
  this->GetTransformNodeBetween( CoordSys::RAS, CoordSys::Patient)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween( CoordSys::Patient, CoordSys::TableTop)->GetID() );
}

//-----------------------------------------------------------------------------
void vtkSlicerIhepTableRobotTransformLogic::ResetRasToPatientIsocenterTranslate()
{
  using CoordSys = CoordinateSystemIdentifier;
  // Update IEC Patient to RAS transform based on the isocenter defined in the beam's parent plan
  vtkMRMLLinearTransformNode* rasToPatientReferenceTransformNode =
    this->GetTransformNodeBetween( CoordSys::RAS, CoordSys::Patient);
  vtkTransform* rasToPatientReferenceTransform = vtkTransform::SafeDownCast(rasToPatientReferenceTransformNode->GetTransformToParent());
  rasToPatientReferenceTransform->Identity();
  rasToPatientReferenceTransform->RotateX(-90.);
  rasToPatientReferenceTransform->RotateZ(180.);
  rasToPatientReferenceTransform->Modified();
}

//-----------------------------------------------------------------------------
void vtkSlicerIhepTableRobotTransformLogic::RestoreRasToPatientIsocenterTranslate(double isocenter[3])
{
  using CoordSys = CoordinateSystemIdentifier;
  // Update IEC Patient to RAS transform based on the isocenter defined in the beam's parent plan
  vtkMRMLLinearTransformNode* rasToPatientReferenceTransformNode =
    this->GetTransformNodeBetween( CoordSys::RAS, CoordSys::Patient);
  vtkTransform* rasToPatientReferenceTransform = vtkTransform::SafeDownCast(rasToPatientReferenceTransformNode->GetTransformToParent());
  rasToPatientReferenceTransform->Identity();
  rasToPatientReferenceTransform->Translate(isocenter[0], isocenter[1], isocenter[2]);
  rasToPatientReferenceTransform->RotateX(-90.);
  rasToPatientReferenceTransform->RotateZ(180.);
  rasToPatientReferenceTransform->Modified();
}

//-----------------------------------------------------------------------------
std::string vtkSlicerIhepTableRobotTransformLogic::GetTransformNodeNameBetween(
  CoordinateSystemIdentifier fromFrame, CoordinateSystemIdentifier toFrame)
{
  return this->CoordinateSystemsMap[fromFrame] + "To" + this->CoordinateSystemsMap[toFrame] + "Transform";
}

//-----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerIhepTableRobotTransformLogic::GetTransformNodeBetween(
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
bool vtkSlicerIhepTableRobotTransformLogic::GetTransformForPointBetweenFrames( 
  CoordinateSystemIdentifier fromFrame, CoordinateSystemIdentifier toFrame,
  const double fromFramePoint[3], double toFramePoint[3], bool transformForBeam)
{
  /// Old version
  //using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;

  //double pointFromFrame[4] = { fromFramePoint[0], fromFramePoint[1], fromFramePoint[2], 1. };
  //double pointInRas[4] = {};

  //// FromFrame -> RAS 
  //vtkNew<vtkTransform> rasFromFrameTransform;
  //if (this->GetTransformBetween( IHEP::RAS, fromFrame, rasFromFrameTransform, transformForBeam))
  //{
    //rasFromFrameTransform->MultiplyPoint( pointFromFrame, pointInRas);
  //}
  //else
  //{
    //return false;
  //}

  //double pointToFrame[4] = {};
  //// RAS -> ToFrame
  //vtkNew<vtkTransform> rasToFrameTransform;
  //if (this->GetTransformBetween( IHEP::RAS, toFrame, rasToFrameTransform, transformForBeam))
  //{
    //rasToFrameTransform->Inverse(); // inverse to get (RAS -> ToFrame)
    //rasToFrameTransform->MultiplyPoint( pointInRas, pointToFrame);
  //}
  //else
  //{
    //return false;
  //}

  //rasFromFrameTransform->Concatenate(rasToFrameTransform);
  //rasFromFrameTransform->MultiplyPoint( pointFromFrame, pointToFrame);

  //toFramePoint[0] = pointToFrame[0];
  //toFramePoint[1] = pointToFrame[1];
  //toFramePoint[2] = pointToFrame[2];

  //return true;

  using CoordSys = CoordinateSystemIdentifier;

  // RAS == World
  // toFrame->RAS transform
  vtkNew<vtkTransform> toFrameToRasTransform;
  if (!this->GetTransformBetween( CoordSys::RAS, toFrame, toFrameToRasTransform, transformForBeam))
  {
    return false;
  }

  // fromFrame->RAS
  vtkNew<vtkTransform> rasToFromFrameTransform;
  if (this->GetTransformBetween( CoordSys::RAS, fromFrame, rasToFromFrameTransform, transformForBeam))
  {
    rasToFromFrameTransform->Inverse(); // inverse to get (RAS->fromFrame)
  }
  else
  {
    return false;
  }

  // Get transform toFrame -> fromFrame
  // toFrame -> RAS -> fromFrame
  toFrameToRasTransform->Concatenate(rasToFromFrameTransform);
  toFrameToRasTransform->TransformPoint( fromFramePoint, toFramePoint);

  return true;
}

//-----------------------------------------------------------------------------
bool vtkSlicerIhepTableRobotTransformLogic::GetTransformBetween(
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
        vtkErrorMacro("GetTransformBetween: Transform node is invalid");
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
        vtkErrorMacro("GetTransformBetween: Transform node is invalid");
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
bool vtkSlicerIhepTableRobotTransformLogic::GetTransformBetween(
  CoordinateSystemIdentifier fromFrame, CoordinateSystemIdentifier toFrame, 
  vtkTransform* outputLinearTransform, bool transformForBeam)
{
  vtkNew<vtkGeneralTransform> inputGeneralTransform;
  if (!this->GetTransformBetween( fromFrame, toFrame, inputGeneralTransform, transformForBeam))
  {
    return false;
  }

  // Convert general transform to linear
  // This call also makes hard copy of the transform so that it doesn't change when input transform changed
  if (!vtkMRMLTransformNode::IsGeneralTransformLinear( inputGeneralTransform, outputLinearTransform))
  {
    vtkErrorMacro("GetTransformBetween: Can't transform general transform to linear! General trasform is not linear.");
    return false;
  }
  return true;
}

//-----------------------------------------------------------------------------
bool vtkSlicerIhepTableRobotTransformLogic::GetPathToRoot( CoordinateSystemIdentifier frame, 
  CoordinateSystemsList& path)
{
  using CoordSys = CoordinateSystemIdentifier;
  if (frame == CoordSys::FixedReference)
  {
    path.push_back(CoordSys::FixedReference);
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
        if (frame != CoordSys::FixedReference)
        {
          found = true;
          break;
        }
        else
        {
          path.push_back(CoordSys::FixedReference);
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
bool vtkSlicerIhepTableRobotTransformLogic::GetPathFromRoot( CoordinateSystemIdentifier frame, 
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
