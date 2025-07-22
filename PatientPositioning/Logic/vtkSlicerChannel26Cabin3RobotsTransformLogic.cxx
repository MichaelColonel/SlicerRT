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

//constexpr std::array< double, 3 > FixedReferenceToFixedBasedOffset{ -1600., -1500., 1400. };

constexpr double BaseFixedHeight = 240.; // mm
constexpr double CArmBaseFixedHeight = 225.6;//240.; // mm
constexpr double BaseRotationHeight = 675. - BaseFixedHeight; // mm
constexpr double CArmBaseRotationHeight = 645. - CArmBaseFixedHeight; // mm
constexpr double BaseRotationShoulderDiskCenterOffsetX = 350; // mm
constexpr double BaseRotationShoulderDiskCenterOffsetY = BaseRotationHeight; // mm
constexpr double CArmBaseRotationShoulderDiskCenterOffsetX = 330; // mm
constexpr double CArmBaseRotationShoulderDiskCenterOffsetY = CArmBaseRotationHeight; // mm

constexpr double TableThickness = 91.; // mm
constexpr double FlangeLength = 300.; // mm
constexpr double FlangeRobotLength = 39.93; // mm
constexpr double WristLength = 240. - FlangeRobotLength; // mm
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
  InitialFlangeOriginOffsetRAS[2] - (FlangeLength + FlangeRobotLength + WristLength) };
constexpr std::array< double, 3 > InitialSholderElbowJointOffsetRAS{
  InitialElbowWristJointOffsetRAS[0] - ElbowLength,
  InitialElbowWristJointOffsetRAS[1], 
  InitialElbowWristJointOffsetRAS[2] };

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
  this->CoordinateSystemsMap[CoordSys::TableRobotBaseRotation] = "TableRobotBaseRotation";
  this->CoordinateSystemsMap[CoordSys::TableRobotShoulder] = "TableRobotShoulder";
  this->CoordinateSystemsMap[CoordSys::TableRobotElbowShoulder] = "TableRobotElbowShoulder";
  this->CoordinateSystemsMap[CoordSys::TableRobotElbowWrist] = "TableRobotElbowWrist";
  this->CoordinateSystemsMap[CoordSys::TableRobotWrist] = "TableRobotWrist";
  this->CoordinateSystemsMap[CoordSys::TableRobotFlange] = "TableRobotFlange";
  this->CoordinateSystemsMap[CoordSys::TableFlange] = "TableFlange";
  this->CoordinateSystemsMap[CoordSys::TableTop] = "TableTop";
  this->CoordinateSystemsMap[CoordSys::Patient] = "Patient";

  this->RobotsTransforms.clear();
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::FixedReference, CoordSys::RAS)); // Dummy, unity, identity
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::TableRobotBaseFixed, CoordSys::FixedReference)); // Translate
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::TableRobotBaseRotation, CoordSys::TableRobotBaseFixed)); // Rotation A1
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::TableRobotShoulder, CoordSys::TableRobotBaseRotation)); // Rotation A2
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::TableRobotElbowShoulder, CoordSys::TableRobotShoulder)); // Rotation A3
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::TableRobotElbowWrist, CoordSys::TableRobotElbowShoulder)); // Rotation A4
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::TableRobotWrist, CoordSys::TableRobotElbowWrist)); // Rotation A5
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::TableRobotFlange, CoordSys::TableRobotWrist)); // Rotation A6
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::TableFlange, CoordSys::TableRobotFlange)); // Dummy, only fixed translation
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::TableTop, CoordSys::TableFlange)); // Dummy, only fixed translation
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::Patient, CoordSys::TableTop)); // Translate from oatient to table top center
  this->RobotsTransforms.push_back(std::make_pair(CoordSys::RAS, CoordSys::Patient));

  this->CoordinateSystemsHierarchy.clear();
  // key - parent, value - children
  this->CoordinateSystemsHierarchy[CoordSys::FixedReference] = { CoordSys::TableRobotBaseFixed };
  this->CoordinateSystemsHierarchy[CoordSys::TableRobotBaseFixed] = { CoordSys::TableRobotBaseRotation };
  this->CoordinateSystemsHierarchy[CoordSys::TableRobotBaseRotation] = { CoordSys::TableRobotShoulder };
  this->CoordinateSystemsHierarchy[CoordSys::TableRobotShoulder] = { CoordSys::TableRobotElbowShoulder };
  this->CoordinateSystemsHierarchy[CoordSys::TableRobotElbowShoulder] = { CoordSys::TableRobotElbowWrist };
  this->CoordinateSystemsHierarchy[CoordSys::TableRobotElbowWrist] = { CoordSys::TableRobotWrist };
  this->CoordinateSystemsHierarchy[CoordSys::TableRobotWrist] = { CoordSys::TableRobotFlange };
  this->CoordinateSystemsHierarchy[CoordSys::TableRobotFlange] = { CoordSys::TableFlange };
  this->CoordinateSystemsHierarchy[CoordSys::TableFlange] = { CoordSys::TableTop };
  this->CoordinateSystemsHierarchy[CoordSys::TableTop] = { CoordSys::Patient };
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
  case TableRobotBaseRotation:
    partAsString = "TableRobotBaseRotation";
    break;
  case TableRobotShoulder:
    partAsString = "TableRobotShoulder";
    break;
  case TableRobotElbowShoulder:
    partAsString = "TableRobotElbowShoulder";
    break;
  case TableRobotElbowWrist:
    partAsString = "TableRobotElbowWrist";
    break;
  case TableRobotWrist:
    partAsString = "TableRobotWrist";
    break;
  case TableRobotFlange:
    partAsString = "TableRobotFlange";
    break;
  case TableFlange:
    partAsString = "TableFlange";
    break;
  case TableTop:
    partAsString = "TableTop";
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

  // BaseFixed parent, rotation of base part of the robot along Z-axis
  this->GetTransformNodeBetween(CoordSys::TableRobotBaseRotation, CoordSys::TableRobotBaseFixed)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::TableRobotBaseFixed, CoordSys::FixedReference)->GetID() );

  // BaseRotation parent, rotation of shoulder part of the robot along Y-axis
  this->GetTransformNodeBetween(CoordSys::TableRobotShoulder, CoordSys::TableRobotBaseRotation)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::TableRobotBaseRotation, CoordSys::TableRobotBaseFixed)->GetID() );

  // Shoulder parent, rotation of elbow part of the robot along Y-axis
  this->GetTransformNodeBetween(CoordSys::TableRobotElbowShoulder, CoordSys::TableRobotShoulder)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::TableRobotShoulder, CoordSys::TableRobotBaseRotation)->GetID() );

  // Elbow Shoulder parent, rotation of elbow part of the robot along Y-axis
  this->GetTransformNodeBetween(CoordSys::TableRobotElbowWrist, CoordSys::TableRobotElbowShoulder)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::TableRobotElbowShoulder, CoordSys::TableRobotShoulder)->GetID() );

  // Elbow parent, rotation of wrist part of the robot along Y-axis
  this->GetTransformNodeBetween(CoordSys::TableRobotWrist, CoordSys::TableRobotElbowWrist)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::TableRobotElbowWrist, CoordSys::TableRobotElbowShoulder)->GetID() );

  // Wrist parent, translation of flange center from wrist center
  this->GetTransformNodeBetween(CoordSys::TableRobotFlange, CoordSys::TableRobotWrist)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::TableRobotWrist, CoordSys::TableRobotElbowWrist)->GetID() );

  // Wrist parent, translation of flange center from wrist center
  this->GetTransformNodeBetween(CoordSys::TableFlange, CoordSys::TableRobotFlange)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::TableRobotFlange, CoordSys::TableRobotWrist)->GetID() );

  // Flange parent, translation of table top center flange center
  this->GetTransformNodeBetween(CoordSys::TableTop, CoordSys::TableFlange)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(CoordSys::TableFlange, CoordSys::TableRobotFlange)->GetID() );

  // TableTop parent, translation of patient from wrist flange center
  this->GetTransformNodeBetween( CoordSys::Patient, CoordSys::TableTop)->SetAndObserveTransformNodeID(
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
  vtkMRMLLinearTransformNode* rasToPatientReferenceTransformNode =
    this->GetTransformNodeBetween( CoordSys::RAS, CoordSys::Patient);
  vtkTransform* rasToPatientReferenceTransform = vtkTransform::SafeDownCast(rasToPatientReferenceTransformNode->GetTransformToParent());
  rasToPatientReferenceTransform->Identity();
  rasToPatientReferenceTransform->RotateX(-90.);
  rasToPatientReferenceTransform->RotateY(180.);
  rasToPatientReferenceTransform->Modified();

  vtkMRMLLinearTransformNode* patientToTableTopTransformNode =
    this->GetTransformNodeBetween(CoordSys::Patient, CoordSys::TableTop);
  vtkTransform* patientToTableTopTransform =
    vtkTransform::SafeDownCast(patientToTableTopTransformNode->GetTransformToParent());
  patientToTableTopTransform->Identity();
  patientToTableTopTransform->Modified();

  vtkMRMLLinearTransformNode* tableTopToTableFlangeTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableTop, CoordSys::TableFlange);
  vtkTransform* tableTopToTableFlangeTransform =
    vtkTransform::SafeDownCast(tableTopToTableFlangeTransformNode->GetTransformToParent());
  tableTopToTableFlangeTransform->Identity();
  tableTopToTableFlangeTransform->Modified();

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

  vtkMRMLLinearTransformNode* tableRobotWristToTableRobotElobowWristTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableRobotWrist, CoordSys::TableRobotElbowWrist);
  vtkTransform* tableRobotWristToTableRobotElobowWristTransform =
    vtkTransform::SafeDownCast(tableRobotWristToTableRobotElobowWristTransformNode->GetTransformToParent());
  tableRobotWristToTableRobotElobowWristTransform->Identity();
  tableRobotWristToTableRobotElobowWristTransform->Modified();

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

  vtkMRMLLinearTransformNode* tableRobotBaseFixedToFixedReferenceTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableRobotBaseFixed, CoordSys::FixedReference);
  vtkTransform* tableRobotBaseFixedToFixedReferenceTransform =
    vtkTransform::SafeDownCast(tableRobotBaseFixedToFixedReferenceTransformNode->GetTransformToParent());
  tableRobotBaseFixedToFixedReferenceTransform->Identity();
  tableRobotBaseFixedToFixedReferenceTransform->Modified();
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
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetPatientTransform()
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
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetTableTopTransform()
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
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetTableRobotWristTransform()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("GetTableRobotWristTransform: Invalid MRML scene");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToTableRobotWristTransform"))
  {
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }

  return transformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetTableRobotElbowWristTransform()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("GetTableRobotElbowWristTransform: Invalid MRML scene");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToTableRobotElbowWristTransform"))
  {
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }

  return transformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetTableRobotElbowShoulderTransform()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("GetTableRobotElbowShoulderTransform: Invalid MRML scene");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToTableRobotElbowShoulderTransform"))
  {
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }

  return transformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetTableRobotShoulderTransform()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("GetTableRobotShoulderTransform: Invalid MRML scene");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToTableRobotShoulderTransform"))
  {
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }

  return transformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetTableFlangeTransform()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("GetTableFlangeTransform: Invalid MRML scene");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToTableFlangeTransform"))
  {
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }

  return transformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::GetTableRobotFlangeTransform()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("GetTableRobotFlangeTransform: Invalid MRML scene");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToTableRobotFlangeTransform"))
  {
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }

  return transformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateRasToTableTopTransform(vtkMRMLChannel26GeometryNode* parameterNode)
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
    std::string singletonTag = std::string("C26C3_") + "RasToTableTopTransform";
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
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateRasToTableFlangeTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateRasToTableFlangeTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateRasToTableFlangeTransform: Invalid MRML scene");
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

  // TableFlange -> RAS
  // TableFlange - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableFlange
  // Find RasToTableFlangeTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableFlangeTransformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToTableFlangeTransform"))
  {
    rasToTableFlangeTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }
  else
  {
    rasToTableFlangeTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    rasToTableFlangeTransformNode->SetName("RasToTableFlangeTransform");
//    rasToTableFlangeTransformNode->SetHideFromEditors(1);
    std::string singletonTag = std::string("C26C3_") + "RasToTableFlangeTransform";
//    rasToTableFlangeTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(rasToTableFlangeTransformNode);
  }

  vtkNew<vtkTransform> rasToTableFlangeTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::TableFlange, 
    rasToTableFlangeTransform, false))
  {
    vtkDebugMacro("UpdateRasToFlangeTransform: RAS->TableFlange transform updated");
    // Transform to RAS, set transform to node, transform the model
    rasToTableFlangeTransform->Concatenate(patientToRasTransform);
  }
  if (rasToTableFlangeTransformNode)
  {
    rasToTableFlangeTransformNode->SetAndObserveTransformToParent(rasToTableFlangeTransform);
  }
  return rasToTableFlangeTransformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateRasToTableRobotFlangeTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateRasToTableRobotFlangeTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateRasToTableRobotFlangeTransform: Invalid MRML scene");
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

  // TableRobotFlange -> RAS
  // TableRobotFlange - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableFlange -> TableRobotFlange
  // Find RasToTableRobotFlangeTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableRobotFlangeTransformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToTableRobotFlangeTransform"))
  {
    rasToTableRobotFlangeTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }
  else
  {
    rasToTableRobotFlangeTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    rasToTableRobotFlangeTransformNode->SetName("RasToTableRobotFlangeTransform");
//    rasToTableRobotFlangeTransformNode->SetHideFromEditors(1);
    std::string singletonTag = std::string("C26C3_") + "RasToTableRobotFlangeTransform";
//    rasToTableRobotFlangeTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(rasToTableRobotFlangeTransformNode);
  }

  vtkNew<vtkTransform> rasToTableRobotFlangeTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::TableRobotFlange, 
    rasToTableRobotFlangeTransform, false))
  {
    vtkDebugMacro("UpdateRasToTableRobotFlangeTransform: RAS->TableRobotFlange transform updated");
    // Transform to RAS, set transform to node, transform the model
    rasToTableRobotFlangeTransform->Concatenate(patientToRasTransform);
  }
  if (rasToTableRobotFlangeTransformNode)
  {
    rasToTableRobotFlangeTransformNode->SetAndObserveTransformToParent(rasToTableRobotFlangeTransform);
  }
  return rasToTableRobotFlangeTransformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateRasToTableRobotWristTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateRasToTableRobotWristTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateRasToTableRobotWristTransform: Invalid MRML scene");
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

  // TableRobotWrist -> RAS
  // TableRobotWrist - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableFlange -> TableRobotFlange -> TableRobotWrist
  // Find RasToTableRobotWristTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableRobotWristTransformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToTableRobotWristTransform"))
  {
    rasToTableRobotWristTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }
  else
  {
    rasToTableRobotWristTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    rasToTableRobotWristTransformNode->SetName("RasToTableRobotWristTransform");
//    rasToTableRobotWristTransformNode->SetHideFromEditors(1);
    std::string singletonTag = std::string("C26C3_") + "RasToTableRobotWristTransform";
//    rasToTableRobotWristTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(rasToTableRobotWristTransformNode);
  }

  vtkNew<vtkTransform> rasToTableRobotWristTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::TableRobotWrist, 
    rasToTableRobotWristTransform, false))
  {
    vtkDebugMacro("UpdateRasToTableRobotWristTransform: RAS->TableRobotWrist transform updated");
    // Transform to RAS, set transform to node, transform the model
    rasToTableRobotWristTransform->Concatenate(patientToRasTransform);
  }
  if (rasToTableRobotWristTransformNode)
  {
    rasToTableRobotWristTransformNode->SetAndObserveTransformToParent(rasToTableRobotWristTransform);
  }
  return rasToTableRobotWristTransformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateRasToTableRobotElbowWristTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateRasToTableRobotElbowWristTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateRasToTableRobotElbowWristTransform: Invalid MRML scene");
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

  // TableRobotElbowWrist -> RAS
  // TableRobotElbowWrist - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableFlange -> TableRobotFlange -> TableRobotWrist
  // TableRobotWrist - > TableRobotElbowWrist
  // Find RasToTableRobotWristTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableRobotElbowWristTransformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToTableRobotElbowWristTransform"))
  {
    rasToTableRobotElbowWristTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }
  else
  {
    rasToTableRobotElbowWristTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    rasToTableRobotElbowWristTransformNode->SetName("RasToTableRobotElbowWristTransform");
//    rasToTableRobotElbowWristTransformNode->SetHideFromEditors(1);
    std::string singletonTag = std::string("C26C3_") + "RasToTableRobotElbowWristTransform";
//    rasToTableRobotElbowWristTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(rasToTableRobotElbowWristTransformNode);
  }

  vtkNew<vtkTransform> rasToTableRobotElbowWristTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::TableRobotElbowWrist, 
    rasToTableRobotElbowWristTransform, false))
  {
    vtkDebugMacro("UpdateRasToTableRobotElbowWristTransform: RAS->TableRobotElbowWrist transform updated");
    // Transform to RAS, set transform to node, transform the model
    rasToTableRobotElbowWristTransform->Concatenate(patientToRasTransform);
  }
  if (rasToTableRobotElbowWristTransformNode)
  {
    rasToTableRobotElbowWristTransformNode->SetAndObserveTransformToParent(rasToTableRobotElbowWristTransform);
  }
  return rasToTableRobotElbowWristTransformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateRasToTableRobotElbowShoulderTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateRasToTableRobotElbowShoulderTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateRasToTableRobotElbowShoulderTransform: Invalid MRML scene");
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

  // TableRobotElbowWrist -> RAS
  // TableRobotElbowWrist - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableFlange -> TableRobotFlange -> TableRobotWrist
  // TableRobotWrist -> TableRobotElbowWrist -> TableRobotElbowShoulder
  // Find RasToTableRobotWristTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableRobotElbowShoulderTransformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToTableRobotElbowShoulderTransform"))
  {
    rasToTableRobotElbowShoulderTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }
  else
  {
    rasToTableRobotElbowShoulderTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    rasToTableRobotElbowShoulderTransformNode->SetName("RasToTableRobotElbowShoulderTransform");
//    rasToTableRobotElbowShoulderTransformNode->SetHideFromEditors(1);
    std::string singletonTag = std::string("C26C3_") + "RasToTableRobotElbowShoulderTransform";
//    rasToTableRobotElbowShoulderTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(rasToTableRobotElbowShoulderTransformNode);
  }

  vtkNew<vtkTransform> rasToTableRobotElbowShoulderTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::TableRobotElbowShoulder, 
    rasToTableRobotElbowShoulderTransform, false))
  {
    vtkDebugMacro("UpdateRasToTableRobotElbowShoulderTransform: RAS->TableRobotElbowShoulder transform updated");
    // Transform to RAS, set transform to node, transform the model
    rasToTableRobotElbowShoulderTransform->Concatenate(patientToRasTransform);
  }
  if (rasToTableRobotElbowShoulderTransformNode)
  {
    rasToTableRobotElbowShoulderTransformNode->SetAndObserveTransformToParent(rasToTableRobotElbowShoulderTransform);
  }
  return rasToTableRobotElbowShoulderTransformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerChannel26Cabin3RobotsTransformLogic::UpdateRasToTableRobotShoulderTransform(vtkMRMLChannel26GeometryNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateRasToTableRobotShoulderTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateRasToTableRobotShoulderTransform: Invalid MRML scene");
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

  // TableRobotElbowWrist -> RAS
  // TableRobotElbowWrist - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableFlange -> TableRobotFlange -> TableRobotWrist
  // TableRobotWrist -> TableRobotElbowWrist -> TableRobotElbowShoulder -> TableRobotShoulder
  // Find RasToTableRobotWristTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableRobotShoulderTransformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToTableRobotShoulderTransform"))
  {
    rasToTableRobotShoulderTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }
  else
  {
    rasToTableRobotShoulderTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    rasToTableRobotShoulderTransformNode->SetName("RasToTableRobotShoulderTransform");
//    rasToTableRobotShoulderTransformNode->SetHideFromEditors(1);
    std::string singletonTag = std::string("C26C3_") + "RasToTableRobotShoulderTransform";
//    rasToTableRobotShoulderTransformNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(rasToTableRobotShoulderTransformNode);
  }

  vtkNew<vtkTransform> rasToTableRobotShoulderTransform;
  if (this->GetTransformBetween( CoordSys::RAS, CoordSys::TableRobotShoulder, 
    rasToTableRobotShoulderTransform, false))
  {
    vtkDebugMacro("UpdateRasToTableRobotShoulderTransform: RAS->TableRobotShoulder transform updated");
    // Transform to RAS, set transform to node, transform the model
    rasToTableRobotShoulderTransform->Concatenate(patientToRasTransform);
  }
  if (rasToTableRobotShoulderTransform)
  {
    rasToTableRobotShoulderTransformNode->SetAndObserveTransformToParent(rasToTableRobotShoulderTransform);
  }
  return rasToTableRobotShoulderTransformNode;
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
  if (!this->GetTransformBetween( CoordSys::TableFlange, CoordSys::Patient, 
    tableFlangeToPatientTransform, false))
  {
    vtkWarningMacro("UpdateTableFlangeToTableRobotFlangeTransform: Can't get TableFlange->Patient transform");
    return;
  }

  // Initial table robot flange model position offset
  const double flangeOffsetX = CoordPos::TABLE_ROBOT_BASE_ROTATION_SHOULDER_OFFSET_X + \
    CoordPos::TABLE_ROBOT_ELBOW_SIZE + CoordPos::TABLE_ROBOT_WRIST_SIZE;
  const double flangeOffsetY = CoordPos::TABLE_ROBOT_BASE_ROTATION_SHOULDER_OFFSET_Y + \
    CoordPos::TABLE_ROBOT_SHOULDER_SIZE + CoordPos::TABLE_ROBOT_SHOULDER_ELBOW_OFFSET_Y;

  tableFlangeToPatientTransform->Translate(flangeOffsetX, flangeOffsetY, 0.); // compansate initial position offset of flange model

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
  if (!this->GetTransformBetween( CoordSys::TableRobotFlange, CoordSys::Patient, 
    tableRobotFlangeToPatientTransform, false))
  {
    vtkWarningMacro("UpdateTableRobotFlangeToTableRobotWristTransform: Can't get TableRobotFlange->Patient transform");
    return;
  }

  // Initial table robot wrist model position offset
  const double wristOffsetX = CoordPos::TABLE_ROBOT_BASE_ROTATION_SHOULDER_OFFSET_X + \
    CoordPos::TABLE_ROBOT_ELBOW_SIZE + CoordPos::TABLE_ROBOT_WRIST_SIZE;
  const double wristOffsetY = CoordPos::TABLE_ROBOT_BASE_ROTATION_SHOULDER_OFFSET_Y + \
    CoordPos::TABLE_ROBOT_SHOULDER_SIZE + CoordPos::TABLE_ROBOT_SHOULDER_ELBOW_OFFSET_Y;

  tableRobotFlangeToPatientTransform->Translate(wristOffsetX, wristOffsetY, 0.); // compansate initial position offset of wrist model

  vtkNew<vtkTransform> ReverseWristVerticalOrientationTransform; // vertical orientation
  ReverseWristVerticalOrientationTransform->Identity();
  ReverseWristVerticalOrientationTransform->RotateY(90.);

  vtkMRMLLinearTransformNode* tableFlangeToTableRobotWristTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableRobotFlange, CoordSys::TableRobotWrist);
  if (tableFlangeToTableRobotWristTransformNode)
  {

    vtkNew<vtkTransform> tableFlangeToTableRobotFlangeTransform;
    double a[6] = {};
    parameterNode->GetTableTopRobotAngles(a);
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
  if (!this->GetTransformBetween( CoordSys::TableRobotWrist, CoordSys::Patient, 
    tableRobotWristToPatientTransform, false))
  {
    vtkWarningMacro("UpdateTableRobotWristToTableRobotElbowWristTransform: Can't get TableRobotWrist->Patient transform");
    return;
  }

  // Initial table robot elbow wrist model position offset
  const double elbowWristOffsetX = CoordPos::TABLE_ROBOT_BASE_ROTATION_SHOULDER_OFFSET_X + \
    CoordPos::TABLE_ROBOT_ELBOW_SIZE;
  const double elbowWristOffsetY = CoordPos::TABLE_ROBOT_BASE_ROTATION_SHOULDER_OFFSET_Y + \
    CoordPos::TABLE_ROBOT_SHOULDER_SIZE + CoordPos::TABLE_ROBOT_SHOULDER_ELBOW_OFFSET_Y;

  tableRobotWristToPatientTransform->Translate(elbowWristOffsetX, elbowWristOffsetY, 0.); // compansate initial position offset of elbow wrist model

  vtkNew<vtkTransform> ReverseWristVerticalOrientationTransform; // vertical orientation
  ReverseWristVerticalOrientationTransform->Identity();
  ReverseWristVerticalOrientationTransform->RotateY(90.);

  vtkMRMLLinearTransformNode* tableRobotWristToTableRobotElbowWristTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableRobotWrist, CoordSys::TableRobotElbowWrist);
  if (tableRobotWristToTableRobotElbowWristTransformNode)
  {
    vtkNew<vtkTransform> tableRobotWristToTableRobotElbowWristTransform;
    double a[6] = {};
    parameterNode->GetTableTopRobotAngles(a);
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
  const double elbowShoulderOffsetX = CoordPos::TABLE_ROBOT_BASE_ROTATION_SHOULDER_OFFSET_X + \
    CoordPos::TABLE_ROBOT_ELBOW_SIZE;
  const double elbowShoulderOffsetY = CoordPos::TABLE_ROBOT_BASE_ROTATION_SHOULDER_OFFSET_Y + \
    CoordPos::TABLE_ROBOT_SHOULDER_SIZE + CoordPos::TABLE_ROBOT_SHOULDER_ELBOW_OFFSET_Y;

  tableRobotElbowWristToPatientTransform->Translate(elbowShoulderOffsetX, elbowShoulderOffsetY, 0.); // compansate initial position offset of elbow shoulder model

  vtkNew<vtkTransform> ReverseWristVerticalOrientationTransform; // vertical orientation
  ReverseWristVerticalOrientationTransform->Identity();
  ReverseWristVerticalOrientationTransform->RotateY(90.);

  vtkMRMLLinearTransformNode* tableRobotElbowWristToTableRobotElbowShoulderTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableRobotElbowWrist, CoordSys::TableRobotElbowShoulder);
  if (tableRobotElbowWristToTableRobotElbowShoulderTransformNode)
  {
    vtkNew<vtkTransform> tableRobotWristToTableRobotElbowWristTransform;
    double a[6] = {};
    parameterNode->GetTableTopRobotAngles(a);
    double patientToTableTopTranslation[3] = {};
    parameterNode->GetPatientToTableTopTranslation(patientToTableTopTranslation);

    // Wrist->Flange (TableTop) rotation
    vtkNew<vtkTransform> WristToFlangeTransform;
    WristToFlangeTransform->RotateZ(a[5]);

    // Apply transform (rotation around Y axis on A5 angle, and around X axis on A4 angle in RAS origin)
    vtkNew<vtkTransform> WristToElbowTransform;
    WristToElbowTransform->RotateY(a[4]);
    WristToElbowTransform->RotateX(-90. + a[3]);
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
  const double shoulderOffsetX = CoordPos::TABLE_ROBOT_BASE_ROTATION_SHOULDER_OFFSET_X;
  const double shoulderOffsetY = CoordPos::TABLE_ROBOT_BASE_ROTATION_SHOULDER_OFFSET_Y + \
    CoordPos::TABLE_ROBOT_SHOULDER_SIZE;

  tableRobotElbowShoulderToPatientTransform->Translate(shoulderOffsetX, shoulderOffsetY, 0.); // compansate initial position offset of shoulder model

  // Transform model to vertical position
  vtkNew<vtkTransform> ShoulderVerticalOrientationTransform; // vertical orientation
  ShoulderVerticalOrientationTransform->Identity();
  ShoulderVerticalOrientationTransform->RotateX(-90.);

  vtkMRMLLinearTransformNode* elbowToShoulderTransformNode =
    this->GetTransformNodeBetween(CoordSys::TableRobotElbowShoulder, CoordSys::TableRobotShoulder);
  if (elbowToShoulderTransformNode)
  {
    double a[6] = {};
    parameterNode->GetTableTopRobotAngles(a);
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

//-----------------------------------------------------------------------------
bool vtkSlicerChannel26Cabin3RobotsTransformLogic::GetTransformBetween(
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
bool vtkSlicerChannel26Cabin3RobotsTransformLogic::GetPathToRoot( CoordinateSystemIdentifier frame, 
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
bool vtkSlicerChannel26Cabin3RobotsTransformLogic::GetPathFromRoot( CoordinateSystemIdentifier frame, 
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
