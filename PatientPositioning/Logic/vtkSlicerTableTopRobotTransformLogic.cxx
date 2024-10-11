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

constexpr std::array< double, 3 > FixedReferenceToFixedBasedOffset{ -1600., -1500., 1400. };

constexpr double BaseFixedHeight = 240.; // mm
constexpr double BaseRotationHeight = 675. - BaseFixedHeight; // mm
constexpr double BaseRotationShoulderDiskCenterOffsetX = 350; // mm
constexpr double BaseRotationShoulderDiskCenterOffsetY = BaseRotationHeight; // mm
constexpr double TableThickness = 90.; // mm
constexpr double FlangeLength = 300.; // mm
constexpr double WristLength = 215.; // mm
constexpr double ElbowLength = 1200.; // mm
constexpr double ShoulderLength = 1150. - 41.; // mm

constexpr std::array< double, 3 > InitialTableTopCenterOffsetRAS{ 0.5, 821.6, -210.};
constexpr std::array< double, 3 > InitialFlangeOriginOffsetRAS{
  InitialTableTopCenterOffsetRAS[0],
  InitialTableTopCenterOffsetRAS[1], 
  InitialTableTopCenterOffsetRAS[2] - TableThickness };
constexpr std::array< double, 3 > InitialElbowWristJointOffsetRAS{
  InitialFlangeOriginOffsetRAS[0],
  InitialFlangeOriginOffsetRAS[1], 
  InitialFlangeOriginOffsetRAS[2] - (FlangeLength + WristLength) };
constexpr std::array< double, 3 > InitialSholderElbowJointOffsetRAS{
  InitialElbowWristJointOffsetRAS[0] - ElbowLength,
  InitialElbowWristJointOffsetRAS[1], 
  InitialElbowWristJointOffsetRAS[2] };

}

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerTableTopRobotTransformLogic);

//-----------------------------------------------------------------------------
vtkSlicerTableTopRobotTransformLogic::vtkSlicerTableTopRobotTransformLogic()
{
  using CoordSys = CoordinateSystemIdentifier;
  // Setup coordinate system ID to name map
  this->CoordinateSystemsMap.clear();
  this->CoordinateSystemsMap[CoordSys::RAS] = "TTRRAS";
  this->CoordinateSystemsMap[CoordSys::FixedReference] = "TTRFixedReference";
  this->CoordinateSystemsMap[CoordSys::BaseFixed] = "TTRBaseFixed";
  this->CoordinateSystemsMap[CoordSys::BaseRotation] = "TTRBaseRotation";
  this->CoordinateSystemsMap[CoordSys::Shoulder] = "TTRShoulder";
  this->CoordinateSystemsMap[CoordSys::Elbow] = "TTRElbow";
  this->CoordinateSystemsMap[CoordSys::Wrist] = "TTRWrist";
  this->CoordinateSystemsMap[CoordSys::Flange] = "TTRFlange";
  this->CoordinateSystemsMap[CoordSys::TableTop] = "TTRTableTop";
  this->CoordinateSystemsMap[CoordSys::Patient] = "TTRPatient";

  this->TableTopRobotTransforms.clear();
  this->TableTopRobotTransforms.push_back(std::make_pair(CoordSys::FixedReference, CoordSys::RAS)); // Dummy
  this->TableTopRobotTransforms.push_back(std::make_pair(CoordSys::BaseFixed, CoordSys::FixedReference)); // Collimator in canyon system
  this->TableTopRobotTransforms.push_back(std::make_pair(CoordSys::BaseRotation, CoordSys::BaseFixed)); // Rotation of patient support platform
  this->TableTopRobotTransforms.push_back(std::make_pair(CoordSys::Shoulder, CoordSys::BaseRotation)); // Lateral movement along Y-axis in RAS of the table top
  this->TableTopRobotTransforms.push_back(std::make_pair(CoordSys::Elbow, CoordSys::Shoulder)); // Longitudinal movement along X-axis in RAS of the table top
  this->TableTopRobotTransforms.push_back(std::make_pair(CoordSys::Wrist, CoordSys::Elbow)); // Vertical movement of table top origin
  this->TableTopRobotTransforms.push_back(std::make_pair(CoordSys::Flange, CoordSys::Wrist)); // Rotation of flange on table top center
  this->TableTopRobotTransforms.push_back(std::make_pair(CoordSys::TableTop, CoordSys::Flange)); // Dummy, only fixed translation
  this->TableTopRobotTransforms.push_back(std::make_pair(CoordSys::Patient, CoordSys::TableTop));
  this->TableTopRobotTransforms.push_back(std::make_pair(CoordSys::RAS, CoordSys::Patient));

  this->CoordinateSystemsHierarchy.clear();
  // key - parent, value - children
  this->CoordinateSystemsHierarchy[CoordSys::FixedReference] = { CoordSys::BaseFixed };
  this->CoordinateSystemsHierarchy[CoordSys::BaseFixed] = { CoordSys::BaseRotation };
  this->CoordinateSystemsHierarchy[CoordSys::BaseRotation] = { CoordSys::Shoulder };
  this->CoordinateSystemsHierarchy[CoordSys::Shoulder] = { CoordSys::Elbow };
  this->CoordinateSystemsHierarchy[CoordSys::Elbow] = { CoordSys::Wrist };
  this->CoordinateSystemsHierarchy[CoordSys::Wrist] = { CoordSys::Flange };
  this->CoordinateSystemsHierarchy[CoordSys::Flange] = { CoordSys::TableTop };
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
    case Flange: return "Flange";
    case Elbow: return "RobotElbow";
    case Wrist: return "RobotWrist";
    case Patient: return "Patient";
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

  // Organize transforms into hierarchy based on table top robot RBS geometry

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
  this->GetTransformNodeBetween(CoordSys::Flange, CoordSys::Wrist)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::Wrist, CoordSys::Elbow)->GetID() );
  this->GetTransformNodeBetween(CoordSys::TableTop, CoordSys::Flange)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::Flange, CoordSys::Wrist)->GetID() );

  // TableTop parent, translation of patient from wrist flange center
  this->GetTransformNodeBetween( CoordSys::Patient, CoordSys::TableTop)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::TableTop, CoordSys::Flange)->GetID() );

  // Patient parent, transform to RAS
  this->GetTransformNodeBetween( CoordSys::RAS, CoordSys::Patient)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween( CoordSys::Patient, CoordSys::TableTop)->GetID() );
}

//-----------------------------------------------------------------------------
void vtkSlicerTableTopRobotTransformLogic::ResetToInitialPositions()
{
  using CoordSys = CoordinateSystemIdentifier;
  // Update IEC Patient to RAS transform based on the isocenter defined in the beam's parent plan
  vtkMRMLLinearTransformNode* rasToPatientReferenceTransformNode =
    this->GetTransformNodeBetween( CoordSys::RAS, CoordSys::Patient);
  vtkTransform* rasToPatientReferenceTransform = vtkTransform::SafeDownCast(rasToPatientReferenceTransformNode->GetTransformToParent());
  rasToPatientReferenceTransform->Identity();
  rasToPatientReferenceTransform->RotateX(-90.);
  rasToPatientReferenceTransform->RotateY(180.);
  rasToPatientReferenceTransform->Modified();

  vtkMRMLLinearTransformNode* patientToTableTopTransformNode =
    this->GetTransformNodeBetween(CoordSys::Patient, CoordSys::TableTop);
  vtkTransform* patientToTableTopTransform = vtkTransform::SafeDownCast(patientToTableTopTransformNode->GetTransformToParent());
  patientToTableTopTransform->Identity();
  patientToTableTopTransform->Modified();

  vtkMRMLLinearTransformNode* tableTopToFlangeTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableTop, CoordSys::Flange);
  vtkTransform* tableTopToFlangeTransform = vtkTransform::SafeDownCast(tableTopToFlangeTransformNode->GetTransformToParent());
  tableTopToFlangeTransform->Identity();
  tableTopToFlangeTransform->Modified();

  vtkMRMLLinearTransformNode* flangeToWristTransformNode =
    this->GetTransformNodeBetween(CoordSys::Flange, CoordSys::Wrist);
  vtkTransform* flangeToWristTransform = vtkTransform::SafeDownCast(flangeToWristTransformNode->GetTransformToParent());
  flangeToWristTransform->Identity();
  flangeToWristTransform->Modified();

  vtkMRMLLinearTransformNode* wristToElbowTransformNode =
    this->GetTransformNodeBetween(CoordSys::Wrist, CoordSys::Elbow);
  vtkTransform* wristToElbowTransform = vtkTransform::SafeDownCast(wristToElbowTransformNode->GetTransformToParent());
  wristToElbowTransform->Identity();
  wristToElbowTransform->Modified();

  vtkMRMLLinearTransformNode* elbowToShoulderTransformNode =
    this->GetTransformNodeBetween(CoordSys::Elbow, CoordSys::Shoulder);
  vtkTransform* elbowToShoulderTransform = vtkTransform::SafeDownCast(elbowToShoulderTransformNode->GetTransformToParent());
  elbowToShoulderTransform->Identity();
  elbowToShoulderTransform->Modified();

  vtkMRMLLinearTransformNode* shoulderToBaseRotationTransformNode =
    this->GetTransformNodeBetween(CoordSys::Shoulder, CoordSys::BaseRotation);
  vtkTransform* shoulderToBaseRotationTransform = vtkTransform::SafeDownCast(shoulderToBaseRotationTransformNode->GetTransformToParent());
  shoulderToBaseRotationTransform->Identity();
  shoulderToBaseRotationTransform->Modified();

  vtkMRMLLinearTransformNode* baseRotationToBaseFixedTransformNode =
    this->GetTransformNodeBetween(CoordSys::BaseRotation, CoordSys::BaseFixed);
  vtkTransform* baseRotationToBaseFixedTransform = vtkTransform::SafeDownCast(baseRotationToBaseFixedTransformNode->GetTransformToParent());
  baseRotationToBaseFixedTransform->Identity();
  baseRotationToBaseFixedTransform->Modified();

  vtkMRMLLinearTransformNode* baseFixedToFixedRerefenceTransformNode =
    this->GetTransformNodeBetween(CoordSys::BaseFixed, CoordSys::FixedReference);
  vtkTransform* baseFixedToFixedRerefenceTransform = vtkTransform::SafeDownCast(baseFixedToFixedRerefenceTransformNode->GetTransformToParent());
  baseFixedToFixedRerefenceTransform->Identity();
  baseFixedToFixedRerefenceTransform->Modified();
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
void vtkSlicerTableTopRobotTransformLogic::UpdateBaseFixedToFixedReferenceTransform(vtkMRMLChannel25GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateBaseRotationToBaseFixedTransform: Invalid scene");
    return;
  }
  if (!parameterNode/* || !parameterNode->GetTreatmentMachineType() */)
  {
    vtkErrorMacro("UpdateBaseRotationToBaseFixedTransform: Invalid parameter node");
    return;
  }

  // Translate the BaseRotation so it's empty disk centre in RAS origin
  vtkNew<vtkTransform> BaseFixedTranslateTransform;
  BaseFixedTranslateTransform->Translate(0., 0., -1. * BaseFixedHeight);
  double BaseFixedToFixedReferenceTranslate[3] = {};
  parameterNode->GetBaseFixedToFixedReferenceTranslation(BaseFixedToFixedReferenceTranslate);
  // Default: FixedReferenceToFixedBasedOffset.data()
  BaseFixedTranslateTransform->Translate(BaseFixedToFixedReferenceTranslate);

  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> baseFixedToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::BaseFixed, CoordSys::Patient, 
    baseFixedToPatientTransform, false))
  {
    vtkWarningMacro("UpdateShoulderToBaseRotationTransform: Can't get BaseFixed->Patient transform");
  }

  double PatientToBaseFixedTranslate[3] = {};
  vtkNew<vtkTransform> patientToBaseFixedTransform;
  if (this->GetTransformBetween( CoordSys::Patient, CoordSys::BaseFixed, 
    patientToBaseFixedTransform, false))
  {
    patientToBaseFixedTransform->GetPosition(PatientToBaseFixedTranslate);
  }

  vtkMRMLLinearTransformNode* baseFixedToFixedReferenceTransformNode =
    this->GetTransformNodeBetween(CoordSys::BaseFixed, CoordSys::FixedReference);
  if (baseFixedToFixedReferenceTransformNode)
  {
    double a[6] = {};
    parameterNode->GetTableTopRobotAngles(a);
    double patientToTableTopTranslation[3] = {};
    parameterNode->GetPatientToTableTopTranslation(patientToTableTopTranslation);

    // BaseRotation->BaseFixed rotation around Z (A1 angle)
    vtkNew<vtkTransform> baseRotationToBaseFixedTransform;
    baseRotationToBaseFixedTransform->RotateZ(a[0]);

    // Shoulder->BaseRotation rotation around Y (A2 angle)
    vtkNew<vtkTransform> shoulderToBaseRotationTransform;
    shoulderToBaseRotationTransform->RotateY(a[1]);
  
    // Elbow->Shoulder rotation around Y (A3 angle)
    vtkNew<vtkTransform> ElbowToShoulderRotationTransform;
    ElbowToShoulderRotationTransform->RotateY(a[2]);

    // Apply transform (rotation around Y axis on A5 angle, around X axis on A4 angle and around Z axis on A6 angle in RAS origin)
    vtkNew<vtkTransform> A6A5A4RotationTransform;
    A6A5A4RotationTransform->RotateZ(a[5]);
    A6A5A4RotationTransform->RotateY(a[4]);
    A6A5A4RotationTransform->RotateX(a[3]);

    // Translate BaseFixed disk end (top) to RAS (Patient) origin so, it's end (top) in RAS origin
    BaseFixedTranslateTransform->Concatenate(baseFixedToPatientTransform);
    // Apply A1 angle transform
    baseRotationToBaseFixedTransform->Concatenate(BaseFixedTranslateTransform);
    // Apply A2 angle transform
    shoulderToBaseRotationTransform->Concatenate(baseRotationToBaseFixedTransform);
    // Apply A3 angle transform
    ElbowToShoulderRotationTransform->Concatenate(shoulderToBaseRotationTransform);
    // Apply A6, A5, A4 angles transform
    A6A5A4RotationTransform->Concatenate(ElbowToShoulderRotationTransform);

    // Translate FixedReference model to the begin (bottom) of BaseFixed model (Patient->BaseFixed translation)
    vtkNew<vtkTransform> PatientToFixedReferenceTranslateTransform;
    PatientToFixedReferenceTranslateTransform->Translate(PatientToBaseFixedTranslate);
    // Apply angles transform
    PatientToFixedReferenceTranslateTransform->Concatenate(A6A5A4RotationTransform);

    baseFixedToFixedReferenceTransformNode->SetAndObserveTransformToParent(PatientToFixedReferenceTranslateTransform);
  }
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
vtkMRMLLinearTransformNode* vtkSlicerTableTopRobotTransformLogic::GetFlangeTransform()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("GetFlangeTransform: Invalid MRML scene");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToFlangeTransform"))
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

  // Transform robot models to RAS
  vtkNew<vtkTransform> patientToRasTransform;
  patientToRasTransform->RotateX(-90.);
  if (parameterNode->GetPatientHeadFeetRotation())
  {
    patientToRasTransform->RotateZ(180.);
  }

  // TableTop -> RAS
  // TableTop - mandatory
  // Inverse transform path: RAS -> Patient -> TableTop
  // Find RasToTableTopTransform or create it
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
    vtkDebugMacro("UpdateRasToTableTopTransform: RAS->TableTop transform updated");
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

  // Transform robot models to RAS
  vtkNew<vtkTransform> patientToRasTransform;
  patientToRasTransform->RotateX(-90.);
  if (parameterNode->GetPatientHeadFeetRotation())
  {
    patientToRasTransform->RotateZ(180.);
  }

  // Fixed Reference -> RAS
  // Fixed Reference - mandatory
  // Transform path: RAS -> Patient -> TableTop -> Flange -> Wrist -> Elbow -> Shoulder -> BaseRotation -> BaseFixed -> FixedReference
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
    vtkDebugMacro("UpdateRasToFixedReferenceTransform: RAS->FixedReference transform updated");
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
vtkMRMLLinearTransformNode* vtkSlicerTableTopRobotTransformLogic::UpdateRasToBaseFixedTransform(vtkMRMLChannel25GeometryNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateRasToBaseFixedTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateRasToBaseFixedTransform: Invalid MRML scene");
    return nullptr;
  }

  // Display all pieces of the treatment room and sets each piece a color to provide realistic representation
  using CoordSys = CoordinateSystemIdentifier;

  // Transform robot models to RAS
  vtkNew<vtkTransform> patientToRasTransform;
  patientToRasTransform->RotateX(-90.);
  if (parameterNode->GetPatientHeadFeetRotation())
  {
    patientToRasTransform->RotateZ(180.);
  }

  // BaseFixed -> RAS
  // BaseFixed - mandatory
  // Transform path: RAS -> Patient -> TableTop -> Flange -> Wrist -> Elbow -> Shoulder -> BaseRotation -> BaseFixed
  // Find RasToBaseFixedTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToBaseFixedTransformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToBaseFixedTransform"))
  {
    rasToBaseFixedTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }
  else
  {
    rasToBaseFixedTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    rasToBaseFixedTransformNode->SetName("RasToBaseFixedTransform");
//    rasToBaseFixedTransformNode->SetHideFromEditors(1);
    std::string singletonTag = std::string("TTR_") + "RasToBaseFixedTransform";
    rasToBaseFixedTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(rasToBaseFixedTransformNode);
  }

  vtkNew<vtkTransform> rasToBaseFixedTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::BaseFixed, 
    rasToBaseFixedTransform, false))
  {
    vtkDebugMacro("UpdateRasToBaseFixedTransform: RAS->BaseFixed transform updated");
    // Transform to RAS, set transform to node, transform the model
    rasToBaseFixedTransform->Concatenate(patientToRasTransform);
  }
  if (rasToBaseFixedTransformNode)
  {
    rasToBaseFixedTransformNode->SetAndObserveTransformToParent(rasToBaseFixedTransform);
  }
  return rasToBaseFixedTransformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerTableTopRobotTransformLogic::UpdateRasToFlangeTransform(vtkMRMLChannel25GeometryNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateRasToFlangeTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateRasToFlangeTransform: Invalid MRML scene");
    return nullptr;
  }

  // Display all pieces of the treatment room and sets each piece a color to provide realistic representation
  using CoordSys = CoordinateSystemIdentifier;

  // Transform robot models to RAS
  vtkNew<vtkTransform> patientToRasTransform;
  patientToRasTransform->RotateX(-90.);
  if (parameterNode->GetPatientHeadFeetRotation())
  {
    patientToRasTransform->RotateZ(180.);
  }

  // Flange -> RAS
  // Flange - mandatory
  // Transform path: RAS -> Patient -> TableTop -> Flange
  // Find RasToFlangeTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToFlangeTransformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToFlangeTransform"))
  {
    rasToFlangeTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }
  else
  {
    rasToFlangeTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    rasToFlangeTransformNode->SetName("RasToFlangeTransform");
//    rasToBaseFixedTransformNode->SetHideFromEditors(1);
    std::string singletonTag = std::string("TTR_") + "RasToFlangeTransform";
    rasToFlangeTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(rasToFlangeTransformNode);
  }

  vtkNew<vtkTransform> rasToFlangeTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::Flange, 
    rasToFlangeTransform, false))
  {
    vtkDebugMacro("UpdateRasToFlangeTransform: RAS->Flange transform updated");
    // Transform to RAS, set transform to node, transform the model
    rasToFlangeTransform->Concatenate(patientToRasTransform);
  }
  if (rasToFlangeTransformNode)
  {
    rasToFlangeTransformNode->SetAndObserveTransformToParent(rasToFlangeTransform);
  }
  return rasToFlangeTransformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerTableTopRobotTransformLogic::UpdateRasToBaseRotationTransform(vtkMRMLChannel25GeometryNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateRasToBaseRotationTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateRasToBaseRotationTransform: Invalid MRML scene");
    return nullptr;
  }

  // Display all pieces of the treatment room and sets each piece a color to provide realistic representation
  using CoordSys = CoordinateSystemIdentifier;

  // Transform robot models to RAS
  vtkNew<vtkTransform> patientToRasTransform;
  patientToRasTransform->RotateX(-90.);
  if (parameterNode->GetPatientHeadFeetRotation())
  {
    patientToRasTransform->RotateZ(180.);
  }

  // BaseRotation -> RAS
  // BaseRotation - mandatory
  // Transform path: RAS -> Patient -> TableTop -> Flange -> Wrist -> Elbow -> Shoulder -> BaseRotation
  // Find RasToBaseRotationTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToBaseRotationTransformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToBaseRotationTransform"))
  {
    rasToBaseRotationTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }
  else
  {
    rasToBaseRotationTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    rasToBaseRotationTransformNode->SetName("RasToBaseRotationTransform");
//    rasToBaseRotationTransformNode->SetHideFromEditors(1);
    std::string singletonTag = std::string("TTR_") + "RasToBaseRotationTransform";
    rasToBaseRotationTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(rasToBaseRotationTransformNode);
  }

  vtkNew<vtkTransform> rasToBaseRotationTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::BaseRotation, 
    rasToBaseRotationTransform, false))
  {
    vtkDebugMacro("UpdateRasToBaseRotationTransform: RAS->BaseRotation transform updated");
    // Transform to RAS, set transform to node, transform the model
    rasToBaseRotationTransform->Concatenate(patientToRasTransform);
  }
  if (rasToBaseRotationTransformNode)
  {
    rasToBaseRotationTransformNode->SetAndObserveTransformToParent(rasToBaseRotationTransform);
  }
  return rasToBaseRotationTransformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerTableTopRobotTransformLogic::UpdateRasToShoulderTransform(vtkMRMLChannel25GeometryNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateRasToShoulderTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateRasToShoulderTransform: Invalid MRML scene");
    return nullptr;
  }

  // Display all pieces of the treatment room and sets each piece a color to provide realistic representation
  using CoordSys = CoordinateSystemIdentifier;

  // Transform robot models to RAS
  vtkNew<vtkTransform> patientToRasTransform;
  patientToRasTransform->RotateX(-90.);
  if (parameterNode->GetPatientHeadFeetRotation())
  {
    patientToRasTransform->RotateZ(180.);
  }

  // Shoulder -> RAS
  // Shoulder - mandatory
  // Transform path: RAS -> Patient -> TableTop -> Flange -> Wrist -> Elbow -> Shoulder
  // Find RasToShoulderTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToShoulderTransformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToShoulderTransform"))
  {
    rasToShoulderTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }
  else
  {
    rasToShoulderTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    rasToShoulderTransformNode->SetName("RasToShoulderTransform");
//    rasToShoulderTransformNode->SetHideFromEditors(1);
    std::string singletonTag = std::string("TTR_") + "RasToShoulderTransform";
    rasToShoulderTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(rasToShoulderTransformNode);
  }

  vtkNew<vtkTransform> rasToShoulderTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::Shoulder, 
    rasToShoulderTransform, false))
  {
    vtkDebugMacro("UpdateRasToShoulderTransform: RAS->Shoulder transform updated");
    // Transform to RAS, set transform to node, transform the model
    rasToShoulderTransform->Concatenate(patientToRasTransform);
  }
  if (rasToShoulderTransformNode)
  {
    rasToShoulderTransformNode->SetAndObserveTransformToParent(rasToShoulderTransform);
  }
  return rasToShoulderTransformNode;
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

  // Transform robot models to RAS
  vtkNew<vtkTransform> patientToRasTransform;
  patientToRasTransform->RotateX(-90.);
  if (parameterNode->GetPatientHeadFeetRotation())
  {
    patientToRasTransform->RotateZ(180.);
  }

  // Wrist -> RAS
  // Wrist - mandatory
  // Transform path: RAS -> Patient -> TableTop -> Flange -> Wrist
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
    vtkDebugMacro("UpdateRasToWristTransform: RAS->Wrist transform updated");
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

  // Transform robot models to RAS
  vtkNew<vtkTransform> patientToRasTransform;
  patientToRasTransform->RotateX(-90.);
  if (parameterNode->GetPatientHeadFeetRotation())
  {
    patientToRasTransform->RotateZ(180.);
  }

  // Elbow -> RAS
  // Elbow - mandatory
  // Transform path: RAS -> Patient -> TableTop -> Flange -> Wrist -> Elbow
  // Find RasToElbowTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToElbowTransformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToElbowTransform"))
  {
    rasToElbowTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }
  else
  {
    rasToElbowTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    rasToElbowTransformNode->SetName("RasToElbowTransform");
//    rasToElbowTransformNode->SetHideFromEditors(1);
    std::string singletonTag = std::string("TTR_") + "RasToElbowTransform";
    rasToElbowTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(rasToElbowTransformNode);
  }

  vtkNew<vtkTransform> rasToElbowTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::Elbow, 
    rasToElbowTransform, false))
  {
    vtkDebugMacro("UpdateRasToElbowTransform: RAS->Elbow transform updated");
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
  if (!parameterNode/* || !parameterNode->GetTreatmentMachineType() */)
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
  if (!parameterNode/* || !parameterNode->GetTreatmentMachineType() */)
  {
    vtkErrorMacro("UpdateBaseRotationToBaseFixedTransform: Invalid parameter node");
    return;
  }

  // Translate the BaseRotation so it's empty disk centre in RAS origin
  vtkNew<vtkTransform> BaseFixedTranslateTransform;
  BaseFixedTranslateTransform->Translate(0., 0., -1. * BaseFixedHeight);

  // Transform model to vertical position
  vtkNew<vtkTransform> BaseFixedVerticalOrientationTransform; // vertical orientation
  BaseFixedVerticalOrientationTransform->Identity();
  BaseFixedVerticalOrientationTransform->RotateX(-90.);

  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> baseRotationToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::BaseRotation, CoordSys::Patient, 
    baseRotationToPatientTransform, false))
  {
    vtkWarningMacro("UpdateShoulderToBaseRotationTransform: Can't get BaseRotation->Patient transform");
  }

  double PatientToBaseRotationTranslate[3] = {};
  vtkNew<vtkTransform> patientToBaseRotationTransform;
  if (this->GetTransformBetween( CoordSys::Patient, CoordSys::BaseRotation, 
    patientToBaseRotationTransform, false))
  {
    patientToBaseRotationTransform->GetPosition(PatientToBaseRotationTranslate);
  }

  vtkMRMLLinearTransformNode* baseRotationToBaseFixedTransformNode =
    this->GetTransformNodeBetween(CoordSys::BaseRotation, CoordSys::BaseFixed);
  if (baseRotationToBaseFixedTransformNode)
  {
    double a[6] = {};
    parameterNode->GetTableTopRobotAngles(a);
    double patientToTableTopTranslation[3] = {};
    parameterNode->GetPatientToTableTopTranslation(patientToTableTopTranslation);

    // BaseRotation->BaseFixed rotation around Z (A1 angle)
    vtkNew<vtkTransform> baseRotationToBaseFixedTransform;
    baseRotationToBaseFixedTransform->RotateZ(a[0]);

    // Shoulder->BaseRotation rotation around Y (A2 angle)
    vtkNew<vtkTransform> shoulderToBaseRotationTransform;
    shoulderToBaseRotationTransform->RotateY(a[1]);
  
    // Elbow->Shoulder rotation around Y (A3 angle)
    vtkNew<vtkTransform> ElbowToShoulderRotationTransform;
    ElbowToShoulderRotationTransform->RotateY(a[2]);

    // Apply transform (rotation around Y axis on A5 angle, around X axis on A4 angle and around Z axis on A6 angle in RAS origin)
    vtkNew<vtkTransform> A6A5A4RotationTransform;
    A6A5A4RotationTransform->RotateZ(a[5]);
    A6A5A4RotationTransform->RotateY(a[4]);
    A6A5A4RotationTransform->RotateX(a[3]);

    // Transform shoulder in RAS (Patient) origin so, it's begin in RAS origin
    // Transform to RAS origin and model vertical orientation
    BaseFixedVerticalOrientationTransform->Concatenate(baseRotationToPatientTransform);
    // Translate BaseFixed disk end (top) to RAS (Patient) origin so, it's end (top) in RAS origin
    BaseFixedTranslateTransform->Concatenate(BaseFixedVerticalOrientationTransform);
    // Apply A1 angle transform
    baseRotationToBaseFixedTransform->Concatenate(BaseFixedTranslateTransform);
    // Apply A2 angle transform
    shoulderToBaseRotationTransform->Concatenate(baseRotationToBaseFixedTransform);
    // Apply A3 angle transform
    ElbowToShoulderRotationTransform->Concatenate(shoulderToBaseRotationTransform);
    // Apply A6, A5, A4 angles transform
    A6A5A4RotationTransform->Concatenate(ElbowToShoulderRotationTransform);

    // Translate BaseFixed model to the begin (bottom) of BaseRotation model (Patient->BaseRotation translation)
    vtkNew<vtkTransform> PatientToBaseFixedTranslateTransform;
    PatientToBaseFixedTranslateTransform->Translate(PatientToBaseRotationTranslate);
    // Apply angles transform
    PatientToBaseFixedTranslateTransform->Concatenate(A6A5A4RotationTransform);

    baseRotationToBaseFixedTransformNode->SetAndObserveTransformToParent(PatientToBaseFixedTranslateTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerTableTopRobotTransformLogic::UpdateTableTopToFlangeTransform(vtkMRMLChannel25GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableTopToWristTransform: Invalid scene");
    return;
  }
  if (!parameterNode/* || !parameterNode->GetTreatmentMachineType() */)
  {
    vtkErrorMacro("UpdateTableTopToWristTransform: Invalid parameter node");
    return;
  }

  using CoordSys = CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* tableTopToFlangeTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableTop, CoordSys::Flange);

  if (tableTopToFlangeTransformNode)
  {
    vtkNew<vtkTransform> tableTopToFlangeTransform;
    double patientToTableTopTranslation[3] = {};
    parameterNode->GetPatientToTableTopTranslation(patientToTableTopTranslation);
    /// Vertical orientation of Flange (rotation on 90 deg along X)
    vtkNew<vtkTransform> FlangeVerticalOrientationTransform; // vertical orientation
    FlangeVerticalOrientationTransform->Identity();
    FlangeVerticalOrientationTransform->RotateX(-90.);
    double posOrig[4] = { patientToTableTopTranslation[0], patientToTableTopTranslation[1], patientToTableTopTranslation[2], 1. };
    double posRes[4] = { };
    // Patient to table top translation in vertical orientation
    FlangeVerticalOrientationTransform->MultiplyPoint(posOrig, posRes);

    // Compensate Patient->TableTop translation
    tableTopToFlangeTransform->Translate(posRes);
    // Concatinate with flange vertical orientation
    tableTopToFlangeTransform->Concatenate(FlangeVerticalOrientationTransform);
    vtkNew<vtkTransform> TableTopToFlangeTranslateTransform;
    // Reverse compensate Patient->TableTop translation
    TableTopToFlangeTranslateTransform->Translate(-1. * patientToTableTopTranslation[0], -1. * patientToTableTopTranslation[1], -1. * patientToTableTopTranslation[2]);
    // Translate to Flange top position under table top center position
    TableTopToFlangeTranslateTransform->Translate(-1. * InitialFlangeOriginOffsetRAS[0], -1. * InitialFlangeOriginOffsetRAS[1], InitialFlangeOriginOffsetRAS[2] - FlangeLength);
    TableTopToFlangeTranslateTransform->Concatenate(tableTopToFlangeTransform);
    tableTopToFlangeTransformNode->SetAndObserveTransformToParent(TableTopToFlangeTranslateTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerTableTopRobotTransformLogic::UpdateFlangeToWristTransform(vtkMRMLChannel25GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateFlangeToWristTransform: Invalid scene");
    return;
  }
  if (!parameterNode/* || !parameterNode->GetTreatmentMachineType() */)
  {
    vtkErrorMacro("UpdateFlangeToWristTransform: Invalid parameter node");
    return;
  }

  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> flangeToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::Flange, CoordSys::Patient, 
    flangeToPatientTransform, false))
  {
    vtkWarningMacro("UpdateFlangeToWristTransform: Can't get Flange->Patient transform");
    return;
  }

  vtkNew<vtkTransform> ReverseWristVerticalOrientationTransform; // vertical orientation
  ReverseWristVerticalOrientationTransform->Identity();
  ReverseWristVerticalOrientationTransform->RotateY(90.);

  vtkMRMLLinearTransformNode* flangeToWristTransformNode =
    this->GetTransformNodeBetween(CoordSys::Flange, CoordSys::Wrist);
  if (flangeToWristTransformNode)
  {
    vtkNew<vtkTransform> FlangeToWristTransform;
    double a[6] = {};
    parameterNode->GetTableTopRobotAngles(a);
    double patientToTableTopTranslation[3] = {};
    parameterNode->GetPatientToTableTopTranslation(patientToTableTopTranslation);

    // Apply transform (rotation around Z axis on A6 angle in RAS origin)
    FlangeToWristTransform->RotateZ(a[5]);
    FlangeToWristTransform->Concatenate(ReverseWristVerticalOrientationTransform);
    FlangeToWristTransform->Concatenate(flangeToPatientTransform);

    vtkNew<vtkTransform> PatientToFlangeTranslateTransform;
    // Reverse compensate Patient->TableTop translation
    PatientToFlangeTranslateTransform->Translate(-1. * patientToTableTopTranslation[0], -1. * patientToTableTopTranslation[1], -1. * patientToTableTopTranslation[2]);
    // Translate to Elbow position under Flange position
    PatientToFlangeTranslateTransform->Translate(-1. * InitialElbowWristJointOffsetRAS[0], -1. * InitialElbowWristJointOffsetRAS[1], InitialElbowWristJointOffsetRAS[2]);
    PatientToFlangeTranslateTransform->Concatenate(FlangeToWristTransform);
    flangeToWristTransformNode->SetAndObserveTransformToParent(PatientToFlangeTranslateTransform);
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
  if (!parameterNode/* || !parameterNode->GetTreatmentMachineType() */)
  {
    vtkErrorMacro("UpdateWristToElbowTransform: Invalid parameter node");
    return;
  }

  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> wristToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::Wrist, CoordSys::Patient, 
    wristToPatientTransform, false))
  {
    vtkWarningMacro("UpdateWristToElbowTransform: Can't get Wrist->Patient transform");
  }

  vtkMRMLLinearTransformNode* wristToElbowTransformNode =
    this->GetTransformNodeBetween(CoordSys::Wrist, CoordSys::Elbow);
  if (wristToElbowTransformNode)
  {
    double a[6] = {};
    parameterNode->GetTableTopRobotAngles(a);
    double patientToTableTopTranslation[3] = {};
    parameterNode->GetPatientToTableTopTranslation(patientToTableTopTranslation);

    // Translate the Elbow so it's end in RAS origin
    vtkNew<vtkTransform> ElbowTranslateTransform;
    ElbowTranslateTransform->Translate(ElbowLength, 0., 0.);

    // Wrist->Flange (TableTop) rotation
    vtkNew<vtkTransform> WristToFlangeTransform;
    WristToFlangeTransform->RotateZ(a[5]);

    // Apply transform (rotation around Y axis on A5 angle, and around X axis on A4 angle in RAS origin)
    vtkNew<vtkTransform> WristToElbowTransform;
    WristToElbowTransform->RotateY(a[4]);
    WristToElbowTransform->RotateX(a[3]);
    // Transform elbow in RAS (Patient) origin so, it's begin in RAS origin
    ElbowTranslateTransform->Concatenate(wristToPatientTransform);
    // Translate elbow in RAS (Patient) origin, so it's end in RAS origin
    WristToElbowTransform->Concatenate(ElbowTranslateTransform);
    // Apply Wrist->Flange (TableTop) rotation transform
    WristToFlangeTransform->Concatenate(WristToElbowTransform);


    vtkNew<vtkTransform> PatientToFlangeTranslateTransform;
    // Reverse compensate Patient->TableTop translation
    PatientToFlangeTranslateTransform->Translate(-1. * patientToTableTopTranslation[0], -1. * patientToTableTopTranslation[1], -1. * patientToTableTopTranslation[2]);
    // Translate to Elbow position under Flange position
    PatientToFlangeTranslateTransform->Translate(-1. * InitialElbowWristJointOffsetRAS[0], -1. * InitialElbowWristJointOffsetRAS[1], InitialElbowWristJointOffsetRAS[2]);
    PatientToFlangeTranslateTransform->Concatenate(WristToFlangeTransform);
    wristToElbowTransformNode->SetAndObserveTransformToParent(PatientToFlangeTranslateTransform);
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
  if (!parameterNode/* || !parameterNode->GetTreatmentMachineType() */)
  {
    vtkErrorMacro("UpdateElbowToShoulderTransform: Invalid parameter node");
    return;
  }

  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> elbowToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::Elbow, CoordSys::Patient, 
    elbowToPatientTransform, false))
  {
    vtkWarningMacro("UpdateElbowToShoulderTransform: Can't get Elbow->Patient transform");
  }

  // Transform model to vertical position
  vtkNew<vtkTransform> ShoulderVerticalOrientationTransform; // vertical orientation
  ShoulderVerticalOrientationTransform->Identity();
  ShoulderVerticalOrientationTransform->RotateX(-90.);

  // Translate the Shoulder so it's end in RAS origin
  vtkNew<vtkTransform> ShoulderTranslateTransform;
  ShoulderTranslateTransform->Translate(0., 0., -1. * ShoulderLength);

  vtkMRMLLinearTransformNode* elbowToShoulderTransformNode =
    this->GetTransformNodeBetween(CoordSys::Elbow, CoordSys::Shoulder);
  if (elbowToShoulderTransformNode)
  {
    double a[6] = {};
    parameterNode->GetTableTopRobotAngles(a);
    double patientToTableTopTranslation[3] = {};
    parameterNode->GetPatientToTableTopTranslation(patientToTableTopTranslation);

    /// Get current position of Elbow origin (begin)
    // Translate the Elbow so it's end in RAS origin
    vtkNew<vtkTransform> ElbowTranslateTransform;
    ElbowTranslateTransform->Translate(ElbowLength, 0., 0.);

    // Wrist->Flange (TableTop) rotation
    vtkNew<vtkTransform> WristToFlangeTransform;
    WristToFlangeTransform->RotateZ(a[5]);

    // Apply transform (rotation around Y axis on A5 angle, and around X axis on A4 angle in RAS origin)
    vtkNew<vtkTransform> WristToElbowTransform;
    WristToElbowTransform->RotateY(a[4]);
    WristToElbowTransform->RotateX(a[3]);
    // Translate elbow in RAS (Patient) origin, so it's end in RAS origin
    WristToElbowTransform->Concatenate(ElbowTranslateTransform);
    // Apply Wrist->Flange (TableTop) rotation transform
    WristToFlangeTransform->Concatenate(WristToElbowTransform);
    double NewElbowBeginPositionTranslate[3] = {};
    WristToFlangeTransform->GetPosition(NewElbowBeginPositionTranslate);

    // Elbow->Shoulder rotation around Y (A3 angle)
    vtkNew<vtkTransform> ElbowToShoulderRotationTransform;
    ElbowToShoulderRotationTransform->RotateY(a[2]);

    // Apply transform (rotation around Y axis on A5 angle, around X axis on A4 angle and around Z axis on A6 angle in RAS origin)
    vtkNew<vtkTransform> A6A5A4RotationTransform;
    A6A5A4RotationTransform->RotateZ(a[5]);
    A6A5A4RotationTransform->RotateY(a[4]);
    A6A5A4RotationTransform->RotateX(a[3]);

    // Transform shoulder in RAS (Patient) origin so, it's begin in RAS origin
    // Transform to RAS origin and model vertical orientation
    ShoulderVerticalOrientationTransform->Concatenate(elbowToPatientTransform);
    ShoulderTranslateTransform->Concatenate(ShoulderVerticalOrientationTransform);
    // Translate  Shoulder end to RAS (Patient) origin so, it's begin in RAS origin
    ElbowToShoulderRotationTransform->Concatenate(ShoulderTranslateTransform);
    // Apply transform of the Elbow (A6, A5, A4 angles) to Shoulder model in RAS origin
    A6A5A4RotationTransform->Concatenate(ElbowToShoulderRotationTransform);

    vtkNew<vtkTransform> PatientToFlangeTranslateTransform;
    // Reverse compensate Patient->TableTop translation
    PatientToFlangeTranslateTransform->Translate(-1. * patientToTableTopTranslation[0], -1. * patientToTableTopTranslation[1], -1. * patientToTableTopTranslation[2]);
    // Translate to Elbow position under Flange position
    PatientToFlangeTranslateTransform->Translate(-1. * InitialElbowWristJointOffsetRAS[0], -1. * InitialElbowWristJointOffsetRAS[1], InitialElbowWristJointOffsetRAS[2]);
    // Translate to new elbow begin position
    PatientToFlangeTranslateTransform->Translate(NewElbowBeginPositionTranslate);
    // Apply Translation of a Shoulder model to the new Elbow begin position
    PatientToFlangeTranslateTransform->Concatenate(A6A5A4RotationTransform);

    elbowToShoulderTransformNode->SetAndObserveTransformToParent(PatientToFlangeTranslateTransform);
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
  if (!parameterNode/* || !parameterNode->GetTreatmentMachineType() */)
  {
    vtkErrorMacro("UpdateShoulderToBaseRotationTransform: Invalid parameter node");
    return;
  }

  // Translate the BaseRotation so it's empty disk centre in RAS origin
  vtkNew<vtkTransform> BaseRotationTranslateTransform;
  BaseRotationTranslateTransform->Translate(BaseRotationShoulderDiskCenterOffsetX, 0., -1. * BaseRotationShoulderDiskCenterOffsetY);

  // Transform model to vertical position
  vtkNew<vtkTransform> BaseRotationVerticalOrientationTransform; // vertical orientation
  BaseRotationVerticalOrientationTransform->Identity();
  BaseRotationVerticalOrientationTransform->RotateX(-90.);

  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> shoulderToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::Shoulder, CoordSys::Patient, 
    shoulderToPatientTransform, false))
  {
    vtkWarningMacro("UpdateShoulderToBaseRotationTransform: Can't get Shoulder->Patient transform");
  }

  double PatientToBaseRotationTranslate[3] = {};
  vtkNew<vtkTransform> patientToShoulderTransform;
  if (this->GetTransformBetween( CoordSys::Patient, CoordSys::Shoulder, 
    patientToShoulderTransform, false))
  {
    patientToShoulderTransform->GetPosition(PatientToBaseRotationTranslate);
  }

  vtkMRMLLinearTransformNode* shoulderToBaseRotationTransformNode =
    this->GetTransformNodeBetween(CoordSys::Shoulder, CoordSys::BaseRotation);
  if (shoulderToBaseRotationTransformNode)
  {
    double a[6] = {};
    parameterNode->GetTableTopRobotAngles(a);
    double patientToTableTopTranslation[3] = {};
    parameterNode->GetPatientToTableTopTranslation(patientToTableTopTranslation);

    // Shoulder->BaseRotation rotation around Y (A2 angle)
    vtkNew<vtkTransform> shoulderToBaseRotationTransform;
    shoulderToBaseRotationTransform->RotateY(a[1]);

    // Elbow->Shoulder rotation around Y (A3 angle)
    vtkNew<vtkTransform> ElbowToShoulderRotationTransform;
    ElbowToShoulderRotationTransform->RotateY(a[2]);

    // Apply transform (rotation around Y axis on A5 angle, around X axis on A4 angle and around Z axis on A6 angle in RAS origin)
    vtkNew<vtkTransform> A6A5A4RotationTransform;
    A6A5A4RotationTransform->RotateZ(a[5]);
    A6A5A4RotationTransform->RotateY(a[4]);
    A6A5A4RotationTransform->RotateX(a[3]);

    // Transform shoulder in RAS (Patient) origin so, it's begin in RAS origin
    // Transform to RAS origin and model vertical orientation
    BaseRotationVerticalOrientationTransform->Concatenate(shoulderToPatientTransform);
    BaseRotationTranslateTransform->Concatenate(BaseRotationVerticalOrientationTransform);
    // Translate  BaseRotation empty disk center to RAS (Patient) origin so, it's begin in RAS origin
    shoulderToBaseRotationTransform->Concatenate(BaseRotationTranslateTransform);
    // Translate  Shoulder end to RAS (Patient) origin so, it's begin in RAS origin
    ElbowToShoulderRotationTransform->Concatenate(shoulderToBaseRotationTransform);
    // Apply transform of the Elbow (A6, A5, A4 angles) to Shoulder model in RAS origin
    A6A5A4RotationTransform->Concatenate(ElbowToShoulderRotationTransform);

    // Translate BaseRotation model to the center of Shoulder disk (Patient->BaseRotation translation)
    vtkNew<vtkTransform> PatientToBaseRotationTranslateTransform;
    PatientToBaseRotationTranslateTransform->Translate(PatientToBaseRotationTranslate);
    PatientToBaseRotationTranslateTransform->Concatenate(A6A5A4RotationTransform);

    shoulderToBaseRotationTransformNode->SetAndObserveTransformToParent(PatientToBaseRotationTranslateTransform);
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
