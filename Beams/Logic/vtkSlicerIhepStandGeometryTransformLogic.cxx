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
  this->CoordinateSystemsMap[IHEP::PatientSupport] = "PatientSupport";
  this->CoordinateSystemsMap[IHEP::TableTopStand] = "TableTopStand";
  this->CoordinateSystemsMap[IHEP::TableTop] = "TableTop";
  this->CoordinateSystemsMap[IHEP::Patient] = "Patient";

  this->IhepTransforms.clear();
  this->IhepTransforms.push_back(std::make_pair(IHEP::FixedReference, IHEP::RAS));
  this->IhepTransforms.push_back(std::make_pair(IHEP::Collimator, IHEP::FixedReference)); // Collimator in canyon system
  this->IhepTransforms.push_back(std::make_pair(IHEP::PatientSupport, IHEP::FixedReference)); // Rotation of patient support platform
  this->IhepTransforms.push_back(std::make_pair(IHEP::TableTopStand, IHEP::PatientSupport)); // Horizontal movements of the table top stand and table top
  this->IhepTransforms.push_back(std::make_pair(IHEP::TableTop, IHEP::TableTopStand)); // Vertical movement and rotation of table top
  this->IhepTransforms.push_back(std::make_pair(IHEP::Patient, IHEP::TableTop));
  this->IhepTransforms.push_back(std::make_pair(IHEP::RAS, IHEP::Patient));

  this->CoordinateSystemsHierarchy.clear();
  // key - parent, value - children
  this->CoordinateSystemsHierarchy[IHEP::FixedReference] = { IHEP::Collimator, IHEP::PatientSupport };
  this->CoordinateSystemsHierarchy[IHEP::PatientSupport] = { IHEP::TableTopStand };
  this->CoordinateSystemsHierarchy[IHEP::TableTopStand] = { IHEP::TableTop };
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
  for ( auto& transformPair : this->IhepTransforms)
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

  // Organize transforms into hierarchy based on IHEP geometry

  // Fixed Reference parent
  this->GetTransformNodeBetween(IHEP::Collimator, IHEP::FixedReference)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(IHEP::FixedReference, IHEP::RAS)->GetID() );
  this->GetTransformNodeBetween(IHEP::PatientSupport, IHEP::FixedReference)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(IHEP::FixedReference, IHEP::RAS)->GetID() );

  // Patient support parent
  this->GetTransformNodeBetween(IHEP::TableTopStand, IHEP::PatientSupport)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(IHEP::PatientSupport, IHEP::FixedReference)->GetID() );

  // Table top inferior superior movement parent
  this->GetTransformNodeBetween(IHEP::TableTop, IHEP::TableTopStand)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(IHEP::TableTopStand, IHEP::PatientSupport)->GetID() );

  // Table top parent
  this->GetTransformNodeBetween( IHEP::Patient, IHEP::TableTop)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween(IHEP::TableTop, IHEP::TableTopStand)->GetID() );

  // Patient parent
  this->GetTransformNodeBetween( IHEP::RAS, IHEP::Patient)->SetAndObserveTransformNodeID(
    this->GetTransformNodeBetween( IHEP::Patient, IHEP::TableTop)->GetID() );
}

//-----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryTransformLogic::UpdateBeamTransform(vtkMRMLRTBeamNode* beamNode)
{
  //TODO: Observe beam node's geometry modified event (vtkMRMLRTBeamNode::BeamGeometryModified)
  // and its parent plan's POI markups fiducial's point modified event (vtkMRMLMarkupsNode::PointModifiedEvent)
  // so that UpdateTransformsFromBeamGeometry is called. It may be needed to change the signature of the
  // update function. It may be also needed to store a reference to the beam node (see defined nodes in SlicerRT)

  if (!beamNode)
  {
    vtkErrorMacro("UpdateBeamTransform: Invalid beam node");
    return;
  }

  // Make sure transform node exists
  beamNode->CreateDefaultTransformNode();

  // Update transform for beam
  vtkMRMLLinearTransformNode* beamTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
    beamNode->GetParentTransformNode() );
  if (!beamTransformNode)
  {
    vtkErrorMacro("UpdateBeamTransform: Failed to access transform node of beam " << beamNode->GetName());
    return;
  }

  // Update transforms in IHEP logic from beam node parameters
  this->UpdateIHEPTransformsFromBeam(beamNode);

  // Dynamic transform from Collimator to RAS
  // Transformation path:
  // Collimator -> FixedReference -> PatientSupport -> TableTopInferiorSuperiorMovement -> TableTop -> Patient -> RAS
  vtkSmartPointer<vtkGeneralTransform> beamGeneralTransform = vtkSmartPointer<vtkGeneralTransform>::New();
  using IHEP = CoordinateSystemIdentifier;
  if (this->GetTransformBetween( IHEP::Collimator, IHEP::RAS, beamGeneralTransform))
  {
    // Convert general transform to linear
    // This call also makes hard copy of the transform so that it doesn't change when other beam transforms change
    vtkSmartPointer<vtkTransform> beamLinearTransform = vtkSmartPointer<vtkTransform>::New();
    if (!vtkMRMLTransformNode::IsGeneralTransformLinear(beamGeneralTransform, beamLinearTransform))
    {
      vtkErrorMacro("UpdateBeamTransform: Unable to set transform with non-linear components to beam " << beamNode->GetName());
      return;
    }

    // Set transform to beam node
    beamTransformNode->SetAndObserveTransformToParent(beamLinearTransform);

    // Update the name of the transform node too
    // (the user may have renamed the beam, but it's very expensive to update the transform name on every beam modified event)
    std::string transformName = std::string(beamNode->GetName()) + vtkMRMLRTBeamNode::BEAM_TRANSFORM_NODE_NAME_POSTFIX;
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryTransformLogic::UpdateBeamTransform( vtkMRMLRTBeamNode* beamNode, vtkMRMLLinearTransformNode* beamTransformNode, double* isocenter)
{
  //TODO: Observe beam node's geometry modified event (vtkMRMLRTBeamNode::BeamGeometryModified)
  // and its parent plan's POI markups fiducial's point modified event (vtkMRMLMarkupsNode::PointModifiedEvent)
  // so that UpdateTransformsFromBeamGeometry is called. It may be needed to change the signature of the
  // update function. It may be also needed to store a reference to the beam node (see defined nodes in SlicerRT)

  if (!beamNode)
  {
    vtkErrorMacro("UpdateBeamTransform: Invalid beam node");
    return;
  }

  if (!beamTransformNode)
  {
    vtkErrorMacro("UpdateBeamTransform: Invalid beam transform node");
    return;
  }

  // Update transforms in IHEP logic from beam node parameters
  this->UpdateIHEPTransformsFromBeam( beamNode, isocenter);

  // Dynamic transform from Collimator to RAS
  // Transformation path:
  // Collimator -> FixedReference -> PatientSupport -> TableTopEccentricRotation -> TableTop -> Patient -> RAS
  vtkSmartPointer<vtkGeneralTransform> beamGeneralTransform = vtkSmartPointer<vtkGeneralTransform>::New();
  using IHEP = CoordinateSystemIdentifier;
  if (this->GetTransformBetween( IHEP::Collimator, IHEP::RAS, beamGeneralTransform))
  {
    // Convert general transform to linear
    // This call also makes hard copy of the transform so that it doesn't change when other beam transforms change
    vtkSmartPointer<vtkTransform> beamLinearTransform = vtkSmartPointer<vtkTransform>::New();
    if (!vtkMRMLTransformNode::IsGeneralTransformLinear(beamGeneralTransform, beamLinearTransform))
    {
      vtkErrorMacro("UpdateBeamTransform: Unable to set transform with non-linear components to beam " << beamNode->GetName());
      return;
    }

    // Set transform to beam node
    beamTransformNode->SetAndObserveTransformToParent(beamLinearTransform);

    // Update the name of the transform node too
    // (the user may have renamed the beam, but it's very expensive to update the transform name on every beam modified event)
    std::string transformName = std::string(beamNode->GetName()) + vtkMRMLRTBeamNode::BEAM_TRANSFORM_NODE_NAME_POSTFIX;
  }
}


//-----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryTransformLogic::UpdateIHEPTransformsFromBeam( vtkMRMLRTBeamNode* beamNode, double* isocenter)
{
  if (!beamNode)
  {
    vtkErrorMacro("UpdateIHEPTransformsFromBeam: Invalid beam node");
    return;
  }
  if (!isocenter)
  {
    vtkMRMLScene* scene = beamNode->GetScene();
    if (!scene || this->GetMRMLScene() != scene)
    {
      vtkErrorMacro("UpdateIHEPTransformsFromBeam: Invalid MRML scene");
      return;
    }
  }

  // Make sure the transform hierarchy is set up
  this->BuildIHEPTransformHierarchy();
/*
  double theta = beamNode->GetGantryAngle() * M_PI / 180.;
  double phi = beamNode->GetCouchAngle() * M_PI / 180.;
  double angle1 = atan(cos(theta) / (sin(theta) * cos(phi))) * 180. / M_PI;
  double angle2 = atan(cos(theta) / (sin(theta) * sin(phi))) * 180. / M_PI;

  vtkWarningMacro("UpdateIHEPTransformsFromBeam: Angle1 " << angle1 << " angel2 " << angle2);
*/
  using IHEP = CoordinateSystemIdentifier;
  // Collimator (beam) -> Fixed Reference
  vtkMRMLLinearTransformNode* collimatorToFixedReferenceTransformNode =
    this->GetTransformNodeBetween(IHEP::Collimator, IHEP::FixedReference);
  vtkTransform* collimatorToFixedReferenceTransform = vtkTransform::SafeDownCast(collimatorToFixedReferenceTransformNode->GetTransformToParent());
  collimatorToFixedReferenceTransform->Identity();
//  collimatorToFixedReferenceTransform->RotateY(beamNode->GetGantryAngle());
  collimatorToFixedReferenceTransform->Modified();

  // Patient support (Patient suppport rotation) -> Fixed Reference
  vtkMRMLLinearTransformNode* patientSupportToFixedReferenceTransformNode =
    this->GetTransformNodeBetween(IHEP::PatientSupport, IHEP::FixedReference);
  vtkTransform* patientSupportToFixedReferenceTransform = vtkTransform::SafeDownCast(patientSupportToFixedReferenceTransformNode->GetTransformToParent());
  patientSupportToFixedReferenceTransform->Identity();
  patientSupportToFixedReferenceTransform->RotateZ(-1. * beamNode->GetCouchAngle());
  patientSupportToFixedReferenceTransform->Modified();

  // Table top stand -> Patient support (Patient suppport rotation)
  vtkMRMLLinearTransformNode* tableTopStandToPatientSupportTransformNode =
    this->GetTransformNodeBetween(IHEP::TableTopStand, IHEP::PatientSupport);
  vtkTransform* tableTopStandToPatientSupportTransform = vtkTransform::SafeDownCast(
    tableTopStandToPatientSupportTransformNode->GetTransformToParent());
  tableTopStandToPatientSupportTransform->Identity();
//  tableTopStandToPatientSupportTransform->RotateY(90. - beamNode->GetGantryAngle());
  tableTopStandToPatientSupportTransform->Modified();

  vtkMRMLLinearTransformNode* tableTopToTableTopStandTransformNode =
    this->GetTransformNodeBetween(IHEP::TableTop, IHEP::TableTopStand);
  vtkTransform* tableTopToTableTopStandTransform = vtkTransform::SafeDownCast(
    tableTopToTableTopStandTransformNode->GetTransformToParent());
  tableTopToTableTopStandTransform->Identity();
  if (beamNode->GetGantryAngle() >= 0 && beamNode->GetGantryAngle() <= 180.)
  {
    tableTopToTableTopStandTransform->RotateY(-90. + beamNode->GetGantryAngle());
  }
  else if (beamNode->GetGantryAngle() > 180. && beamNode->GetGantryAngle() < 360)
  {
    tableTopToTableTopStandTransform->RotateY(-270. + beamNode->GetGantryAngle());
  }

  tableTopToTableTopStandTransform->Modified();

  // Update IHEP Patient to RAS transform based on the isocenter defined in the beam's parent plan
  vtkMRMLLinearTransformNode* rasToPatientTransformNode = 
    this->GetTransformNodeBetween( IHEP::RAS, IHEP::Patient);
  vtkTransform* rasToPatientTransform = vtkTransform::SafeDownCast(rasToPatientTransformNode->GetTransformToParent());
  rasToPatientTransform->Identity();

  rasToPatientTransform->RotateX(-90.);
  rasToPatientTransform->RotateZ(180.);
  rasToPatientTransform->Modified();

  // Update IHEP FixedReference to RAS transform based on the isocenter defined in the beam's parent plan
  vtkMRMLLinearTransformNode* fixedReferenceToRasTransformNode =
    this->GetTransformNodeBetween(IHEP::FixedReference, IHEP::RAS);
  vtkTransform* fixedReferenceToRasTransform = vtkTransform::SafeDownCast(fixedReferenceToRasTransformNode->GetTransformToParent());
  fixedReferenceToRasTransform->Identity();

  // Apply isocenter translation
  std::array< double, 3 > isocenterPosition({ 0.0, 0.0, 0.0 });

  // translation for a static beam
  if (beamNode->GetPlanIsocenterPosition(isocenterPosition.data()))
  {
    fixedReferenceToRasTransform->Translate(isocenterPosition[0], isocenterPosition[1], isocenterPosition[2]);
  }
  else
  {
    vtkErrorMacro("UpdateIHEPTransformsFromBeam: Failed to get isocenter position for beam " << beamNode->GetName());
  }

  // The "S" direction in RAS is the "A" direction in FixedReference
  fixedReferenceToRasTransform->RotateX(-90.0);
  // The "S" direction to be toward the gantry (head first position) by default
  fixedReferenceToRasTransform->RotateZ(180.0);
  fixedReferenceToRasTransform->Modified();
}

//-----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryTransformLogic::UpdateIHEPTransformsFromParameter( vtkMRMLIhepStandGeometryNode* parameterNode, double* isocenter)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateIHEPTransformsFromBeam: Invalid parameter node");
    return;
  }

  if (this->GetMRMLScene())
  {
    vtkErrorMacro("UpdateIHEPTransformsFromBeam: Invalid MRML scene");
    return;
  }

  // Make sure the transform hierarchy is set up
  this->BuildIHEPTransformHierarchy();
/*
  using IHEP = CoordinateSystemIdentifier;
  // Collimator (beam) -> Fixed Reference
  vtkMRMLLinearTransformNode* collimatorToFixedReferenceTransformNode =
    this->GetTransformNodeBetween(IHEP::Collimator, IHEP::FixedReference);
  vtkTransform* collimatorToFixedReferenceTransform = vtkTransform::SafeDownCast(collimatorToFixedReferenceTransformNode->GetTransformToParent());
  collimatorToFixedReferenceTransform->Identity();
  collimatorToFixedReferenceTransform->RotateY(beamNode->GetGantryAngle());
//  collimatorToFixedReferenceTransform->RotateY(90. - beamNode->GetGantryAngle());
//  collimatorToFixedReferenceTransform->RotateY(90.);
  collimatorToFixedReferenceTransform->Modified();

  // Patient support (Patient suppport rotation) -> Fixed Reference
  vtkMRMLLinearTransformNode* patientSupportToFixedReferenceTransformNode =
    this->GetTransformNodeBetween(IHEP::PatientSupport, IHEP::FixedReference);
  vtkTransform* patientSupportToFixedReferenceTransform = vtkTransform::SafeDownCast(patientSupportToFixedReferenceTransformNode->GetTransformToParent());
  patientSupportToFixedReferenceTransform->Identity();
  patientSupportToFixedReferenceTransform->RotateZ(-1. * beamNode->GetCouchAngle());
  patientSupportToFixedReferenceTransform->Modified();

  // Inferior-Superior and Left-Right -> Patient support (Patient suppport rotation)
  vtkMRMLLinearTransformNode* tableTopInferiorSuperiorMovementToPatientSupportTransformNode =
    this->GetTransformNodeBetween(IHEP::TableTopInferiorSuperiorMovement, IHEP::PatientSupport);
  vtkTransform* tableTopInferiorSuperiorMovementToPatientSupportTransform = vtkTransform::SafeDownCast(
    tableTopInferiorSuperiorMovementToPatientSupportTransformNode->GetTransformToParent());
  tableTopInferiorSuperiorMovementToPatientSupportTransform->Identity();
//  tableTopInferiorSuperiorMovementToPatientSupportTransform->RotateX(0.);
  tableTopInferiorSuperiorMovementToPatientSupportTransform->RotateY(-1. * beamNode->GetGantryAngle());
  tableTopInferiorSuperiorMovementToPatientSupportTransform->Modified();

  vtkMRMLLinearTransformNode* tableTopToTableTopInferiorSuperiorMovementTransformNode =
    this->GetTransformNodeBetween(IHEP::TableTop, IHEP::TableTopInferiorSuperiorMovement);
  vtkTransform* tableTopToTableTopInferiorSuperiorMovementTransform = vtkTransform::SafeDownCast(
    tableTopToTableTopInferiorSuperiorMovementTransformNode->GetTransformToParent());
  tableTopToTableTopInferiorSuperiorMovementTransform->Identity();
//  tableTopToTableTopInferiorSuperiorMovementTransform->RotateY(-90. + beamNode->GetGantryAngle());
//  tableTopToTableTopInferiorSuperiorMovementTransform->RotateY(0.);
//  tableTopToTableTopInferiorSuperiorMovementTransform->RotateX(0.);
  tableTopToTableTopInferiorSuperiorMovementTransform->Modified();

  // Update IHEP Patient to RAS transform based on the isocenter defined in the beam's parent plan
  vtkMRMLLinearTransformNode* rasToPatientReferenceTransformNode = 
    this->GetTransformNodeBetween( IHEP::RAS, IHEP::Patient);
  vtkTransform* rasToPatientReferenceTransform = vtkTransform::SafeDownCast(rasToPatientReferenceTransformNode->GetTransformToParent());
  rasToPatientReferenceTransform->Identity();
  // Apply isocenter translation
  std::array< double, 3 > isocenterPosition = { 0.0, 0.0, 0.0 };
  if (isocenter)
  {
    // This is dirty hack for dynamic beams, the actual translation 
    // will be in vtkSlicerDicomRtImportExportModuleLogic::vtkInternal::LoadDynamicBeamSequence method  
    rasToPatientReferenceTransform->Translate(isocenterPosition[0], isocenterPosition[1], isocenterPosition[2]);
  }
  else
  {
    // translation for a static beam
    if (beamNode->GetPlanIsocenterPosition(isocenterPosition.data()))
    {
      rasToPatientReferenceTransform->Translate(isocenterPosition[0], isocenterPosition[1], isocenterPosition[2]);
    }
    else
    {
      vtkErrorMacro("UpdateIECTransformsFromBeam: Failed to get isocenter position for beam " << beamNode->GetName());
    }
  }

  rasToPatientReferenceTransform->RotateX(-90.);
  rasToPatientReferenceTransform->RotateZ(180.);
  rasToPatientReferenceTransform->Modified();

  // Update IHEP FixedReference to RAS transform based on the isocenter defined in the beam's parent plan
  vtkMRMLLinearTransformNode* fixedReferenceToRasTransformNode =
    this->GetTransformNodeBetween(IHEP::FixedReference, IHEP::RAS);
  vtkTransform* fixedReferenceToRasTransform = vtkTransform::SafeDownCast(fixedReferenceToRasTransformNode->GetTransformToParent());
  fixedReferenceToRasTransform->Identity();

  // Apply isocenter translation
  if (isocenter)
  {
    // Once again the dirty hack for dynamic beams, the actual translation 
    // will be in vtkSlicerDicomRtImportExportModuleLogic::vtkInternal::LoadDynamicBeamSequence method  
    rasToPatientReferenceTransform->Translate(isocenterPosition[0], isocenterPosition[1], isocenterPosition[2]);
  }
  else
  {
    // translation for a static beam
    if (beamNode->GetPlanIsocenterPosition(isocenterPosition.data()))
    {
      fixedReferenceToRasTransform->Translate(isocenterPosition[0], isocenterPosition[1], isocenterPosition[2]);
    }
    else
    {
      vtkErrorMacro("UpdateIHEPTransformsFromBeam: Failed to get isocenter position for beam " << beamNode->GetName());
    }
  }

  // The "S" direction in RAS is the "A" direction in FixedReference
  fixedReferenceToRasTransform->RotateX(-90.0);
  // The "S" direction to be toward the gantry (head first position) by default
  fixedReferenceToRasTransform->RotateZ(180.0);
  fixedReferenceToRasTransform->Modified();
*/
}

/*
//-----------------------------------------------------------------------------
void vtkSlicerIECTransformLogic::UpdateStandTransform(double patientSupportRotationAngle)
{

  // Make sure the transform hierarchy is set up
  this->BuildIECTransformHierarchy();

  //TODO: Code duplication (RevLogic::Update...)
  using IEC = CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* gantryToFixedReferenceTransformNode =
    this->GetTransformNodeBetween(IEC::Gantry, IEC::FixedReference);
  vtkTransform* gantryToFixedReferenceTransform = vtkTransform::SafeDownCast(gantryToFixedReferenceTransformNode->GetTransformToParent());
  gantryToFixedReferenceTransform->Identity();
  gantryToFixedReferenceTransform->RotateY(0.0);//beamNode->GetGantryAngle());
  gantryToFixedReferenceTransform->Modified();

  vtkMRMLLinearTransformNode* collimatorToGantryTransformNode =
    this->GetTransformNodeBetween(IEC::Collimator, IEC::Gantry);
  vtkTransform* collimatorToGantryTransform = vtkTransform::SafeDownCast(collimatorToGantryTransformNode->GetTransformToParent());
  collimatorToGantryTransform->Identity();
  collimatorToGantryTransform->RotateZ(0.0);//beamNode->GetCollimatorAngle());
  collimatorToGantryTransform->Modified();

  vtkMRMLLinearTransformNode* patientSupportRotationToFixedReferenceTransformNode =
    this->GetTransformNodeBetween(IEC::PatientSupportRotation, IEC::FixedReference);
  vtkTransform* patientSupportToFixedReferenceTransform = vtkTransform::SafeDownCast(patientSupportRotationToFixedReferenceTransformNode->GetTransformToParent());
  patientSupportToFixedReferenceTransform->Identity();
  patientSupportToFixedReferenceTransform->RotateZ(-1. * patientSupportRotationAngle);//beamNode->GetCouchAngle());
  patientSupportToFixedReferenceTransform->Modified();
*/
//  vtkMRMLLinearTransformNode* tableTopInferiorSuperiorMovementToEccentricRotationTransformNode =
//    this->GetTransformNodeBetween(IEC::TableTopEccentricRotation, IEC::TableTopInferiorSuperiorMovement);
//  vtkTransform* tableTopInferiorSuperiorMovementToEccentricRotationTransform = vtkTransform::SafeDownCast(tableTopInferiorSuperiorMovementToEccentricRotationTransformNode->GetTransformToParent());
//  tableTopInferiorSuperiorMovementToEccentricRotationTransform->Identity();
//  tableTopInferiorSuperiorMovementToEccentricRotationTransform->Translate( 0., 0., 0. /* put inverse value here */ );
//  tableTopInferiorSuperiorMovementToEccentricRotationTransform->Modified();

/*
  // Update IEC Patient to RAS transform based on the isocenter defined in the beam's parent plan
  vtkMRMLLinearTransformNode* rasToPatientReferenceTransformNode =
    this->GetTransformNodeBetween( IEC::RAS, IEC::Patient);
  vtkTransform* rasToPatientReferenceTransform = vtkTransform::SafeDownCast(rasToPatientReferenceTransformNode->GetTransformToParent());
  rasToPatientReferenceTransform->Identity();
  // Apply isocenter translation
  std::array< double, 3 > isocenterPosition = { 0.0, 0.0, 0.0 };
  if (isocenter)
  {
    // This is dirty hack for dynamic beams, the actual translation 
    // will be in vtkSlicerDicomRtImportExportModuleLogic::vtkInternal::LoadDynamicBeamSequence method  
    rasToPatientReferenceTransform->Translate(isocenterPosition[0], isocenterPosition[1], isocenterPosition[2]);
  }
  else
  {
    // translation for a static beam
    if (beamNode->GetPlanIsocenterPosition(isocenterPosition.data()))
    {
      rasToPatientReferenceTransform->Translate(isocenterPosition[0], isocenterPosition[1], isocenterPosition[2]);
    }
    else
    {
      vtkErrorMacro("UpdateIECTransformsFromBeam: Failed to get isocenter position for beam " << beamNode->GetName());
    }
  }

  rasToPatientReferenceTransform->RotateX(-90.);
  rasToPatientReferenceTransform->RotateZ(180.);
  rasToPatientReferenceTransform->Modified();
*/
/*
  // Update IEC FixedReference to RAS transform based on the isocenter defined in the beam's parent plan
  vtkMRMLLinearTransformNode* fixedReferenceToRasTransformNode =
    this->GetTransformNodeBetween(IEC::FixedReference, IEC::RAS);
  vtkTransform* fixedReferenceToRasTransform = vtkTransform::SafeDownCast(fixedReferenceToRasTransformNode->GetTransformToParent());
  fixedReferenceToRasTransform->Identity();

  // Apply isocenter translation
  if (isocenter)
  {
    // Once again the dirty hack for dynamic beams, the actual translation 
    // will be in vtkSlicerDicomRtImportExportModuleLogic::vtkInternal::LoadDynamicBeamSequence method  
    rasToPatientReferenceTransform->Translate(isocenterPosition[0], isocenterPosition[1], isocenterPosition[2]);
  }
  else
  {
    // translation for a static beam
    if (beamNode->GetPlanIsocenterPosition(isocenterPosition.data()))
    {
      fixedReferenceToRasTransform->Translate(isocenterPosition[0], isocenterPosition[1], isocenterPosition[2]);
    }
    else
    {
      vtkErrorMacro("UpdateIECTransformsFromBeam: Failed to get isocenter position for beam " << beamNode->GetName());
    }
  }

  // The "S" direction in RAS is the "A" direction in FixedReference
  fixedReferenceToRasTransform->RotateX(-90.0);
  // The "S" direction to be toward the gantry (head first position) by default
  fixedReferenceToRasTransform->RotateZ(180.0);
  fixedReferenceToRasTransform->Modified();

}
*/
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
    for ( auto& pair : this->CoordinateSystemsHierarchy)
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
