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
#include "vtkSlicerTableTopRobotTransformLogic.h"

#include "vtkMRMLRTBeamNode.h"
#include "vtkMRMLRTPlanNode.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLLinearTransformNode.h>

// Channel-25 geometry MRML node
#include <vtkMRMLChannel25GeometryNode.h>

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
vtkStandardNewMacro(vtkSlicerTableTopRobotTransformLogic);

//-----------------------------------------------------------------------------
vtkSlicerTableTopRobotTransformLogic::vtkSlicerTableTopRobotTransformLogic()
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

  this->TableTopRobotTransforms.clear();
  this->TableTopRobotTransforms.push_back(std::make_pair(CoordSys::FixedReference, CoordSys::RAS)); // Dummy
  this->TableTopRobotTransforms.push_back(std::make_pair(CoordSys::BaseFixed, CoordSys::FixedReference)); // Collimator in canyon system
  this->TableTopRobotTransforms.push_back(std::make_pair(CoordSys::BaseRotation, CoordSys::BaseFixed)); // Rotation of patient support platform
  this->TableTopRobotTransforms.push_back(std::make_pair(CoordSys::Shoulder, CoordSys::BaseRotation)); // Lateral movement along Y-axis in RAS of the table top
  this->TableTopRobotTransforms.push_back(std::make_pair(CoordSys::Elbow, CoordSys::Shoulder)); // Longitudinal movement along X-axis in RAS of the table top
  this->TableTopRobotTransforms.push_back(std::make_pair(CoordSys::Wrist, CoordSys::Elbow)); // Vertical movement of table top origin
  this->TableTopRobotTransforms.push_back(std::make_pair(CoordSys::TableTop, CoordSys::Wrist)); // Movement and rotation of table top on origin point
  this->TableTopRobotTransforms.push_back(std::make_pair(CoordSys::Patient, CoordSys::TableTop));
  this->TableTopRobotTransforms.push_back(std::make_pair(CoordSys::RAS, CoordSys::Patient));

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
vtkSlicerTableTopRobotTransformLogic::~vtkSlicerTableTopRobotTransformLogic()
{
  this->CoordinateSystemsMap.clear();
  this->TableTopRobotTransforms.clear();
}

//----------------------------------------------------------------------------
void vtkSlicerTableTopRobotTransformLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  // Transforms
  os << indent << "Transforms:" << std::endl;
  vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
  for ( auto& transformPair : this->TableTopRobotTransforms)
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
const char* vtkSlicerTableTopRobotTransformLogic::GetTreatmentMachinePartTypeAsString(CoordinateSystemIdentifier type)
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
      return nullptr;
  }
}

//---------------------------------------------------------------------------
void vtkSlicerTableTopRobotTransformLogic::BuildTableRobotTransformHierarchy()
{
  if (!this->GetMRMLScene())
  {
    vtkErrorMacro("BuildTableRobotTransformHierarchy: Invalid MRML scene");
    return;
  }

  // Create transform nodes if they do not exist
  for (auto& transformPair : this->TableTopRobotTransforms)
  {
    std::string transformNodeName = this->GetTransformNodeNameBetween( transformPair.first, transformPair.second);
    if (!this->GetMRMLScene()->GetFirstNodeByName(transformNodeName.c_str()))
    {
      vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      transformNode->SetName(transformNodeName.c_str());
//      transformNode->SetHideFromEditors(1);
      std::string singletonTag = std::string("TTR_") + transformNodeName;
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
void vtkSlicerTableTopRobotTransformLogic::ResetRasToPatientIsocenterTranslate()
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
void vtkSlicerTableTopRobotTransformLogic::RestoreRasToPatientIsocenterTranslate(double isocenter[3])
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
std::string vtkSlicerTableTopRobotTransformLogic::GetTransformNodeNameBetween(
  CoordinateSystemIdentifier fromFrame, CoordinateSystemIdentifier toFrame)
{
  return this->CoordinateSystemsMap[fromFrame] + "To" + this->CoordinateSystemsMap[toFrame] + "Transform";
}

//-----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerTableTopRobotTransformLogic::GetTransformNodeBetween(
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
bool vtkSlicerTableTopRobotTransformLogic::GetTransformForPointBetweenFrames( 
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
void vtkSlicerTableTopRobotTransformLogic::UpdateFixedReferenceToRASTransform(vtkMRMLChannel25GeometryNode* channelNode, double* isocenter)
{
  if (!this->GetMRMLScene())
  {
    vtkErrorMacro("UpdateFixedReferenceToRasTransform: Invalid MRML scene");
    return;
  }

  using CoordSys = CoordinateSystemIdentifier;

  // Update IEC FixedReference to RAS transform based on the isocenter defined in the beam's parent plan
  vtkMRMLLinearTransformNode* fixedReferenceToRasTransformNode = this->GetTransformNodeBetween(CoordSys::FixedReference, CoordSys::RAS);

  // Apply isocenter translation
  vtkNew<vtkTransform> fixedReferenceToRASTransformBeamComponent;
  if (channelNode)
  {
    std::array<double, 3> isocenterPosition = {0.0, 0.0, 0.0};
    if (isocenter)
    {
      // Once again the dirty hack for dynamic beams, the actual translation 
      // will be in vtkSlicerDicomRtImportExportModuleLogic::vtkInternal::LoadDynamicBeamSequence method  
      fixedReferenceToRASTransformBeamComponent->Translate(isocenterPosition[0], isocenterPosition[1], isocenterPosition[2]);
    }
    else
    {
      // translation for a static beam
//      if (planNode->GetIsocenterPosition(isocenterPosition.data()))
//      {
//        fixedReferenceToRASTransformBeamComponent->Translate(isocenterPosition[0], isocenterPosition[1], isocenterPosition[2]);
//      }
//      else
//      {
//        vtkErrorMacro("UpdateFixedReferenceToRasTransform: Failed to get isocenter position for plan " << planNode->GetName());
//      }
    }
  }

  // The "S" direction in RAS is the "A" direction in FixedReference
  fixedReferenceToRASTransformBeamComponent->RotateX(-90.0);
  // The "S" direction to be toward the gantry (head first position) by default
  fixedReferenceToRASTransformBeamComponent->RotateZ(180.0);
  fixedReferenceToRASTransformBeamComponent->Modified();

  vtkMRMLLinearTransformNode* baseFixedToFixedReferenceTransformNode =
    this->GetTransformNodeBetween(CoordSys::BaseFixed, CoordSys::FixedReference);
//  vtkMRMLLinearTransformNode* tableTopToTableTopEccentricRotationTransformNode =
//    this->GetTransformNodeBetween(TableTop, TableTopEccentricRotation);

  vtkNew<vtkTransform> fixedReferenceToRASTransform;
  fixedReferenceToRASTransform->Concatenate(fixedReferenceToRASTransformBeamComponent);
//  fixedReferenceToRASTransform->Concatenate(vtkTransform::SafeDownCast(tableTopToTableTopEccentricRotationTransformNode->GetTransformFromParent()));
  fixedReferenceToRASTransform->Concatenate(vtkTransform::SafeDownCast(baseFixedToFixedReferenceTransformNode->GetTransformFromParent()));

  fixedReferenceToRasTransformNode->SetAndObserveTransformToParent(fixedReferenceToRASTransform);
}

//-----------------------------------------------------------------------------
void vtkSlicerTableTopRobotTransformLogic::UpdateBaseFixedToFixedReferenceTransform(vtkMRMLChannel25GeometryNode* channelNode)
{
  if (!channelNode)
  {
    vtkErrorMacro("UpdateBaseFixedToFixedReferenceTransform: Invalid parameter set node");
    return;
  }
  using CoordSys = CoordinateSystemIdentifier;

  vtkMRMLLinearTransformNode* baseFixedToFixedReferenceTransformNode =
    this->GetTransformNodeBetween(CoordSys::BaseFixed, CoordSys::FixedReference);

  vtkNew<vtkTransform> baseFixedToFixedReferenceTransform;
  baseFixedToFixedReferenceTransform->RotateZ(channelNode->GetPatientSupportRotationAngle());
  baseFixedToFixedReferenceTransformNode->SetAndObserveTransformToParent(baseFixedToFixedReferenceTransform);
}

//-----------------------------------------------------------------------------
bool vtkSlicerTableTopRobotTransformLogic::GetTransformBetween(
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

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerTableTopRobotTransformLogic::GetElbowTransform()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("GetElbowTransform: Invalid MRML scene");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToElbowTransform"))
  {
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }

  return transformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerTableTopRobotTransformLogic::GetFixedReferenceTransform()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("GetFixedReferenceTransform: Invalid MRML scene");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToFixedReferenceTransform"))
  {
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }

  return transformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerTableTopRobotTransformLogic::GetPatientTransform()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("GetPatientTransform: Invalid MRML scene");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToPatientTransform"))
  {
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }

  return transformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerTableTopRobotTransformLogic::GetTableTopTransform()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("GetTableTopTransform: Invalid MRML scene");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToTableTopTransform"))
  {
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }

  return transformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerTableTopRobotTransformLogic::GetWristTransform()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("GetWristTransform: Invalid MRML scene");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToWristTransform"))
  {
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }

  return transformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerTableTopRobotTransformLogic::GetShoulderTransform()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("GetShoulderTransform: Invalid MRML scene");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToShoulderTransform"))
  {
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }

  return transformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerTableTopRobotTransformLogic::GetBaseRotationTransform()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("GetBaseRotationTransform: Invalid MRML scene");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToBaseRotationTransform"))
  {
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }

  return transformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerTableTopRobotTransformLogic::GetBaseFixedTransform()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("GetBaseFixedTransform: Invalid MRML scene");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToBaseFixedTransform"))
  {
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }

  return transformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerTableTopRobotTransformLogic::UpdateRasToTableTopTransform(vtkMRMLChannel25GeometryNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateRasToTableTopTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateRasToTableTopTransform: Invalid MRML scene");
    return nullptr;
  }

  // Display all pieces of the treatment room and sets each piece a color to provide realistic representation
  using CoordSys = CoordinateSystemIdentifier;

  // Transform IHEP stand models (IEC Patient) to RAS
  vtkNew<vtkTransform> patientToRasTransform;
  patientToRasTransform->RotateX(-90.);
  if (!parameterNode->GetPatientHeadFeetRotation())
  {
    patientToRasTransform->RotateZ(180.);
  }

  // TableTop -> RAS
  // Inverse transform path: RAS -> Patient -> TableTop
  // Find RasToTableTopMiddleTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableTopTransformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToTableTopTransform"))
  {
    rasToTableTopTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }
  else
  {
    rasToTableTopTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    rasToTableTopTransformNode->SetName("RasToTableTopTransform");
//    rasToTableTopTransformNode->SetHideFromEditors(1);
    std::string singletonTag = std::string("TTR_") + "RasToTableTopTransform";
    rasToTableTopTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(rasToTableTopTransformNode);
  }

  vtkNew<vtkTransform> rasToTableTopTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::TableTop, 
    rasToTableTopTransform, false))
  {
    vtkWarningMacro("UpdateRasToTableTopTransform: RAS->TableTop transform updated");
    // Transform to RAS, set transform to node, transform the model
    rasToTableTopTransform->Concatenate(patientToRasTransform);
  }
  if (rasToTableTopTransformNode)
  {
    rasToTableTopTransformNode->SetAndObserveTransformToParent(rasToTableTopTransform);
  }
  return rasToTableTopTransformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerTableTopRobotTransformLogic::UpdateRasToFixedReferenceTransform(vtkMRMLChannel25GeometryNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateRasToFixedReferenceTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateRasToFixedReferenceTransform: Invalid MRML scene");
    return nullptr;
  }

  // Display all pieces of the treatment room and sets each piece a color to provide realistic representation
  using CoordSys = CoordinateSystemIdentifier;

  // Transform IHEP stand models (IEC Patient) to RAS
  vtkNew<vtkTransform> patientToRasTransform;
  patientToRasTransform->RotateX(-90.);
  if (!parameterNode->GetPatientHeadFeetRotation())
  {
    patientToRasTransform->RotateZ(180.);
  }

  // Fixed Reference -> RAS
  // Fixed Reference - mandatory
  // Transform path: RAS -> Patient -> TableTop -> Wrist -> Elbow -> Shoulder -> BaseRotation -> BaseFixed -> FixedReference
  // Find RasToFixedReferenceTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToFixedReferenceTransformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToFixedReferenceTransform"))
  {
    rasToFixedReferenceTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }
  else
  {
    rasToFixedReferenceTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    rasToFixedReferenceTransformNode->SetName("RasToFixedReferenceTransform");
//    rasToFixedReferenceTransformNode->SetHideFromEditors(1);
    std::string singletonTag = std::string("TTR_") + "RasToFixedReferenceTransform";
    rasToFixedReferenceTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(rasToFixedReferenceTransformNode);
  }

  vtkNew<vtkTransform> rasToFixedReferenceTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::FixedReference, 
    rasToFixedReferenceTransform, false))
  {
    vtkWarningMacro("UpdateRasToFixedReferenceTransform: RAS->FixedReference transform updated");
    // Transform to RAS, set transform to node, transform the model
    rasToFixedReferenceTransform->Concatenate(patientToRasTransform);
  }
  if (rasToFixedReferenceTransformNode)
  {
    rasToFixedReferenceTransformNode->SetAndObserveTransformToParent(rasToFixedReferenceTransform);
  }
  return rasToFixedReferenceTransformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerTableTopRobotTransformLogic::UpdateRasToWristTransform(vtkMRMLChannel25GeometryNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateRasToWristTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateRasToWristTransform: Invalid MRML scene");
    return nullptr;
  }

  // Display all pieces of the treatment room and sets each piece a color to provide realistic representation
  using CoordSys = CoordinateSystemIdentifier;

  // Transform IHEP stand models (IEC Patient) to RAS
  vtkNew<vtkTransform> patientToRasTransform;
  patientToRasTransform->RotateX(-90.);
  if (!parameterNode->GetPatientHeadFeetRotation())
  {
    patientToRasTransform->RotateZ(180.);
  }

  // Wrist -> RAS
  // Wrist - mandatory
  // Transform path: RAS -> Patient -> TableTop -> Wrist
  // Find RasToWristTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToWristTransformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToWristTransform"))
  {
    rasToWristTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }
  else
  {
    rasToWristTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    rasToWristTransformNode->SetName("RasToWristTransform");
//    rasToWristTransformNode->SetHideFromEditors(1);
    std::string singletonTag = std::string("TTR_") + "RasToWristTransform";
    rasToWristTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(rasToWristTransformNode);
  }

  vtkNew<vtkTransform> rasToWristTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::Wrist, 
    rasToWristTransform, false))
  {
    vtkWarningMacro("UpdateRasToWristTransform: RAS->Wrist transform updated");
    // Transform to RAS, set transform to node, transform the model
    rasToWristTransform->Concatenate(patientToRasTransform);
  }
  if (rasToWristTransformNode)
  {
    rasToWristTransformNode->SetAndObserveTransformToParent(rasToWristTransform);
  }
  return rasToWristTransformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerTableTopRobotTransformLogic::UpdateRasToElbowTransform(vtkMRMLChannel25GeometryNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateRasToElbowTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateRasToElbowTransform: Invalid MRML scene");
    return nullptr;
  }

  // Display all pieces of the treatment room and sets each piece a color to provide realistic representation
  using CoordSys = CoordinateSystemIdentifier;

  // Transform IHEP stand models (IEC Patient) to RAS
  vtkNew<vtkTransform> patientToRasTransform;
  patientToRasTransform->RotateX(-90.);
  if (!parameterNode->GetPatientHeadFeetRotation())
  {
    patientToRasTransform->RotateZ(180.);
  }

  // Wrist -> RAS
  // Wrist - mandatory
  // Transform path: RAS -> Patient -> TableTop -> Wrist -> Elbow
  // Find RasToWristTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToElbowTransformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToElbowTransform"))
  {
    rasToElbowTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }
  else
  {
    rasToElbowTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    rasToElbowTransformNode->SetName("RasToElbowTransform");
//    rasToWristTransformNode->SetHideFromEditors(1);
    std::string singletonTag = std::string("TTR_") + "RasToElbowTransform";
    rasToElbowTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(rasToElbowTransformNode);
  }

  vtkNew<vtkTransform> rasToElbowTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::Elbow, 
    rasToElbowTransform, false))
  {
    vtkWarningMacro("UpdateRasToWristTransform: RAS->Wrist transform updated");
    // Transform to RAS, set transform to node, transform the model
    rasToElbowTransform->Concatenate(patientToRasTransform);
  }
  if (rasToElbowTransformNode)
  {
    rasToElbowTransformNode->SetAndObserveTransformToParent(rasToElbowTransform);
  }
  return rasToElbowTransformNode;
}

//----------------------------------------------------------------------------
void vtkSlicerTableTopRobotTransformLogic::UpdatePatientToTableTopTransform(vtkMRMLChannel25GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdatePatientToTableTopTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdatePatientToTableTopTransform: Invalid parameter node");
    return;
  }

  using CoordSys = CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* patientToTableTopTransformNode =
    this->GetTransformNodeBetween(CoordSys::Patient, CoordSys::TableTop);

  if (patientToTableTopTransformNode)
  {
    double patientToTableTopTranslation[3] = {};
    parameterNode->GetPatientToTableTopTranslation(patientToTableTopTranslation);
    patientToTableTopTranslation[0] *= -1;
    patientToTableTopTranslation[1] *= -1;
    patientToTableTopTranslation[2] *= -1;
    vtkNew<vtkTransform> patientToTableTopTransform;
    patientToTableTopTransform->RotateY(180);
    patientToTableTopTransform->Translate(patientToTableTopTranslation);
    patientToTableTopTransformNode->SetAndObserveTransformToParent(patientToTableTopTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerTableTopRobotTransformLogic::UpdateBaseRotationToBaseFixedTransform(vtkMRMLChannel25GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateBaseRotationToBaseFixedTransform: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("UpdateBaseRotationToBaseFixedTransform: Invalid parameter node");
    return;
  }

  using CoordSys = CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* baseRotationToBaseFixedTransformNode =
    this->GetTransformNodeBetween(CoordSys::BaseRotation, CoordSys::BaseFixed);

  if (baseRotationToBaseFixedTransformNode)
  {
    vtkNew<vtkTransform> baseRotationToBaseFixedTransform;
    baseRotationToBaseFixedTransform->RotateZ(-1. * parameterNode->GetPatientSupportRotationAngle());
    // apply transform
    baseRotationToBaseFixedTransformNode->SetAndObserveTransformToParent(baseRotationToBaseFixedTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerTableTopRobotTransformLogic::UpdateTableTopToWristTransform(vtkMRMLChannel25GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableTopToWristTransform: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("UpdateTableTopToWristTransform: Invalid parameter node");
    return;
  }

  using CoordSys = CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* tableTopToWristTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableTop, CoordSys::Wrist);

  if (tableTopToWristTransformNode)
  {
    vtkErrorMacro("UpdateTableTopToWristTransform: 1");
    vtkNew<vtkTransform> tableTopToWristTransform;
    double a[6] = {};
    parameterNode->GetTableTopRobotAngles(a);
    double ttw[3] = {};
    parameterNode->GetTableTopToWristTranslation(ttw);
//    tableTopToWristTransform->RotateZ(a[5]);
    tableTopToWristTransform->Translate(ttw);
    tableTopToWristTransformNode->SetAndObserveTransformToParent(tableTopToWristTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerTableTopRobotTransformLogic::UpdateWristToElbowTransform(vtkMRMLChannel25GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateWristToElbowTransform: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("UpdateWristToElbowTransform: Invalid parameter node");
    return;
  }

  using CoordSys = CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* wristToElbowTransformNode =
    this->GetTransformNodeBetween(CoordSys::Wrist, CoordSys::Elbow);

  if (wristToElbowTransformNode)
  {
    double a[6] = {};
    parameterNode->GetTableTopRobotAngles(a);
    vtkNew<vtkTransform> wristToElbowTransform;
    wristToElbowTransform->RotateY(a[5]);
    wristToElbowTransform->RotateZ(a[4]);
    wristToElbowTransformNode->SetAndObserveTransformToParent(wristToElbowTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerTableTopRobotTransformLogic::UpdateElbowToShoulderTransform(vtkMRMLChannel25GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateElbowToShoulderTransform: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("UpdateElbowToShoulderTransform: Invalid parameter node");
    return;
  }

  using CoordSys = CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* elbowToShoulderTransformNode =
    this->GetTransformNodeBetween(CoordSys::Elbow, CoordSys::Shoulder);

  if (elbowToShoulderTransformNode)
  {
    double a[6] = {};
    parameterNode->GetTableTopRobotAngles(a);
    vtkNew<vtkTransform> elbowToShoulderTransform;
    elbowToShoulderTransform->RotateY(a[3]);
    elbowToShoulderTransformNode->SetAndObserveTransformToParent(elbowToShoulderTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerTableTopRobotTransformLogic::UpdateShoulderToBaseRotationTransform(vtkMRMLChannel25GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateShoulderToBaseRotationTransform: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("UpdateShoulderToBaseRotationTransform: Invalid parameter node");
    return;
  }

  using CoordSys = CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* shoulderToBaseRotationTransformNode =
    this->GetTransformNodeBetween(CoordSys::Shoulder, CoordSys::BaseRotation);

  if (shoulderToBaseRotationTransformNode)
  {
    double a[6] = {};
    parameterNode->GetTableTopRobotAngles(a);
    vtkNew<vtkTransform> shoulderToBaseRotationTransform;
    shoulderToBaseRotationTransform->RotateY(a[3]);
    shoulderToBaseRotationTransformNode->SetAndObserveTransformToParent(shoulderToBaseRotationTransform);
  }
}

//-----------------------------------------------------------------------------
bool vtkSlicerTableTopRobotTransformLogic::GetTransformBetween(
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
bool vtkSlicerTableTopRobotTransformLogic::GetPathToRoot( CoordinateSystemIdentifier frame, 
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
bool vtkSlicerTableTopRobotTransformLogic::GetPathFromRoot( CoordinateSystemIdentifier frame, 
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
