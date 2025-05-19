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
#include "vtkSlicerCabin26ARobotsTransformLogic.h"

#include "vtkMRMLRTBeamNode.h"
#include "vtkMRMLRTPlanNode.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLLinearTransformNode.h>

// Cabin26A geometry MRML node
#include <vtkMRMLCabin26AGeometryNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkGeneralTransform.h>
#include <vtkTransform.h>

// STD includes
#include <array>

namespace {

//constexpr std::array< double, 3 > FixedReferenceToFixedBasedOffset{ -1600., -1500., 1400. };

constexpr double BaseFixedHeight = 240.; // mm
constexpr double CArmBaseFixedHeight = 240.; // mm
constexpr double BaseRotationHeight = 675. - BaseFixedHeight; // mm
constexpr double CArmBaseRotationHeight = 645. - CArmBaseFixedHeight; // mm
constexpr double BaseRotationShoulderDiskCenterOffsetX = 350; // mm
constexpr double BaseRotationShoulderDiskCenterOffsetY = BaseRotationHeight; // mm
constexpr double CArmBaseRotationShoulderDiskCenterOffsetX = 330; // mm
constexpr double CArmBaseRotationShoulderDiskCenterOffsetY = CArmBaseRotationHeight; // mm

constexpr double TableThickness = 90.; // mm
constexpr double FlangeLength = 300.; // mm
constexpr double WristLength = 215.; // mm
constexpr double ElbowLength = 1200.; // mm
constexpr double ShoulderLength = 1150.; // mm
constexpr double ShoulderElbowVerticalOffset = 41.; // mm

constexpr double CArmElbowLength = 1420.; // mm
constexpr double CArmWristLength = 240.; // mm

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
vtkStandardNewMacro(vtkSlicerCabin26ARobotsTransformLogic);

//-----------------------------------------------------------------------------
vtkSlicerCabin26ARobotsTransformLogic::vtkSlicerCabin26ARobotsTransformLogic()
{
  using CoordSys = CoordinateSystemIdentifier;
  // Setup coordinate system ID to name map
  this->CoordinateSystemsMap.clear();
  this->CoordinateSystemsMap[CoordSys::RAS] = "RAS";
  this->CoordinateSystemsMap[CoordSys::FixedReference] = "FixedReference";
  this->CoordinateSystemsMap[CoordSys::TableBaseFixed] = "TableBaseFixed";
  this->CoordinateSystemsMap[CoordSys::TableBaseRotation] = "TableBaseRotation";
  this->CoordinateSystemsMap[CoordSys::TableShoulder] = "TableShoulder";
  this->CoordinateSystemsMap[CoordSys::TableElbow] = "TableElbow";
  this->CoordinateSystemsMap[CoordSys::TableWrist] = "TableWrist";
  this->CoordinateSystemsMap[CoordSys::TableFlange] = "TableFlange";
  this->CoordinateSystemsMap[CoordSys::TableTop] = "TableTop";
  this->CoordinateSystemsMap[CoordSys::Patient] = "Patient";
  this->CoordinateSystemsMap[CoordSys::CArmBaseFixed] = "CarmBaseFixed";
  this->CoordinateSystemsMap[CoordSys::CArmBaseRotation] = "CarmBaseRotation";
  this->CoordinateSystemsMap[CoordSys::CArmShoulder] = "CarmRotation";
  this->CoordinateSystemsMap[CoordSys::CArmElbow] = "CarmElbow";
  this->CoordinateSystemsMap[CoordSys::CArmWrist] = "CarmWrist";
  this->CoordinateSystemsMap[CoordSys::XrayImager] = "XrayImager";
  this->CoordinateSystemsMap[CoordSys::XrayImageReceptor] = "XrayImageReceptor";
  this->CoordinateSystemsMap[CoordSys::ExternalXrayBeam] = "ExternalXrayBeam";

  this->RobotsTransforms.clear();
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::FixedReference, CoordSys::RAS)); // Dummy
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::TableBaseFixed, CoordSys::FixedReference)); // Table robot basement translation in cabin 26a (FixedReference) system
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::TableBaseRotation, CoordSys::TableBaseFixed)); // Rotation of patient support platform
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::TableShoulder, CoordSys::TableBaseRotation)); // Lateral movement along Y-axis in RAS of the table top
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::TableElbow, CoordSys::TableShoulder)); // Longitudinal movement along X-axis in RAS of the table top
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::TableWrist, CoordSys::TableElbow)); // Vertical movement of table top origin
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::TableFlange, CoordSys::TableWrist)); // Rotation of flange on table top center
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::TableTop, CoordSys::TableFlange)); // Dummy, only fixed translation
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::Patient, CoordSys::TableTop)); // Translate from oatient to table top center
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::RAS, CoordSys::Patient));
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::CArmBaseFixed, CoordSys::FixedReference)); // C-Arm robot basement translation in cabin 26a (FixedReference) system
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::CArmBaseRotation, CoordSys::CArmBaseFixed)); // C-Arm robot basement rotation around C-Arm robot basement along Z-axis
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::CArmShoulder, CoordSys::CArmBaseRotation)); // C-Arm robot shoulder to C-arm basement rotation around C-Arm robot basement along Y-axis
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::CArmElbow, CoordSys::CArmShoulder)); // C-Arm robot elbow to C-arm robot shoulder around C-Arm shoulder along Y-axis
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::CArmWrist, CoordSys::CArmElbow)); // C-Arm robot wrist to C-arm robot elbow around C-Arm elbow along Y-axis and Z-axis
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::CArm, CoordSys::CArmWrist)); // C-Arm to C-arm robot wrist
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::XrayImager, CoordSys::CArm)); // Xray Imager to C-arm
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::ExternalXrayBeam, CoordSys::XrayImager)); // External x-ray beam to C-arm
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::XrayImageReceptor, CoordSys::CArm)); // Xray image receptor to C-arm

  this->CoordinateSystemsHierarchy.clear();
  // key - parent, value - children
  this->CoordinateSystemsHierarchy[CoordSys::FixedReference] = { CoordSys::TableBaseFixed, CoordSys::CArmBaseFixed };
  this->CoordinateSystemsHierarchy[CoordSys::TableBaseFixed] = { CoordSys::TableBaseRotation };
  this->CoordinateSystemsHierarchy[CoordSys::TableBaseRotation] = { CoordSys::TableShoulder };
  this->CoordinateSystemsHierarchy[CoordSys::TableShoulder] = { CoordSys::TableElbow };
  this->CoordinateSystemsHierarchy[CoordSys::TableElbow] = { CoordSys::TableWrist };
  this->CoordinateSystemsHierarchy[CoordSys::TableWrist] = { CoordSys::TableFlange };
  this->CoordinateSystemsHierarchy[CoordSys::TableFlange] = { CoordSys::TableTop };
  this->CoordinateSystemsHierarchy[CoordSys::TableTop] = { CoordSys::Patient };
  this->CoordinateSystemsHierarchy[CoordSys::Patient] = { CoordSys::RAS };
  this->CoordinateSystemsHierarchy[CoordSys::CArmBaseFixed] = { CoordSys::CArmBaseRotation };
  this->CoordinateSystemsHierarchy[CoordSys::CArmBaseRotation] = { CoordSys::CArmShoulder };
  this->CoordinateSystemsHierarchy[CoordSys::CArmShoulder] = { CoordSys::CArmElbow };
  this->CoordinateSystemsHierarchy[CoordSys::CArmElbow] = { CoordSys::CArmWrist };
  this->CoordinateSystemsHierarchy[CoordSys::CArmWrist] = { CoordSys::CArm };
  this->CoordinateSystemsHierarchy[CoordSys::CArm] = { CoordSys::XrayImager, CoordSys::XrayImageReceptor };
  this->CoordinateSystemsHierarchy[CoordSys::XrayImager] = { CoordSys::ExternalXrayBeam };
}

//-----------------------------------------------------------------------------
vtkSlicerCabin26ARobotsTransformLogic::~vtkSlicerCabin26ARobotsTransformLogic()
{
  this->CoordinateSystemsMap.clear();
  this->RobotsTransforms.clear();
}

//----------------------------------------------------------------------------
void vtkSlicerCabin26ARobotsTransformLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  // Transforms
  os << indent << "Transforms:" << std::endl;
  vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
  for ( auto& transformPair : this->RobotsTransforms)
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
const char* vtkSlicerCabin26ARobotsTransformLogic::GetTreatmentMachinePartTypeAsString(CoordinateSystemIdentifier type)
{
  switch (type)
  {
    case FixedReference: return "FixedReference";
    case TableBaseFixed: return "TableRobotBaseFixed";
    case CArmBaseFixed: return "CArmRobotBaseFixed";
    case TableBaseRotation: return "TableRobotBaseRotation";
    case CArmBaseRotation: return "CArmRobotBaseRotation";
    case TableShoulder: return "TableRobotShoulder";
    case CArmShoulder: return "CArmRobotShoulder";
    case TableFlange: return "TableFlange";
    case TableElbow: return "TableRobotElbow";
    case CArmElbow: return "CArmRobotElbow";
    case TableWrist: return "TableRobotWrist";
    case CArmWrist: return "CArmRobotWrist";
    case TableTop: return "TableTop";
    case CArm: return "CArm";
    case XrayImager: return "XrayImager";
    case XrayImageReceptor: return "XrayImageReceptor";
    case Patient: return "Patient";
    default:
      // invalid type
      return nullptr;
  }
}

//---------------------------------------------------------------------------
void vtkSlicerCabin26ARobotsTransformLogic::BuildRobotsTransformHierarchy()
{
  if (!this->GetMRMLScene())
  {
    vtkErrorMacro("BuildTableRobotTransformHierarchy: Invalid MRML scene");
    return;
  }

  // Create transform nodes if they do not exist
  for (auto& transformPair : this->RobotsTransforms)
  {
    std::string transformNodeName = this->GetTransformNodeNameBetween( transformPair.first, transformPair.second);
    if (!this->GetMRMLScene()->GetFirstNodeByName(transformNodeName.c_str()))
    {
      vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      transformNode->SetName(transformNodeName.c_str());
//      transformNode->SetHideFromEditors(1);
      std::string singletonTag = std::string("C26A_") + transformNodeName;
//      transformNode->SetSingletonTag(singletonTag.c_str());
      this->GetMRMLScene()->AddNode(transformNode);
    }
  }

  using CoordSys = CoordinateSystemIdentifier;

  // Organize transforms into hierarchy based on table top robot RBS geometry

  // FixedReference parent, translation of fixed base part of the robot from fixed reference isocenter
  this->GetTransformNodeBetween(CoordSys::TableBaseFixed, CoordSys::FixedReference)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::FixedReference, CoordSys::RAS)->GetID() );
  this->GetTransformNodeBetween(CoordSys::CArmBaseFixed, CoordSys::FixedReference)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::FixedReference, CoordSys::RAS)->GetID() );

  // BaseFixed parent, rotation of base part of the robot along Z-axis
  this->GetTransformNodeBetween(CoordSys::TableBaseRotation, CoordSys::TableBaseFixed)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::TableBaseFixed, CoordSys::FixedReference)->GetID() );
  this->GetTransformNodeBetween(CoordSys::CArmBaseRotation, CoordSys::CArmBaseFixed)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::CArmBaseFixed, CoordSys::FixedReference)->GetID() );

  // BaseRotation parent, rotation of shoulder part of the robot along Y-axis
  this->GetTransformNodeBetween(CoordSys::TableShoulder, CoordSys::TableBaseRotation)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::TableBaseRotation, CoordSys::TableBaseFixed)->GetID() );
  this->GetTransformNodeBetween(CoordSys::CArmShoulder, CoordSys::CArmBaseRotation)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::CArmBaseRotation, CoordSys::CArmBaseFixed)->GetID() );

  // Shoulder parent, rotation of elbow part of the robot along Y-axis
  this->GetTransformNodeBetween(CoordSys::TableElbow, CoordSys::TableShoulder)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::TableShoulder, CoordSys::TableBaseRotation)->GetID() );
  this->GetTransformNodeBetween(CoordSys::CArmElbow, CoordSys::CArmShoulder)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::CArmShoulder, CoordSys::CArmBaseRotation)->GetID() );

  // Elbow parent, rotation of wrist part of the robot along Y-axis
  this->GetTransformNodeBetween(CoordSys::TableWrist, CoordSys::TableElbow)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::TableElbow, CoordSys::TableShoulder)->GetID() );
  this->GetTransformNodeBetween(CoordSys::CArmWrist, CoordSys::CArmElbow)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::CArmElbow, CoordSys::CArmShoulder)->GetID() );

  // Wrist parent, translation of flange center from wrist center
  this->GetTransformNodeBetween(CoordSys::TableFlange, CoordSys::TableWrist)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::TableWrist, CoordSys::TableElbow)->GetID() );

  // Flange parent, translation of table top center flange center
  this->GetTransformNodeBetween(CoordSys::TableTop, CoordSys::TableFlange)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::TableFlange, CoordSys::TableWrist)->GetID() );

  // TableTop parent, translation of patient from wrist flange center
  this->GetTransformNodeBetween( CoordSys::Patient, CoordSys::TableTop)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::TableTop, CoordSys::TableFlange)->GetID() );

  // Patient parent, transform to RAS
  this->GetTransformNodeBetween( CoordSys::RAS, CoordSys::Patient)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween( CoordSys::Patient, CoordSys::TableTop)->GetID() );
  // CArm, Imager, Receptor, Beam model
  this->GetTransformNodeBetween(CoordSys::CArm, CoordSys::CArmWrist)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::CArmWrist, CoordSys::CArmElbow)->GetID() );
  this->GetTransformNodeBetween(CoordSys::XrayImageReceptor, CoordSys::CArm)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::CArm, CoordSys::CArmWrist)->GetID() );
  this->GetTransformNodeBetween(CoordSys::XrayImager, CoordSys::CArm)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::CArm, CoordSys::CArmWrist)->GetID() );
  this->GetTransformNodeBetween(CoordSys::ExternalXrayBeam, CoordSys::XrayImager)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::XrayImager, CoordSys::CArm)->GetID() );
}

//-----------------------------------------------------------------------------
void vtkSlicerCabin26ARobotsTransformLogic::ResetToInitialPositions()
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
    this->GetTransformNodeBetween(CoordSys::TableTop, CoordSys::TableFlange);
  vtkTransform* tableTopToFlangeTransform = vtkTransform::SafeDownCast(tableTopToFlangeTransformNode->GetTransformToParent());
  tableTopToFlangeTransform->Identity();
  tableTopToFlangeTransform->Modified();

  vtkMRMLLinearTransformNode* flangeToWristTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableFlange, CoordSys::TableWrist);
  vtkTransform* flangeToWristTransform = vtkTransform::SafeDownCast(flangeToWristTransformNode->GetTransformToParent());
  flangeToWristTransform->Identity();
  flangeToWristTransform->Modified();

  vtkMRMLLinearTransformNode* wristToElbowTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableWrist, CoordSys::TableElbow);
  vtkTransform* wristToElbowTransform = vtkTransform::SafeDownCast(wristToElbowTransformNode->GetTransformToParent());
  wristToElbowTransform->Identity();
  wristToElbowTransform->Modified();

  vtkMRMLLinearTransformNode* elbowToShoulderTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableElbow, CoordSys::TableShoulder);
  vtkTransform* elbowToShoulderTransform = vtkTransform::SafeDownCast(elbowToShoulderTransformNode->GetTransformToParent());
  elbowToShoulderTransform->Identity();
  elbowToShoulderTransform->Modified();

  vtkMRMLLinearTransformNode* shoulderToBaseRotationTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableShoulder, CoordSys::TableBaseRotation);
  vtkTransform* shoulderToBaseRotationTransform = vtkTransform::SafeDownCast(shoulderToBaseRotationTransformNode->GetTransformToParent());
  shoulderToBaseRotationTransform->Identity();
  shoulderToBaseRotationTransform->Modified();

  vtkMRMLLinearTransformNode* baseRotationToBaseFixedTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableBaseRotation, CoordSys::TableBaseFixed);
  vtkTransform* baseRotationToBaseFixedTransform = vtkTransform::SafeDownCast(baseRotationToBaseFixedTransformNode->GetTransformToParent());
  baseRotationToBaseFixedTransform->Identity();
  baseRotationToBaseFixedTransform->Modified();

  vtkMRMLLinearTransformNode* tableBaseFixedToFixedRerefenceTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableBaseFixed, CoordSys::FixedReference);
  vtkTransform* tableBaseFixedToFixedRerefenceTransform = vtkTransform::SafeDownCast(tableBaseFixedToFixedRerefenceTransformNode->GetTransformToParent());
  tableBaseFixedToFixedRerefenceTransform->Identity();
  tableBaseFixedToFixedRerefenceTransform->Modified();

  vtkMRMLLinearTransformNode* cArmBaseFixedToFixedRerefenceTransformNode =
    this->GetTransformNodeBetween(CoordSys::CArmBaseFixed, CoordSys::FixedReference);
  vtkTransform* cArmBaseFixedToFixedRerefenceTransform = vtkTransform::SafeDownCast(cArmBaseFixedToFixedRerefenceTransformNode->GetTransformToParent());
  cArmBaseFixedToFixedRerefenceTransform->Identity();
  cArmBaseFixedToFixedRerefenceTransform->Modified();

  vtkMRMLLinearTransformNode* cArmBaseRotationToCArmBaseFixedTransformNode =
    this->GetTransformNodeBetween(CoordSys::CArmBaseRotation, CoordSys::CArmBaseFixed);
  vtkTransform* cArmBaseRotationToCArmBaseFixedTransform = vtkTransform::SafeDownCast(cArmBaseRotationToCArmBaseFixedTransformNode->GetTransformToParent());
  cArmBaseRotationToCArmBaseFixedTransform->Identity();
  cArmBaseRotationToCArmBaseFixedTransform->Modified();

  vtkMRMLLinearTransformNode* cArmShoulderToCArmBaseRotationTransformNode =
    this->GetTransformNodeBetween(CoordSys::CArmShoulder, CoordSys::CArmBaseRotation);
  vtkTransform* cArmShoulderToCArmBaseRotationTransform = vtkTransform::SafeDownCast(cArmShoulderToCArmBaseRotationTransformNode->GetTransformToParent());
  cArmShoulderToCArmBaseRotationTransform->Identity();
  cArmShoulderToCArmBaseRotationTransform->Modified();

  vtkMRMLLinearTransformNode* cArmElbowToCArmShoulderTransformNode =
    this->GetTransformNodeBetween(CoordSys::CArmElbow, CoordSys::CArmShoulder);
  vtkTransform* cArmElbowToCArmShoulderTransform = vtkTransform::SafeDownCast(cArmElbowToCArmShoulderTransformNode->GetTransformToParent());
  cArmElbowToCArmShoulderTransform->Identity();
  cArmElbowToCArmShoulderTransform->Modified();

  vtkMRMLLinearTransformNode* cArmWristToCArmElbowTransformNode =
    this->GetTransformNodeBetween(CoordSys::CArmWrist, CoordSys::CArmElbow);
  vtkTransform* cArmWristToCArmElbowTransform = vtkTransform::SafeDownCast(cArmWristToCArmElbowTransformNode->GetTransformToParent());
  cArmWristToCArmElbowTransform->Identity();
  cArmWristToCArmElbowTransform->Modified();

  vtkMRMLLinearTransformNode* cArmToCArmWristTransformNode =
    this->GetTransformNodeBetween(CoordSys::CArm, CoordSys::CArmWrist);
  vtkTransform* cArmToCArmWristTransform = vtkTransform::SafeDownCast(cArmToCArmWristTransformNode->GetTransformToParent());
  cArmToCArmWristTransform->Identity();
  cArmToCArmWristTransform->Modified();

  vtkMRMLLinearTransformNode* xrayImageReceptorToCArmTransformNode =
    this->GetTransformNodeBetween(CoordSys::XrayImageReceptor, CoordSys::CArm);
  vtkTransform* xrayImageReceptorToCArmTransform = vtkTransform::SafeDownCast(xrayImageReceptorToCArmTransformNode->GetTransformToParent());
  xrayImageReceptorToCArmTransform->Identity();
  xrayImageReceptorToCArmTransform->Modified();

  vtkMRMLLinearTransformNode* xrayImagerToCArmTransformNode =
    this->GetTransformNodeBetween(CoordSys::XrayImager, CoordSys::CArm);
  vtkTransform* xrayImagerToCArmTransform = vtkTransform::SafeDownCast(xrayImagerToCArmTransformNode->GetTransformToParent());
  xrayImagerToCArmTransform->Identity();
  xrayImagerToCArmTransform->Modified();

  vtkMRMLLinearTransformNode* externalXrayToXrayImagerTransformNode =
    this->GetTransformNodeBetween(CoordSys::ExternalXrayBeam, CoordSys::XrayImager);
  vtkTransform* externalXrayToXrayImagerTransform = vtkTransform::SafeDownCast(externalXrayToXrayImagerTransformNode->GetTransformToParent());
  externalXrayToXrayImagerTransform->Identity();
  externalXrayToXrayImagerTransform->Modified();
}

//-----------------------------------------------------------------------------
std::string vtkSlicerCabin26ARobotsTransformLogic::GetTransformNodeNameBetween(
  CoordinateSystemIdentifier fromFrame, CoordinateSystemIdentifier toFrame)
{
  return this->CoordinateSystemsMap[fromFrame] + "To" + this->CoordinateSystemsMap[toFrame] + "Transform";
}

//-----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerCabin26ARobotsTransformLogic::GetTransformNodeBetween(
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
bool vtkSlicerCabin26ARobotsTransformLogic::GetTransformForPointBetweenFrames( 
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
void vtkSlicerCabin26ARobotsTransformLogic::UpdateBaseFixedToFixedReferenceTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateBaseRotationToBaseFixedTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
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
  BaseFixedToFixedReferenceTranslate[2] *= -1.; // Make negative Zt (vertical position) value
  BaseFixedTranslateTransform->Translate(BaseFixedToFixedReferenceTranslate);

  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> baseFixedToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::TableBaseFixed, CoordSys::Patient, 
    baseFixedToPatientTransform, false))
  {
    vtkWarningMacro("UpdateBaseRotationToBaseFixedTransform: Can't get TableBaseFixed->Patient transform");
  }

  double PatientToBaseFixedTranslate[3] = {};
  vtkNew<vtkTransform> patientToBaseFixedTransform;
  if (this->GetTransformBetween( CoordSys::Patient, CoordSys::TableBaseFixed, 
    patientToBaseFixedTransform, false))
  {
    patientToBaseFixedTransform->GetPosition(PatientToBaseFixedTranslate);
  }

  vtkMRMLLinearTransformNode* baseFixedToFixedReferenceTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableBaseFixed, CoordSys::FixedReference);
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
void vtkSlicerCabin26ARobotsTransformLogic::UpdateCArmBaseFixedToFixedReferenceTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateCArmBaseFixedToFixedReferenceTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateCArmBaseFixedToFixedReferenceTransform: Invalid parameter node");
    return;
  }

  // Translate the BaseRotation so it's empty disk centre in RAS origin
  vtkNew<vtkTransform> BaseFixedTranslateTransform;
  BaseFixedTranslateTransform->Translate(0., 0., -1. * BaseFixedHeight);
  double BaseFixedToFixedReferenceTranslate[3] = {};
  double CArmBaseFixedToTableTopBaseFixedOffset[3] = {};
  parameterNode->GetBaseFixedToFixedReferenceTranslation(BaseFixedToFixedReferenceTranslate);
  parameterNode->GetCArmBaseFixedToTableTopBaseFixedOffset(CArmBaseFixedToTableTopBaseFixedOffset);
  // Default: FixedReferenceToFixedBasedOffset.data()
  BaseFixedToFixedReferenceTranslate[2] *= -1.; // Make negative Zt (vertical position) value
  CArmBaseFixedToTableTopBaseFixedOffset[2] *= -1.; // Make negative Zt (vertical position) value
  BaseFixedTranslateTransform->Translate(BaseFixedToFixedReferenceTranslate);
  BaseFixedTranslateTransform->Translate(CArmBaseFixedToTableTopBaseFixedOffset);

  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> baseFixedToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::TableBaseFixed, CoordSys::Patient, 
    baseFixedToPatientTransform, false))
  {
    vtkWarningMacro("UpdateBaseRotationToBaseFixedTransform: Can't get TableBaseFixed->Patient transform");
  }

  double PatientToBaseFixedTranslate[3] = {};
  vtkNew<vtkTransform> patientToBaseFixedTransform;
  if (this->GetTransformBetween( CoordSys::Patient, CoordSys::TableBaseFixed, 
    patientToBaseFixedTransform, false))
  {
    patientToBaseFixedTransform->GetPosition(PatientToBaseFixedTranslate);
  }

  vtkMRMLLinearTransformNode* baseFixedToFixedReferenceTransformNode =
    this->GetTransformNodeBetween(CoordSys::CArmBaseFixed, CoordSys::FixedReference);
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
/*
  // Translate the BaseRotation so it's empty disk centre in RAS origin
  vtkNew<vtkTransform> BaseFixedTranslateTransform;
///  BaseFixedTranslateTransform->Translate(0., 0., -1. * CArmBaseFixedHeight);
  double CArmBaseFixedToFixedReferenceTranslation[3] = {};
///  parameterNode->GetCArmBaseFixedToFixedReferenceTranslation(CArmBaseFixedToFixedReferenceTranslation);
  // Default: FixedReferenceToFixedBasedOffset.data()
///  CArmBaseFixedToFixedReferenceTranslation[2] *= -1.; // Make negative Zt (vertical position) value
///  BaseFixedTranslateTransform->Translate(CArmBaseFixedToFixedReferenceTranslation);


  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> tableBaseFixedToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::TableBaseFixed, CoordSys::Patient, 
    tableBaseFixedToPatientTransform, false))
  {
    vtkWarningMacro("UpdateCArmBaseFixedToFixedReferenceTransform: Can't get TableBaseFixed->Patient transform");
  }

  double PatientToBaseFixedTranslate[3] = {};
  vtkNew<vtkTransform> patientToBaseFixedTransform;
  if (this->GetTransformBetween( CoordSys::Patient, CoordSys::CArmBaseFixed, 
    patientToBaseFixedTransform, false))
  {
///    patientToBaseFixedTransform->GetPosition(PatientToBaseFixedTranslate);
  }

  vtkMRMLLinearTransformNode* baseFixedToFixedReferenceTransformNode =
    this->GetTransformNodeBetween(CoordSys::CArmBaseFixed, CoordSys::FixedReference);
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
    BaseFixedTranslateTransform->Concatenate(tableBaseFixedToPatientTransform);
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
*/
}

//-----------------------------------------------------------------------------
void vtkSlicerCabin26ARobotsTransformLogic::UpdateCArmBaseRotationToCArmBaseFixedTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateCArmBaseRotationToCArmBaseFixedTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateCArmBaseRotationToCArmBaseFixedTransform: Invalid parameter node");
    return;
  }

  vtkNew<vtkTransform> carmBaseFixedTranslate;
  carmBaseFixedTranslate->Translate(0., CArmBaseFixedHeight, 0.);

  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> patientToCarmBaseFixedTransform;
  if (!this->GetTransformBetween( CoordSys::Patient, CoordSys::CArmBaseFixed, 
    patientToCarmBaseFixedTransform, false))
  {
    vtkWarningMacro("UpdateCArmBaseRotationToCArmBaseFixedTransform: Can't get Patient->CArmBaseFixed transform");
  }
  vtkNew<vtkTransform> carmBaseFixedToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::CArmBaseFixed, CoordSys::Patient, 
    carmBaseFixedToPatientTransform, false))
  {
    vtkWarningMacro("UpdateCArmBaseRotationToCArmBaseFixedTransform: Can't get CArmBaseFixed->Patient transform1");
  }

  vtkMRMLLinearTransformNode* carmBaseRotationToCArmBaseFixedTransformNode =
    this->GetTransformNodeBetween(CoordSys::CArmBaseRotation, CoordSys::CArmBaseFixed);
  if (carmBaseRotationToCArmBaseFixedTransformNode)
  {
    double a[6] = {};
    parameterNode->GetCArmRobotAngles(a);

    // BaseRotation->BaseFixed rotation around Y (A1 angle)
    vtkNew<vtkTransform> baseRotationToBaseFixedTransform;
    baseRotationToBaseFixedTransform->RotateY(a[0]);

    carmBaseFixedToPatientTransform->Concatenate(carmBaseFixedTranslate);
    carmBaseFixedToPatientTransform->Concatenate(baseRotationToBaseFixedTransform);
    carmBaseFixedToPatientTransform->Concatenate(patientToCarmBaseFixedTransform);
    carmBaseRotationToCArmBaseFixedTransformNode->SetAndObserveTransformToParent(carmBaseFixedToPatientTransform);
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerCabin26ARobotsTransformLogic::UpdateCArmShoulderToCArmBaseRotationTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateCArmShoulderToCArmBaseRotationTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateCArmShoulderToCArmBaseRotationTransform: Invalid parameter node");
    return;
  }
  // Translate the C-Arm Shoulder to C-Arm BaseRotation empty disk centre
  vtkNew<vtkTransform> carmShoulderTranslateTransform;
  carmShoulderTranslateTransform->Translate( CArmBaseRotationShoulderDiskCenterOffsetX, CArmBaseRotationShoulderDiskCenterOffsetY, 0.);

  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> patientToCarmBaseRotationTransform;
  if (!this->GetTransformBetween( CoordSys::Patient, CoordSys::CArmBaseRotation, 
    patientToCarmBaseRotationTransform, false))
  {
    vtkWarningMacro("UpdateCArmShoulderToCArmBaseRotationTransform: Can't get Patient->CArmBaseRotation transform");
  }
  vtkNew<vtkTransform> carmBaseRotationToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::CArmBaseRotation, CoordSys::Patient, 
    carmBaseRotationToPatientTransform, false))
  {
    vtkWarningMacro("UpdateCArmShoulderToCArmBaseRotationTransform: Can't get CArmBaseRotation->Patient transform1");
  }

  vtkMRMLLinearTransformNode* carmShoulderToCArmBaseRotationTransformNode =
    this->GetTransformNodeBetween(CoordSys::CArmShoulder, CoordSys::CArmBaseRotation);
  if (carmShoulderToCArmBaseRotationTransformNode)
  {
    double a[6] = {};
    parameterNode->GetCArmRobotAngles(a);

    // Shoulder->BaseRotation rotation around Z (A2 angle)
    vtkNew<vtkTransform> shoulderToBaseRotationTransform;
    shoulderToBaseRotationTransform->RotateZ(a[1]);

    carmBaseRotationToPatientTransform->Concatenate(carmShoulderTranslateTransform);
    carmBaseRotationToPatientTransform->Concatenate(shoulderToBaseRotationTransform);
    carmBaseRotationToPatientTransform->Concatenate(patientToCarmBaseRotationTransform);
    carmShoulderToCArmBaseRotationTransformNode->SetAndObserveTransformToParent(carmBaseRotationToPatientTransform);
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerCabin26ARobotsTransformLogic::UpdateCArmElbowToCArmShoulderTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateCArmElbowToCArmShoulderTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateCArmElbowToCArmShoulderTransform: Invalid parameter node");
    return;
  }
  // Translate the C-Arm Elbow to C-Arm BaseRotation empty disk centre
  vtkNew<vtkTransform> carmElbowTranslateTransform;
  carmElbowTranslateTransform->Translate( 0., 1350., 0.);

  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> patientToCarmShoulderTransform;
  if (!this->GetTransformBetween( CoordSys::Patient, CoordSys::CArmShoulder, 
    patientToCarmShoulderTransform, false))
  {
    vtkWarningMacro("UpdateCArmElbowToCArmShoulderTransform: Can't get Patient->CArmShoulder transform");
  }
  vtkNew<vtkTransform> carmShoulderToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::CArmShoulder, CoordSys::Patient, 
    carmShoulderToPatientTransform, false))
  {
    vtkWarningMacro("UpdateCArmElbowToCArmShoulderTransform: Can't get CArmShoulder->Patient transform1");
  }

  vtkMRMLLinearTransformNode* carmElbowToCArmShoulderTransformNode =
    this->GetTransformNodeBetween(CoordSys::CArmElbow, CoordSys::CArmShoulder);
  if (carmElbowToCArmShoulderTransformNode)
  {
    double a[6] = {};
    parameterNode->GetCArmRobotAngles(a);

    // Wrist->Elbow rotation around X (A4 angle)
    vtkNew<vtkTransform> a4Transform;
    a4Transform->RotateX(a[3]);

    // Shoulder->BaseRotation rotation around Z (A3 angle)
    vtkNew<vtkTransform> elbowToShoulderTransform;
    elbowToShoulderTransform->Translate( 0., 115., 0.);
    elbowToShoulderTransform->RotateZ(a[2]);

    a4Transform->Concatenate(elbowToShoulderTransform);
    carmShoulderToPatientTransform->Concatenate(carmElbowTranslateTransform);
    carmShoulderToPatientTransform->Concatenate(a4Transform);
    carmShoulderToPatientTransform->Concatenate(patientToCarmShoulderTransform);
    carmElbowToCArmShoulderTransformNode->SetAndObserveTransformToParent(carmShoulderToPatientTransform);
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerCabin26ARobotsTransformLogic::UpdateCArmWristToCArmElbowTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateCArmWristToCArmElbowTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateCArmWristToCArmElbowTransform: Invalid parameter node");
    return;
  }
  // Translate the C-Arm Wrist to C-Arm Elbow origin
  vtkNew<vtkTransform> carmWristTranslateTransform;
  carmWristTranslateTransform->Translate( CArmElbowLength, 0., 0.);

  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> patientToCarmElbowTransform;
  if (!this->GetTransformBetween( CoordSys::Patient, CoordSys::CArmElbow, 
    patientToCarmElbowTransform, false))
  {
    vtkWarningMacro("UpdateCArmWristToCArmElbowTransform: Can't get Patient->CArmElbow transform");
  }
  vtkNew<vtkTransform> carmElbowToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::CArmElbow, CoordSys::Patient, 
    carmElbowToPatientTransform, false))
  {
    vtkWarningMacro("UpdateCArmWristToCArmElbowTransform: Can't get CArmElbow->Patient transform");
  }

  vtkMRMLLinearTransformNode* carmWristToCArmElbowTransformNode =
    this->GetTransformNodeBetween(CoordSys::CArmWrist, CoordSys::CArmElbow);
  if (carmWristToCArmElbowTransformNode)
  {
    double a[6] = {};
    parameterNode->GetCArmRobotAngles(a);

    // Shoulder->BaseRotation rotation around Z (A5 angle)
    vtkNew<vtkTransform> wristToElbowTransform;
//    wristToElbowTransform->RotateY(a[3]);
    wristToElbowTransform->RotateZ(a[4]);

    carmElbowToPatientTransform->Concatenate(carmWristTranslateTransform);
    carmElbowToPatientTransform->Concatenate(wristToElbowTransform);
    carmElbowToPatientTransform->Concatenate(patientToCarmElbowTransform);
    carmWristToCArmElbowTransformNode->SetAndObserveTransformToParent(carmElbowToPatientTransform);
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerCabin26ARobotsTransformLogic::UpdateCArmToCArmWristTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateCArmToCArmWristTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateCArmToCArmWristTransform: Invalid parameter node");
    return;
  }

  // Translate the C-Arm Wrist to C-Arm Elbow origin
  vtkNew<vtkTransform> carmWristTranslateTransform;
  carmWristTranslateTransform->Translate( CArmWristLength, 0., 0.);

  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> patientToCarmWristTransform;
  if (!this->GetTransformBetween( CoordSys::Patient, CoordSys::CArmWrist, 
    patientToCarmWristTransform, false))
  {
    vtkWarningMacro("UpdateCArmToCArmWristTransform: Can't get Patient->CArmWrist transform");
  }
  vtkNew<vtkTransform> carmWristToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::CArmWrist, CoordSys::Patient, 
    carmWristToPatientTransform, false))
  {
    vtkWarningMacro("UpdateCArmToCArmWristTransform: Can't get CArmWrist->Patient transform");
  }

  vtkMRMLLinearTransformNode* carmToCArmWristTransformNode =
    this->GetTransformNodeBetween(CoordSys::CArm, CoordSys::CArmWrist);
  if (carmToCArmWristTransformNode)
  {
    double a[6] = {};
    parameterNode->GetCArmRobotAngles(a);

    // Carm->CarmRobotWrist rotation around X (A6 angle)
    vtkNew<vtkTransform> carmToWristTransform;
    carmToWristTransform->RotateX(a[5]);
//    wristToElbowTransform->RotateZ(a[4]);

    carmWristToPatientTransform->Concatenate(carmWristTranslateTransform);
    carmWristToPatientTransform->Concatenate(carmToWristTransform);
    carmWristToPatientTransform->Concatenate(patientToCarmWristTransform);
    carmToCArmWristTransformNode->SetAndObserveTransformToParent(carmWristToPatientTransform);
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerCabin26ARobotsTransformLogic::UpdateXrayImagerToCArmTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateXrayImagerToCArmTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateXrayImagerToCArmTransform: Invalid parameter node");
    return;
  }

  // Translate the C-Arm Wrist to C-Arm Elbow origin
  vtkNew<vtkTransform> xrayImagerCarmTranslateTransform;
  xrayImagerCarmTranslateTransform->Translate( 1000, 865., 0.);

  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> patientToCarmTransform;
  if (!this->GetTransformBetween( CoordSys::Patient, CoordSys::CArm, 
    patientToCarmTransform, false))
  {
    vtkWarningMacro("UpdateXrayImagerToCArmTransform: Can't get Patient->CArm transform");
  }
  vtkNew<vtkTransform> carmToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::CArm, CoordSys::Patient, 
    carmToPatientTransform, false))
  {
    vtkWarningMacro("UpdateXrayImagerToCArmTransform: Can't get CArm->Patient transform");
  }

  vtkMRMLLinearTransformNode* xrayImagerToCArmTransformNode =
    this->GetTransformNodeBetween(CoordSys::XrayImager, CoordSys::CArm);
  if (xrayImagerToCArmTransformNode)
  {

    carmToPatientTransform->Concatenate(xrayImagerCarmTranslateTransform);
    carmToPatientTransform->Concatenate(patientToCarmTransform);
    xrayImagerToCArmTransformNode->SetAndObserveTransformToParent(carmToPatientTransform);
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerCabin26ARobotsTransformLogic::UpdateExternalXrayBeamToXrayImagerTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateExternalXrayBeamToXrayImagerTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateExternalXrayBeamToXrayImagerTransform: Invalid parameter node");
    return;
  }

  // Translate the C-Arm Wrist to C-Arm Elbow origin
  vtkNew<vtkTransform> externalXrayBeamToXrayImagerTranslateTransform;
  externalXrayBeamToXrayImagerTranslateTransform->Translate( 0, -865. - 335., 0.);

  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> patientToXrayImagerTransform;
  if (!this->GetTransformBetween( CoordSys::Patient, CoordSys::XrayImager, 
    patientToXrayImagerTransform, false))
  {
    vtkWarningMacro("UpdateExternalXrayBeamToXrayImagerTransform: Can't get Patient->XrayImager transform");
  }
  vtkNew<vtkTransform> xrayImagerToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::XrayImager, CoordSys::Patient, 
    xrayImagerToPatientTransform, false))
  {
    vtkWarningMacro("UpdateExternalXrayBeamToXrayImagerTransform: Can't get XrayImager->Patient transform");
  }

  vtkMRMLLinearTransformNode* externalXrayBeamToXrayImagerTransformNode =
    this->GetTransformNodeBetween(CoordSys::ExternalXrayBeam, CoordSys::XrayImager);
  if (externalXrayBeamToXrayImagerTransformNode)
  {
    // Carm->CarmRobotWrist rotation around X (A6 angle)
    vtkNew<vtkTransform> carmToWristTransform;
    carmToWristTransform->RotateX(90);
//    wristToElbowTransform->RotateZ(a[4]);

    xrayImagerToPatientTransform->Concatenate(externalXrayBeamToXrayImagerTranslateTransform);
    xrayImagerToPatientTransform->Concatenate(carmToWristTransform);
    xrayImagerToPatientTransform->Concatenate(patientToXrayImagerTransform);
    externalXrayBeamToXrayImagerTransformNode->SetAndObserveTransformToParent(xrayImagerToPatientTransform);
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerCabin26ARobotsTransformLogic::UpdateXrayReceptorToCArmTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateXrayReceptorToCArmTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateXrayReceptorToCArmTransform: Invalid parameter node");
    return;
  }

  // Translate the C-Arm Wrist to C-Arm Elbow origin
  vtkNew<vtkTransform> xrayReceptorCarmTranslateTransform;
  xrayReceptorCarmTranslateTransform->Translate( 1000, -865., 0.);

  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> patientToCarmTransform;
  if (!this->GetTransformBetween( CoordSys::Patient, CoordSys::CArm, 
    patientToCarmTransform, false))
  {
    vtkWarningMacro("UpdateXrayReceptorToCArmTransform: Can't get Patient->CArm transform");
  }
  vtkNew<vtkTransform> carmToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::CArm, CoordSys::Patient, 
    carmToPatientTransform, false))
  {
    vtkWarningMacro("UpdateXrayReceptorToCArmTransform: Can't get CArm->Patient transform");
  }

  vtkMRMLLinearTransformNode* xrayReceptorToCArmTransformNode =
    this->GetTransformNodeBetween(CoordSys::XrayImageReceptor, CoordSys::CArm);
  if (xrayReceptorToCArmTransformNode)
  {

    carmToPatientTransform->Concatenate(xrayReceptorCarmTranslateTransform);
    carmToPatientTransform->Concatenate(patientToCarmTransform);
    xrayReceptorToCArmTransformNode->SetAndObserveTransformToParent(carmToPatientTransform);
  }
}

//-----------------------------------------------------------------------------
bool vtkSlicerCabin26ARobotsTransformLogic::GetTransformBetween(
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
vtkMRMLLinearTransformNode* vtkSlicerCabin26ARobotsTransformLogic::GetElbowTransform()
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
vtkMRMLLinearTransformNode* vtkSlicerCabin26ARobotsTransformLogic::GetFixedReferenceTransform()
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
vtkMRMLLinearTransformNode* vtkSlicerCabin26ARobotsTransformLogic::GetExternalXrayBeamTransform()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("GetExternalXrayBeamTransform: Invalid MRML scene");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToExternalXrayBeamTransform"))
  {
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }

  return transformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerCabin26ARobotsTransformLogic::GetPatientTransform()
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
vtkMRMLLinearTransformNode* vtkSlicerCabin26ARobotsTransformLogic::GetTableTopTransform()
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
vtkMRMLLinearTransformNode* vtkSlicerCabin26ARobotsTransformLogic::GetWristTransform()
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
vtkMRMLLinearTransformNode* vtkSlicerCabin26ARobotsTransformLogic::GetShoulderTransform()
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
vtkMRMLLinearTransformNode* vtkSlicerCabin26ARobotsTransformLogic::GetFlangeTransform()
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
vtkMRMLLinearTransformNode* vtkSlicerCabin26ARobotsTransformLogic::GetBaseRotationTransform()
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
vtkMRMLLinearTransformNode* vtkSlicerCabin26ARobotsTransformLogic::GetBaseFixedTransform()
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
vtkMRMLLinearTransformNode* vtkSlicerCabin26ARobotsTransformLogic::UpdateRasToTableTopTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
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
    std::string singletonTag = std::string("C26A_") + "RasToTableTopTransform";
//    rasToTableTopTransformNode->SetSingletonTag(singletonTag.c_str());
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
vtkMRMLLinearTransformNode* vtkSlicerCabin26ARobotsTransformLogic::UpdateRasToFixedReferenceTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
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
    std::string singletonTag = std::string("C26A_") + "RasToFixedReferenceTransform";
//    rasToFixedReferenceTransformNode->SetSingletonTag(singletonTag.c_str());
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
vtkMRMLLinearTransformNode* vtkSlicerCabin26ARobotsTransformLogic::UpdateRasToBaseFixedTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
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
    std::string singletonTag = std::string("C26A_") + "RasToBaseFixedTransform";
//    rasToBaseFixedTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(rasToBaseFixedTransformNode);
  }

  vtkNew<vtkTransform> rasToBaseFixedTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::TableBaseFixed, 
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
vtkMRMLLinearTransformNode* vtkSlicerCabin26ARobotsTransformLogic::UpdateRasToCArmBaseFixedTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateRasToCArmBaseFixedTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateRasToCArmBaseFixedTransform: Invalid MRML scene");
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
  // BaseFixed -> FixedReference -> CArmBaseFixed
  // Find RasToCArmBaseFixedTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToCArmBaseFixedTransformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToCArmBaseFixedTransform"))
  {
    rasToCArmBaseFixedTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }
  else
  {
    rasToCArmBaseFixedTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    rasToCArmBaseFixedTransformNode->SetName("RasToCArmBaseFixedTransform");
//    rasToBaseRotationTransformNode->SetHideFromEditors(1);
    std::string singletonTag = std::string("C26A_") + "RasToCArmBaseFixedTransform";
//    rasToCArmBaseFixedTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(rasToCArmBaseFixedTransformNode);
  }

  vtkNew<vtkTransform> rasToCArmBaseFixedTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::CArmBaseFixed, 
    rasToCArmBaseFixedTransform, false))
  {
    vtkWarningMacro("UpdateRasToBaseFixedTransform: RAS->CArmBaseFixed transform updated");
    // Transform to RAS, set transform to node, transform the model
    rasToCArmBaseFixedTransform->Concatenate(patientToRasTransform);
  }
  if (rasToCArmBaseFixedTransformNode)
  {
    rasToCArmBaseFixedTransformNode->SetAndObserveTransformToParent(rasToCArmBaseFixedTransform);
  }
  return rasToCArmBaseFixedTransformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerCabin26ARobotsTransformLogic::UpdateRasToCArmBaseRotationTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateRasToCArmBaseRotationTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateRasToCArmBaseRotationTransform: Invalid MRML scene");
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
  // BaseFixed -> FixedReference -> CArmBaseFixed -> CArmBaseRotation
  // Find RasToCArmBaseFixedTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToCArmBaseRotationTransformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToCArmBaseRotationTransform"))
  {
    rasToCArmBaseRotationTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }
  else
  {
    rasToCArmBaseRotationTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    rasToCArmBaseRotationTransformNode->SetName("RasToCArmBaseRotationTransform");
//    rasToCArmBaseRotationTransformNode->SetHideFromEditors(1);
    std::string singletonTag = std::string("C26A_") + "RasToCArmBaseRotationTransform";
//    rasToCArmBaseRotationTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(rasToCArmBaseRotationTransformNode);
  }

  vtkNew<vtkTransform> rasToCArmBaseRotationTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::CArmBaseRotation, 
    rasToCArmBaseRotationTransform, false))
  {
    // Transform to RAS, set transform to node, transform the model
    rasToCArmBaseRotationTransform->Concatenate(patientToRasTransform);
  }
  if (rasToCArmBaseRotationTransform)
  {
    rasToCArmBaseRotationTransformNode->SetAndObserveTransformToParent(rasToCArmBaseRotationTransform);
  }
  return rasToCArmBaseRotationTransformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerCabin26ARobotsTransformLogic::UpdateRasToCArmShoulderTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateRasToCArmShoulderTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateRasToCArmShoulderTransform: Invalid MRML scene");
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
  // BaseFixed -> FixedReference -> CArmBaseFixed -> CArmBaseRotation -> CArmShoulder
  // Find RasToCArmBaseFixedTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToCArmShoulderTransformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToCArmShoulderTransform"))
  {
    rasToCArmShoulderTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }
  else
  {
    rasToCArmShoulderTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    rasToCArmShoulderTransformNode->SetName("RasToCArmShoulderTransform");
//    rasToCArmShoulderTransformNode->SetHideFromEditors(1);
    std::string singletonTag = std::string("C26A_") + "RasToCArmShoulderTransform";
//    rasToCArmShoulderTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(rasToCArmShoulderTransformNode);
  }

  vtkNew<vtkTransform> rasToCArmShoulderTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::CArmShoulder, 
    rasToCArmShoulderTransform, false))
  {
    vtkWarningMacro("UpdateRasToCArmShoulderTransform: RAS->CArmShoulder transform updated");
    // Transform to RAS, set transform to node, transform the model
    rasToCArmShoulderTransform->Concatenate(patientToRasTransform);
  }
  if (rasToCArmShoulderTransform)
  {
    rasToCArmShoulderTransformNode->SetAndObserveTransformToParent(rasToCArmShoulderTransform);
  }
  return rasToCArmShoulderTransformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerCabin26ARobotsTransformLogic::UpdateRasToCArmElbowTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateRasToCArmElbowTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateRasToCArmElbowTransform: Invalid MRML scene");
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
  // BaseFixed -> FixedReference -> CArmBaseFixed -> CArmBaseRotation -> CArmShoulder -> CArmElbow
  // Find RasToCArmBaseFixedTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToCArmElbowTransformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToCArmElbowTransform"))
  {
    rasToCArmElbowTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }
  else
  {
    rasToCArmElbowTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    rasToCArmElbowTransformNode->SetName("RasToCArmElbowTransform");
//    rasToCArmElbowTransformNode->SetHideFromEditors(1);
    std::string singletonTag = std::string("C26A_") + "RasToCArmElbowTransform";
//    rasToCArmElbowTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(rasToCArmElbowTransformNode);
  }

  vtkNew<vtkTransform> rasToCArmElbowTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::CArmElbow, 
    rasToCArmElbowTransform, false))
  {
    vtkWarningMacro("UpdateRasToCArmElbowTransform: RAS->CArmShoulder transform updated");
    // Transform to RAS, set transform to node, transform the model
    rasToCArmElbowTransform->Concatenate(patientToRasTransform);
  }
  if (rasToCArmElbowTransform)
  {
    rasToCArmElbowTransformNode->SetAndObserveTransformToParent(rasToCArmElbowTransform);
  }
  return rasToCArmElbowTransformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerCabin26ARobotsTransformLogic::UpdateRasToCArmWristTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateRasToCArmWristTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateRasToCArmWristTransform: Invalid MRML scene");
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
  // BaseFixed -> FixedReference -> CArmBaseFixed -> CArmBaseRotation -> CArmShoulder -> CArmElbow -> CArmWrist
  // Find RasToCArmBaseFixedTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToCArmWristTransformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToCArmWristTransform"))
  {
    rasToCArmWristTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }
  else
  {
    rasToCArmWristTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    rasToCArmWristTransformNode->SetName("RasToCArmWristTransform");
//    rasToCArmWristTransformNode->SetHideFromEditors(1);
    std::string singletonTag = std::string("C26A_") + "RasToCArmWristTransform";
//    rasToCArmWristTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(rasToCArmWristTransformNode);
  }

  vtkNew<vtkTransform> rasToCArmWristTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::CArmWrist, 
    rasToCArmWristTransform, false))
  {
    vtkWarningMacro("UpdateRasToCArmWristTransform: RAS->CArmWrist transform updated");
    // Transform to RAS, set transform to node, transform the model
    rasToCArmWristTransform->Concatenate(patientToRasTransform);
  }
  if (rasToCArmWristTransform)
  {
    rasToCArmWristTransformNode->SetAndObserveTransformToParent(rasToCArmWristTransform);
  }
  return rasToCArmWristTransformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerCabin26ARobotsTransformLogic::UpdateRasToCArmTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateRasToCArmTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateRasToCArmTransform: Invalid MRML scene");
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
  // BaseFixed -> FixedReference -> CArmBaseFixed -> CArmBaseRotation -> CArmShoulder -> CArmElbow -> CArmWrist -> CArm
  // Find RasToCArmBaseFixedTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToCArmTransformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToCArmTransform"))
  {
    rasToCArmTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }
  else
  {
    rasToCArmTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    rasToCArmTransformNode->SetName("RasToCArmTransform");
//    rasToCArmTransformNode->SetHideFromEditors(1);
    std::string singletonTag = std::string("C26A_") + "RasToCArmTransform";
//    rasToCArmTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(rasToCArmTransformNode);
  }

  vtkNew<vtkTransform> rasToCArmTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::CArm, 
    rasToCArmTransform, false))
  {
    vtkWarningMacro("UpdateRasToCArmTransform: RAS->CArm transform updated");
    // Transform to RAS, set transform to node, transform the model
    rasToCArmTransform->Concatenate(patientToRasTransform);
  }
  if (rasToCArmTransform)
  {
    rasToCArmTransformNode->SetAndObserveTransformToParent(rasToCArmTransform);
  }
  return rasToCArmTransformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerCabin26ARobotsTransformLogic::UpdateRasToXrayImagerTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateRasToXrayImagerTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateRasToXrayImagerTransform: Invalid MRML scene");
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
  // BaseFixed -> FixedReference -> CArmBaseFixed -> CArmBaseRotation -> CArmShoulder -> CArmElbow -> CArmWrist -> CArm -> XrayImager
  // Find RasToCArmBaseFixedTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToXrayImagerTransformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToXrayImagerTransform"))
  {
    rasToXrayImagerTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }
  else
  {
    rasToXrayImagerTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    rasToXrayImagerTransformNode->SetName("RasToXrayImagerTransform");
//    rasToXrayImagerTransformNode->SetHideFromEditors(1);
    std::string singletonTag = std::string("C26A_") + "RasToXrayImagerTransform";
//    rasToCArmTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(rasToXrayImagerTransformNode);
  }

  vtkNew<vtkTransform> rasToXrayImagerTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::XrayImager, 
    rasToXrayImagerTransform, false))
  {
    vtkWarningMacro("UpdateRasToXrayImagerTransform: RAS->XrayImager transform updated");
    // Transform to RAS, set transform to node, transform the model
    rasToXrayImagerTransform->Concatenate(patientToRasTransform);
  }
  if (rasToXrayImagerTransform)
  {
    rasToXrayImagerTransformNode->SetAndObserveTransformToParent(rasToXrayImagerTransform);
  }
  return rasToXrayImagerTransformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerCabin26ARobotsTransformLogic::UpdateRasToExternalXrayBeamTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateRasToExternalXrayBeamTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateRasToExternalXrayBeamTransform: Invalid MRML scene");
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
  // BaseFixed -> FixedReference -> CArmBaseFixed -> CArmBaseRotation -> CArmShoulder -> CArmElbow -> CArmWrist
  // CArmWrist -> CArm -> XrayImager ->ExternalXrayBeam
  // Find RasToExternalXrayBeamTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToExternalXrayBeamTransformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToExternalXrayBeamTransform"))
  {
    rasToExternalXrayBeamTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }
  else
  {
    rasToExternalXrayBeamTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    rasToExternalXrayBeamTransformNode->SetName("RasToExternalXrayBeamTransform");
//    rasToExternalXrayBeamTransformNode->SetHideFromEditors(1);
    std::string singletonTag = std::string("C26A_") + "RasToExternalXrayBeamTransform";
//    rasToExternalXrayBeamTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(rasToExternalXrayBeamTransformNode);
  }

  vtkNew<vtkTransform> rasToExternalXrayBeamTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::ExternalXrayBeam, 
    rasToExternalXrayBeamTransform, false))
  {
    vtkWarningMacro("UpdateRasToXrayImagerTransform: RAS->ExternalXrayBeam transform updated");
    // Transform to RAS, set transform to node, transform the model
    rasToExternalXrayBeamTransform->Concatenate(patientToRasTransform);
  }
  if (rasToExternalXrayBeamTransform)
  {
    rasToExternalXrayBeamTransformNode->SetAndObserveTransformToParent(rasToExternalXrayBeamTransform);
  }
  return rasToExternalXrayBeamTransformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerCabin26ARobotsTransformLogic::UpdateRasToXrayReceptorTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateRasToXrayReceptorTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateRasToXrayReceptorTransform: Invalid MRML scene");
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
  // BaseFixed -> FixedReference -> CArmBaseFixed -> CArmBaseRotation -> CArmShoulder -> CArmElbow -> CArmWrist -> CArm -> XrayImageReceptor
  // Find RasToCArmBaseFixedTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToXrayReceptorTransformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToXrayReceptorTransform"))
  {
    rasToXrayReceptorTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }
  else
  {
    rasToXrayReceptorTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    rasToXrayReceptorTransformNode->SetName("RasToXrayReceptorTransform");
//    rasToXrayReceptorTransformNode->SetHideFromEditors(1);
    std::string singletonTag = std::string("C26A_") + "RasToXrayReceptorTransform";
//    rasToXrayReceptorTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(rasToXrayReceptorTransformNode);
  }

  vtkNew<vtkTransform> rasToXrayReceptorTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::XrayImageReceptor, 
    rasToXrayReceptorTransform, false))
  {
    vtkWarningMacro("UpdateRasToXrayReceptorTransform: RAS->XrayImageReceptor transform updated");
    // Transform to RAS, set transform to node, transform the model
    rasToXrayReceptorTransform->Concatenate(patientToRasTransform);
  }
  if (rasToXrayReceptorTransform)
  {
    rasToXrayReceptorTransformNode->SetAndObserveTransformToParent(rasToXrayReceptorTransform);
  }
  return rasToXrayReceptorTransformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerCabin26ARobotsTransformLogic::UpdateRasToFlangeTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
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
    std::string singletonTag = std::string("C26A_") + "RasToFlangeTransform";
//    rasToFlangeTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(rasToFlangeTransformNode);
  }

  vtkNew<vtkTransform> rasToFlangeTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::TableFlange, 
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
vtkMRMLLinearTransformNode* vtkSlicerCabin26ARobotsTransformLogic::UpdateRasToBaseRotationTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
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
    std::string singletonTag = std::string("C26A_") + "RasToBaseRotationTransform";
//    rasToBaseRotationTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(rasToBaseRotationTransformNode);
  }

  vtkNew<vtkTransform> rasToBaseRotationTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::TableBaseRotation, 
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
vtkMRMLLinearTransformNode* vtkSlicerCabin26ARobotsTransformLogic::UpdateRasToShoulderTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
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
    std::string singletonTag = std::string("C26A_") + "RasToShoulderTransform";
//    rasToShoulderTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(rasToShoulderTransformNode);
  }

  vtkNew<vtkTransform> rasToShoulderTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::TableShoulder, 
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
vtkMRMLLinearTransformNode* vtkSlicerCabin26ARobotsTransformLogic::UpdateRasToWristTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
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
    std::string singletonTag = std::string("C26A_") + "RasToWristTransform";
//    rasToWristTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(rasToWristTransformNode);
  }

  vtkNew<vtkTransform> rasToWristTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::TableWrist, 
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
vtkMRMLLinearTransformNode* vtkSlicerCabin26ARobotsTransformLogic::UpdateRasToElbowTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
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
    std::string singletonTag = std::string("C26A_") + "RasToElbowTransform";
//    rasToElbowTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(rasToElbowTransformNode);
  }

  vtkNew<vtkTransform> rasToElbowTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::TableElbow, 
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
void vtkSlicerCabin26ARobotsTransformLogic::UpdatePatientToTableTopTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
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
    patientToTableTopTransform->Translate(patientToTableTopTranslation);
    patientToTableTopTransformNode->SetAndObserveTransformToParent(patientToTableTopTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerCabin26ARobotsTransformLogic::UpdateBaseRotationToBaseFixedTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateBaseRotationToBaseFixedTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateBaseRotationToBaseFixedTransform:  Invalid parameter node");
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
  if (!this->GetTransformBetween( CoordSys::TableBaseRotation, CoordSys::Patient, 
    baseRotationToPatientTransform, false))
  {
    vtkWarningMacro("UpdateShoulderToBaseRotationTransform: Can't get BaseRotation->Patient transform");
  }

  double PatientToBaseRotationTranslate[3] = {};
  vtkNew<vtkTransform> patientToBaseRotationTransform;
  if (this->GetTransformBetween( CoordSys::Patient, CoordSys::TableBaseRotation, 
    patientToBaseRotationTransform, false))
  {
    patientToBaseRotationTransform->GetPosition(PatientToBaseRotationTranslate);
  }

  vtkMRMLLinearTransformNode* baseRotationToBaseFixedTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableBaseRotation, CoordSys::TableBaseFixed);
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
void vtkSlicerCabin26ARobotsTransformLogic::UpdateTableTopToFlangeTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableTopToWristTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateTableTopToWristTransform: Invalid parameter node");
    return;
  }

  using CoordSys = CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* tableTopToFlangeTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableTop, CoordSys::TableFlange);

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
void vtkSlicerCabin26ARobotsTransformLogic::UpdateFlangeToWristTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateFlangeToWristTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateFlangeToWristTransform: Invalid parameter node");
    return;
  }

  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> flangeToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::TableFlange, CoordSys::Patient, 
    flangeToPatientTransform, false))
  {
    vtkWarningMacro("UpdateFlangeToWristTransform: Can't get Flange->Patient transform");
    return;
  }

  vtkNew<vtkTransform> ReverseWristVerticalOrientationTransform; // vertical orientation
  ReverseWristVerticalOrientationTransform->Identity();
  ReverseWristVerticalOrientationTransform->RotateY(90.);

  vtkMRMLLinearTransformNode* flangeToWristTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableFlange, CoordSys::TableWrist);
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
void vtkSlicerCabin26ARobotsTransformLogic::UpdateWristToElbowTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateWristToElbowTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateWristToElbowTransform: Invalid parameter node");
    return;
  }

  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> wristToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::TableWrist, CoordSys::Patient, 
    wristToPatientTransform, false))
  {
    vtkWarningMacro("UpdateWristToElbowTransform: Can't get Wrist->Patient transform");
  }

  vtkMRMLLinearTransformNode* wristToElbowTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableWrist, CoordSys::TableElbow);
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
void vtkSlicerCabin26ARobotsTransformLogic::UpdateElbowToShoulderTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateElbowToShoulderTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateElbowToShoulderTransform:  Invalid parameter node");
    return;
  }

  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> elbowToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::TableElbow, CoordSys::Patient, 
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
  ShoulderTranslateTransform->Translate(0., 0., -1. * ShoulderLength + ShoulderElbowVerticalOffset);

  vtkMRMLLinearTransformNode* elbowToShoulderTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableElbow, CoordSys::TableShoulder);
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
void vtkSlicerCabin26ARobotsTransformLogic::UpdateShoulderToBaseRotationTransform(vtkMRMLCabin26AGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateShoulderToBaseRotationTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
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
  if (!this->GetTransformBetween( CoordSys::TableShoulder, CoordSys::Patient, 
    shoulderToPatientTransform, false))
  {
    vtkWarningMacro("UpdateShoulderToBaseRotationTransform: Can't get Shoulder->Patient transform");
  }

  double PatientToBaseRotationTranslate[3] = {};
  vtkNew<vtkTransform> patientToShoulderTransform;
  if (this->GetTransformBetween( CoordSys::Patient, CoordSys::TableShoulder, 
    patientToShoulderTransform, false))
  {
    patientToShoulderTransform->GetPosition(PatientToBaseRotationTranslate);
  }

  vtkMRMLLinearTransformNode* shoulderToBaseRotationTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableShoulder, CoordSys::TableBaseRotation);
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
bool vtkSlicerCabin26ARobotsTransformLogic::GetTransformBetween(
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
bool vtkSlicerCabin26ARobotsTransformLogic::GetPathToRoot( CoordinateSystemIdentifier frame, 
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
bool vtkSlicerCabin26ARobotsTransformLogic::GetPathFromRoot( CoordinateSystemIdentifier frame, 
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
