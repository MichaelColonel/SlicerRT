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

// Logic includes
#include "vtkSlicerChannel26Cabin3RobotsTransformLogic.h"
#include "vtkSlicerChannel26Cabin3RobotsGeometryCommon.h"

#include "vtkMRMLRTBeamNode.h"
#include "vtkMRMLRTPlanNode.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLLinearTransformNode.h>

// Cabin26A geometry MRML node
#include <vtkMRMLChannel26GeometryNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkGeneralTransform.h>
#include <vtkTransform.h>

// STD includes
#include <array>

namespace {

}

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerChannel26Cabin3RobotsTransformLogic);

//-----------------------------------------------------------------------------
vtkSlicerChannel26Cabin3RobotsTransformLogic::vtkSlicerChannel26Cabin3RobotsTransformLogic()
{
  using CoordSys = CoordinateSystemIdentifier;
  // Setup coordinate system ID to name map
  this->CoordinateSystemsMap.clear();
  this->CoordinateSystemsMap[CoordSys::RAS] = "RAS";
  this->CoordinateSystemsMap[CoordSys::FixedReference] = "FixedReference";
  this->CoordinateSystemsMap[CoordSys::TableRobotBaseFixed] = "TableRobotBaseFixed";
  this->CoordinateSystemsMap[CoordSys::CarmRobotBaseFixed] = "CarmRobotBaseFixed";
  this->CoordinateSystemsMap[CoordSys::TableRobotBaseRotation] = "TableRobotBaseRotation";
  this->CoordinateSystemsMap[CoordSys::CarmRobotBaseRotation] = "CarmRobotBaseRotation";
  this->CoordinateSystemsMap[CoordSys::TableRobotShoulder] = "TableRobotShoulder";
  this->CoordinateSystemsMap[CoordSys::CarmRobotShoulder] = "CarmRobotShoulder";
  this->CoordinateSystemsMap[CoordSys::TableRobotElbowShoulder] = "TableRobotElbowShoulder";
  this->CoordinateSystemsMap[CoordSys::CarmRobotElbowShoulder] = "CarmRobotElbowShoulder";
  this->CoordinateSystemsMap[CoordSys::TableRobotElbowWrist] = "TableRobotElbowWrist";
  this->CoordinateSystemsMap[CoordSys::CarmRobotElbowWrist] = "CarmRobotElbowWrist";
  this->CoordinateSystemsMap[CoordSys::TableRobotWrist] = "TableRobotWrist";
  this->CoordinateSystemsMap[CoordSys::CarmRobotWrist] = "CarmRobotWrist";
  this->CoordinateSystemsMap[CoordSys::TableRobotFlange] = "TableRobotFlange";
  this->CoordinateSystemsMap[CoordSys::CarmRobotFlange] = "CarmRobotFlange";
  this->CoordinateSystemsMap[CoordSys::Carm] = "Carm";
  this->CoordinateSystemsMap[CoordSys::CarmXrayBeam] = "CarmXrayBeam";
  this->CoordinateSystemsMap[CoordSys::CarmXrayDetector] = "CarmXrayDetector";
  this->CoordinateSystemsMap[CoordSys::TableFlange] = "TableFlange";
  this->CoordinateSystemsMap[CoordSys::TableTop] = "TableTop";
  this->CoordinateSystemsMap[CoordSys::TableTopPlaneCorrection] = "TableTopPlaneCorrection";
  this->CoordinateSystemsMap[CoordSys::TableRobotBaseRotationDisk] = "TableRobotBaseRotationDisk";
  this->CoordinateSystemsMap[CoordSys::Patient] = "Patient";

  this->RobotsTransforms.clear();
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::FixedReference, CoordSys::RAS)); // Dummy, unity, identity
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::TableRobotBaseFixed, CoordSys::FixedReference)); // Translate
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::CarmRobotBaseFixed, CoordSys::FixedReference)); // Translate
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::TableRobotBaseRotation, CoordSys::TableRobotBaseFixed)); // Rotation A1
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::TableRobotBaseRotationDisk, CoordSys::TableRobotBaseFixed)); // Rotation A1
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::CarmRobotBaseRotation, CoordSys::CarmRobotBaseFixed)); // Rotation A1
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::TableRobotShoulder, CoordSys::TableRobotBaseRotation)); // Rotation A2
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::CarmRobotShoulder, CoordSys::CarmRobotBaseRotation)); // Rotation A2
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::TableRobotElbowShoulder, CoordSys::TableRobotShoulder)); // Rotation A3
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::CarmRobotElbowShoulder, CoordSys::CarmRobotShoulder)); // Rotation A3
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::TableRobotElbowWrist, CoordSys::TableRobotElbowShoulder)); // Rotation A4
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::CarmRobotElbowWrist, CoordSys::CarmRobotElbowShoulder)); // Rotation A4
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::TableRobotWrist, CoordSys::TableRobotElbowWrist)); // Rotation A5
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::CarmRobotWrist, CoordSys::CarmRobotElbowWrist)); // Rotation A5
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::TableRobotFlange, CoordSys::TableRobotWrist)); // Rotation A6
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::CarmRobotFlange, CoordSys::CarmRobotWrist)); // Rotation A6
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::Carm, CoordSys::CarmRobotFlange)); // Translate
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::CarmXrayBeam, CoordSys::Carm)); // Translate
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::CarmXrayDetector, CoordSys::Carm)); // Translate
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::TableFlange, CoordSys::TableRobotFlange)); // Dummy, only fixed translation
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::TableTop, CoordSys::TableFlange)); // Dummy, only fixed translation
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::TableTopPlaneCorrection, CoordSys::TableTop)); // Translate from table top plane to table top center
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::Patient, CoordSys::TableTop)); // Translate from patient to table top center
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::RAS, CoordSys::Patient));

  this->CoordinateSystemsHierarchy.clear();
  // key - parent, value - children
  this->CoordinateSystemsHierarchy[CoordSys::FixedReference] = { CoordSys::TableRobotBaseFixed, CoordSys::CarmRobotBaseFixed };
  this->CoordinateSystemsHierarchy[CoordSys::TableRobotBaseFixed] = { CoordSys::TableRobotBaseRotation, CoordSys::TableRobotBaseRotationDisk };
  this->CoordinateSystemsHierarchy[CoordSys::CarmRobotBaseFixed] = { CoordSys::CarmRobotBaseRotation };
  this->CoordinateSystemsHierarchy[CoordSys::TableRobotBaseRotation] = { CoordSys::TableRobotShoulder };
  this->CoordinateSystemsHierarchy[CoordSys::CarmRobotBaseRotation] = { CoordSys::CarmRobotShoulder };
  this->CoordinateSystemsHierarchy[CoordSys::TableRobotShoulder] = { CoordSys::TableRobotElbowShoulder };
  this->CoordinateSystemsHierarchy[CoordSys::CarmRobotShoulder] = { CoordSys::CarmRobotElbowShoulder };
  this->CoordinateSystemsHierarchy[CoordSys::TableRobotElbowShoulder] = { CoordSys::TableRobotElbowWrist };
  this->CoordinateSystemsHierarchy[CoordSys::CarmRobotElbowShoulder] = { CoordSys::CarmRobotElbowWrist };
  this->CoordinateSystemsHierarchy[CoordSys::TableRobotElbowWrist] = { CoordSys::TableRobotWrist };
  this->CoordinateSystemsHierarchy[CoordSys::CarmRobotElbowWrist] = { CoordSys::CarmRobotWrist };
  this->CoordinateSystemsHierarchy[CoordSys::TableRobotWrist] = { CoordSys::TableRobotFlange };
  this->CoordinateSystemsHierarchy[CoordSys::CarmRobotWrist] = { CoordSys::CarmRobotFlange };
  this->CoordinateSystemsHierarchy[CoordSys::CarmRobotFlange] = { CoordSys::Carm };
  this->CoordinateSystemsHierarchy[CoordSys::Carm] = { CoordSys::CarmXrayBeam, CoordSys::CarmXrayDetector };
  this->CoordinateSystemsHierarchy[CoordSys::TableRobotFlange] = { CoordSys::TableFlange };
  this->CoordinateSystemsHierarchy[CoordSys::TableFlange] = { CoordSys::TableTop };
  this->CoordinateSystemsHierarchy[CoordSys::TableTop] = { CoordSys::Patient, CoordSys::TableTopPlaneCorrection };
  this->CoordinateSystemsHierarchy[CoordSys::Patient] = { CoordSys::RAS };
}

//-----------------------------------------------------------------------------
vtkSlicerChannel26Cabin3RobotsTransformLogic::~vtkSlicerChannel26Cabin3RobotsTransformLogic()
{
  this->CoordinateSystemsMap.clear();
  this->RobotsTransforms.clear();
}

//----------------------------------------------------------------------------
void vtkSlicerChannel26Cabin3RobotsTransformLogic::PrintSelf(ostream& os, vtkIndent indent)
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
const char* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetTreatmentMachinePartTypeAsString(CoordinateSystemIdentifier type)
{
  const char* partAsString = nullptr;
  switch (type)
  {
  case FixedReference:
    partAsString = "FixedReference";
    break;
  case TableRobotBaseFixed:
    partAsString = "TableRobotBaseFixed";
    break;
  case CarmRobotBaseFixed:
    partAsString = "CarmRobotBaseFixed";
    break;
  case TableRobotBaseRotation:
    partAsString = "TableRobotBaseRotation";
    break;
  case CarmRobotBaseRotation:
    partAsString = "CarmRobotBaseRotation";
    break;
  case TableRobotShoulder:
    partAsString = "TableRobotShoulder";
    break;
  case CarmRobotShoulder:
    partAsString = "CarmRobotShoulder";
    break;
  case TableRobotElbowShoulder:
    partAsString = "TableRobotElbowShoulder";
    break;
  case CarmRobotElbowShoulder:
    partAsString = "CarmRobotElbowShoulder";
    break;
  case TableRobotElbowWrist:
    partAsString = "TableRobotElbowWrist";
    break;
  case CarmRobotElbowWrist:
    partAsString = "CarmRobotElbowWrist";
    break;
  case TableRobotWrist:
    partAsString = "TableRobotWrist";
    break;
  case CarmRobotWrist:
    partAsString = "CarmRobotWrist";
    break;
  case TableRobotFlange:
    partAsString = "TableRobotFlange";
    break;
  case CarmRobotFlange:
    partAsString = "CarmRobotFlange";
    break;
  case Carm:
    partAsString = "Carm";
    break;
  case CarmXrayBeam:
    partAsString = "CarmXrayBeam";
    break;
  case CarmXrayDetector:
    partAsString = "CarmXrayDetector";
    break;
  case TableFlange:
    partAsString = "TableFlange";
    break;
  case TableTop:
    partAsString = "TableTop";
    break;
  case TableTopPlaneCorrection:
    partAsString = "TableTopPlaneCorrection";
    break;
  case TableRobotBaseRotationDisk:
    partAsString = "TableRobotBaseRotationDisk";
    break;
  default:
    // invalid type
    break;
  }
  return partAsString;
}

//---------------------------------------------------------------------------
void vtkSlicerChannel26Cabin3RobotsTransformLogic::BuildRobotsTransformHierarchy()
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
      std::string singletonTag = std::string("C26C3_") + transformNodeName;
//      transformNode->SetSingletonTag(singletonTag.c_str());
      this->GetMRMLScene()->AddNode(transformNode);
    }
  }

  using CoordSys = CoordinateSystemIdentifier;

  // Organize transforms into hierarchy based on Channel26, Cabin3 geometry

  // FixedReference parent, translation of fixed base part of the robot from fixed reference isocenter
  this->GetTransformNodeBetween(CoordSys::TableRobotBaseFixed, CoordSys::FixedReference)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::FixedReference, CoordSys::RAS)->GetID() );
  this->GetTransformNodeBetween(CoordSys::CarmRobotBaseFixed, CoordSys::FixedReference)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::FixedReference, CoordSys::RAS)->GetID() );

  // BaseFixed parent, rotation of base part of the robot along Z-axis
  this->GetTransformNodeBetween(CoordSys::TableRobotBaseRotation, CoordSys::TableRobotBaseFixed)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::TableRobotBaseFixed, CoordSys::FixedReference)->GetID() );
  this->GetTransformNodeBetween(CoordSys::TableRobotBaseRotationDisk, CoordSys::TableRobotBaseFixed)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::TableRobotBaseFixed, CoordSys::FixedReference)->GetID() );
  this->GetTransformNodeBetween(CoordSys::CarmRobotBaseRotation, CoordSys::CarmRobotBaseFixed)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::CarmRobotBaseFixed, CoordSys::FixedReference)->GetID() );

  // BaseRotation parent, rotation of shoulder part of the robot along Y-axis
  this->GetTransformNodeBetween(CoordSys::TableRobotShoulder, CoordSys::TableRobotBaseRotation)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::TableRobotBaseRotation, CoordSys::TableRobotBaseFixed)->GetID() );
  this->GetTransformNodeBetween(CoordSys::CarmRobotShoulder, CoordSys::CarmRobotBaseRotation)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::CarmRobotBaseRotation, CoordSys::CarmRobotBaseFixed)->GetID() );

  // Shoulder parent, rotation of elbow part of the robot along Y-axis
  this->GetTransformNodeBetween(CoordSys::TableRobotElbowShoulder, CoordSys::TableRobotShoulder)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::TableRobotShoulder, CoordSys::TableRobotBaseRotation)->GetID() );
  this->GetTransformNodeBetween(CoordSys::CarmRobotElbowShoulder, CoordSys::CarmRobotShoulder)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::CarmRobotShoulder, CoordSys::CarmRobotBaseRotation)->GetID() );

  // Elbow Shoulder parent, rotation of elbow part of the robot along Y-axis
  this->GetTransformNodeBetween(CoordSys::TableRobotElbowWrist, CoordSys::TableRobotElbowShoulder)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::TableRobotElbowShoulder, CoordSys::TableRobotShoulder)->GetID() );
  this->GetTransformNodeBetween(CoordSys::CarmRobotElbowWrist, CoordSys::CarmRobotElbowShoulder)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::CarmRobotElbowShoulder, CoordSys::CarmRobotShoulder)->GetID() );

  // Elbow parent, rotation of wrist part of the robot along Y-axis
  this->GetTransformNodeBetween(CoordSys::TableRobotWrist, CoordSys::TableRobotElbowWrist)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::TableRobotElbowWrist, CoordSys::TableRobotElbowShoulder)->GetID() );
  this->GetTransformNodeBetween(CoordSys::CarmRobotWrist, CoordSys::CarmRobotElbowWrist)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::CarmRobotElbowWrist, CoordSys::CarmRobotElbowShoulder)->GetID() );

  // Wrist parent, translation of flange center from wrist center
  this->GetTransformNodeBetween(CoordSys::TableRobotFlange, CoordSys::TableRobotWrist)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::TableRobotWrist, CoordSys::TableRobotElbowWrist)->GetID() );
  this->GetTransformNodeBetween(CoordSys::CarmRobotFlange, CoordSys::CarmRobotWrist)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::CarmRobotWrist, CoordSys::CarmRobotElbowWrist)->GetID() );

  // Wrist parent, translation of flange center from wrist center
  this->GetTransformNodeBetween(CoordSys::TableFlange, CoordSys::TableRobotFlange)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::TableRobotFlange, CoordSys::TableRobotWrist)->GetID() );

  // CarmRobotWristFlange parent, translation of C-arm center from wrist flange center
  this->GetTransformNodeBetween(CoordSys::Carm, CoordSys::CarmRobotFlange)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::CarmRobotFlange, CoordSys::CarmRobotWrist)->GetID() );

  // Carm parent, translation of C-arm X-ray beam center from C-arm origin
  this->GetTransformNodeBetween(CoordSys::CarmXrayBeam, CoordSys::Carm)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::Carm, CoordSys::CarmRobotFlange)->GetID() );

  // Carm parent, translation of C-arm X-ray detector center from C-arm origin
  this->GetTransformNodeBetween(CoordSys::CarmXrayDetector, CoordSys::Carm)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::Carm, CoordSys::CarmRobotFlange)->GetID() );

  // Flange parent, translation of table top center flange center
  this->GetTransformNodeBetween(CoordSys::TableTop, CoordSys::TableFlange)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::TableFlange, CoordSys::TableRobotFlange)->GetID() );

  // TableTop parent, translation of patient from wrist flange center
  this->GetTransformNodeBetween( CoordSys::Patient, CoordSys::TableTop)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::TableTop, CoordSys::TableFlange)->GetID() );
  // TableTop parent, external transform of the TableTop plane
  this->GetTransformNodeBetween( CoordSys::TableTopPlaneCorrection, CoordSys::TableTop)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::TableTop, CoordSys::TableFlange)->GetID() );

  // Patient parent, transform to RAS
  this->GetTransformNodeBetween( CoordSys::RAS, CoordSys::Patient)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween( CoordSys::Patient, CoordSys::TableTop)->GetID() );
}

//-----------------------------------------------------------------------------
void vtkSlicerChannel26Cabin3RobotsTransformLogic::ResetToInitialPositions()
{
  using CoordSys = CoordinateSystemIdentifier;
  // Update IEC Patient to RAS transform based on the isocenter defined in the beam's parent plan
  vtkMRMLLinearTransformNode* iecPatientToRasTransformNode =
    this->GetTransformNodeBetween( CoordSys::RAS, CoordSys::Patient);
  vtkTransform* iecPatientToRasTransform = vtkTransform::SafeDownCast(iecPatientToRasTransformNode->GetTransformToParent());
  iecPatientToRasTransform->Identity();
  iecPatientToRasTransform->RotateX(-90.);
  iecPatientToRasTransform->RotateY(180.);
  iecPatientToRasTransform->Modified();

  // Table top
  vtkMRMLLinearTransformNode* patientToTableTopTransformNode =
    this->GetTransformNodeBetween(CoordSys::Patient, CoordSys::TableTop);
  vtkTransform* patientToTableTopTransform =
    vtkTransform::SafeDownCast(patientToTableTopTransformNode->GetTransformToParent());
  patientToTableTopTransform->Identity();
  patientToTableTopTransform->Modified();

  // Table top plane correction
  vtkMRMLLinearTransformNode* planeCorrToTableTopTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableTopPlaneCorrection, CoordSys::TableTop);
  vtkTransform* planeCorrToTableTopTransform =
    vtkTransform::SafeDownCast(planeCorrToTableTopTransformNode->GetTransformToParent());
  planeCorrToTableTopTransform->Identity();
  planeCorrToTableTopTransform->Modified();

  vtkMRMLLinearTransformNode* tableTopToTableFlangeTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableTop, CoordSys::TableFlange);
  vtkTransform* tableTopToTableFlangeTransform =
    vtkTransform::SafeDownCast(tableTopToTableFlangeTransformNode->GetTransformToParent());
  tableTopToTableFlangeTransform->Identity();
  tableTopToTableFlangeTransform->Modified();

  // Table robot
  vtkMRMLLinearTransformNode* tableFlangeToTableRobotFlangeTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableFlange, CoordSys::TableRobotFlange);
  vtkTransform* tableFlangeToTableRobotFlangeTransform =
    vtkTransform::SafeDownCast(tableFlangeToTableRobotFlangeTransformNode->GetTransformToParent());
  tableFlangeToTableRobotFlangeTransform->Identity();
  tableFlangeToTableRobotFlangeTransform->Modified();

  vtkMRMLLinearTransformNode* tableRobotFlangeToTableRobotWristTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableRobotFlange, CoordSys::TableRobotWrist);
  vtkTransform* tableRobotFlangeToTableRobotWristTransform =
    vtkTransform::SafeDownCast(tableRobotFlangeToTableRobotWristTransformNode->GetTransformToParent());
  tableRobotFlangeToTableRobotWristTransform->Identity();
  tableRobotFlangeToTableRobotWristTransform->Modified();

  vtkMRMLLinearTransformNode* tableRobotWristToTableRobotElbowWristTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableRobotWrist, CoordSys::TableRobotElbowWrist);
  vtkTransform* tableRobotWristToTableRobotElbowWristTransform =
    vtkTransform::SafeDownCast(tableRobotWristToTableRobotElbowWristTransformNode->GetTransformToParent());
  tableRobotWristToTableRobotElbowWristTransform->Identity();
  tableRobotWristToTableRobotElbowWristTransform->Modified();

  vtkMRMLLinearTransformNode* tableRobotElbowWristToTableRobotElbowShoulderTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableRobotElbowWrist, CoordSys::TableRobotElbowShoulder);
  vtkTransform* tableRobotElbowWristToTableRobotElbowShoulderTransform =
    vtkTransform::SafeDownCast(tableRobotElbowWristToTableRobotElbowShoulderTransformNode->GetTransformToParent());
  tableRobotElbowWristToTableRobotElbowShoulderTransform->Identity();
  tableRobotElbowWristToTableRobotElbowShoulderTransform->Modified();

  vtkMRMLLinearTransformNode* tableRobotElbowShoulderToTableRobotShoulderTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableRobotElbowShoulder, CoordSys::TableRobotShoulder);
  vtkTransform* tableRobotElbowShoulderToTableRobotShoulderTransform =
    vtkTransform::SafeDownCast(tableRobotElbowShoulderToTableRobotShoulderTransformNode->GetTransformToParent());
  tableRobotElbowShoulderToTableRobotShoulderTransform->Identity();
  tableRobotElbowShoulderToTableRobotShoulderTransform->Modified();

  vtkMRMLLinearTransformNode* tableRobotShoulderToTableRobotBaseRotationTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableRobotShoulder, CoordSys::TableRobotBaseRotation);
  vtkTransform* tableRobotShoulderToTableRobotBaseRotationTransform =
    vtkTransform::SafeDownCast(tableRobotShoulderToTableRobotBaseRotationTransformNode->GetTransformToParent());
  tableRobotShoulderToTableRobotBaseRotationTransform->Identity();
  tableRobotShoulderToTableRobotBaseRotationTransform->Modified();

  vtkMRMLLinearTransformNode* tableRobotBaseRotationToTableRobotBaseFixedTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableRobotBaseRotation, CoordSys::TableRobotBaseFixed);
  vtkTransform* tableRobotBaseRotationToTableRobotBaseFixedTransform =
    vtkTransform::SafeDownCast(tableRobotBaseRotationToTableRobotBaseFixedTransformNode->GetTransformToParent());
  tableRobotBaseRotationToTableRobotBaseFixedTransform->Identity();
  tableRobotBaseRotationToTableRobotBaseFixedTransform->Modified();

  vtkMRMLLinearTransformNode* tableRobotBaseRotationDiskToTableRobotBaseFixedTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableRobotBaseRotationDisk, CoordSys::TableRobotBaseFixed);
  vtkTransform* tableRobotBaseRotationDiskToTableRobotBaseFixedTransform =
    vtkTransform::SafeDownCast(tableRobotBaseRotationDiskToTableRobotBaseFixedTransformNode->GetTransformToParent());
  tableRobotBaseRotationDiskToTableRobotBaseFixedTransform->Identity();
  tableRobotBaseRotationDiskToTableRobotBaseFixedTransform->Modified();

  vtkMRMLLinearTransformNode* tableRobotBaseFixedToFixedReferenceTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableRobotBaseFixed, CoordSys::FixedReference);
  vtkTransform* tableRobotBaseFixedToFixedReferenceTransform =
    vtkTransform::SafeDownCast(tableRobotBaseFixedToFixedReferenceTransformNode->GetTransformToParent());
  tableRobotBaseFixedToFixedReferenceTransform->Identity();
  tableRobotBaseFixedToFixedReferenceTransform->Modified();

  // C-arm robot
  vtkMRMLLinearTransformNode* carmRobotBaseFixedToFixedReferenceTransformNode =
    this->GetTransformNodeBetween(CoordSys::CarmRobotBaseFixed, CoordSys::FixedReference);
  vtkTransform* carmRobotBaseFixedToFixedReferenceTransform =
    vtkTransform::SafeDownCast(carmRobotBaseFixedToFixedReferenceTransformNode->GetTransformToParent());
  carmRobotBaseFixedToFixedReferenceTransform->Identity();
  carmRobotBaseFixedToFixedReferenceTransform->Modified();

  vtkMRMLLinearTransformNode* carmRobotBaseRotationToCarmRobotBaseFixedTransformNode =
    this->GetTransformNodeBetween(CoordSys::CarmRobotBaseRotation, CoordSys::CarmRobotBaseFixed);
  vtkTransform* carmRobotBaseRotationToCarmRobotBaseFixedTransform =
    vtkTransform::SafeDownCast(carmRobotBaseRotationToCarmRobotBaseFixedTransformNode->GetTransformToParent());
  carmRobotBaseRotationToCarmRobotBaseFixedTransform->Identity();
  carmRobotBaseRotationToCarmRobotBaseFixedTransform->Modified();

  vtkMRMLLinearTransformNode* carmRobotShoulderToCarmRobotBaseRotationTransformNode =
    this->GetTransformNodeBetween(CoordSys::CarmRobotShoulder, CoordSys::CarmRobotBaseRotation);
  vtkTransform* carmRobotShoulderToCarmRobotBaseRotationTransform =
    vtkTransform::SafeDownCast(carmRobotShoulderToCarmRobotBaseRotationTransformNode->GetTransformToParent());
  carmRobotShoulderToCarmRobotBaseRotationTransform->Identity();
  carmRobotShoulderToCarmRobotBaseRotationTransform->Modified();

  vtkMRMLLinearTransformNode* carmRobotElbowShoulderToCarmRobotShoulderTransformNode =
    this->GetTransformNodeBetween(CoordSys::CarmRobotElbowShoulder, CoordSys::CarmRobotShoulder);
  vtkTransform* carmRobotElbowShoulderToCarmRobotShoulderTransform =
    vtkTransform::SafeDownCast(carmRobotElbowShoulderToCarmRobotShoulderTransformNode->GetTransformToParent());
  carmRobotElbowShoulderToCarmRobotShoulderTransform->Identity();
  carmRobotElbowShoulderToCarmRobotShoulderTransform->Modified();

  vtkMRMLLinearTransformNode* carmRobotElbowWristToCarmRobotElbowShoulderTransformNode =
    this->GetTransformNodeBetween(CoordSys::CarmRobotElbowWrist, CoordSys::CarmRobotElbowShoulder);
  vtkTransform* carmRobotElbowWristToCarmRobotElbowShoulderTransform =
    vtkTransform::SafeDownCast(carmRobotElbowWristToCarmRobotElbowShoulderTransformNode->GetTransformToParent());
  carmRobotElbowWristToCarmRobotElbowShoulderTransform->Identity();
  carmRobotElbowWristToCarmRobotElbowShoulderTransform->Modified();

  vtkMRMLLinearTransformNode* carmRobotWristToCarmRobotElbowWristTransformNode =
    this->GetTransformNodeBetween(CoordSys::CarmRobotWrist, CoordSys::CarmRobotElbowWrist);
  vtkTransform* carmRobotWristToCarmRobotElbowWristTransform =
    vtkTransform::SafeDownCast(carmRobotWristToCarmRobotElbowWristTransformNode->GetTransformToParent());
  carmRobotWristToCarmRobotElbowWristTransform->Identity();
  carmRobotWristToCarmRobotElbowWristTransform->Modified();

  vtkMRMLLinearTransformNode* carmRobotFlangeToCarmRobotWristTransformNode =
    this->GetTransformNodeBetween(CoordSys::CarmRobotFlange, CoordSys::CarmRobotWrist);
  vtkTransform* carmRobotFlangeToCarmRobotWristTransform =
    vtkTransform::SafeDownCast(carmRobotFlangeToCarmRobotWristTransformNode->GetTransformToParent());
  carmRobotFlangeToCarmRobotWristTransform->Identity();
  carmRobotFlangeToCarmRobotWristTransform->Modified();

  vtkMRMLLinearTransformNode* carmToCarmRobotFlangeTransformNode =
    this->GetTransformNodeBetween(CoordSys::Carm, CoordSys::CarmRobotFlange);
  vtkTransform* carmToCarmRobotFlangeTransform =
    vtkTransform::SafeDownCast(carmToCarmRobotFlangeTransformNode->GetTransformToParent());
  carmToCarmRobotFlangeTransform->Identity();
  carmToCarmRobotFlangeTransform->Modified();

  vtkMRMLLinearTransformNode* carmXrayBeamToCarmTransformNode =
    this->GetTransformNodeBetween(CoordSys::CarmXrayBeam, CoordSys::Carm);
  vtkTransform* carmXrayBeamToCarmTransform =
    vtkTransform::SafeDownCast(carmXrayBeamToCarmTransformNode->GetTransformToParent());
  carmXrayBeamToCarmTransform->Identity();
  carmXrayBeamToCarmTransform->Modified();

  vtkMRMLLinearTransformNode* carmXrayDetectorToCarmTransformNode =
    this->GetTransformNodeBetween(CoordSys::CarmXrayDetector, CoordSys::Carm);
  vtkTransform* carmXrayDetectorToCarmTransform =
    vtkTransform::SafeDownCast(carmXrayDetectorToCarmTransformNode->GetTransformToParent());
  carmXrayDetectorToCarmTransform->Identity();
  carmXrayDetectorToCarmTransform->Modified();
}

//-----------------------------------------------------------------------------
std::string vtkSlicerChannel26Cabin3RobotsTransformLogic::GetTransformNodeNameBetween(
  CoordinateSystemIdentifier fromFrame, CoordinateSystemIdentifier toFrame)
{
  return this->CoordinateSystemsMap[fromFrame] + "To" + this->CoordinateSystemsMap[toFrame] + "Transform";
}

//-----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetTransformNodeBetween(
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
bool vtkSlicerChannel26Cabin3RobotsTransformLogic::GetTransformBetween(
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
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetTableTopTransform()
{
  return this->GetFrameToRasTransform(CoordinateSystemIdentifier::TableTop);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetTableTopPlaneCorrectionTransform()
{
  return this->GetFrameToRasTransform(CoordinateSystemIdentifier::TableTopPlaneCorrection);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetTableRobotWristTransform()
{
  return this->GetFrameToRasTransform(CoordinateSystemIdentifier::TableRobotWrist);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetTableRobotElbowWristTransform()
{
  return this->GetFrameToRasTransform(CoordinateSystemIdentifier::TableRobotElbowWrist);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetTableRobotElbowShoulderTransform()
{
  return this->GetFrameToRasTransform(CoordinateSystemIdentifier::TableRobotElbowShoulder);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetTableRobotShoulderTransform()
{
  return this->GetFrameToRasTransform(CoordinateSystemIdentifier::TableRobotShoulder);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetTableRobotBaseRotationTransform()
{
  return this->GetFrameToRasTransform(CoordinateSystemIdentifier::TableRobotBaseRotation);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetTableRobotBaseRotationDiskTransform()
{
  return this->GetFrameToRasTransform(CoordinateSystemIdentifier::TableRobotBaseRotationDisk);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetTableRobotBaseFixedTransform()
{
  return this->GetFrameToRasTransform(CoordinateSystemIdentifier::TableRobotBaseFixed);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetFixedReferenceTransform()
{
  return this->GetFrameToRasTransform(CoordinateSystemIdentifier::FixedReference);
/*
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("GetFixedReferenceTransform: Invalid MRML scene");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("FixedReferenceToRasTransform"))
  {
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }

  return transformNode;
*/
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetTableFlangeTransform()
{
  return this->GetFrameToRasTransform(CoordinateSystemIdentifier::TableFlange);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetTableRobotFlangeTransform()
{
  return this->GetFrameToRasTransform(CoordinateSystemIdentifier::TableRobotFlange);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetCarmRobotBaseFixedTransform()
{
  return this->GetFrameToRasTransform(CoordinateSystemIdentifier::CarmRobotBaseFixed);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetCarmRobotBaseRotationTransform()
{
  return this->GetFrameToRasTransform(CoordinateSystemIdentifier::CarmRobotBaseRotation);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetCarmRobotShoulderTransform()
{
  return this->GetFrameToRasTransform(CoordinateSystemIdentifier::CarmRobotShoulder);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetCarmRobotElbowShoulderTransform()
{
  return this->GetFrameToRasTransform(CoordinateSystemIdentifier::CarmRobotElbowShoulder);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetCarmRobotElbowWristTransform()
{
  return this->GetFrameToRasTransform(CoordinateSystemIdentifier::CarmRobotElbowWrist);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetCarmRobotWristTransform()
{
  return this->GetFrameToRasTransform(CoordinateSystemIdentifier::CarmRobotWrist);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetCarmRobotFlangeTransform()
{
  return this->GetFrameToRasTransform(CoordinateSystemIdentifier::CarmRobotFlange);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetCarmTransform()
{
  return this->GetFrameToRasTransform(CoordinateSystemIdentifier::Carm);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetCarmXrayBeamTransform()
{
  return this->GetFrameToRasTransform(CoordinateSystemIdentifier::CarmXrayBeam);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetCarmXrayDetectorTransform()
{
  return this->GetFrameToRasTransform(CoordinateSystemIdentifier::CarmXrayDetector);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateTableTopToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  return this->UpdateFrameToRasTransform(parameterNode, CoordinateSystemIdentifier::TableTop);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateTableTopPlaneCorrectionToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  return this->UpdateFrameToRasTransform(parameterNode, CoordinateSystemIdentifier::TableTopPlaneCorrection);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateTableFlangeToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  return this->UpdateFrameToRasTransform(parameterNode, CoordinateSystemIdentifier::TableFlange);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateTableRobotFlangeToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  return this->UpdateFrameToRasTransform(parameterNode, CoordinateSystemIdentifier::TableRobotFlange);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateTableRobotWristToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  return this->UpdateFrameToRasTransform(parameterNode, CoordinateSystemIdentifier::TableRobotWrist);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateTableRobotElbowWristToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  return this->UpdateFrameToRasTransform(parameterNode, CoordinateSystemIdentifier::TableRobotElbowWrist);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateTableRobotElbowShoulderToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  return this->UpdateFrameToRasTransform(parameterNode, CoordinateSystemIdentifier::TableRobotElbowShoulder);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateTableRobotShoulderToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  return this->UpdateFrameToRasTransform(parameterNode, CoordinateSystemIdentifier::TableRobotShoulder);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateTableRobotBaseRotationToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  return this->UpdateFrameToRasTransform(parameterNode, CoordinateSystemIdentifier::TableRobotBaseRotation);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateTableRobotBaseRotationDiskToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  return this->UpdateFrameToRasTransform(parameterNode, CoordinateSystemIdentifier::TableRobotBaseRotationDisk);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateTableRobotBaseFixedToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  return this->UpdateFrameToRasTransform(parameterNode, CoordinateSystemIdentifier::TableRobotBaseFixed);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateFixedReferenceToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  return this->UpdateFrameToRasTransform(parameterNode, CoordinateSystemIdentifier::FixedReference);
/*
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateFixedReferenceToRasTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateFixedReferenceToRasTransform: Invalid MRML scene");
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

  // FixedReference -> RAS
  // FixedReference - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableFlange -> TableRobotFlange -> TableRobotWrist
  // TableRobotWrist -> TableRobotElbowWrist -> TableRobotElbowShoulder -> TableRobotShoulder
  // TableRobotShoulder -> TableRobotBaseRotation -> TableRobotBaseFixed -> FixedReference
  // Find RasToTableRobotWristTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> FixedReferenceToRasTransformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("FixedReferenceToRasTransform"))
  {
    FixedReferenceToRasTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }
  else
  {
    FixedReferenceToRasTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    FixedReferenceToRasTransformNode->SetName("FixedReferenceToRasTransform");
//    FixedReferenceToRasTransformNode->SetHideFromEditors(1);
    std::string singletonTag = std::string("C26C3_") + "FixedReferenceToRasTransform";
//    FixedReferenceToRasTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(FixedReferenceToRasTransformNode);
  }

  vtkNew<vtkTransform> FixedReferenceToRasTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::FixedReference, 
    FixedReferenceToRasTransform, false))
  {
    vtkDebugMacro("UpdateFixedReferenceTransform: FixedReference->RAS transform updated");
    // Transform to RAS, set transform to node, transform the model
    FixedReferenceToRasTransform->Concatenate(patientToRasTransform);
  }
  if (FixedReferenceToRasTransform)
  {
    FixedReferenceToRasTransformNode->SetAndObserveTransformToParent(FixedReferenceToRasTransform);
  }
  return FixedReferenceToRasTransformNode;
*/
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateCarmRobotBaseFixedToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  return this->UpdateFrameToRasTransform(parameterNode, CoordinateSystemIdentifier::CarmRobotBaseFixed);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateCarmRobotBaseRotationToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  return this->UpdateFrameToRasTransform(parameterNode, CoordinateSystemIdentifier::CarmRobotBaseRotation);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateCarmRobotShoulderToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  return this->UpdateFrameToRasTransform(parameterNode, CoordinateSystemIdentifier::CarmRobotShoulder);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateCarmRobotElbowShoulderToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  return this->UpdateFrameToRasTransform(parameterNode, CoordinateSystemIdentifier::CarmRobotElbowShoulder);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateCarmRobotElbowWristToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  return this->UpdateFrameToRasTransform(parameterNode, CoordinateSystemIdentifier::CarmRobotElbowWrist);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateCarmRobotWristToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  return this->UpdateFrameToRasTransform(parameterNode, CoordinateSystemIdentifier::CarmRobotWrist);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateCarmRobotFlangeToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  return this->UpdateFrameToRasTransform(parameterNode, CoordinateSystemIdentifier::CarmRobotFlange);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateCarmToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  return this->UpdateFrameToRasTransform(parameterNode, CoordinateSystemIdentifier::Carm);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateCarmXrayBeamToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  return this->UpdateFrameToRasTransform(parameterNode, CoordinateSystemIdentifier::CarmXrayBeam);
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateCarmXrayDetectorToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  return this->UpdateFrameToRasTransform(parameterNode, CoordinateSystemIdentifier::CarmXrayDetector);
}

//----------------------------------------------------------------------------
void vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdatePatientToTableTopTransform(vtkMRMLChannel26GeometryNode* parameterNode)
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
void vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateTableTopPlaneCorrectionToTableTopTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableTopPlaneCorrectionToTableTopTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateTableTopPlaneCorrectionToTableTopTransform: Invalid parameter node");
    return;
  }

  using CoordSys = CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* tableTopPlaneCorrectionToTableTopTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableTopPlaneCorrection, CoordSys::TableTop);

  if (tableTopPlaneCorrectionToTableTopTransformNode)
  {
    double patientToTableTopTranslation[3] = {};
    parameterNode->GetPatientToTableTopTranslation(patientToTableTopTranslation);
//    patientToTableTopTranslation[0] *= -1;
//    patientToTableTopTranslation[1] *= -1;
//    patientToTableTopTranslation[2] *= -1;
//    vtkNew<vtkTransform> patientToTableTopTransform;
//    patientToTableTopTransform->Translate(patientToTableTopTranslation);

    vtkNew<vtkTransform> planeCorrectionToTableTopTransform;
    double planeAnglesABC[3] = {};
    parameterNode->GetAnglesABC(planeAnglesABC);
    planeCorrectionToTableTopTransform->Identity();
    vtkWarningMacro("UpdateTableTopPlaneCorrectionToTableTopTransform:" << planeAnglesABC[0] << ' ' << planeAnglesABC[1] << ' ' << planeAnglesABC[2]);
    planeCorrectionToTableTopTransform->RotateX(planeAnglesABC[0]);
    planeCorrectionToTableTopTransform->RotateY(planeAnglesABC[1]);
    planeCorrectionToTableTopTransform->RotateZ(planeAnglesABC[2]);
//    planeCorrectionToTableTopTransform->Translate(patientToTableTopTranslation);
    
//    planeCorrectionToTableTopTransform->Concatenate(patientToTableTopTransform);
    tableTopPlaneCorrectionToTableTopTransformNode->SetAndObserveTransformToParent(planeCorrectionToTableTopTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateTableTopToTableFlangeTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableTopToTableFlangeTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateTableTopToTableFlangeTransform: Invalid parameter node");
    return;
  }

  using CoordPos = vtkSlicerChannel26Cabin3RobotsGeometryCommon;
  using CoordSys = CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* tableTopToTableFlangeTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableTop, CoordSys::TableFlange);

  if (tableTopToTableFlangeTransformNode)
  {
    vtkNew<vtkTransform> tableTopToTableFlangeTransform;
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
    tableTopToTableFlangeTransform->Translate(posRes);
    // Concatinate with table flange vertical orientation
    tableTopToTableFlangeTransform->Concatenate(FlangeVerticalOrientationTransform);
    vtkNew<vtkTransform> tableTopToTableFlangeTranslateTransform;
    // Reverse compensate Patient->TableTop translation
    tableTopToTableFlangeTranslateTransform->Translate(-1. * patientToTableTopTranslation[0],
      -1. * patientToTableTopTranslation[1], -1. * patientToTableTopTranslation[2]);
    // Translate to Flange top position under table top center position
    tableTopToTableFlangeTranslateTransform->Translate(-1. * CoordPos::INIT_TABLE_FLANGE_ORIGIN_OFFSET_RAS[0],
      -1. * CoordPos::INIT_TABLE_FLANGE_ORIGIN_OFFSET_RAS[1],
      CoordPos::INIT_TABLE_FLANGE_ORIGIN_OFFSET_RAS[2] - CoordPos::TABLE_FLANGE_HEIGHT);
    tableTopToTableFlangeTranslateTransform->Concatenate(tableTopToTableFlangeTransform);
    tableTopToTableFlangeTransformNode->SetAndObserveTransformToParent(tableTopToTableFlangeTranslateTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateTableFlangeToTableRobotFlangeTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableFlangeToTableRobotFlangeTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateTableFlangeToTableRobotFlangeTransform: Invalid parameter node");
    return;
  }

  using CoordPos = vtkSlicerChannel26Cabin3RobotsGeometryCommon;
  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> tableFlangeToPatientTransform;
  if (!this->GetTransformBetween(CoordSys::TableFlange, CoordSys::Patient, 
    tableFlangeToPatientTransform, false))
  {
    vtkWarningMacro("UpdateTableFlangeToTableRobotFlangeTransform: Can't get TableFlange->Patient transform");
    return;
  }

  // Initial table robot flange model position offset
  const double flangeOffsetX = CoordPos::TABLE_ROBOT_WRIST_SIZE;
  const double flangeOffsetY = 0.;

  // compansate initial position offset of flange model
  tableFlangeToPatientTransform->Translate(flangeOffsetX, flangeOffsetY, 0.);

  vtkNew<vtkTransform> ReverseWristVerticalOrientationTransform; // vertical orientation
  ReverseWristVerticalOrientationTransform->Identity();
  ReverseWristVerticalOrientationTransform->RotateY(90.);

  vtkMRMLLinearTransformNode* tableFlangeToTableRobotWristTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableFlange, CoordSys::TableRobotFlange);
  if (tableFlangeToTableRobotWristTransformNode)
  {
    vtkNew<vtkTransform> tableFlangeToTableRobotFlangeTransform;
    double patientToTableTopTranslation[3] = {};
    parameterNode->GetPatientToTableTopTranslation(patientToTableTopTranslation);

    // Apply transform
    tableFlangeToTableRobotFlangeTransform->Concatenate(ReverseWristVerticalOrientationTransform);
    tableFlangeToTableRobotFlangeTransform->Concatenate(tableFlangeToPatientTransform);

    vtkNew<vtkTransform> patientToTableRobotFlangeTranslateTransform;
    // Reverse compensate Patient->TableTop translation

    patientToTableRobotFlangeTranslateTransform->Translate(-1. * patientToTableTopTranslation[0],
      -1. * patientToTableTopTranslation[1], -1. * patientToTableTopTranslation[2]);

    // Translate to Table robot flange position under Table flange position
    patientToTableRobotFlangeTranslateTransform->Translate(-1. * CoordPos::INIT_TABLE_FLANGE_ORIGIN_OFFSET_RAS[0],
      -1. * CoordPos::INIT_TABLE_FLANGE_ORIGIN_OFFSET_RAS[1],
      CoordPos::INIT_TABLE_FLANGE_ORIGIN_OFFSET_RAS[2] - CoordPos::TABLE_FLANGE_HEIGHT);

    patientToTableRobotFlangeTranslateTransform->Concatenate(tableFlangeToTableRobotFlangeTransform);
    tableFlangeToTableRobotWristTransformNode->SetAndObserveTransformToParent(patientToTableRobotFlangeTranslateTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateTableRobotFlangeToTableRobotWristTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableRobotFlangeToTableRobotWristTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateTableRobotFlangeToTableRobotWristTransform: Invalid parameter node");
    return;
  }

  using CoordPos = vtkSlicerChannel26Cabin3RobotsGeometryCommon;
  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> tableRobotFlangeToPatientTransform;
  if (!this->GetTransformBetween(CoordSys::TableRobotFlange, CoordSys::Patient, 
    tableRobotFlangeToPatientTransform, false))
  {
    vtkWarningMacro("UpdateTableRobotFlangeToTableRobotWristTransform: Can't get TableRobotFlange->Patient transform");
    return;
  }

  // Initial table robot wrist model position offset
  const double wristOffsetX = CoordPos::TABLE_ROBOT_WRIST_SIZE;
  const double wristOffsetY = 0.;

  // compansate initial position offset of wrist model
  tableRobotFlangeToPatientTransform->Translate(wristOffsetX, wristOffsetY, 0.);

  vtkNew<vtkTransform> ReverseWristVerticalOrientationTransform; // vertical orientation
  ReverseWristVerticalOrientationTransform->Identity();
  ReverseWristVerticalOrientationTransform->RotateY(90.);

  vtkMRMLLinearTransformNode* tableFlangeToTableRobotWristTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableRobotFlange, CoordSys::TableRobotWrist);
  if (tableFlangeToTableRobotWristTransformNode)
  {

    vtkNew<vtkTransform> tableFlangeToTableRobotFlangeTransform;
    double a[6] = {};
    parameterNode->GetTableRobotAngles(a);
    double patientToTableTopTranslation[3] = {};
    parameterNode->GetPatientToTableTopTranslation(patientToTableTopTranslation);

    // Apply transform (rotation around Z axis on A6 angle in RAS origin)
    double angle = 90. + a[5]; // setup angle according to orientation
    tableFlangeToTableRobotFlangeTransform->RotateZ(angle);
    tableFlangeToTableRobotFlangeTransform->Concatenate(ReverseWristVerticalOrientationTransform);
    tableFlangeToTableRobotFlangeTransform->Concatenate(tableRobotFlangeToPatientTransform);

    vtkNew<vtkTransform> patientToTableRobotFlangeTranslateTransform;
    // Reverse compensate Patient->TableTop translation
    patientToTableRobotFlangeTranslateTransform->Translate(-1. * patientToTableTopTranslation[0],
      -1. * patientToTableTopTranslation[1], -1. * patientToTableTopTranslation[2]);
  
    // Translate to Table robot wrist position under Table robot flange position (A6 angle rotation)
    patientToTableRobotFlangeTranslateTransform->Translate(-1. * CoordPos::INIT_TABLE_ROBOT_FLANGE_ORIGIN_OFFSET_RAS[0],
      -1. * CoordPos::INIT_TABLE_ROBOT_FLANGE_ORIGIN_OFFSET_RAS[1], CoordPos::INIT_TABLE_ROBOT_FLANGE_ORIGIN_OFFSET_RAS[2]);

    patientToTableRobotFlangeTranslateTransform->Concatenate(tableFlangeToTableRobotFlangeTransform);

    tableFlangeToTableRobotWristTransformNode->SetAndObserveTransformToParent(patientToTableRobotFlangeTranslateTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateTableRobotWristToTableRobotElbowWristTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableRobotWristToTableRobotElbowWristTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateTableRobotWristToTableRobotElbowWristTransform: Invalid parameter node");
    return;
  }

  using CoordPos = vtkSlicerChannel26Cabin3RobotsGeometryCommon;
  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> tableRobotWristToPatientTransform;
  if (!this->GetTransformBetween(CoordSys::TableRobotWrist, CoordSys::Patient, 
    tableRobotWristToPatientTransform, false))
  {
    vtkWarningMacro("UpdateTableRobotWristToTableRobotElbowWristTransform: Can't get TableRobotWrist->Patient transform");
    return;
  }

  // Initial table robot elbow wrist model position offset
  const double elbowWristOffsetX = CoordPos::TABLE_ROBOT_ELBOW_SIZE;
  const double elbowWristOffsetY = 0.;
  // compansate initial position offset of elbow wrist model
  tableRobotWristToPatientTransform->Translate(elbowWristOffsetX, elbowWristOffsetY, 0.);

  vtkNew<vtkTransform> ReverseWristVerticalOrientationTransform; // vertical orientation
  ReverseWristVerticalOrientationTransform->Identity();
  ReverseWristVerticalOrientationTransform->RotateY(90.);

  vtkMRMLLinearTransformNode* tableRobotWristToTableRobotElbowWristTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableRobotWrist, CoordSys::TableRobotElbowWrist);
  if (tableRobotWristToTableRobotElbowWristTransformNode)
  {
    vtkNew<vtkTransform> tableRobotWristToTableRobotElbowWristTransform;
    double a[6] = {};
    parameterNode->GetTableRobotAngles(a);
    double patientToTableTopTranslation[3] = {};
    parameterNode->GetPatientToTableTopTranslation(patientToTableTopTranslation);

    // Wrist->Flange (TableTop) rotation
    vtkNew<vtkTransform> WristToFlangeTransform;
    // rotation around Z axis on A6 angle in RAS origin
    WristToFlangeTransform->RotateZ(a[5]);

    // Apply transform (rotation around Y axis on A5 angle, and compansate model orientation around X axis)
    vtkNew<vtkTransform> WristToElbowTransform;
    WristToElbowTransform->RotateY(a[4]);
    WristToElbowTransform->RotateX(90.);
    // Translate table robot wrist to table robot elbow wrist (origin) in RAS (Patient) origin, so it's end in RAS origin
    WristToElbowTransform->Concatenate(tableRobotWristToPatientTransform);
    // Apply Wrist->Flange (TableTop) rotation transform
    WristToFlangeTransform->Concatenate(WristToElbowTransform);

    vtkNew<vtkTransform> PatientToTableRobotElbowWristTranslateTransform;

    // Reverse compensate Patient->TableTop translation
    PatientToTableRobotElbowWristTranslateTransform->Translate(-1. * patientToTableTopTranslation[0],
      -1. * patientToTableTopTranslation[1], -1. * patientToTableTopTranslation[2]);

    // Translate to Table robot wrist elbow position under Table robot wrist position (A5 angle rotation)
    PatientToTableRobotElbowWristTranslateTransform->Translate(-1. * CoordPos::INIT_TABLE_ROBOT_WRIST_ORIGIN_OFFSET_RAS[0],
      -1. * CoordPos::INIT_TABLE_ROBOT_WRIST_ORIGIN_OFFSET_RAS[1],
      CoordPos::INIT_TABLE_ROBOT_WRIST_ORIGIN_OFFSET_RAS[2]);

    PatientToTableRobotElbowWristTranslateTransform->Concatenate(WristToFlangeTransform);
    tableRobotWristToTableRobotElbowWristTransformNode->SetAndObserveTransformToParent(PatientToTableRobotElbowWristTranslateTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateTableRobotElbowWristToTableRobotElbowShoulderTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableRobotElbowWristToTableRobotElbowShoulderTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateTableRobotElbowWristToTableRobotElbowShoulderTransform: Invalid parameter node");
    return;
  }

  using CoordPos = vtkSlicerChannel26Cabin3RobotsGeometryCommon;
  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> tableRobotElbowWristToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::TableRobotElbowWrist, CoordSys::Patient, 
    tableRobotElbowWristToPatientTransform, false))
  {
    vtkWarningMacro("UpdateTableRobotElbowWristToTableRobotElbowShoulderTransform: Can't get TableRobotElbowWrist->Patient transform");
    return;
  }

  // Initial table robot elbow shoulder model position offset
  const double elbowShoulderOffsetX = CoordPos::TABLE_ROBOT_ELBOW_SIZE;
  const double elbowShoulderOffsetY = 0.;

  // compansate initial position offset of elbow shoulder model
  tableRobotElbowWristToPatientTransform->Translate(elbowShoulderOffsetX, elbowShoulderOffsetY, 0.);

  vtkNew<vtkTransform> ReverseWristVerticalOrientationTransform; // vertical orientation
  ReverseWristVerticalOrientationTransform->Identity();
  ReverseWristVerticalOrientationTransform->RotateY(90.);

  vtkMRMLLinearTransformNode* tableRobotElbowWristToTableRobotElbowShoulderTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableRobotElbowWrist, CoordSys::TableRobotElbowShoulder);
  if (tableRobotElbowWristToTableRobotElbowShoulderTransformNode)
  {
    vtkNew<vtkTransform> tableRobotWristToTableRobotElbowWristTransform;
    double a[6] = {};
    parameterNode->GetTableRobotAngles(a);
    double patientToTableTopTranslation[3] = {};
    parameterNode->GetPatientToTableTopTranslation(patientToTableTopTranslation);

    // Wrist->Flange (TableTop) rotation
    vtkNew<vtkTransform> WristToFlangeTransform;
    WristToFlangeTransform->RotateZ(a[5]);

    // Apply transform (rotation around Y axis on A5 angle, and around X axis on A4 angle in RAS origin)
    vtkNew<vtkTransform> WristToElbowTransform;
    WristToElbowTransform->RotateY(a[4]);
    WristToElbowTransform->RotateX(270. - a[3]);
    // Transform robot elbow wrist in RAS (Patient) origin so, it's begin in RAS origin
    WristToElbowTransform->Concatenate(tableRobotElbowWristToPatientTransform);
    // Apply Wrist->Flange (TableTop) rotation transform
    WristToFlangeTransform->Concatenate(WristToElbowTransform);

    vtkNew<vtkTransform> PatientToTableRobotElbowWristTranslateTransform;

    // Reverse compensate Patient->TableTop translation
    PatientToTableRobotElbowWristTranslateTransform->Translate(-1. * patientToTableTopTranslation[0],
      -1. * patientToTableTopTranslation[1], -1. * patientToTableTopTranslation[2]);

    // Translate to Table robot wrist elbow position under Table robot wrist position (A4 angle rotation)
    PatientToTableRobotElbowWristTranslateTransform->Translate(-1. * CoordPos::INIT_TABLE_ROBOT_WRIST_ORIGIN_OFFSET_RAS[0],
      -1. * CoordPos::INIT_TABLE_ROBOT_WRIST_ORIGIN_OFFSET_RAS[1],
      CoordPos::INIT_TABLE_ROBOT_WRIST_ORIGIN_OFFSET_RAS[2]);

    PatientToTableRobotElbowWristTranslateTransform->Concatenate(WristToFlangeTransform);

    tableRobotElbowWristToTableRobotElbowShoulderTransformNode->SetAndObserveTransformToParent(PatientToTableRobotElbowWristTranslateTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateTableRobotElbowShoulderToTableRobotShoulderTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableRobotElbowShoulderToTableRobotShoulderTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateTableRobotElbowShoulderToTableRobotShoulderTransform: Invalid parameter node");
    return;
  }

  using CoordPos = vtkSlicerChannel26Cabin3RobotsGeometryCommon;
  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> tableRobotElbowShoulderToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::TableRobotElbowShoulder, CoordSys::Patient, 
    tableRobotElbowShoulderToPatientTransform, false))
  {
    vtkWarningMacro("UpdateTableRobotElbowShoulderToTableRobotShoulderTransform: Can't get TableRobotElbowShoulder->Patient transform");
    return;
  }

  // Initial table robot shoulder model position offset
  const double shoulderOffsetX = 0.0;
  const double shoulderOffsetY = CoordPos::TABLE_ROBOT_SHOULDER_SIZE;

  // compansate initial position offset of shoulder model
  tableRobotElbowShoulderToPatientTransform->Translate(shoulderOffsetX, shoulderOffsetY, 0.);

  // Transform model to vertical position
  vtkNew<vtkTransform> ShoulderVerticalOrientationTransform; // vertical orientation
  ShoulderVerticalOrientationTransform->Identity();
  ShoulderVerticalOrientationTransform->RotateX(-90.);

  vtkMRMLLinearTransformNode* elbowToShoulderTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableRobotElbowShoulder, CoordSys::TableRobotShoulder);
  if (elbowToShoulderTransformNode)
  {
    double a[6] = {};
    parameterNode->GetTableRobotAngles(a);
    double patientToTableTopTranslation[3] = {};
    parameterNode->GetPatientToTableTopTranslation(patientToTableTopTranslation);

    /// Get current position of Elbow origin (begin)
    // Translate the Elbow so it's end in RAS origin
    vtkNew<vtkTransform> ElbowTranslateTransform;
    ElbowTranslateTransform->Translate(CoordPos::TABLE_ROBOT_ELBOW_SIZE,
      0., -1. * CoordPos::TABLE_ROBOT_SHOULDER_ELBOW_OFFSET_Y);

    // Wrist->Flange (TableTop) rotation
    vtkNew<vtkTransform> WristToFlangeTransform;
    WristToFlangeTransform->RotateZ(a[5]);

    // Apply transform (rotation around Y axis on A5 angle, and around X axis on A4 angle in RAS origin)
    vtkNew<vtkTransform> WristToElbowTransform;
    WristToElbowTransform->RotateY(a[4]);
    WristToElbowTransform->RotateX(-1. * a[3]);
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
    A6A5A4RotationTransform->RotateX(-1. * a[3]);

    // Transform shoulder in RAS (Patient) origin so, it's begin in RAS origin
    // Transform to RAS origin and model vertical orientation
    ShoulderVerticalOrientationTransform->Concatenate(tableRobotElbowShoulderToPatientTransform);
    // Translate  Shoulder end to RAS (Patient) origin so, it's begin in RAS origin
    ElbowToShoulderRotationTransform->Concatenate(ShoulderVerticalOrientationTransform);
    // Apply transform of the Elbow (A6, A5, A4 angles) to Shoulder model in RAS origin
    A6A5A4RotationTransform->Concatenate(ElbowToShoulderRotationTransform);

    vtkNew<vtkTransform> PatientToFlangeTranslateTransform;
    // Reverse compensate Patient->TableTop translation
    PatientToFlangeTranslateTransform->Translate(-1. * patientToTableTopTranslation[0],
      -1. * patientToTableTopTranslation[1], -1. * patientToTableTopTranslation[2]);

    // Translate to Table robot wrist elbow position under Table robot wrist position (A4 angle rotation)
    PatientToFlangeTranslateTransform->Translate(-1. * CoordPos::INIT_TABLE_ROBOT_WRIST_ORIGIN_OFFSET_RAS[0],
      -1. * CoordPos::INIT_TABLE_ROBOT_WRIST_ORIGIN_OFFSET_RAS[1],
      CoordPos::INIT_TABLE_ROBOT_WRIST_ORIGIN_OFFSET_RAS[2]);
    // Translate to new elbow begin position
    PatientToFlangeTranslateTransform->Translate(NewElbowBeginPositionTranslate);
    // Apply Translation of a Shoulder model to the new Elbow begin position
    PatientToFlangeTranslateTransform->Concatenate(A6A5A4RotationTransform);

    elbowToShoulderTransformNode->SetAndObserveTransformToParent(PatientToFlangeTranslateTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateTableRobotShoulderToTableRobotBaseRotationTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableRobotShoulderToTableRobotBaseRotationTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateTableRobotShoulderToTableRobotBaseRotationTransform: Invalid parameter node");
    return;
  }

  using CoordPos = vtkSlicerChannel26Cabin3RobotsGeometryCommon;
  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> tableRobotShoulderToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::TableRobotShoulder, CoordSys::Patient, 
    tableRobotShoulderToPatientTransform, false))
  {
    vtkWarningMacro("UpdateTableRobotElbowShoulderToTableRobotShoulderTransform: Can't get TableRobotShoulder->Patient transform");
    return;
  }

  // Initial table robot base rotation model position offset
  const double baseRotationOffsetX = CoordPos::TABLE_ROBOT_BASE_ROTATION_SHOULDER_OFFSET_X;
  const double baseRotationOffsetY = CoordPos::TABLE_ROBOT_BASE_ROTATION_SHOULDER_OFFSET_Y + CoordPos::TABLE_ROBOT_SHOULDER_SIZE;

  // compansate initial position offset of shoulder model
  tableRobotShoulderToPatientTransform->Translate(baseRotationOffsetX, baseRotationOffsetY, 0.);

  // Transform model to vertical position
  vtkNew<vtkTransform> ShoulderVerticalOrientationTransform; // vertical orientation
  ShoulderVerticalOrientationTransform->Identity();
  ShoulderVerticalOrientationTransform->RotateX(-90.);

  vtkMRMLLinearTransformNode* shoulderToBaseRotationTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableRobotShoulder, CoordSys::TableRobotBaseRotation);
  if (shoulderToBaseRotationTransformNode)
  {
    double a[6] = {};
    parameterNode->GetTableRobotAngles(a);
    double patientToTableTopTranslation[3] = {};
    parameterNode->GetPatientToTableTopTranslation(patientToTableTopTranslation);

    /// Get current position of Elbow origin (begin)
    // Translate the Elbow so it's end in RAS origin
    vtkNew<vtkTransform> ElbowTranslateTransform;
    ElbowTranslateTransform->Translate(CoordPos::TABLE_ROBOT_ELBOW_SIZE, 0., -1. * CoordPos::TABLE_ROBOT_SHOULDER_ELBOW_OFFSET_Y);

    // Wrist->Flange (TableTop) rotation
    vtkNew<vtkTransform> WristToFlangeTransform;
    WristToFlangeTransform->RotateZ(a[5]);

    // Apply transform (rotation around Y axis on A5 angle, and around X axis on A4 angle in RAS origin)
    vtkNew<vtkTransform> WristToElbowTransform;
    WristToElbowTransform->RotateY(a[4]);
    WristToElbowTransform->RotateX(-1. * a[3]);
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
    A6A5A4RotationTransform->RotateX(-1. * a[3]);

    // Shoulder->BaseRotation rotation around Y (A2 angle)
    vtkNew<vtkTransform> ShoulderToBaseRotationTransform;
    ShoulderToBaseRotationTransform->RotateY(a[1]);
    // BaseRotation model -> move to Shoulder rotation origin A3
    vtkNew<vtkTransform> BaseRotationOriginToShoulderOriginTransform;
    BaseRotationOriginToShoulderOriginTransform->Translate(0., 0.,
      CoordPos::TABLE_ROBOT_SHOULDER_SIZE);
    // BaseRotation model -> move back to BaseRotation rotation origin A2
    vtkNew<vtkTransform> InverseBaseRotationOriginToShoulderOriginTransform;
    InverseBaseRotationOriginToShoulderOriginTransform->Translate(0., 0.,
      -1. * CoordPos::TABLE_ROBOT_SHOULDER_SIZE);

    ShoulderToBaseRotationTransform->Concatenate(BaseRotationOriginToShoulderOriginTransform);
    InverseBaseRotationOriginToShoulderOriginTransform->Concatenate(ShoulderToBaseRotationTransform);
    
    // Transform shoulder in RAS (Patient) origin so, it's begin in RAS origin
    // Transform to RAS origin and model vertical orientation
    ShoulderVerticalOrientationTransform->Concatenate(tableRobotShoulderToPatientTransform);
    // Translate  Shoulder end to RAS (Patient) origin so, it's begin in RAS origin
    InverseBaseRotationOriginToShoulderOriginTransform->Concatenate(ShoulderVerticalOrientationTransform);
    // Apply A2 angle to Shoulder A3 model 
    ElbowToShoulderRotationTransform->Concatenate(InverseBaseRotationOriginToShoulderOriginTransform);
    // Apply transform of the Elbow (A6, A5, A4 angles) to Shoulder model in RAS origin
    A6A5A4RotationTransform->Concatenate(ElbowToShoulderRotationTransform);

    vtkNew<vtkTransform> PatientToFlangeTranslateTransform;
    // Reverse compensate Patient->TableTop translation
    PatientToFlangeTranslateTransform->Translate(-1. * patientToTableTopTranslation[0],
      -1. * patientToTableTopTranslation[1], -1. * patientToTableTopTranslation[2]);

    // Translate to Table robot wrist elbow position under Table robot wrist position (A4 angle rotation)
    PatientToFlangeTranslateTransform->Translate(-1. * CoordPos::INIT_TABLE_ROBOT_WRIST_ORIGIN_OFFSET_RAS[0],
      -1. * CoordPos::INIT_TABLE_ROBOT_WRIST_ORIGIN_OFFSET_RAS[1],
      CoordPos::INIT_TABLE_ROBOT_WRIST_ORIGIN_OFFSET_RAS[2]);
    // Translate to new elbow begin position
    PatientToFlangeTranslateTransform->Translate(NewElbowBeginPositionTranslate);
    // Apply Translation of a Shoulder model to the new Elbow begin position
    PatientToFlangeTranslateTransform->Concatenate(A6A5A4RotationTransform);

    shoulderToBaseRotationTransformNode->SetAndObserveTransformToParent(PatientToFlangeTranslateTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateTableRobotBaseRotationToTableRobotBaseFixedTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableRobotBaseRotationToTableRobotBaseFixedTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateTableRobotBaseRotationToTableRobotBaseFixedTransform: Invalid parameter node");
    return;
  }

  using CoordPos = vtkSlicerChannel26Cabin3RobotsGeometryCommon;
  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> tableRobotBaseRotationToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::TableRobotBaseRotation, CoordSys::Patient, 
    tableRobotBaseRotationToPatientTransform, false))
  {
    vtkWarningMacro("UpdateTableRobotBaseRotationToTableRobotBaseFixedTransform: Can't get TableRobotBaseRotation->Patient transform");
    return;
  }

  // Initial table robot base rotation model position offset
  const double baseRotationOffsetX = CoordPos::TABLE_ROBOT_BASE_ROTATION_SHOULDER_OFFSET_X;
  const double baseRotationOffsetY = CoordPos::TABLE_ROBOT_BASE_ROTATION_SHOULDER_OFFSET_Y + CoordPos::TABLE_ROBOT_SHOULDER_SIZE;

  // compansate initial position offset of shoulder model
  tableRobotBaseRotationToPatientTransform->Translate(baseRotationOffsetX, baseRotationOffsetY, 0.);

  // Transform model to vertical position
  vtkNew<vtkTransform> ShoulderVerticalOrientationTransform; // vertical orientation
  ShoulderVerticalOrientationTransform->Identity();
  ShoulderVerticalOrientationTransform->RotateX(-90.);

  vtkMRMLLinearTransformNode* baseRotatioToBaseFixedTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableRobotBaseRotation, CoordSys::TableRobotBaseFixed);
  if (baseRotatioToBaseFixedTransformNode)
  {
    double a[6] = {};
    parameterNode->GetTableRobotAngles(a);
    double patientToTableTopTranslation[3] = {};
    parameterNode->GetPatientToTableTopTranslation(patientToTableTopTranslation);

    /// Get current position of Elbow origin (begin)
    // Translate the Elbow so it's end in RAS origin
    vtkNew<vtkTransform> ElbowTranslateTransform;
    ElbowTranslateTransform->Translate(CoordPos::TABLE_ROBOT_ELBOW_SIZE, 0., -1. * CoordPos::TABLE_ROBOT_SHOULDER_ELBOW_OFFSET_Y);

    // Wrist->Flange (TableTop) rotation
    vtkNew<vtkTransform> WristToFlangeTransform;
    WristToFlangeTransform->RotateZ(a[5]);

    // Apply transform (rotation around Y axis on A5 angle, and around X axis on A4 angle in RAS origin)
    vtkNew<vtkTransform> WristToElbowTransform;
    WristToElbowTransform->RotateY(a[4]);
    WristToElbowTransform->RotateX(-1. * a[3]);
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
    A6A5A4RotationTransform->RotateX(-1. * a[3]);

    // Shoulder->BaseRotation rotation around Y (A2 angle)
    vtkNew<vtkTransform> ShoulderToBaseRotationTransform;
    ShoulderToBaseRotationTransform->RotateY(a[1]);
    // BaseRotation model -> move to Shoulder rotation origin A3
    vtkNew<vtkTransform> BaseRotationOriginToShoulderOriginTransform;
    BaseRotationOriginToShoulderOriginTransform->Translate(0., 0.,
      CoordPos::TABLE_ROBOT_SHOULDER_SIZE);
    // BaseRotation model -> move back to BaseRotation rotation origin A2
    vtkNew<vtkTransform> InverseBaseRotationOriginToShoulderOriginTransform;
    InverseBaseRotationOriginToShoulderOriginTransform->Translate(0., 0.,
      -1. * CoordPos::TABLE_ROBOT_SHOULDER_SIZE);

    ShoulderToBaseRotationTransform->Concatenate(BaseRotationOriginToShoulderOriginTransform);
    InverseBaseRotationOriginToShoulderOriginTransform->Concatenate(ShoulderToBaseRotationTransform);


    // BaseRotation->BaseFixed rotation around Z (A1 angle)
    vtkNew<vtkTransform> BaseRotationToBaseFixedTransform;
    BaseRotationToBaseFixedTransform->RotateZ(a[0]);
    // BaseFixed model -> move to BaseRotation rotation origin A2 along X
    vtkNew<vtkTransform> BaseFixedOriginToBaseRotationOriginTransform;
    BaseFixedOriginToBaseRotationOriginTransform->Translate(-1. * CoordPos::TABLE_ROBOT_BASE_ROTATION_SHOULDER_OFFSET_X,
      0., 0.);
    // BaseFixed model -> move back to BaseFixed rotation origin A1 along X
    vtkNew<vtkTransform> InverseBaseFixedOriginToBaseRotationOriginTransform;
    InverseBaseFixedOriginToBaseRotationOriginTransform->Translate(CoordPos::TABLE_ROBOT_BASE_ROTATION_SHOULDER_OFFSET_X,
      0., 0.);

    BaseRotationToBaseFixedTransform->Concatenate(BaseFixedOriginToBaseRotationOriginTransform);
    InverseBaseFixedOriginToBaseRotationOriginTransform->Concatenate(BaseRotationToBaseFixedTransform);

    // Transform shoulder in RAS (Patient) origin so, it's begin in RAS origin
    // Transform to RAS origin and model vertical orientation
    ShoulderVerticalOrientationTransform->Concatenate(tableRobotBaseRotationToPatientTransform);
    // Translate  Shoulder end to RAS (Patient) origin so, it's begin in RAS origin
    InverseBaseFixedOriginToBaseRotationOriginTransform->Concatenate(ShoulderVerticalOrientationTransform);
    InverseBaseRotationOriginToShoulderOriginTransform->Concatenate(InverseBaseFixedOriginToBaseRotationOriginTransform);
    // Apply A2 angle to Shoulder A3 model 
    ElbowToShoulderRotationTransform->Concatenate(InverseBaseRotationOriginToShoulderOriginTransform);
    // Apply transform of the Elbow (A6, A5, A4 angles) to Shoulder model in RAS origin
    A6A5A4RotationTransform->Concatenate(ElbowToShoulderRotationTransform);

    vtkNew<vtkTransform> PatientToFlangeTranslateTransform;
    // Reverse compensate Patient->TableTop translation
    PatientToFlangeTranslateTransform->Translate(-1. * patientToTableTopTranslation[0],
      -1. * patientToTableTopTranslation[1], -1. * patientToTableTopTranslation[2]);

    // Translate to Table robot wrist elbow position under Table robot wrist position (A4 angle rotation)
    PatientToFlangeTranslateTransform->Translate(-1. * CoordPos::INIT_TABLE_ROBOT_WRIST_ORIGIN_OFFSET_RAS[0],
      -1. * CoordPos::INIT_TABLE_ROBOT_WRIST_ORIGIN_OFFSET_RAS[1],
      CoordPos::INIT_TABLE_ROBOT_WRIST_ORIGIN_OFFSET_RAS[2]);
    // Translate to new elbow begin position
    PatientToFlangeTranslateTransform->Translate(NewElbowBeginPositionTranslate);
    // Apply Translation of a Shoulder model to the new Elbow begin position
    PatientToFlangeTranslateTransform->Concatenate(A6A5A4RotationTransform);

    baseRotatioToBaseFixedTransformNode->SetAndObserveTransformToParent(PatientToFlangeTranslateTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateTableRobotBaseRotationDiskToTableRobotBaseFixedTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableRobotBaseRotationDiskToTableRobotBaseFixedTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateTableRobotBaseRotationDiskToTableRobotBaseFixedTransform: Invalid parameter node");
    return;
  }

  using CoordPos = vtkSlicerChannel26Cabin3RobotsGeometryCommon;
  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> tableRobotBaseRotationToPatientTransform;
  if (!this->GetTransformBetween(CoordSys::TableRobotBaseRotation, CoordSys::Patient, 
    tableRobotBaseRotationToPatientTransform, false))
  {
    vtkWarningMacro("UpdateTableRobotBaseRotationDiskToTableRobotBaseFixedTransform: Can't get TableRobotBaseRotation->Patient transform");
    return;
  }

  // Initial table robot base rotation model position offset
  const double baseRotationOffsetX = CoordPos::TABLE_ROBOT_BASE_ROTATION_SHOULDER_OFFSET_X;
  const double baseRotationOffsetY = CoordPos::TABLE_ROBOT_BASE_ROTATION_SHOULDER_OFFSET_Y + CoordPos::TABLE_ROBOT_SHOULDER_SIZE;

  // compansate initial position offset of shoulder model
  tableRobotBaseRotationToPatientTransform->Translate(baseRotationOffsetX, baseRotationOffsetY, 0);

  // Transform model to vertical position
  vtkNew<vtkTransform> ShoulderVerticalOrientationTransform; // vertical orientation
  ShoulderVerticalOrientationTransform->Identity();
  ShoulderVerticalOrientationTransform->RotateX(-90.);

  vtkMRMLLinearTransformNode* baseRotatioToBaseFixedTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableRobotBaseRotationDisk, CoordSys::TableRobotBaseFixed);
  if (baseRotatioToBaseFixedTransformNode)
  {
    double a[6] = {};
    parameterNode->GetTableRobotAngles(a);
    double patientToTableTopTranslation[3] = {};
    parameterNode->GetPatientToTableTopTranslation(patientToTableTopTranslation);

    /// Get current position of Elbow origin (begin)
    // Translate the Elbow so it's end in RAS origin
    vtkNew<vtkTransform> ElbowTranslateTransform;
    ElbowTranslateTransform->Translate(CoordPos::TABLE_ROBOT_ELBOW_SIZE, 0., -1. * CoordPos::TABLE_ROBOT_SHOULDER_ELBOW_OFFSET_Y);

    // Wrist->Flange (TableTop) rotation
    vtkNew<vtkTransform> WristToFlangeTransform;
    WristToFlangeTransform->RotateZ(a[5]);

    // Apply transform (rotation around Y axis on A5 angle, and around X axis on A4 angle in RAS origin)
    vtkNew<vtkTransform> WristToElbowTransform;
    WristToElbowTransform->RotateY(a[4]);
    WristToElbowTransform->RotateX(-1. * a[3]);
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
    A6A5A4RotationTransform->RotateX(-1. * a[3]);

    // Shoulder->BaseRotation rotation around Y (A2 angle)
    vtkNew<vtkTransform> ShoulderToBaseRotationTransform;
    ShoulderToBaseRotationTransform->RotateY(a[1]);
    // BaseRotation model -> move to Shoulder rotation origin A3
    vtkNew<vtkTransform> BaseRotationOriginToShoulderOriginTransform;
    BaseRotationOriginToShoulderOriginTransform->Translate(0., 0.,
      CoordPos::TABLE_ROBOT_SHOULDER_SIZE);
    // BaseRotation model -> move back to BaseRotation rotation origin A2
    vtkNew<vtkTransform> InverseBaseRotationOriginToShoulderOriginTransform;
    InverseBaseRotationOriginToShoulderOriginTransform->Translate(0., 0.,
      -1. * CoordPos::TABLE_ROBOT_SHOULDER_SIZE);

    ShoulderToBaseRotationTransform->Concatenate(BaseRotationOriginToShoulderOriginTransform);
    InverseBaseRotationOriginToShoulderOriginTransform->Concatenate(ShoulderToBaseRotationTransform);


    // BaseRotation->BaseFixed rotation around Z (A1 angle)
    vtkNew<vtkTransform> BaseRotationToBaseFixedTransform;
    BaseRotationToBaseFixedTransform->RotateZ(a[0]);
    // correct vertical offset of base rotation disk and fake floor
    BaseRotationToBaseFixedTransform->Translate(0.,
      0., -1 * CoordPos::TABLE_TOP_ROBOT_DISK_BASEMENT_HEIGHT);

    // BaseFixed model -> move to BaseRotation rotation origin A2 along X
    vtkNew<vtkTransform> BaseFixedOriginToBaseRotationOriginTransform;
    BaseFixedOriginToBaseRotationOriginTransform->Translate(-1. * CoordPos::TABLE_ROBOT_BASE_ROTATION_SHOULDER_OFFSET_X,
      0., 0.);
    // BaseFixed model -> move back to BaseFixed rotation origin A1 along X
    vtkNew<vtkTransform> InverseBaseFixedOriginToBaseRotationOriginTransform;
    InverseBaseFixedOriginToBaseRotationOriginTransform->Translate(CoordPos::TABLE_ROBOT_BASE_ROTATION_SHOULDER_OFFSET_X,
      0., 0.);

    BaseRotationToBaseFixedTransform->Concatenate(BaseFixedOriginToBaseRotationOriginTransform);
    InverseBaseFixedOriginToBaseRotationOriginTransform->Concatenate(BaseRotationToBaseFixedTransform);

    // Transform shoulder in RAS (Patient) origin so, it's begin in RAS origin
    // Transform to RAS origin and model vertical orientation
    ShoulderVerticalOrientationTransform->Concatenate(tableRobotBaseRotationToPatientTransform);
    // Translate  Shoulder end to RAS (Patient) origin so, it's begin in RAS origin
    InverseBaseFixedOriginToBaseRotationOriginTransform->Concatenate(ShoulderVerticalOrientationTransform);
    InverseBaseRotationOriginToShoulderOriginTransform->Concatenate(InverseBaseFixedOriginToBaseRotationOriginTransform);
    // Apply A2 angle to Shoulder A3 model 
    ElbowToShoulderRotationTransform->Concatenate(InverseBaseRotationOriginToShoulderOriginTransform);
    // Apply transform of the Elbow (A6, A5, A4 angles) to Shoulder model in RAS origin
    A6A5A4RotationTransform->Concatenate(ElbowToShoulderRotationTransform);

    vtkNew<vtkTransform> PatientToFlangeTranslateTransform;
    // Reverse compensate Patient->TableTop translation
    PatientToFlangeTranslateTransform->Translate(-1. * patientToTableTopTranslation[0],
      -1. * patientToTableTopTranslation[1], -1. * patientToTableTopTranslation[2]);

    // Translate to Table robot wrist elbow position under Table robot wrist position (A4 angle rotation)
    PatientToFlangeTranslateTransform->Translate(-1. * CoordPos::INIT_TABLE_ROBOT_WRIST_ORIGIN_OFFSET_RAS[0],
      -1. * CoordPos::INIT_TABLE_ROBOT_WRIST_ORIGIN_OFFSET_RAS[1],
      CoordPos::INIT_TABLE_ROBOT_WRIST_ORIGIN_OFFSET_RAS[2]);
    // Translate to new elbow begin position
    PatientToFlangeTranslateTransform->Translate(NewElbowBeginPositionTranslate);
    // Apply Translation of a Shoulder model to the new Elbow begin position
    PatientToFlangeTranslateTransform->Concatenate(A6A5A4RotationTransform);

    baseRotatioToBaseFixedTransformNode->SetAndObserveTransformToParent(PatientToFlangeTranslateTransform);
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateTableRobotBaseFixedToFixedReferenceTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableRobotBaseFixedToFixedReferenceTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateTableRobotBaseFixedToFixedReferenceTransform: Invalid parameter node");
    return;
  }

  using CoordPos = vtkSlicerChannel26Cabin3RobotsGeometryCommon;
  double BaseFixedToFixedReferenceTranslate[3] = {};
  parameterNode->GetTableBaseFixedToFixedReferenceTranslation(BaseFixedToFixedReferenceTranslate);
  // Default: FixedReferenceToFixedBasedOffset.data()
  BaseFixedToFixedReferenceTranslate[2] *= -1.; // Make negative Zt (vertical position) value
  // Translate the FixedReference model origin (isocenter position)
  // to BaseFixed model origin (0,0,0 position in robot model file)
  vtkNew<vtkTransform> BaseFixedTranslateTransform;
  BaseFixedTranslateTransform->Translate(0., 0., CoordPos::TABLE_ROBOT_BASE_MOUNTING_OFFSET_Y); // 30. mm offset
  BaseFixedTranslateTransform->Translate(BaseFixedToFixedReferenceTranslate);

  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> baseFixedToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::TableRobotBaseFixed, CoordSys::Patient, 
    baseFixedToPatientTransform, false))
  {
    vtkWarningMacro("UpdateTableRobotBaseFixedToFixedReferenceTransform: Can't get TableRobotBaseFixed->Patient transform");
  }

  double PatientToBaseFixedTranslate[3] = {};
  vtkNew<vtkTransform> patientToBaseFixedTransform;
  if (this->GetTransformBetween( CoordSys::Patient, CoordSys::TableRobotBaseFixed, 
    patientToBaseFixedTransform, false))
  {
    patientToBaseFixedTransform->GetPosition(PatientToBaseFixedTranslate);
  }

  vtkMRMLLinearTransformNode* tableRobotBaseFixedToFixedReferenceTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableRobotBaseFixed, CoordSys::FixedReference);
  if (tableRobotBaseFixedToFixedReferenceTransformNode)
  {
    double a[6] = {};
    parameterNode->GetTableRobotAngles(a);
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
    A6A5A4RotationTransform->RotateX(-1. * a[3]);

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

    tableRobotBaseFixedToFixedReferenceTransformNode->SetAndObserveTransformToParent(PatientToFixedReferenceTranslateTransform);
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateCarmRobotBaseFixedToFixedReferenceTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateCarmRobotBaseFixedToFixedReferenceTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateCarmRobotBaseFixedToFixedReferenceTransform: Invalid parameter node");
    return;
  }

  // Translate the BaseRotation so it's empty disk centre in RAS origin
  vtkNew<vtkTransform> BaseFixedTranslateTransform;
  BaseFixedTranslateTransform->Translate(0., 0., 0.);
  double TableBaseFixedToFixedReferenceTranslate[3] = {};
  double CarmBaseFixedToTableBaseFixedOffset[3] = {};
  parameterNode->GetTableBaseFixedToFixedReferenceTranslation(TableBaseFixedToFixedReferenceTranslate);
  parameterNode->GetCarmBaseFixedToTableBaseFixedOffset(CarmBaseFixedToTableBaseFixedOffset);
  // Default: FixedReferenceToFixedBasedOffset.data()
  TableBaseFixedToFixedReferenceTranslate[2] *= -1.; // Make negative Zt (vertical position) value
  CarmBaseFixedToTableBaseFixedOffset[2] *= -1.; // Make negative Zt (vertical position) value
  BaseFixedTranslateTransform->Translate(TableBaseFixedToFixedReferenceTranslate);
  BaseFixedTranslateTransform->Translate(CarmBaseFixedToTableBaseFixedOffset);

  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> baseFixedToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::TableRobotBaseFixed, CoordSys::Patient, 
    baseFixedToPatientTransform, false))
  {
    vtkWarningMacro("UpdateCarmRobotBaseFixedToFixedReferenceTransform: Can't get TableRobotBaseFixed->Patient transform");
  }

  double PatientToBaseFixedTranslate[3] = {};
  vtkNew<vtkTransform> patientToBaseFixedTransform;
  if (this->GetTransformBetween( CoordSys::Patient, CoordSys::TableRobotBaseFixed, 
    patientToBaseFixedTransform, false))
  {
    patientToBaseFixedTransform->GetPosition(PatientToBaseFixedTranslate);
  }

  vtkMRMLLinearTransformNode* baseFixedToFixedReferenceTransformNode =
    this->GetTransformNodeBetween(CoordSys::CarmRobotBaseFixed, CoordSys::FixedReference);
  if (baseFixedToFixedReferenceTransformNode)
  {
    double a[6] = {};
    parameterNode->GetTableRobotAngles(a);
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
    A6A5A4RotationTransform->RotateX(-1. * a[3]);

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
void vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateCarmRobotBaseRotationToCarmRobotBaseFixedTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateCarmRobotBaseRotationToCarmRobotBaseFixedTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateCarmRobotBaseRotationToCarmRobotBaseFixedTransform: Invalid parameter node");
    return;
  }

  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> patientToCarmRobotBaseFixedTransform;
  if (!this->GetTransformBetween( CoordSys::Patient, CoordSys::CarmRobotBaseFixed, 
    patientToCarmRobotBaseFixedTransform, false))
  {
    vtkWarningMacro("UpdateCarmRobotBaseRotationToCarmRobotBaseFixedTransform: Can't get Patient->CarmRobotBaseFixed transform");
  }

  vtkNew<vtkTransform> carmRobotBaseFixedToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::CarmRobotBaseFixed, CoordSys::Patient, 
    carmRobotBaseFixedToPatientTransform, false))
  {
    vtkWarningMacro("UpdateCarmRobotBaseRotationToCarmRobotBaseFixedTransform: Can't get CarmRobotBaseFixed->Patient transform");
  }

  vtkMRMLLinearTransformNode* carmRobotBaseRotationToCarmRobotBaseFixedTransformNode =
    this->GetTransformNodeBetween(CoordSys::CarmRobotBaseRotation, CoordSys::CarmRobotBaseFixed);
  if (carmRobotBaseRotationToCarmRobotBaseFixedTransformNode)
  {
    double a[6] = {};
    parameterNode->GetCarmRobotAngles(a);

    // BaseRotation->BaseFixed rotation around Y (A1 angle)
    vtkNew<vtkTransform> baseRotationToBaseFixedTransform;
    baseRotationToBaseFixedTransform->RotateY(a[0]);

    carmRobotBaseFixedToPatientTransform->Concatenate(baseRotationToBaseFixedTransform);
    carmRobotBaseFixedToPatientTransform->Concatenate(patientToCarmRobotBaseFixedTransform);
    carmRobotBaseRotationToCarmRobotBaseFixedTransformNode->SetAndObserveTransformToParent(carmRobotBaseFixedToPatientTransform);
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateCarmRobotShoulderToCarmRobotBaseRotationTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateCarmRobotShoulderToCarmRobotBaseRotationTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateCarmRobotShoulderToCarmRobotBaseRotationTransform: Invalid parameter node");
    return;
  }
  // Translate the C-Arm Shoulder to C-Arm BaseRotation empty disk centre
  vtkNew<vtkTransform> carmRobotShoulderToCarmRobotBaseRotationTranslate;
  using CoordPos = vtkSlicerChannel26Cabin3RobotsGeometryCommon;
  carmRobotShoulderToCarmRobotBaseRotationTranslate->Identity();
  carmRobotShoulderToCarmRobotBaseRotationTranslate->Translate(CoordPos::CARM_ROBOT_BASE_ROTATION_SHOULDER_OFFSET_X,
    CoordPos::CARM_ROBOT_BASE_ROTATION_SHOULDER_OFFSET_Y, 0.);

  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> patientToCarmRobotBaseRotationTransform;
  if (!this->GetTransformBetween( CoordSys::Patient, CoordSys::CarmRobotBaseRotation, 
    patientToCarmRobotBaseRotationTransform, false))
  {
    vtkWarningMacro("UpdateCarmRobotShoulderToCarmRobotBaseRotationTransform: Can't get Patient->CarmRobotBaseRotation transform");
  }
  vtkNew<vtkTransform> carmRobotBaseRotationToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::CarmRobotBaseRotation, CoordSys::Patient, 
    carmRobotBaseRotationToPatientTransform, false))
  {
    vtkWarningMacro("UpdateCarmRobotShoulderToCarmRobotBaseRotationTransform: Can't get CarmRobotBaseRotation->Patient transform");
  }

  vtkMRMLLinearTransformNode* carmRobotShoulderToCArmRobotBaseRotationTransformNode =
    this->GetTransformNodeBetween(CoordSys::CarmRobotShoulder, CoordSys::CarmRobotBaseRotation);
  if (carmRobotShoulderToCArmRobotBaseRotationTransformNode)
  {
    double a[6] = {};
    parameterNode->GetCarmRobotAngles(a);

    // Shoulder->BaseRotation rotation around Z (A2 angle)
    vtkNew<vtkTransform> shoulderToBaseRotationTransform;
    shoulderToBaseRotationTransform->RotateZ(a[1]);

    carmRobotBaseRotationToPatientTransform->Concatenate(carmRobotShoulderToCarmRobotBaseRotationTranslate);
    carmRobotBaseRotationToPatientTransform->Concatenate(shoulderToBaseRotationTransform);
    carmRobotBaseRotationToPatientTransform->Concatenate(patientToCarmRobotBaseRotationTransform);
    carmRobotShoulderToCArmRobotBaseRotationTransformNode->SetAndObserveTransformToParent(carmRobotBaseRotationToPatientTransform);
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateCarmRobotElbowShoulderToCarmRobotShoulderTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateCarmRobotElbowShoulderToCarmRobotShoulderTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateCarmRobotElbowShoulderToCarmRobotShoulderTransform: Invalid parameter node");
    return;
  }
  using CoordPos = vtkSlicerChannel26Cabin3RobotsGeometryCommon;
  // Translate the C-Arm Elbow to C-Arm Shoulder size up
  vtkNew<vtkTransform> carmElbowTranslateTransform;
  carmElbowTranslateTransform->Translate( 0., CoordPos::CARM_ROBOT_SHOULDER_SIZE, 0.);

  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> patientToCarmShoulderTransform;
  if (!this->GetTransformBetween( CoordSys::Patient, CoordSys::CarmRobotShoulder, 
    patientToCarmShoulderTransform, false))
  {
    vtkWarningMacro("UpdateCarmRobotElbowShoulderToCarmRobotShoulderTransform: Can't get Patient->CarmRobotShoulder transform");
  }
  vtkNew<vtkTransform> carmShoulderToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::CarmRobotShoulder, CoordSys::Patient, 
    carmShoulderToPatientTransform, false))
  {
    vtkWarningMacro("UpdateCarmRobotElbowShoulderToCarmRobotShoulderTransform: Can't get CarmRobotShoulder->Patient transform");
  }

  vtkMRMLLinearTransformNode* carmElbowToCArmShoulderTransformNode =
    this->GetTransformNodeBetween(CoordSys::CarmRobotElbowShoulder, CoordSys::CarmRobotShoulder);
  if (carmElbowToCArmShoulderTransformNode)
  {
    double a[6] = {};
    parameterNode->GetCarmRobotAngles(a);

    // Elbow->Shoulder rotation around Z (A3 angle)
    vtkNew<vtkTransform> elbowToShoulderTransform;
    // Elbow->Shoulder translation from point of rotation on Shoulder->Elbow offset (115 mm) along Y-axis
    elbowToShoulderTransform->Translate( 0., CoordPos::CARM_ROBOT_SHOULDER_ELBOW_OFFSET_Y, 0.);
    elbowToShoulderTransform->RotateZ(a[2]);

    carmShoulderToPatientTransform->Concatenate(carmElbowTranslateTransform);
    carmShoulderToPatientTransform->Concatenate(elbowToShoulderTransform);
    carmShoulderToPatientTransform->Concatenate(patientToCarmShoulderTransform);
    carmElbowToCArmShoulderTransformNode->SetAndObserveTransformToParent(carmShoulderToPatientTransform);
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateCarmRobotElbowWristToCarmRobotElbowShoulderTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateCarmRobotElbowWristToCarmRobotElbowShoulderTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateCarmRobotElbowWristToCarmRobotElbowShoulderTransform: Invalid parameter node");
    return;
  }
  using CoordPos = vtkSlicerChannel26Cabin3RobotsGeometryCommon;
  // Translate the C-Arm Elbow to C-Arm BaseRotation empty disk centre
  vtkNew<vtkTransform> carmElbowTranslateTransform;
  carmElbowTranslateTransform->Translate( 0., -1. * CoordPos::CARM_ROBOT_SHOULDER_ELBOW_OFFSET_Y, 0.);

  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> patientToCarmShoulderTransform;
  if (!this->GetTransformBetween( CoordSys::Patient, CoordSys::CarmRobotElbowShoulder, 
    patientToCarmShoulderTransform, false))
  {
    vtkWarningMacro("UpdateCarmRobotElbowWristToCarmRobotElbowShoulderTransform: Can't get Patient->CarmRobotElbowShoulder transform");
  }
  vtkNew<vtkTransform> carmShoulderToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::CarmRobotElbowShoulder, CoordSys::Patient, 
    carmShoulderToPatientTransform, false))
  {
    vtkWarningMacro("UpdateCarmRobotElbowWristToCarmRobotElbowShoulderTransform: Can't get CarmRobotElbowShoulder->Patient transform");
  }

  vtkMRMLLinearTransformNode* carmElbowToCArmShoulderTransformNode =
    this->GetTransformNodeBetween(CoordSys::CarmRobotElbowWrist, CoordSys::CarmRobotElbowShoulder);
  if (carmElbowToCArmShoulderTransformNode)
  {
    double a[6] = {};
    parameterNode->GetCarmRobotAngles(a);

    // Wrist->Elbow rotation around X (A4 angle)
    vtkNew<vtkTransform> a4Transform;
    a4Transform->RotateX(-1. * a[3]);

    // Elbow->Shoulder translation from point of rotation on Shoulder->Elbow offset (115 mm) along Y-axis
    vtkNew<vtkTransform> elbowToShoulderTransform;
    elbowToShoulderTransform->Translate( 0., CoordPos::CARM_ROBOT_SHOULDER_ELBOW_OFFSET_Y, 0.);

    a4Transform->Concatenate(elbowToShoulderTransform);
    carmShoulderToPatientTransform->Concatenate(carmElbowTranslateTransform);
    carmShoulderToPatientTransform->Concatenate(a4Transform);
    carmShoulderToPatientTransform->Concatenate(patientToCarmShoulderTransform);
    carmElbowToCArmShoulderTransformNode->SetAndObserveTransformToParent(carmShoulderToPatientTransform);
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateCarmRobotWristToCarmRobotElbowWristTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateCarmRobotWristToCarmRobotElbowWristTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateCarmRobotWristToCarmRobotElbowWristTransform: Invalid parameter node");
    return;
  }
  // Translate the C-arm wrist from C-arm elbow origin along X-axis
  using CoordPos = vtkSlicerChannel26Cabin3RobotsGeometryCommon;
  vtkNew<vtkTransform> carmWristTranslateTransform;
  carmWristTranslateTransform->Translate(CoordPos::CARM_ROBOT_ELBOW_SIZE, 0., 0.);

  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> patientToCarmElbowTransform;
  if (!this->GetTransformBetween( CoordSys::Patient, CoordSys::CarmRobotElbowWrist, 
    patientToCarmElbowTransform, false))
  {
    vtkWarningMacro("UpdateCarmRobotWristToCarmRobotElbowWristTransform: Can't get Patient->CarmRobotElbowWrist transform");
  }
  vtkNew<vtkTransform> carmElbowToPatientTransform;
  if (!this->GetTransformBetween( CoordSys::CarmRobotElbowWrist, CoordSys::Patient, 
    carmElbowToPatientTransform, false))
  {
    vtkWarningMacro("UpdateCarmRobotWristToCarmRobotElbowWristTransform: Can't get CarmRobotElbowWrist->Patient transform");
  }

  vtkMRMLLinearTransformNode* carmWristToCArmElbowTransformNode =
    this->GetTransformNodeBetween(CoordSys::CarmRobotWrist, CoordSys::CarmRobotElbowWrist);
  if (carmWristToCArmElbowTransformNode)
  {
    double a[6] = {};
    parameterNode->GetCarmRobotAngles(a);

    // Wrist->Elbow rotation around Z (A5 angle)
    vtkNew<vtkTransform> wristToElbowTransform;
    wristToElbowTransform->RotateZ(a[4]);

    carmElbowToPatientTransform->Concatenate(carmWristTranslateTransform);
    carmElbowToPatientTransform->Concatenate(wristToElbowTransform);
    carmElbowToPatientTransform->Concatenate(patientToCarmElbowTransform);
    carmWristToCArmElbowTransformNode->SetAndObserveTransformToParent(carmElbowToPatientTransform);
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateCarmRobotFlangeToCarmRobotWristTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateCarmRobotFlangeToCarmRobotWristTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateCarmRobotFlangeToCarmRobotWristTransform: Invalid parameter node");
    return;
  }

  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> patientToCarmWristTransform;
  if (!this->GetTransformBetween(CoordSys::Patient, CoordSys::CarmRobotWrist, 
    patientToCarmWristTransform, false))
  {
    vtkWarningMacro("UpdateCarmRobotFlangeToCarmRobotWristTransform: Can't get Patient->CarmRobotWrist transform");
  }
  vtkNew<vtkTransform> carmWristToPatientTransform;
  if (!this->GetTransformBetween(CoordSys::CarmRobotWrist, CoordSys::Patient, 
    carmWristToPatientTransform, false))
  {
    vtkWarningMacro("UpdateCarmRobotFlangeToCarmRobotWristTransform: Can't get CarmRobotWrist->Patient transform");
  }

  vtkMRMLLinearTransformNode* carmFlangeToCarmWristTransformNode =
    this->GetTransformNodeBetween(CoordSys::CarmRobotFlange, CoordSys::CarmRobotWrist);
  if (carmFlangeToCarmWristTransformNode)
  {
    double a[6] = {};
    parameterNode->GetCarmRobotAngles(a);

    // Flange->Wrist rotation around X (A6 angle)
    vtkNew<vtkTransform> flangeToWristTransform;
    flangeToWristTransform->RotateX(a[5]);

    carmWristToPatientTransform->Concatenate(flangeToWristTransform);
    carmWristToPatientTransform->Concatenate(patientToCarmWristTransform);
    carmFlangeToCarmWristTransformNode->SetAndObserveTransformToParent(carmWristToPatientTransform);
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateCarmToCarmRobotFlangeTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateCarmToCarmRobotFlangeTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateCarmToCarmRobotFlangeTransform: Invalid parameter node");
    return;
  }

  // Translate the C-arm from C-arm wrist origin along X-axis
  using CoordPos = vtkSlicerChannel26Cabin3RobotsGeometryCommon;
  vtkNew<vtkTransform> carmToCarmFlangeTranslate;
  carmToCarmFlangeTranslate->Translate(CoordPos::CARM_ROBOT_WRIST_SIZE, 0., 0.);

  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> patientToCarmFlangeTransform;
  if (!this->GetTransformBetween(CoordSys::Patient, CoordSys::CarmRobotFlange, 
    patientToCarmFlangeTransform, false))
  {
    vtkWarningMacro("UpdateCarmToCarmRobotFlangeTransform: Can't get Patient->CarmRobotFlange transform");
  }
  vtkNew<vtkTransform> carmFlangeToPatientTransform;
  if (!this->GetTransformBetween(CoordSys::CarmRobotFlange, CoordSys::Patient, 
    carmFlangeToPatientTransform, false))
  {
    vtkWarningMacro("UpdateCarmToCarmRobotFlangeTransform: Can't get CarmRobotFlange->Patient transform");
  }

  vtkMRMLLinearTransformNode* carmToCarmFlangeTransformNode =
    this->GetTransformNodeBetween(CoordSys::Carm, CoordSys::CarmRobotFlange);
  if (carmToCarmFlangeTransformNode)
  {
    carmFlangeToPatientTransform->Concatenate(carmToCarmFlangeTranslate);
    carmFlangeToPatientTransform->Concatenate(patientToCarmFlangeTransform);
    carmToCarmFlangeTransformNode->SetAndObserveTransformToParent(carmFlangeToPatientTransform);
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateCarmXrayBeamToCarmTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateCarmXrayBeamToCarmTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateCarmXrayBeamToCarmTransform: Invalid parameter node");
    return;
  }

  // Translate the C-arm X-ray beam mount position to from C-arm origin along X-axis and Y-axis
  using CoordPos = vtkSlicerChannel26Cabin3RobotsGeometryCommon;
  vtkNew<vtkTransform> carmXrayBeamToCarmTranslate;
  carmXrayBeamToCarmTranslate->Translate(CoordPos::CARM_XRAY_ORIGIN_OFFSET_X,
    CoordPos::CARM_XRAY_ORIGIN_OFFSET_Z, CoordPos::CARM_XRAY_ORIGIN_OFFSET_Y);

  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> patientToCarmTransform;
  if (!this->GetTransformBetween(CoordSys::Patient, CoordSys::Carm, patientToCarmTransform, false))
  {
    vtkWarningMacro("UpdateCarmXrayBeamToCarmTransform: Can't get Patient->Carm transform");
  }
  vtkNew<vtkTransform> carmToPatientTransform;
  if (!this->GetTransformBetween(CoordSys::Carm, CoordSys::Patient, carmToPatientTransform, false))
  {
    vtkWarningMacro("UpdateCarmXrayBeamToCarmTransform: Can't get Carm->Patient transform");
  }

  vtkMRMLLinearTransformNode* carmXrayBeamToCarmTransformNode =
    this->GetTransformNodeBetween(CoordSys::CarmXrayBeam, CoordSys::Carm);
  if (carmXrayBeamToCarmTransformNode)
  {
    carmToPatientTransform->Concatenate(carmXrayBeamToCarmTranslate);
    carmToPatientTransform->Concatenate(patientToCarmTransform);
    carmXrayBeamToCarmTransformNode->SetAndObserveTransformToParent(carmToPatientTransform);
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateCarmXrayDetectorToCarmTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateCarmXrayDetectorToCarmTransform: Invalid scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateCarmXrayDetectorToCarmTransform: Invalid parameter node");
    return;
  }

  // Translate the C-arm X-ray beam mount position to from C-arm origin along X-axis and Y-axis
  using CoordPos = vtkSlicerChannel26Cabin3RobotsGeometryCommon;
  vtkNew<vtkTransform> carmXrayDetectorToCarmTranslate;
  carmXrayDetectorToCarmTranslate->Translate(CoordPos::CARM_DETECTOR_ORIGIN_OFFSET_Z,
    -1. * CoordPos::CARM_DETECTOR_ORIGIN_OFFSET_X, -1. * CoordPos::CARM_DETECTOR_ORIGIN_OFFSET_Y);

  using CoordSys = CoordinateSystemIdentifier;
  vtkNew<vtkTransform> patientToCarmTransform;
  if (!this->GetTransformBetween(CoordSys::Patient, CoordSys::Carm, patientToCarmTransform, false))
  {
    vtkWarningMacro("UpdateCarmXrayDetectorToCarmTransform: Can't get Patient->Carm transform");
  }
  vtkNew<vtkTransform> carmToPatientTransform;
  if (!this->GetTransformBetween(CoordSys::Carm, CoordSys::Patient, carmToPatientTransform, false))
  {
    vtkWarningMacro("UpdateCarmXrayDetectorToCarmTransform: Can't get Carm->Patient transform");
  }

  vtkMRMLLinearTransformNode* carmXrayDetectorToCarmTransformNode =
    this->GetTransformNodeBetween(CoordSys::CarmXrayDetector, CoordSys::Carm);
  if (carmXrayDetectorToCarmTransformNode)
  {
    carmToPatientTransform->Concatenate(carmXrayDetectorToCarmTranslate);
    carmToPatientTransform->Concatenate(patientToCarmTransform);
    carmXrayDetectorToCarmTransformNode->SetAndObserveTransformToParent(carmToPatientTransform);
  }
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateFrameToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode,
  CoordinateSystemIdentifier frame)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateFrameToRasTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateFrameToRasTransform: Invalid MRML scene");
    return nullptr;
  }

  // Display all pieces of the treatment room and sets each piece a color to provide realistic representation
  using CoordSys = CoordinateSystemIdentifier;

  // Transform robots, table, C-arm models to RAS, according to patient orientation
  // on the table top
  vtkNew<vtkTransform> iecPatientToRasTransform;
  iecPatientToRasTransform->RotateX(-90.);
  if (parameterNode->GetPatientHeadFeetRotation())
  {
    iecPatientToRasTransform->RotateZ(180.);
  }

  // Frame -> RAS
  // Frame - mandatory
  // Find FrameToRasTransform or create it
  const char* frameName = this->GetTreatmentMachinePartTypeAsString(frame);
  if (!frameName)
  {
    vtkErrorMacro("UpdateFrameToRasTransform: Frame name is invalid");
    return nullptr;
  }

  std::string rasName = this->CoordinateSystemsMap[CoordinateSystemIdentifier::RAS];
  std::string frameToRasTransformName = std::string(frameName) + "To" + rasName + "Transform";

  vtkSmartPointer<vtkMRMLLinearTransformNode> frameToRasTransformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName(frameToRasTransformName.c_str()))
  {
    frameToRasTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }
  else
  {
    frameToRasTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    frameToRasTransformNode->SetName(frameToRasTransformName.c_str());
//    frameToRasTransformNode->SetHideFromEditors(1);
    std::string singletonTag = std::string("C26C3_") + frameToRasTransformName;
//    frameToRasTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(frameToRasTransformNode);
  }

  vtkNew<vtkTransform> frameToRasTransform;
  if (this->GetTransformBetween(CoordSys::RAS, frame, frameToRasTransform, false))
  {
    vtkDebugMacro("UpdateFrameToRasTransform: " << frameName << "->RAS transform updated");
    // Transform to RAS, set transform to node, transform the model
    frameToRasTransform->Concatenate(iecPatientToRasTransform);
  }
  if (frameToRasTransformNode)
  {
    frameToRasTransformNode->SetAndObserveTransformToParent(frameToRasTransform);
  }
  return frameToRasTransformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetFrameToRasTransform(CoordinateSystemIdentifier frame)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("GetFrameToRasTransform: Invalid MRML scene");
    return nullptr;
  }

  const char* frameName = this->GetTreatmentMachinePartTypeAsString(frame);
  if (!frameName)
  {
    vtkErrorMacro("GetFrameToRasTransform: Frame name is invalid");
    return nullptr;
  }

  std::string rasName = this->CoordinateSystemsMap[CoordinateSystemIdentifier::RAS];
  std::string frameToRasTransformName = std::string(frameName) + "To" + rasName + "Transform";

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName(frameToRasTransformName.c_str()))
  {
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }

  return transformNode;
}

//-----------------------------------------------------------------------------
bool vtkSlicerChannel26Cabin3RobotsTransformLogic::GetTransformBetween(
  CoordinateSystemIdentifier fromFrame, CoordinateSystemIdentifier toFrame, 
  vtkTransform* outputLinearTransform, bool transformForBeam)
{
  vtkNew<vtkGeneralTransform> inputGeneralTransform;
  if (!this->GetTransformBetween(fromFrame, toFrame, inputGeneralTransform, transformForBeam))
  {
    return false;
  }

  // Convert general transform to linear
  // This call also makes hard copy of the transform so that it doesn't change when input transform changed
  if (!vtkMRMLTransformNode::IsGeneralTransformLinear(inputGeneralTransform, outputLinearTransform))
  {
    vtkErrorMacro("GetTransformBetween: Can't transform general transform to linear! General trasform is not linear.");
    return false;
  }
  return true;
}

//-----------------------------------------------------------------------------
bool vtkSlicerChannel26Cabin3RobotsTransformLogic::GetTransformForPointBetweenFrames( 
  CoordinateSystemIdentifier fromFrame, CoordinateSystemIdentifier toFrame,
  const double fromFramePoint[3], double toFramePoint[3], bool transformForBeam)
{
  // RAS == World
  // fromFrame->RAS transform
  vtkNew<vtkTransform> fromFrameToRasTransform;
  if (!this->GetTransformBetween(CoordinateSystemIdentifier::RAS,
    fromFrame, fromFrameToRasTransform, transformForBeam))
  {
    return false;
  }
  double pointInRas[3] = {};
  fromFrameToRasTransform->TransformPoint(fromFramePoint, pointInRas);

  // toFrame->RAS transform
  vtkNew<vtkTransform> rasToToFrameTransform;
  if (this->GetTransformBetween(toFrame,
    CoordinateSystemIdentifier::RAS, rasToToFrameTransform, transformForBeam))
  {
//    rasToToFrameTransform->Inverse(); // inverse to get (RAS->toFrame)
  }
  else
  {
    return false;
  }

  // Get transform fromFrame -> toFrame
  // fromFrame -> RAS -> RAS -> toFrame
  fromFrameToRasTransform->Concatenate(rasToToFrameTransform);
  fromFrameToRasTransform->TransformPoint(fromFramePoint, toFramePoint);
///  rasToToFrameTransform->TransformPoint(pointInRas, toFramePoint);
  return true;
}

//-----------------------------------------------------------------------------
void vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateFrameToRasHierarchy(vtkMRMLChannel26GeometryNode* parameterNode,
  CoordinateSystemIdentifier type)
{
  switch (type)
  {
  case CoordinateSystemIdentifier::Patient:
  case CoordinateSystemIdentifier::TableTop:
    this->UpdateTableTopToRasTransform(parameterNode);
  case CoordinateSystemIdentifier::TableTopPlaneCorrection:
    this->UpdateTableTopPlaneCorrectionToRasTransform(parameterNode);
  case CoordinateSystemIdentifier::TableFlange:
    this->UpdateTableFlangeToRasTransform(parameterNode);
  case CoordinateSystemIdentifier::TableRobotFlange:
    this->UpdateTableRobotFlangeToRasTransform(parameterNode);
  case CoordinateSystemIdentifier::TableRobotWrist:
    this->UpdateTableRobotWristToRasTransform(parameterNode);
  case CoordinateSystemIdentifier::TableRobotElbowWrist:
    this->UpdateTableRobotElbowWristToRasTransform(parameterNode);
  case CoordinateSystemIdentifier::TableRobotElbowShoulder:
    this->UpdateTableRobotElbowShoulderToRasTransform(parameterNode);
  case CoordinateSystemIdentifier::TableRobotShoulder:
    this->UpdateTableRobotShoulderToRasTransform(parameterNode);
  case CoordinateSystemIdentifier::TableRobotBaseRotation:
  case CoordinateSystemIdentifier::TableRobotBaseRotationDisk:
    this->UpdateTableRobotBaseRotationToRasTransform(parameterNode);
    this->UpdateTableRobotBaseRotationDiskToRasTransform(parameterNode);
  case CoordinateSystemIdentifier::TableRobotBaseFixed:
    this->UpdateTableRobotBaseFixedToRasTransform(parameterNode);
  case CoordinateSystemIdentifier::FixedReference:
    this->UpdateFixedReferenceToRasTransform(parameterNode);
  case CoordinateSystemIdentifier::CarmRobotBaseFixed:
    this->UpdateCarmRobotBaseFixedToRasTransform(parameterNode);
  case CoordinateSystemIdentifier::CarmRobotBaseRotation:
    this->UpdateCarmRobotBaseRotationToRasTransform(parameterNode);
  case CoordinateSystemIdentifier::CarmRobotShoulder:
    this->UpdateCarmRobotShoulderToRasTransform(parameterNode);
  case CoordinateSystemIdentifier::CarmRobotElbowShoulder:
    this->UpdateCarmRobotElbowShoulderToRasTransform(parameterNode);
  case CoordinateSystemIdentifier::CarmRobotElbowWrist:
    this->UpdateCarmRobotElbowWristToRasTransform(parameterNode);
  case CoordinateSystemIdentifier::CarmRobotWrist:
    this->UpdateCarmRobotWristToRasTransform(parameterNode);
  case CoordinateSystemIdentifier::CarmRobotFlange:
    this->UpdateCarmRobotFlangeToRasTransform(parameterNode);
  case CoordinateSystemIdentifier::Carm:
    this->UpdateCarmToRasTransform(parameterNode);
  case CoordinateSystemIdentifier::CarmXrayBeam:
    this->UpdateCarmXrayBeamToRasTransform(parameterNode);
  case CoordinateSystemIdentifier::CarmXrayDetector:
    this->UpdateCarmXrayDetectorToRasTransform(parameterNode);
  default:
    break;
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateTransformsHierarchy(vtkMRMLChannel26GeometryNode* parameterNode,
  CoordinateSystemIdentifier type)
{
  switch (type)
  {
  case CoordinateSystemIdentifier::Patient:
    this->UpdatePatientToTableTopTransform(parameterNode);
  case CoordinateSystemIdentifier::TableTop:
    this->UpdateTableTopToTableFlangeTransform(parameterNode);
  case CoordinateSystemIdentifier::TableTopPlaneCorrection:
    this->UpdateTableTopPlaneCorrectionToTableTopTransform(parameterNode);
  case CoordinateSystemIdentifier::TableFlange:
    this->UpdateTableFlangeToTableRobotFlangeTransform(parameterNode);
  case CoordinateSystemIdentifier::TableRobotFlange:
    this->UpdateTableRobotFlangeToTableRobotWristTransform(parameterNode);
  case CoordinateSystemIdentifier::TableRobotWrist:
    this->UpdateTableRobotWristToTableRobotElbowWristTransform(parameterNode);
  case CoordinateSystemIdentifier::TableRobotElbowWrist:
    this->UpdateTableRobotElbowWristToTableRobotElbowShoulderTransform(parameterNode);
  case CoordinateSystemIdentifier::TableRobotElbowShoulder:
    this->UpdateTableRobotElbowShoulderToTableRobotShoulderTransform(parameterNode);
  case CoordinateSystemIdentifier::TableRobotShoulder:
    this->UpdateTableRobotShoulderToTableRobotBaseRotationTransform(parameterNode);
  case CoordinateSystemIdentifier::TableRobotBaseRotation:
    this->UpdateTableRobotBaseRotationToTableRobotBaseFixedTransform(parameterNode);
    this->UpdateTableRobotBaseRotationDiskToTableRobotBaseFixedTransform(parameterNode);
  case CoordinateSystemIdentifier::TableRobotBaseFixed:
  case CoordinateSystemIdentifier::FixedReference:
    this->UpdateTableRobotBaseFixedToFixedReferenceTransform(parameterNode);
  case CoordinateSystemIdentifier::CarmRobotBaseFixed:
    this->UpdateCarmRobotBaseFixedToFixedReferenceTransform(parameterNode);
  case CoordinateSystemIdentifier::CarmRobotBaseRotation:
    this->UpdateCarmRobotBaseRotationToCarmRobotBaseFixedTransform(parameterNode);
  case CoordinateSystemIdentifier::CarmRobotShoulder:
    this->UpdateCarmRobotShoulderToCarmRobotBaseRotationTransform(parameterNode);
  case CoordinateSystemIdentifier::CarmRobotElbowShoulder:
    this->UpdateCarmRobotElbowShoulderToCarmRobotShoulderTransform(parameterNode);
  case CoordinateSystemIdentifier::CarmRobotElbowWrist:
    this->UpdateCarmRobotElbowWristToCarmRobotElbowShoulderTransform(parameterNode);
  case CoordinateSystemIdentifier::CarmRobotWrist:
    this->UpdateCarmRobotWristToCarmRobotElbowWristTransform(parameterNode);
  case CoordinateSystemIdentifier::CarmRobotFlange:
    this->UpdateCarmRobotFlangeToCarmRobotWristTransform(parameterNode);
  case CoordinateSystemIdentifier::Carm:
    this->UpdateCarmToCarmRobotFlangeTransform(parameterNode);
  case CoordinateSystemIdentifier::CarmXrayBeam:
    this->UpdateCarmXrayBeamToCarmTransform(parameterNode);
  case CoordinateSystemIdentifier::CarmXrayDetector:
    this->UpdateCarmXrayDetectorToCarmTransform(parameterNode);
  default:
    break;
  }
}

//-----------------------------------------------------------------------------
bool vtkSlicerChannel26Cabin3RobotsTransformLogic::GetPathToRoot(CoordinateSystemIdentifier frame, 
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
bool vtkSlicerChannel26Cabin3RobotsTransformLogic::GetPathFromRoot(CoordinateSystemIdentifier frame, 
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
