/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// IhepStandGeometry Logic includes
#include "vtkSlicerIhepStandGeometryLogic.h"

// SlicerRT MRML includes
#include <vtkMRMLIhepStandGeometryNode.h>

// SubjectHierarchy includes
#include "vtkMRMLSubjectHierarchyConstants.h"
#include "vtkMRMLSubjectHierarchyNode.h"
#include "vtkSlicerSubjectHierarchyModuleLogic.h"

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// SlicerRT IHEP transformation logic from Beam module
#include <vtkSlicerIhepStandGeometryTransformLogic.h>

// SlicerRT includes
#include <vtkMRMLRTPlanNode.h>
#include <vtkMRMLRTBeamNode.h>
#include <vtkMRMLRTIonBeamNode.h>
#include <vtkMRMLRTFixedIonBeamNode.h>
#include <vtkSlicerRtCommon.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLLinearTransformNode.h>

#include <vtkMRMLMarkupsLineNode.h>
#include <vtkMRMLMarkupsPlaneNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>

#include <vtkMRMLScene.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLCameraNode.h>
#include <vtkMRMLModelHierarchyNode.h>
#include <vtkMRMLModelDisplayNode.h>

// Slicer includes
#include <vtkSlicerModelsLogic.h>
#include <vtkSlicerSegmentationsModuleLogic.h>

// vtkSegmentationCore includes
#include <vtkSegmentationConverter.h>

// VTK includes
#include <vtksys/SystemTools.hxx>

#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>
#include <vtkAppendPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkGeneralTransform.h>
#include <vtkTransformFilter.h>
#include <vtkCamera.h>

#include <vtkPlaneSource.h>
#include <vtkPlane.h>

namespace {

const double TableTopOriginFixedReference[3] = { 265.5, 1116.6, -352. }; // Origin, LPS coordinate system
const double TableTopMirrorFixedReference[3] = { -264.5, 1116.6, -352. }; // Mirror, LPS coordinate system
const double TableTopMiddleFixedReference[3] = { 0.5, 1771.6, -352. }; // Middle, LPS coordinate system

const double TableTopUpLeftFixedReference[3] = { -264.5, 1821.6, -210. }; // table top point A, LPS coordinate system
const double TableTopUpRightFixedReference[3] = { 265.5, 1821.6, -210. }; // table top point B, LPS coordinate system
const double TableTopDownRightFixedReference[3] = { 265.5, -348.4, -210. }; // table top point C, LPS coordinate system
const double TableTopDownLeftFixedReference[3] = { -264.5, -348.4, -210. }; // table top point D, LPS coordinate system

const double TableTopCenterFixedReference[3] = {
  TableTopUpLeftFixedReference[0] + (TableTopUpRightFixedReference[0] - TableTopUpLeftFixedReference[0]) / 2.,
  TableTopDownRightFixedReference[1] + (TableTopUpRightFixedReference[1] - TableTopDownRightFixedReference[1]) / 2., 
  TableTopDownRightFixedReference[2] + (TableTopUpRightFixedReference[2] - TableTopDownRightFixedReference[2]) / 2.
  }; // table top center, LPS coordinate system

const double TableTopUpFixedReference[3] = {
  TableTopUpLeftFixedReference[0] + (TableTopUpRightFixedReference[0] - TableTopUpLeftFixedReference[0]) / 2.,
  TableTopUpLeftFixedReference[1] + (TableTopUpRightFixedReference[1] - TableTopUpLeftFixedReference[1]) / 2., 
  TableTopUpLeftFixedReference[2] + (TableTopUpRightFixedReference[2] - TableTopUpLeftFixedReference[2]) / 2.
  }; // table top middle up, LPS coordinate system

const double TableTopLeftFixedReference[3] = {
  TableTopUpLeftFixedReference[0] + (TableTopDownLeftFixedReference[0] - TableTopUpLeftFixedReference[0]) / 2.,
  TableTopUpLeftFixedReference[1] + (TableTopDownLeftFixedReference[1] - TableTopUpLeftFixedReference[1]) / 2., 
  TableTopUpLeftFixedReference[2] + (TableTopDownLeftFixedReference[2] - TableTopUpLeftFixedReference[2]) / 2.
  }; // table top mirror left, LPS coordinate system

}

//----------------------------------------------------------------------------
// Treatment machine component names
const char* vtkSlicerIhepStandGeometryLogic::FIXEDREFERENCE_MODEL_NAME = "FixedReference";

const char* vtkSlicerIhepStandGeometryLogic::PATIENTSUPPORT_MODEL_NAME = "PatientSupportPart";

const char* vtkSlicerIhepStandGeometryLogic::TABLE_PLATFORM_MODEL_NAME = "TablePlatform";

const char* vtkSlicerIhepStandGeometryLogic::TABLE_SUPPORT_MODEL_NAME = "TableTopSupport";

const char* vtkSlicerIhepStandGeometryLogic::TABLE_ORIGIN_MODEL_NAME = "TableTopOrigin";
const char* vtkSlicerIhepStandGeometryLogic::TABLE_MIRROR_MODEL_NAME = "TableTopMirror";
const char* vtkSlicerIhepStandGeometryLogic::TABLE_MIDDLE_MODEL_NAME = "TableTopMiddle";

const char* vtkSlicerIhepStandGeometryLogic::TABLETOP_MODEL_NAME = "TableTop";

const char* vtkSlicerIhepStandGeometryLogic::TABLETOP_MARKUPS_PLANE_NODE_NAME = "TableTopMarkupsPlane";
const char* vtkSlicerIhepStandGeometryLogic::TABLE_ORIGIN_MARKUPS_FIDUCIAL_NODE_NAME = "TableOriginMarkupsFiducial";
const char* vtkSlicerIhepStandGeometryLogic::TABLE_MIRROR_MARKUPS_FIDUCIAL_NODE_NAME = "TableMirrorMarkupsFiducial";
const char* vtkSlicerIhepStandGeometryLogic::TABLE_MIDDLE_MARKUPS_FIDUCIAL_NODE_NAME = "TableMiddleMarkupsFiducial";
const char* vtkSlicerIhepStandGeometryLogic::FIXEDREFERENCE_MARKUPS_LINE_NODE_NAME = "FixedReferenceMarkupsLine";
const char* vtkSlicerIhepStandGeometryLogic::FIXEDISOCENTER_MARKUPS_FIDUCIAL_NODE_NAME = "FixedIsocenter";

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerIhepStandGeometryLogic);

//----------------------------------------------------------------------------
vtkSlicerIhepStandGeometryLogic::vtkSlicerIhepStandGeometryLogic()
  :
  IhepLogic(vtkSlicerIhepStandGeometryTransformLogic::New()),
  Camera3DViewNode(nullptr)
{
}

//----------------------------------------------------------------------------
vtkSlicerIhepStandGeometryLogic::~vtkSlicerIhepStandGeometryLogic()
{
  if (this->IhepLogic)
  {
    this->IhepLogic->Delete();
    this->IhepLogic = nullptr;
  }
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
  this->IhepLogic->SetMRMLScene(newScene);
}

//-----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::RegisterNodes()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("RegisterNodes: Invalid MRML scene");
    return;
  }
  if (!scene->IsNodeClassRegistered("vtkMRMLIhepStandGeometryNode"))
  {
    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLIhepStandGeometryNode>::New());
  }
}

//---------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateFromMRMLScene()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    return;
  }
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  Superclass::ProcessMRMLNodesEvents(caller, event, callData);

  vtkMRMLScene* mrmlScene = this->GetMRMLScene();
  if (!mrmlScene)
  {
    vtkErrorMacro("ProcessMRMLNodesEvents: Invalid MRML scene");
    return;
  }
  if (mrmlScene->IsBatchProcessing())
  {
    return;
  }

  if (caller->IsA("vtkMRMLIhepStandGeometryNode"))
  {
    vtkMRMLIhepStandGeometryNode* parameterNode = vtkMRMLIhepStandGeometryNode::SafeDownCast(caller);

    if (event == vtkCommand::ModifiedEvent)
    {
      // Update model transforms using node data
      if (parameterNode->GetTreatmentMachineType())
      {
        this->SetupTreatmentMachineModels(parameterNode);
      }
      if (parameterNode->GetUseStandCoordinateSystem())
      {
        this->UpdateFixedReferenceCamera(parameterNode);
      }
    }
  }
}

//---------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (!node || !this->GetMRMLScene())
  {
    vtkErrorMacro("OnMRMLSceneNodeAdded: Invalid MRML scene or input node");
    return;
  }

  if (node->IsA("vtkMRMLIhepStandGeometryNode"))
  {
    vtkNew<vtkIntArray> events;
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    vtkObserveMRMLNodeEventsMacro(node, events);
  }
}

//---------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::InitialSetupTransformTranslations(vtkMRMLIhepStandGeometryNode* parameterNode)
{

  vtkNew<vtkTransform> rotateYTransform;
  rotateYTransform->Identity();
//  rotateYTransform->RotateX(-90.);
//  rotateYTransform->RotateZ(180.);

  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;

  this->IhepLogic->ResetRasToPatientIsocenterTranslate();

  // Update TableTop -> TableTopOrigin
  // Transformation (translation) of the TableTop to TableTopOrigin
  vtkMRMLLinearTransformNode* tableTopToTableTopOriginTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableTop, IHEP::TableTopOrigin);
  if (tableTopToTableTopOriginTransformNode)
  {

    vtkTransform* tableTopToTableTopOriginTransform = vtkTransform::SafeDownCast(
      tableTopToTableTopOriginTransformNode->GetTransformToParent() );
    tableTopToTableTopOriginTransform->Identity();
//    tableTopToTableTopOriginTransform->Translate( 0.,-1400., -675.);
//    tableTopToTableTopOriginTransform->Concatenate(rotateYTransform);
    tableTopToTableTopOriginTransform->Modified();
  }

  // Update TableTopMiddle -> TableTopSupport
  // Translation of the TableTopMiddle to TableTopSupport
  vtkMRMLLinearTransformNode* tableTopMiddleToTableTopSupportTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableTopMiddle, IHEP::TableTopSupport);
  if (tableTopMiddleToTableTopSupportTransformNode)
  {
    vtkTransform* tableTopMiddleToTableTopSupportTransform = vtkTransform::SafeDownCast(
      tableTopMiddleToTableTopSupportTransformNode->GetTransformToParent() );
    tableTopMiddleToTableTopSupportTransform->Identity();
//    tableTopMiddleToTableTopSupportTransform->Translate( 0.,-1400., -675.);
//    tableTopMiddleToTableTopSupportTransform->Concatenate(rotateYTransform);
    tableTopMiddleToTableTopSupportTransform->Modified();
  }

  // Update TableTopMirror -> TableTopSupport
  // Translation of the TableTopMirror to TableTopSupport
  vtkMRMLLinearTransformNode* tableTopMirrorToTableTopSupportTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableTopMirror, IHEP::TableTopSupport);
  if (tableTopMirrorToTableTopSupportTransformNode)
  {
    vtkTransform* tableTopMirrorToTableTopSupportTransform = vtkTransform::SafeDownCast(
      tableTopMirrorToTableTopSupportTransformNode->GetTransformToParent() );
    tableTopMirrorToTableTopSupportTransform->Identity();
//    tableTopMirrorToTableTopSupportTransform->Translate( 0.,-1400., -675.);
//    tableTopMirrorToTableTopSupportTransform->Concatenate(rotateYTransform);
    tableTopMirrorToTableTopSupportTransform->Modified();
  }

  // Update TableTopOrigin -> TableTopSupport
  // Translation of the TableTopOrigin to TableTopSupport
  vtkMRMLLinearTransformNode* tableTopOriginToTableTopSupportTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableTopOrigin, IHEP::TableTopSupport);
  if (tableTopOriginToTableTopSupportTransformNode)
  {
    vtkTransform* tableTopOriginToTableTopSupportTransform = vtkTransform::SafeDownCast(
      tableTopOriginToTableTopSupportTransformNode->GetTransformToParent() );
    tableTopOriginToTableTopSupportTransform->Identity();
//    tableTopOriginToTableTopSupportTransform->Translate( 0.,-1400., -675.);
//    tableTopOriginToTableTopSupportTransform->Concatenate(rotateYTransform);
    tableTopOriginToTableTopSupportTransform->Modified();
  }

  // Update TableTopSupport -> TablePlatform
  // Transformation (translation) of the TableTopSupport to TablePlatform
  vtkMRMLLinearTransformNode* tableTopSupportToTablePlatformTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableTopSupport, IHEP::TablePlatform);
  if (tableTopSupportToTablePlatformTransformNode)
  {
    vtkTransform* tableTopSupportToTablePlatformTransform = vtkTransform::SafeDownCast(
      tableTopSupportToTablePlatformTransformNode->GetTransformToParent() );
    tableTopSupportToTablePlatformTransform->Identity();
//    tableTopSupportToTablePlatformTransform->Translate( 0., 1400., -1550.);
//    tableTopSupportToTablePlatformTransform->Concatenate(rotateYTransform);
    tableTopSupportToTablePlatformTransform->Modified();
  }

  // Update TablePlatform -> PatientSupport
  // Transformation (translation) of the TablePlatform to PatientSupport
  vtkMRMLLinearTransformNode* tablePlatformToPatientSupportTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TablePlatform, IHEP::PatientSupport);
  if (tablePlatformToPatientSupportTransformNode)
  {
    vtkTransform* tablePlatformToPatientSupportTransform = vtkTransform::SafeDownCast(
      tablePlatformToPatientSupportTransformNode->GetTransformToParent() );
    tablePlatformToPatientSupportTransform->Identity();
//    tablePlatformToPatientSupportTransform->Translate( 0., -1400., -1550.);
//    tablePlatformToPatientSupportTransform->Translate( 0., 0., -1850.);
//    tablePlatformToPatientSupportTransform->Concatenate(rotateYTransform);
    tablePlatformToPatientSupportTransform->Modified();
  }

  // Update PatientSupport -> FixedReference
  // Translation of the PatientSupport to FixedReference
  vtkMRMLLinearTransformNode* patientSupportToFixedReferenceTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::PatientSupport, IHEP::FixedReference);
  if (patientSupportToFixedReferenceTransformNode)
  {
    vtkTransform* patientSupportToFixedReferenceTransform = vtkTransform::SafeDownCast(
      patientSupportToFixedReferenceTransformNode->GetTransformToParent() );
    patientSupportToFixedReferenceTransform->Identity();
//    patientSupportToFixedReferenceTransform->Translate( 0., 0., -1850.);
//    patientSupportToFixedReferenceTransform->Concatenate(rotateYTransform);
    patientSupportToFixedReferenceTransform->Modified();
  }
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialNode* vtkSlicerIhepStandGeometryLogic::CreateTableOriginFiducialNode(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLMarkupsFiducialNode* pointMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(this->GetMRMLScene()->AddNewNodeByClass("vtkMRMLMarkupsFiducialNode"));
  pointMarkupsNode->SetName(TABLE_ORIGIN_MARKUPS_FIDUCIAL_NODE_NAME);
//  pointsMarkupsNode->SetHideFromEditors(1);
  std::string singletonTag = std::string("IHEP_") + TABLE_ORIGIN_MARKUPS_FIDUCIAL_NODE_NAME;
  pointMarkupsNode->SetSingletonTag(singletonTag.c_str());
  pointMarkupsNode->LockedOn();

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("CreateTableOriginFiducialNode: Invalid MRML scene");
    return nullptr;
  }

  if (parameterNode)
  {
    // add point to fiducial node (initial position)
    vtkVector3d p0( -1. * TableTopOriginFixedReference[0], TableTopOriginFixedReference[2], TableTopOriginFixedReference[1]); // Origin
    vtkVector3d p1( -1. * TableTopOriginFixedReference[0], TableTopOriginFixedReference[2] + 142., TableTopOriginFixedReference[1]); // Origin Table Top
    pointMarkupsNode->AddControlPoint( p0, "Origin");
    pointMarkupsNode->AddControlPoint( p1, "OriginTableTop");
    vtkMRMLTransformNode* transformNode = this->UpdateTableOriginMarkupsTransform(parameterNode);

    // add transform to fiducial node
    if (transformNode)
    {
      pointMarkupsNode->SetAndObserveTransformNodeID(transformNode->GetID());
    }
  }
  return pointMarkupsNode;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialNode* vtkSlicerIhepStandGeometryLogic::CreateTableMirrorFiducialNode(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLMarkupsFiducialNode* pointMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(this->GetMRMLScene()->AddNewNodeByClass("vtkMRMLMarkupsFiducialNode"));
  pointMarkupsNode->SetName(TABLE_MIRROR_MARKUPS_FIDUCIAL_NODE_NAME);
//  pointsMarkupsNode->SetHideFromEditors(1);
  std::string singletonTag = std::string("IHEP_") + TABLE_MIRROR_MARKUPS_FIDUCIAL_NODE_NAME;
  pointMarkupsNode->SetSingletonTag(singletonTag.c_str());
  pointMarkupsNode->LockedOn();

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("CreateTableMirrorFiducialNode: Invalid MRML scene");
    return nullptr;
  }

  if (parameterNode)
  {
    // add point to fiducial node (initial position)
    vtkVector3d p0( -1. * TableTopMirrorFixedReference[0], TableTopMirrorFixedReference[2], TableTopMirrorFixedReference[1]); // Mirror
    vtkVector3d p1( -1. * TableTopMirrorFixedReference[0], TableTopMirrorFixedReference[2] + 142., TableTopMirrorFixedReference[1]); // Mirror Table Top
    pointMarkupsNode->AddControlPoint( p0, "Mirror");
    pointMarkupsNode->AddControlPoint( p1, "MirrorTableTop");

    vtkMRMLTransformNode* transformNode = this->UpdateTableMirrorMarkupsTransform(parameterNode);

    // add transform to fiducial node
    if (transformNode)
    {
      pointMarkupsNode->SetAndObserveTransformNodeID(transformNode->GetID());
    }
  }

  return pointMarkupsNode;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialNode* vtkSlicerIhepStandGeometryLogic::CreateTableMiddleFiducialNode(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLMarkupsFiducialNode* pointMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(this->GetMRMLScene()->AddNewNodeByClass("vtkMRMLMarkupsFiducialNode"));
  pointMarkupsNode->SetName(TABLE_MIDDLE_MARKUPS_FIDUCIAL_NODE_NAME);
//  pointsMarkupsNode->SetHideFromEditors(1);
  std::string singletonTag = std::string("IHEP_") + TABLE_MIDDLE_MARKUPS_FIDUCIAL_NODE_NAME;
  pointMarkupsNode->SetSingletonTag(singletonTag.c_str());
  pointMarkupsNode->LockedOn();

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("CreateTableMiddleFiducialNode: Invalid MRML scene");
    return nullptr;
  }

  if (parameterNode)
  {
    // add point to fiducial node (initial position)
    vtkVector3d p0( -1. * TableTopMiddleFixedReference[0], TableTopMiddleFixedReference[2], TableTopMiddleFixedReference[1]); // Mirror
    vtkVector3d p1( -1. * TableTopMiddleFixedReference[0], TableTopMiddleFixedReference[2] + 142., TableTopMiddleFixedReference[1]); // Mirror Table Top
    pointMarkupsNode->AddControlPoint( p0, "Middle");
    pointMarkupsNode->AddControlPoint( p1, "MiddleTableTop");
    vtkMRMLTransformNode* transformNode = this->UpdateTableMiddleMarkupsTransform(parameterNode);

    // add transform to fiducial node
    if (transformNode)
    {
      pointMarkupsNode->SetAndObserveTransformNodeID(transformNode->GetID());
    }
  }

  return pointMarkupsNode;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsPlaneNode* vtkSlicerIhepStandGeometryLogic::CreateTableTopPlaneNode(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkNew<vtkMRMLMarkupsPlaneNode> tableTopPlaneNode;
  this->GetMRMLScene()->AddNode(tableTopPlaneNode);
  tableTopPlaneNode->SetName(TABLETOP_MARKUPS_PLANE_NODE_NAME);
//  tableTopPlaneNode->SetHideFromEditors(1);
  std::string singletonTag = std::string("IHEP_") + TABLETOP_MARKUPS_PLANE_NODE_NAME;
  tableTopPlaneNode->SetSingletonTag(singletonTag.c_str());
  tableTopPlaneNode->LockedOn();

  // Transform IHEP stand models (IEC Patient) to RAS
  vtkNew<vtkMatrix4x4> patientToRasMatrix;
  vtkNew<vtkTransform> patientToRasTransform;
  patientToRasTransform->Identity();
  patientToRasTransform->RotateX(-90.);
  if (!parameterNode->GetPatientHeadFeetRotation())
  {
    patientToRasTransform->RotateZ(180.);
  }
  patientToRasTransform->GetMatrix(patientToRasMatrix);


  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("CreateTableTopStandPlaneNode: Invalid MRML scene");
    return nullptr;
  }

//  double tableTopA[3] = { -264.5, 1821.6, -210. }; // table top point A
//  double tableTopB[3] = { 265.5, 1821.6, -210. }; // table top point B
//  double tableTopC[3] = { 265.5, -348.4, -210. }; // table top point C
//  double tableTopD[3] = { -264.5, -348.4, -210. }; // table top point D

//  double originTranslation[3] = { 265.5, 1116.6, -352. }; // translation of table top origin to origin of fixed reference (RAS)

  if (parameterNode)
  {
    // add points from markups fiducial node
//    double originWorld[3] = { -0.5, -210., 736.6 };
//    double mirrorWorld[3] = { 264.5, -210., 736.6 };
//    double middleWorld[3] = { -0.5, -210., 1821.6 };

    double tableTopCenter[4] = { TableTopCenterFixedReference[0], TableTopCenterFixedReference[1], TableTopCenterFixedReference[2], 1. };
    double tableTopCenterRAS[4] = { };
    double tableTopUp[4] = { TableTopUpFixedReference[0], TableTopUpFixedReference[1], TableTopUpFixedReference[2], 1. };
    double tableTopUpRAS[4] = { };
    double tableTopLeft[4] = { TableTopLeftFixedReference[0], TableTopLeftFixedReference[1], TableTopLeftFixedReference[2], 1. };
    double tableTopLeftRAS[4] = { };
    patientToRasMatrix->MultiplyPoint( tableTopCenter, tableTopCenterRAS);
    patientToRasMatrix->MultiplyPoint( tableTopUp, tableTopUpRAS);
    patientToRasMatrix->MultiplyPoint( tableTopLeft, tableTopLeftRAS);

    tableTopPlaneNode->SetOrigin(tableTopCenterRAS);
    tableTopPlaneNode->SetPlaneBounds( -264.5, 265.5, -1085., 1085.);
    tableTopPlaneNode->SetSize( 530., 2170.);
    tableTopPlaneNode->SetNormal( 0., 1., 0.);
    tableTopPlaneNode->SetSizeMode(vtkMRMLMarkupsPlaneNode::SizeModeAuto);
    tableTopPlaneNode->SetPlaneType(vtkMRMLMarkupsPlaneNode::PlaneType3Points);

    this->UpdateTableTopToTableTopSupportTransform(tableTopCenterRAS, tableTopLeftRAS, tableTopUpRAS);

    vtkMRMLTransformNode* transformNode = this->UpdateTableTopPlaneTransform(parameterNode);

    // add transform to fiducial node
    if (transformNode)
    {
      tableTopPlaneNode->SetAndObserveTransformNodeID(transformNode->GetID());
    }
  }

  return tableTopPlaneNode;
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateTableTopToTableTopSupportTransform( double posOrigin[3], 
  double posMirror[3], double posMiddle[3])
{
/*
  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;

  // Find RasToTableTopTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableTopTransformNode;
  if (scene->GetFirstNodeByName("RasToTableTopTransform"))
  {
    rasToTableTopTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
      scene->GetFirstNodeByName("RasToTableTopTransform"));
  }
  else
  {
    rasToTableTopTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    rasToTableTopTransformNode->SetName("RasToTableTopTransform");
//    rasToTableTopTransformNode->SetHideFromEditors(1);
//    rasToTableTopTransformNode->SetSingletonTag("IHEP_");
    scene->AddNode(rasToTableTopTransformNode);
  }
*/
  vtkNew<vtkPlaneSource> planeSource;
  planeSource->SetOrigin(posOrigin);
  planeSource->SetPoint1(posMiddle);
  planeSource->SetPoint2(posMirror);
  planeSource->Update();
  double norm[3];
  planeSource->GetNormal(norm);
  vtkWarningMacro("UpdateTableTopToTableTopSupportTransform: Plane source normal " << norm[0] << " " << norm[1] << " " << norm[2]);
  // Update TableTop -> TableLateralMovement
  // Tranformation of the TableTop to TableLateral
//  vtkMRMLLinearTransformNode* tableTopToTableLateralTransformNode =
//    this->IhepLogic->GetTransformNodeBetween(IHEP::TableTop, IHEP::TableLateralMovement);
 // vtkTransform* tableTopToTableLateralTransform = vtkTransform::SafeDownCast(
//    tableTopToTableLateralTransformNode->GetTransformToParent() );

  // Calculate transform from tree points
//  tableTopToTableLateralTransform->Identity();
//  tableTopToTableLateralTransform->Modified();
}

//----------------------------------------------------------------------------
bool vtkSlicerIhepStandGeometryLogic::CalculateTableTopPositionsFromPlaneNode( vtkMRMLIhepStandGeometryNode* parameterNode, double& mirrorPosition, double& middlePosition)
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("UpdateTableTopPositions: Invalid MRML scene");
    return false;
  }

  if (!parameterNode)
  {
    vtkErrorMacro("UpdateTableTopPositions: Invalid parameter node");
    return false;
  }

  vtkMRMLMarkupsFiducialNode* originMarkupsNode = nullptr;
  vtkMRMLMarkupsFiducialNode* middleMarkupsNode = nullptr;
  vtkMRMLMarkupsFiducialNode* mirrorMarkupsNode = nullptr;
  vtkMRMLMarkupsPlaneNode* tableTopPlaneNode = nullptr;

  // origin fiducial markups node
  if (scene->GetFirstNodeByName(TABLE_ORIGIN_MARKUPS_FIDUCIAL_NODE_NAME))
  {
    originMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(scene->GetFirstNodeByName(TABLE_ORIGIN_MARKUPS_FIDUCIAL_NODE_NAME));
  }
  // middle fiducial markups node
  if (scene->GetFirstNodeByName(TABLE_MIDDLE_MARKUPS_FIDUCIAL_NODE_NAME))
  {
    middleMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(scene->GetFirstNodeByName(TABLE_MIDDLE_MARKUPS_FIDUCIAL_NODE_NAME));
  }
  // mirror fiducial markups node
  if (scene->GetFirstNodeByName(TABLE_MIRROR_MARKUPS_FIDUCIAL_NODE_NAME))
  {
    mirrorMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(scene->GetFirstNodeByName(TABLE_MIRROR_MARKUPS_FIDUCIAL_NODE_NAME));
  }
  // table top plane markups node
  if (scene->GetFirstNodeByName(TABLETOP_MARKUPS_PLANE_NODE_NAME))
  {
    tableTopPlaneNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(scene->GetFirstNodeByName(TABLETOP_MARKUPS_PLANE_NODE_NAME));
  }

  double* posMiddleInTableTopOrigin = middleMarkupsNode->GetNthControlPointPosition(0);
  double* posMirrorInTableTopOrigin = mirrorMarkupsNode->GetNthControlPointPosition(0);
  double* posOriginInTableTopOrigin = originMarkupsNode->GetNthControlPointPosition(0);

  double* posMiddleInTableTopOrigin1 = middleMarkupsNode->GetNthControlPointPosition(1);
  double* posMirrorInTableTopOrigin1 = mirrorMarkupsNode->GetNthControlPointPosition(1);

  double* planeOrigin = tableTopPlaneNode->GetOrigin();
  double* planePoint1 = tableTopPlaneNode->GetNthControlPointPosition(1);
  double* planePoint2 = tableTopPlaneNode->GetNthControlPointPosition(2);

  double planeOriginPos[4] = { planeOrigin[0], planeOrigin[1], planeOrigin[2], 1. };
  double planePoint1Pos[4] = { planePoint1[0], planePoint1[1], planePoint1[2], 1. };
  double planePoint2Pos[4] = { planePoint2[0], planePoint2[1], planePoint2[2], 1. };

  double originPos[4] = { posOriginInTableTopOrigin[0], posOriginInTableTopOrigin[1], posOriginInTableTopOrigin[2], 1. };
  double middlePos[4] = { posMiddleInTableTopOrigin[0], posMiddleInTableTopOrigin[1], posMiddleInTableTopOrigin[2], 1. };
  double mirrorPos[4] = { posMirrorInTableTopOrigin[0], posMirrorInTableTopOrigin[1], posMirrorInTableTopOrigin[2], 1. };

  double middlePos1[4] = { posMiddleInTableTopOrigin1[0], posMiddleInTableTopOrigin1[1], posMiddleInTableTopOrigin1[2], 1. };
  double mirrorPos1[4] = { posMirrorInTableTopOrigin1[0], posMirrorInTableTopOrigin1[1], posMirrorInTableTopOrigin1[2], 1. };

  double planePos0InTableTopSupport[4] = {};
  double planePos1InTableTopSupport[4] = {};
  double planePos2InTableTopSupport[4] = {};

  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;

  // table top plane origin point in table top support frame
  this->IhepLogic->GetTransformForPointThroughtRAS( IHEP::TableTop, 
    IHEP::TableTopSupport, planeOriginPos, planePos0InTableTopSupport);

  // table top plane point-1 in table top support frame
  this->IhepLogic->GetTransformForPointThroughtRAS( IHEP::TableTop, 
    IHEP::TableTopSupport, planePoint1Pos, planePos1InTableTopSupport);

  // table top plane point-2 in table top support frame
  this->IhepLogic->GetTransformForPointThroughtRAS( IHEP::TableTop, 
    IHEP::TableTopSupport, planePoint2Pos, planePos2InTableTopSupport);

  // Calculate table top plane normal
  vtkNew<vtkPlaneSource> planeSource;
  planeSource->SetOrigin(planePos0InTableTopSupport);
  planeSource->SetPoint1(planePos1InTableTopSupport);
  planeSource->SetPoint2(planePos2InTableTopSupport);
  planeSource->Update();
  double norm[3];
  planeSource->GetNormal(norm);

  // set vertical line coordinate lenght about 2 meters to calculate intersection point
  middlePos[1] -= 1000.;
  middlePos1[1] += 1000.;

  mirrorPos[1] -= 1000.;
  mirrorPos1[1] += 1000.;

  double t, middleLine[3], mirrorLine[3];
  if (vtkPlane::IntersectWithLine( mirrorPos, mirrorPos1, norm, originPos, t, mirrorLine))
  {
    mirrorPosition = mirrorLine[1] - posMirrorInTableTopOrigin[1];
  }
  else
  {
    return false;
  }

  if (vtkPlane::IntersectWithLine( middlePos, middlePos1, norm, originPos, t, middleLine))
  {
    middlePosition = middleLine[1] - posMiddleInTableTopOrigin[1];
  }
  else
  {
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsLineNode* vtkSlicerIhepStandGeometryLogic::CreateFixedReferenceLineNode(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLMarkupsFiducialNode* pointMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(this->GetMRMLScene()->AddNewNodeByClass("vtkMRMLMarkupsFiducialNode"));
  pointMarkupsNode->SetName(FIXEDISOCENTER_MARKUPS_FIDUCIAL_NODE_NAME);
//  pointMarkupsNode->SetHideFromEditors(1);
  std::string singletonTag = std::string("IHEP_") + FIXEDISOCENTER_MARKUPS_FIDUCIAL_NODE_NAME;
  pointMarkupsNode->SetSingletonTag(singletonTag.c_str());
  pointMarkupsNode->LockedOn();

  vtkNew<vtkMRMLMarkupsLineNode> lineMarkupsNode;
  this->GetMRMLScene()->AddNode(lineMarkupsNode);
  lineMarkupsNode->SetName(FIXEDREFERENCE_MARKUPS_LINE_NODE_NAME);
//  pointsMarkupsNode->SetHideFromEditors(1);
  singletonTag = std::string("IHEP_") + FIXEDREFERENCE_MARKUPS_LINE_NODE_NAME;
  lineMarkupsNode->SetSingletonTag(singletonTag.c_str());
  lineMarkupsNode->LockedOn();

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("CreateFixedReferenceMarkupsNodes: Invalid MRML scene");
    return nullptr;
  }

  if (parameterNode)
  {
    // add points to line node
    vtkVector3d p0( -4000., 0., 0.); // FixedBegin
    vtkVector3d p1( 4000., 0., 0.); // FixedEnd

    lineMarkupsNode->AddControlPoint( p0, "FixedBegin");
    lineMarkupsNode->AddControlPoint( p1, "FixedEnd");

    vtkVector3d pFixedIsocenter( 0., 0., 0.); // FixedIsocenter
    pointMarkupsNode->AddControlPoint( pFixedIsocenter, "FixedIsocenter");

    vtkMRMLTransformNode* transformNode = this->UpdateFixedReferenceMarkupsTransform(parameterNode);

    // add transform to fiducial node
    if (transformNode)
    {
      pointMarkupsNode->SetAndObserveTransformNodeID(transformNode->GetID());
      lineMarkupsNode->SetAndObserveTransformNodeID(transformNode->GetID());
    }
  }

  return lineMarkupsNode;
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateTableOriginFiducialNode(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("UpdateTableOriginFiducialNode: Invalid MRML scene");
    return;
  }

  if (!parameterNode)
  {
    vtkErrorMacro("UpdateTableOriginFiducialNode: Invalid parameter node");
    return;
  }

  // fiducial markups node
  if (scene->GetFirstNodeByName(TABLE_ORIGIN_MARKUPS_FIDUCIAL_NODE_NAME))
  {
    vtkMRMLMarkupsFiducialNode* pointMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
      scene->GetFirstNodeByName(TABLE_ORIGIN_MARKUPS_FIDUCIAL_NODE_NAME));
    if (pointMarkupsNode && pointMarkupsNode->GetNumberOfControlPoints() == 0)
    {
      // add points to fiducial node once again
      vtkVector3d p0( -1. * TableTopOriginFixedReference[0], TableTopOriginFixedReference[2], TableTopOriginFixedReference[1]); // Origin
      vtkVector3d p1( -1. * TableTopOriginFixedReference[0], TableTopOriginFixedReference[2] + 142., TableTopOriginFixedReference[1]); // Origin Table Top
      pointMarkupsNode->AddControlPoint( p0, "Origin");
      pointMarkupsNode->AddControlPoint( p1, "OriginTableTop");
    }

    // Update fiducial markups transform node if it's changed    
    vtkMRMLTransformNode* markupsTransformNode = this->UpdateTableOriginMarkupsTransform(parameterNode);

    if (markupsTransformNode)
    {
      pointMarkupsNode->SetAndObserveTransformNodeID(markupsTransformNode->GetID());
    }

  }
  else
  {
    this->CreateTableOriginFiducialNode(parameterNode);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateTableMirrorFiducialNode(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("UpdateTableMirrorFiducialNode: Invalid MRML scene");
    return;
  }

  if (!parameterNode)
  {
    vtkErrorMacro("UpdateTableMirrorFiducialNode: Invalid parameter node");
    return;
  }

  // fiducial markups node
  if (scene->GetFirstNodeByName(TABLE_MIRROR_MARKUPS_FIDUCIAL_NODE_NAME))
  {
    vtkMRMLMarkupsFiducialNode* pointMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
      scene->GetFirstNodeByName(TABLE_MIRROR_MARKUPS_FIDUCIAL_NODE_NAME));
    if (pointMarkupsNode && pointMarkupsNode->GetNumberOfControlPoints() == 0)
    {
      // add points to fiducial node once again
      vtkVector3d p0( -1. * TableTopMirrorFixedReference[0], TableTopMirrorFixedReference[2], TableTopMirrorFixedReference[1]); // Mirror
      vtkVector3d p1( -1. * TableTopMirrorFixedReference[0], TableTopMirrorFixedReference[2] + 142., TableTopMirrorFixedReference[1]); // Mirror Table Top
      pointMarkupsNode->AddControlPoint( p0, "Mirror");
      pointMarkupsNode->AddControlPoint( p1, "MirrorTableTop");
    }

    // Update fiducial markups transform node if it's changed    
    vtkMRMLTransformNode* markupsTransformNode = this->UpdateTableMirrorMarkupsTransform(parameterNode);

    if (markupsTransformNode)
    {
      pointMarkupsNode->SetAndObserveTransformNodeID(markupsTransformNode->GetID());
    }

  }
  else
  {
    this->CreateTableMirrorFiducialNode(parameterNode);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateTableMiddleFiducialNode(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("UpdateTableMiddleFiducialNode: Invalid MRML scene");
    return;
  }

  if (!parameterNode)
  {
    vtkErrorMacro("UpdateTableMiddleFiducialNode: Invalid parameter node");
    return;
  }

  // fiducial markups node
  if (scene->GetFirstNodeByName(TABLE_MIDDLE_MARKUPS_FIDUCIAL_NODE_NAME))
  {
    vtkMRMLMarkupsFiducialNode* pointMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
      scene->GetFirstNodeByName(TABLE_MIDDLE_MARKUPS_FIDUCIAL_NODE_NAME));
    if (pointMarkupsNode && pointMarkupsNode->GetNumberOfControlPoints() == 0)
    {
      // add points to fiducial node once again
      vtkVector3d p0( -1. * TableTopMiddleFixedReference[0], TableTopMiddleFixedReference[2], TableTopMiddleFixedReference[1]); // Mirror
      vtkVector3d p1( -1. * TableTopMiddleFixedReference[0], TableTopMiddleFixedReference[2] + 142., TableTopMiddleFixedReference[1]); // Mirror Table Top
      pointMarkupsNode->AddControlPoint( p0, "Middle");
      pointMarkupsNode->AddControlPoint( p1, "MiddleTableTop");
    }
    // Update fiducial markups transform node if it's changed    
    vtkMRMLTransformNode* markupsTransformNode = this->UpdateTableMiddleMarkupsTransform(parameterNode);

    if (markupsTransformNode)
    {
      pointMarkupsNode->SetAndObserveTransformNodeID(markupsTransformNode->GetID());
    }

  }
  else
  {
    this->CreateTableMiddleFiducialNode(parameterNode);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateTableTopPlaneNode(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("UpdateTableTopMarkupsNodes: Invalid MRML scene");
    return;
  }

  if (!parameterNode)
  {
    vtkErrorMacro("UpdateTableTopMarkupsNodes: Invalid parameter node");
    return;
  }

  // Transform IHEP stand models (IEC Patient) to RAS
  vtkNew<vtkMatrix4x4> patientToRasMatrix;
  vtkNew<vtkTransform> patientToRasTransform;
  patientToRasTransform->Identity();
  patientToRasTransform->RotateX(-90.);
  if (!parameterNode->GetPatientHeadFeetRotation())
  {
    patientToRasTransform->RotateZ(180.);
  }
  patientToRasTransform->GetMatrix(patientToRasMatrix);

  if (scene->GetFirstNodeByName(TABLETOP_MARKUPS_PLANE_NODE_NAME))
  {
    vtkMRMLMarkupsPlaneNode* tableTopPlaneNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(
      scene->GetFirstNodeByName(TABLETOP_MARKUPS_PLANE_NODE_NAME));
    if (tableTopPlaneNode && tableTopPlaneNode->GetNumberOfControlPoints() == 0)
    {

      double tableTopCenter[4] = { TableTopCenterFixedReference[0], TableTopCenterFixedReference[1], TableTopCenterFixedReference[2], 1. };
      double tableTopCenterRAS[4] = { };
      double tableTopUp[4] = { TableTopUpFixedReference[0], TableTopUpFixedReference[1], TableTopUpFixedReference[2], 1. };
      double tableTopUpRAS[4] = { };
      double tableTopLeft[4] = { TableTopLeftFixedReference[0], TableTopLeftFixedReference[1], TableTopLeftFixedReference[2], 1. };
      double tableTopLeftRAS[4] = { };
      patientToRasMatrix->MultiplyPoint( tableTopCenter, tableTopCenterRAS);
      patientToRasMatrix->MultiplyPoint( tableTopUp, tableTopUpRAS);
      patientToRasMatrix->MultiplyPoint( tableTopLeft, tableTopLeftRAS);

      // add points to markups fiducial node once again
//      double originWorld[3] = { -0.5, -210., 736.6 };
//      double mirrorWorld[3] = { 264.5, -210., 736.6 };
//      double middleWorld[3] = { -0.5, -210., 1821.6 };

//      originWorld[1] += 142.;
//      mirrorWorld[1] += 142.;
//      middleWorld[1] += 142.;

//      vtkVector3d plane0( originWorld[0], originWorld[1], originWorld[2]); // Origin
      vtkVector3d plane1( tableTopLeftRAS[0], tableTopLeftRAS[1], tableTopLeftRAS[2]); // Mirror
      vtkVector3d plane2( tableTopUpRAS[0], tableTopUpRAS[1], tableTopUpRAS[2]); // Middle

      tableTopPlaneNode->SetOrigin(tableTopCenterRAS);
      tableTopPlaneNode->AddControlPoint( plane1, "MirrorPlane");
      tableTopPlaneNode->AddControlPoint( plane2, "MiddlePlane");

      this->UpdateTableTopToTableTopSupportTransform(tableTopCenterRAS, tableTopLeftRAS, tableTopUpRAS);
    }

    // Update markups plane transform node if it's changed    
    vtkMRMLTransformNode* markupsPlaneTransformNode = this->UpdateTableTopPlaneTransform(parameterNode);

    if (markupsPlaneTransformNode)
    {
      tableTopPlaneNode->SetAndObserveTransformNodeID(markupsPlaneTransformNode->GetID());
    }
  }
  else
  {
    this->CreateTableTopPlaneNode(parameterNode);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateFixedReferenceLineNode(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("UpdateFixedReferenceLineNode: Invalid MRML scene");
    return;
  }

  if (!parameterNode)
  {
    vtkErrorMacro("UpdateFixedReferenceLineNode: Invalid parameter node");
    return;
  }

  // line markups node
  if (scene->GetFirstNodeByName(FIXEDREFERENCE_MARKUPS_LINE_NODE_NAME))
  {
    vtkMRMLMarkupsLineNode* lineMarkupsNode = vtkMRMLMarkupsLineNode::SafeDownCast(
      scene->GetFirstNodeByName(FIXEDREFERENCE_MARKUPS_LINE_NODE_NAME));
    if (lineMarkupsNode && lineMarkupsNode->GetNumberOfControlPoints() == 0)
    {
      // create points once again
      // add points to line node
      vtkVector3d p0( -4000., 0., 0.); // FixedBegin
      vtkVector3d p1( 4000., 0., 0.); // FixedEnd

      lineMarkupsNode->AddControlPoint( p0, "FixedBegin");
      lineMarkupsNode->AddControlPoint( p1, "FixedEnd");
    }
    vtkMRMLMarkupsFiducialNode* pointMarkupsNode = nullptr;
    if (scene->GetFirstNodeByName(FIXEDISOCENTER_MARKUPS_FIDUCIAL_NODE_NAME))
    {
      pointMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
        scene->GetFirstNodeByName(FIXEDISOCENTER_MARKUPS_FIDUCIAL_NODE_NAME));
    }
    if (pointMarkupsNode && pointMarkupsNode->GetNumberOfControlPoints() == 0)
    {
      // create point once again
      // add point to fiducial node
      vtkVector3d p0( 0., 0., 0.); // FixedIsocenter

      pointMarkupsNode->AddControlPoint( p0, "FixedIsocenter");
    }
/*
    // update points in line node
    vtkVector3d p0( -4000., 0., 0.); // FixedBegin
    vtkVector3d p2( 4000., 0., 0.); // FixedEnd

    // update pints
    double* p = lineMarkupsNode->GetNthControlPointPosition(0);
    if (p)
    {
      lineMarkupsNode->SetNthControlPointPosition( 0, p0.GetX(), p0.GetY(), p0.GetZ());
    }
    else
    {
      lineMarkupsNode->AddControlPoint(p0);
    }
    
    p = lineMarkupsNode->GetNthControlPointPosition(1);
    if (p)
    {
      lineMarkupsNode->SetNthControlPointPosition( 1, p2.GetX(), p2.GetY(), p2.GetZ());
    }
    else
    {
      lineMarkupsNode->AddControlPoint(p2);
    }
*/
    // Update fiducials markups transform node if it's changed    
    vtkMRMLTransformNode* markupsTransformNode = this->UpdateFixedReferenceMarkupsTransform(parameterNode);

    if (markupsTransformNode)
    {
      lineMarkupsNode->SetAndObserveTransformNodeID(markupsTransformNode->GetID());
      pointMarkupsNode->SetAndObserveTransformNodeID(markupsTransformNode->GetID());
    }
  }
  else
  {
    this->CreateFixedReferenceLineNode(parameterNode);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::ShowMarkupsNodes(bool toggled)
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("ShowMarkupsNodes: Invalid MRML scene");
    return;
  }

  // beamline markups line node
  if (scene->GetFirstNodeByName(TABLE_ORIGIN_MARKUPS_FIDUCIAL_NODE_NAME))
  {
    vtkMRMLMarkupsFiducialNode* pointsMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
      scene->GetFirstNodeByName(TABLE_ORIGIN_MARKUPS_FIDUCIAL_NODE_NAME));
    pointsMarkupsNode->SetDisplayVisibility(int(toggled));
  }
}

//---------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::BuildIhepStangGeometryTransformHierarchy()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("BuildIhepStangGeometryTransformHierarchy: Invalid MRML scene");
    return;
  }

  // Build IHEP hierarchy
  this->IhepLogic->BuildIHEPTransformHierarchy();
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::LoadTreatmentMachineModels(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("LoadTreatmentMachineModels: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("LoadTreatmentMachineModels: Invalid parameter node");
    return;
  }

  // Make sure the transform hierarchy is in place
  this->BuildIhepStangGeometryTransformHierarchy();

  std::string moduleShareDirectory = this->GetModuleShareDirectory();
  std::string machineType(parameterNode->GetTreatmentMachineType());
  std::string treatmentMachineModelsDirectory = moduleShareDirectory + "/" + machineType;

  // Create a models logic for convenient loading of components
  vtkNew<vtkSlicerModelsLogic> modelsLogic;
  modelsLogic->SetMRMLScene(scene);

  // Create model hierarchy so that the treatment machine can be shown/hidden easily
  std::string rootModelHierarchyNodeName = machineType + std::string("_Components");
  vtkSmartPointer<vtkMRMLModelHierarchyNode> rootModelHierarchyNode = vtkMRMLModelHierarchyNode::SafeDownCast(
    scene->GetSingletonNode(rootModelHierarchyNodeName.c_str(), "vtkMRMLModelHierarchyNode") );
  if (!rootModelHierarchyNode)
  {
    rootModelHierarchyNode = vtkSmartPointer<vtkMRMLModelHierarchyNode>::New();
    scene->AddNode(rootModelHierarchyNode);
    rootModelHierarchyNode->SetName(rootModelHierarchyNodeName.c_str());
    rootModelHierarchyNode->SetSingletonTag(rootModelHierarchyNodeName.c_str());
  }
  if (!rootModelHierarchyNode->GetDisplayNode())
  {
    vtkSmartPointer<vtkMRMLModelDisplayNode> rootModelHierarchyDisplayNode = vtkSmartPointer<vtkMRMLModelDisplayNode>::New();
    scene->AddNode(rootModelHierarchyDisplayNode);
    rootModelHierarchyNode->SetAndObserveDisplayNodeID( rootModelHierarchyDisplayNode->GetID() );
  }

  //
  // Load treatment machine models

  // FixedReference - mandatory
  std::string fixedReferenceModelSingletonTag = machineType + "_" + FIXEDREFERENCE_MODEL_NAME;
  vtkMRMLModelNode* fixedReferenceModelNode = vtkMRMLModelNode::SafeDownCast(
    scene->GetSingletonNode(fixedReferenceModelSingletonTag.c_str(), "vtkMRMLModelNode") );
  if (fixedReferenceModelNode && !fixedReferenceModelNode->GetPolyData())
  {
    // Remove node if contains empty polydata (e.g. after closing scene), so that it can be loaded again
    scene->RemoveNode(fixedReferenceModelNode);
    fixedReferenceModelNode = nullptr;
  }
  if (!fixedReferenceModelNode)
  {
    std::string fixedReferenceModelFilePath = treatmentMachineModelsDirectory + "/" + FIXEDREFERENCE_MODEL_NAME + ".stl";
    if (vtksys::SystemTools::FileExists(fixedReferenceModelFilePath))
    {
      fixedReferenceModelNode = modelsLogic->AddModel(fixedReferenceModelFilePath.c_str());
    }
    if (fixedReferenceModelNode)
    {
      fixedReferenceModelNode->SetSingletonTag(fixedReferenceModelSingletonTag.c_str());
      vtkNew<vtkMRMLModelHierarchyNode> fixedReferenceModelHierarchyNode;
      scene->AddNode(fixedReferenceModelHierarchyNode);
      fixedReferenceModelHierarchyNode->SetModelNodeID(fixedReferenceModelNode->GetID());
      fixedReferenceModelHierarchyNode->SetParentNodeID(rootModelHierarchyNode->GetID());
      fixedReferenceModelHierarchyNode->HideFromEditorsOn();
    }
    else
    {
      vtkErrorMacro("LoadTreatmentMachineModels: Failed to load fixed reference model");
    }
  }

  // Patient support - mandatory
  std::string patientSupportModelSingletonTag = machineType + "_" + PATIENTSUPPORT_MODEL_NAME;
  vtkMRMLModelNode* patientSupportModelNode = vtkMRMLModelNode::SafeDownCast(
    scene->GetSingletonNode(patientSupportModelSingletonTag.c_str(), "vtkMRMLModelNode") );
  if (patientSupportModelNode && !patientSupportModelNode->GetPolyData())
  {
    // Remove node if contains empty polydata (e.g. after closing scene), so that it can be loaded again
    scene->RemoveNode(patientSupportModelNode);
    patientSupportModelNode = nullptr;
  }
  if (!patientSupportModelNode)
  {
    std::string patientSupportModelFilePath = treatmentMachineModelsDirectory + "/" + PATIENTSUPPORT_MODEL_NAME + ".stl";
    if (vtksys::SystemTools::FileExists(patientSupportModelFilePath))
    {
      patientSupportModelNode = modelsLogic->AddModel(patientSupportModelFilePath.c_str());
    }
    if (patientSupportModelNode)
    {
      patientSupportModelNode->SetSingletonTag(patientSupportModelSingletonTag.c_str());
      vtkNew<vtkMRMLModelHierarchyNode> patientSupportModelHierarchyNode;
      scene->AddNode(patientSupportModelHierarchyNode);
      patientSupportModelHierarchyNode->SetModelNodeID(patientSupportModelNode->GetID());
      patientSupportModelHierarchyNode->SetParentNodeID(rootModelHierarchyNode->GetID());
      patientSupportModelHierarchyNode->HideFromEditorsOn();
    }
    else
    {
      vtkErrorMacro("LoadTreatmentMachineModels: Failed to load patient support model");
    }
  }

  // Table platform for lateral movement - optional
  std::string tablePlatformModelSingletonTag = machineType + "_" + TABLE_PLATFORM_MODEL_NAME;
  vtkMRMLModelNode* tablePlatformModelNode = vtkMRMLModelNode::SafeDownCast(
    scene->GetSingletonNode(tablePlatformModelSingletonTag.c_str(), "vtkMRMLModelNode") );
  if (tablePlatformModelNode && !tablePlatformModelNode->GetPolyData())
  {
    // Remove node if contains empty polydata (e.g. after closing scene), so that it can be loaded again
    scene->RemoveNode(tablePlatformModelNode);
    tablePlatformModelNode = nullptr;
  }
  if (!tablePlatformModelNode)
  {
    std::string tablePlatformModelFilePath = treatmentMachineModelsDirectory + "/" + TABLE_PLATFORM_MODEL_NAME + ".stl";
    if (vtksys::SystemTools::FileExists(tablePlatformModelFilePath))
    {
      tablePlatformModelNode = modelsLogic->AddModel(tablePlatformModelFilePath.c_str());
    }
    if (tablePlatformModelNode)
    {
      tablePlatformModelNode->SetSingletonTag(tablePlatformModelSingletonTag.c_str());
      vtkNew<vtkMRMLModelHierarchyNode> tablePlatformModelHierarchyNode;
      scene->AddNode(tablePlatformModelHierarchyNode);
      tablePlatformModelHierarchyNode->SetModelNodeID(tablePlatformModelNode->GetID());
      tablePlatformModelHierarchyNode->SetParentNodeID(rootModelHierarchyNode->GetID());
      tablePlatformModelHierarchyNode->HideFromEditorsOn();
    }
    else
    {
      vtkErrorMacro("LoadTreatmentMachineModels: Failed to load table platform model for longitudinal movement");
    }
  }

  // Table support for longitudinal movement  - mandatory
  std::string tableSupportModelSingletonTag = machineType + "_" + TABLE_SUPPORT_MODEL_NAME;
  vtkMRMLModelNode* tableSupportModelNode = vtkMRMLModelNode::SafeDownCast(
    scene->GetSingletonNode(tableSupportModelSingletonTag.c_str(), "vtkMRMLModelNode") );
  if (tableSupportModelNode && !tableSupportModelNode->GetPolyData())
  {
    // Remove node if contains empty polydata (e.g. after closing scene), so that it can be loaded again
    scene->RemoveNode(tableSupportModelNode);
    tableSupportModelNode = nullptr;
  }
  if (!tableSupportModelNode)
  {
    std::string tableSupportModelFilePath = treatmentMachineModelsDirectory + "/" + TABLE_SUPPORT_MODEL_NAME + ".stl";
    if (vtksys::SystemTools::FileExists(tableSupportModelFilePath))
    {
      tableSupportModelNode = modelsLogic->AddModel(tableSupportModelFilePath.c_str());
    }
    if (tableSupportModelNode)
    {
      tableSupportModelNode->SetSingletonTag(tableSupportModelSingletonTag.c_str());
      vtkNew<vtkMRMLModelHierarchyNode> tableSupportModelHierarchyNode;
      scene->AddNode(tableSupportModelHierarchyNode);
      tableSupportModelHierarchyNode->SetModelNodeID(tableSupportModelNode->GetID());
      tableSupportModelHierarchyNode->SetParentNodeID(rootModelHierarchyNode->GetID());
      tableSupportModelHierarchyNode->HideFromEditorsOn();
    }
    else
    {
      vtkErrorMacro("LoadTreatmentMachineModels: Failed to load table support model for lateral movement");
    }
  }

  // Table origin - mandatory
  std::string tableOriginModelSingletonTag = machineType + "_" + TABLE_ORIGIN_MODEL_NAME;
  vtkMRMLModelNode* tableOriginModelNode = vtkMRMLModelNode::SafeDownCast(
    scene->GetSingletonNode(tableOriginModelSingletonTag.c_str(), "vtkMRMLModelNode") );
  if (tableOriginModelNode && !tableOriginModelNode->GetPolyData())
  {
    // Remove node if contains empty polydata (e.g. after closing scene), so that it can be loaded again
    scene->RemoveNode(tableOriginModelNode);
    tableOriginModelNode = nullptr;
  }
  if (!tableOriginModelNode)
  {
    std::string tableOriginModelFilePath = treatmentMachineModelsDirectory + "/" + TABLE_ORIGIN_MODEL_NAME + ".stl";
    if (vtksys::SystemTools::FileExists(tableOriginModelFilePath))
    {
      tableOriginModelNode = modelsLogic->AddModel(tableOriginModelFilePath.c_str());
    }
    if (tableOriginModelNode)
    {
      tableOriginModelNode->SetSingletonTag(tableOriginModelSingletonTag.c_str());
      vtkNew<vtkMRMLModelHierarchyNode> tableOriginModelHierarchyNode;
      scene->AddNode(tableOriginModelHierarchyNode);
      tableOriginModelHierarchyNode->SetModelNodeID(tableOriginModelNode->GetID());
      tableOriginModelHierarchyNode->SetParentNodeID(rootModelHierarchyNode->GetID());
      tableOriginModelHierarchyNode->HideFromEditorsOn();
    }
    else
    {
      vtkErrorMacro("LoadTreatmentMachineModels: Failed to load table origin model for vertical movement");
    }
  }

  // Table mirror - mandatory
  std::string tableMirrorModelSingletonTag = machineType + "_" + TABLE_MIRROR_MODEL_NAME;
  vtkMRMLModelNode* tableMirrorModelNode = vtkMRMLModelNode::SafeDownCast(
    scene->GetSingletonNode(tableMirrorModelSingletonTag.c_str(), "vtkMRMLModelNode") );
  if (tableMirrorModelNode && !tableMirrorModelNode->GetPolyData())
  {
    // Remove node if contains empty polydata (e.g. after closing scene), so that it can be loaded again
    scene->RemoveNode(tableMirrorModelNode);
    tableMirrorModelNode = nullptr;
  }
  if (!tableMirrorModelNode)
  {
    std::string tableMirrorModelFilePath = treatmentMachineModelsDirectory + "/" + TABLE_MIRROR_MODEL_NAME + ".stl";
    if (vtksys::SystemTools::FileExists(tableMirrorModelFilePath))
    {
      tableMirrorModelNode = modelsLogic->AddModel(tableMirrorModelFilePath.c_str());
    }
    if (tableMirrorModelNode)
    {
      tableMirrorModelNode->SetSingletonTag(tableMirrorModelSingletonTag.c_str());
      vtkNew<vtkMRMLModelHierarchyNode> tableMirrorModelHierarchyNode;
      scene->AddNode(tableMirrorModelHierarchyNode);
      tableMirrorModelHierarchyNode->SetModelNodeID(tableMirrorModelNode->GetID());
      tableMirrorModelHierarchyNode->SetParentNodeID(rootModelHierarchyNode->GetID());
      tableMirrorModelHierarchyNode->HideFromEditorsOn();
    }
    else
    {
      vtkErrorMacro("LoadTreatmentMachineModels: Failed to load table mirror model for vertical movement");
    }
  }

  // Table middle - mandatory
  std::string tableMiddleModelSingletonTag = machineType + "_" + TABLE_MIDDLE_MODEL_NAME;
  vtkMRMLModelNode* tableMiddleModelNode = vtkMRMLModelNode::SafeDownCast(
    scene->GetSingletonNode(tableMiddleModelSingletonTag.c_str(), "vtkMRMLModelNode") );
  if (tableMiddleModelNode && !tableMiddleModelNode->GetPolyData())
  {
    // Remove node if contains empty polydata (e.g. after closing scene), so that it can be loaded again
    scene->RemoveNode(tableMiddleModelNode);
    tableMiddleModelNode = nullptr;
  }
  if (!tableMiddleModelNode)
  {
    std::string tableMiddleModelFilePath = treatmentMachineModelsDirectory + "/" + TABLE_MIDDLE_MODEL_NAME + ".stl";
    if (vtksys::SystemTools::FileExists(tableMiddleModelFilePath))
    {
      tableMiddleModelNode = modelsLogic->AddModel(tableMiddleModelFilePath.c_str());
    }
    if (tableMiddleModelNode)
    {
      tableMiddleModelNode->SetSingletonTag(tableMiddleModelSingletonTag.c_str());
      vtkNew<vtkMRMLModelHierarchyNode> tableMiddleModelHierarchyNode;
      scene->AddNode(tableMiddleModelHierarchyNode);
      tableMiddleModelHierarchyNode->SetModelNodeID(tableMiddleModelNode->GetID());
      tableMiddleModelHierarchyNode->SetParentNodeID(rootModelHierarchyNode->GetID());
      tableMiddleModelHierarchyNode->HideFromEditorsOn();
    }
    else
    {
      vtkErrorMacro("LoadTreatmentMachineModels: Failed to load table middle model for vertical movement");
    }
  }

  // Table top - mandatory
  std::string tableTopModelSingletonTag = machineType + "_" + TABLETOP_MODEL_NAME;
  vtkMRMLModelNode* tableTopModelNode = vtkMRMLModelNode::SafeDownCast(
    scene->GetSingletonNode(tableTopModelSingletonTag.c_str(), "vtkMRMLModelNode") );
  if (tableTopModelNode && !tableTopModelNode->GetPolyData())
  {
    // Remove node if contains empty polydata (e.g. after closing scene), so that it can be loaded again
    scene->RemoveNode(tableTopModelNode);
    tableTopModelNode = nullptr;
  }
  if (!tableTopModelNode)
  {
    std::string tableTopModelFilePath = treatmentMachineModelsDirectory + "/" + TABLETOP_MODEL_NAME + ".stl";
    if (vtksys::SystemTools::FileExists(tableTopModelFilePath))
    {
      tableTopModelNode = modelsLogic->AddModel(tableTopModelFilePath.c_str());
    }
    if (tableTopModelNode)
    {
      tableTopModelNode->SetSingletonTag(tableTopModelSingletonTag.c_str());
      vtkNew<vtkMRMLModelHierarchyNode> tableTopModelHierarchyNode;
      scene->AddNode(tableTopModelHierarchyNode);
      tableTopModelHierarchyNode->SetModelNodeID(tableTopModelNode->GetID());
      tableTopModelHierarchyNode->SetParentNodeID(rootModelHierarchyNode->GetID());
      tableTopModelHierarchyNode->HideFromEditorsOn();
    }
    else
    {
      vtkErrorMacro("LoadTreatmentMachineModels: Failed to load table top model");
    }
  }

  if (!fixedReferenceModelNode || !fixedReferenceModelNode->GetPolyData() 
    || !tableSupportModelNode || !tableSupportModelNode->GetPolyData() 
    || !tablePlatformModelNode || !tablePlatformModelNode->GetPolyData() 
    || !patientSupportModelNode || !patientSupportModelNode->GetPolyData() 
    || !tableTopModelNode || !tableTopModelNode->GetPolyData()
    || !tableOriginModelNode || !tableOriginModelNode->GetPolyData()
    || !tableMiddleModelNode || !tableMiddleModelNode->GetPolyData()
    || !tableMirrorModelNode || !tableOriginModelNode->GetPolyData())
  {
    vtkErrorMacro("LoadTreatmentMachineModels: Failed to load every mandatory treatment machine component");
    return;
  }

  // Create / Update markups nodes if they already exists 
  // Update markups (Table fiducials, plane node, FixedReference line node)
  this->UpdateTableOriginFiducialNode(parameterNode);
  this->UpdateTableMirrorFiducialNode(parameterNode);
  this->UpdateTableMiddleFiducialNode(parameterNode);
  this->UpdateTableTopPlaneNode(parameterNode);
  this->UpdateFixedReferenceLineNode(parameterNode);
  this->CreateFixedBeamPlanAndNode(parameterNode);
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::ResetModelsToInitialPosition(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("ResetModelsToInitialPosition: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("ResetModelsToInitialPosition: Invalid parameter node");
    return;
  }
  // Reset all positions into zeros
  parameterNode->ResetModelsToInitialPositions();

  // Initial transforms for the models (if any)
  this->InitialSetupTransformTranslations(parameterNode);
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::SetupTreatmentMachineModels(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();

  if (!scene)
  {
    vtkErrorMacro("SetupTreatmentMachineModels: Invalid scene");
    return;
  }
  
//  vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode();
  //TODO: Store treatment machine component color and other properties in JSON

  // Display all pieces of the treatment room and sets each piece a color to provide realistic representation
  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;

  // Transform IHEP stand models (IEC Patient) to RAS
  vtkNew<vtkTransform> patientToRasTransform;
  patientToRasTransform->Identity();
  patientToRasTransform->RotateX(-90.);
  if (!parameterNode->GetPatientHeadFeetRotation())
  {
    patientToRasTransform->RotateZ(180.);
  }

  // Table top - mandatory
  // Transform path: RAS -> Patient -> TableTop
  vtkNew<vtkTransform> rasToTableTopTransform;
  if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::TableTop, 
    rasToTableTopTransform, false))
  {
    // Transform to RAS, set transform to node, transform the model
    rasToTableTopTransform->Concatenate(patientToRasTransform);

    // Find RasToTableTopTransform or create it
    vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableTopTransformNode;
    if (scene->GetFirstNodeByName("RasToTableTopTransform"))
    {
      rasToTableTopTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
        scene->GetFirstNodeByName("RasToTableTopTransform"));
    }
    else
    {
      rasToTableTopTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      rasToTableTopTransformNode->SetName("RasToTableTopTransform");
//      rasToTableTopTransformNode->SetHideFromEditors(1);
      std::string singletonTag = std::string("IHEP_") + "RasToTableTopTransform";
      rasToTableTopTransformNode->SetSingletonTag(singletonTag.c_str());

      scene->AddNode(rasToTableTopTransformNode);
    }
    if (rasToTableTopTransformNode)
    {
      rasToTableTopTransformNode->SetAndObserveTransformToParent(rasToTableTopTransform);
    }

    vtkMRMLModelNode* tableTopModel = vtkMRMLModelNode::SafeDownCast(
      this->GetMRMLScene()->GetFirstNodeByName(TABLETOP_MODEL_NAME) );
    if (!tableTopModel)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to access table top model");
      return;
    }
    if (rasToTableTopTransformNode)
    {
      tableTopModel->SetAndObserveTransformNodeID(rasToTableTopTransformNode->GetID());
      tableTopModel->CreateDefaultDisplayNodes();
      tableTopModel->GetDisplayNode()->SetColor(0, 0, 0);
    }
  }

  // Table top origin - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableTopOrigin
  vtkNew<vtkTransform> rasToTableTopOriginTransform;
  if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::TableTopOrigin, 
    rasToTableTopOriginTransform, false))
  {
    // Transform to RAS, set transform to node, transform the model
    rasToTableTopOriginTransform->Concatenate(patientToRasTransform);

    // Find RasToTableTopOriginTransform or create it
    vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableTopOriginTransformNode;
    if (scene->GetFirstNodeByName("RasToTableTopOriginTransform"))
    {
      rasToTableTopOriginTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
        scene->GetFirstNodeByName("RasToTableTopOriginTransform"));
    }
    else
    {
      rasToTableTopOriginTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      rasToTableTopOriginTransformNode->SetName("RasToTableTopOriginTransform");
//      rasToTableTopOriginTransformNode->SetHideFromEditors(1);
      std::string singletonTag = std::string("IHEP_") + "RasToTableTopOriginTransform";
      rasToTableTopOriginTransformNode->SetSingletonTag(singletonTag.c_str());
      scene->AddNode(rasToTableTopOriginTransformNode);
    }
    if (rasToTableTopOriginTransformNode)
    {
      rasToTableTopOriginTransformNode->SetAndObserveTransformToParent(rasToTableTopOriginTransform);
    }

    vtkMRMLModelNode* tableTopOriginModel = vtkMRMLModelNode::SafeDownCast(
      this->GetMRMLScene()->GetFirstNodeByName(TABLE_ORIGIN_MODEL_NAME) );
    if (!tableTopOriginModel)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to access table origin model");
      return;
    }
    if (rasToTableTopOriginTransformNode)
    {
      tableTopOriginModel->SetAndObserveTransformNodeID(rasToTableTopOriginTransformNode->GetID());
      tableTopOriginModel->CreateDefaultDisplayNodes();
      tableTopOriginModel->GetDisplayNode()->SetColor(0.3, 0.3, 0.3);
    }
  }

  // Table mirror - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableTopMirror
  vtkNew<vtkTransform> rasToTableTopMirrorTransform;
  if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::TableTopMirror, 
    rasToTableTopMirrorTransform, false))
  {
    // Transform to RAS, set transform to node, transform the model
    rasToTableTopMirrorTransform->Concatenate(patientToRasTransform);

    // Find RasToTableTopMirrorTransform or create it
    vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableTopMirrorTransformNode;
    if (scene->GetFirstNodeByName("RasToTableTopMirrorTransform"))
    {
      rasToTableTopMirrorTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
        scene->GetFirstNodeByName("RasToTableTopMirrorTransform"));
    }
    else
    {
      rasToTableTopMirrorTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      rasToTableTopMirrorTransformNode->SetName("RasToTableTopMirrorTransform");
//      rasToTableTopMirrorTransformNode->SetHideFromEditors(1);
      std::string singletonTag = std::string("IHEP_") + "RasToTableTopMirrorTransform";
      rasToTableTopMirrorTransformNode->SetSingletonTag(singletonTag.c_str());
      scene->AddNode(rasToTableTopMirrorTransformNode);
    }
    if (rasToTableTopMirrorTransformNode)
    {
      rasToTableTopMirrorTransformNode->SetAndObserveTransformToParent(rasToTableTopMirrorTransform);
    }

    vtkMRMLModelNode* tableTopMirrorModel = vtkMRMLModelNode::SafeDownCast(
      this->GetMRMLScene()->GetFirstNodeByName(TABLE_MIRROR_MODEL_NAME) );
    if (!tableTopMirrorModel)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to access table mirror model");
      return;
    }
    if (rasToTableTopMirrorTransformNode)
    {
      tableTopMirrorModel->SetAndObserveTransformNodeID(rasToTableTopMirrorTransformNode->GetID());
      tableTopMirrorModel->CreateDefaultDisplayNodes();
      tableTopMirrorModel->GetDisplayNode()->SetColor(0.3, 0.3, 0.3);
    }
  }

  // Table top middle - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableTopMiddle
  vtkNew<vtkTransform> rasToTableTopMiddleTransform;
  if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::TableTopMiddle, 
    rasToTableTopMiddleTransform, false))
  {
    // Transform to RAS, set transform to node, transform the model
    rasToTableTopMiddleTransform->Concatenate(patientToRasTransform);

    // Find RasToTableMiddleTransform or create it
    vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableTopMiddleTransformNode;
    if (scene->GetFirstNodeByName("RasToTableTopMiddleTransform"))
    {
      rasToTableTopMiddleTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
        scene->GetFirstNodeByName("RasToTableTopMiddleTransform"));
    }
    else
    {
      rasToTableTopMiddleTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      rasToTableTopMiddleTransformNode->SetName("RasToTableTopMiddleTransform");
//      rasToTableTopMiddleTransformNode->SetHideFromEditors(1);
      std::string singletonTag = std::string("IHEP_") + "RasToTableTopMiddleTransform";
      rasToTableTopMiddleTransformNode->SetSingletonTag(singletonTag.c_str());
      scene->AddNode(rasToTableTopMiddleTransformNode);
    }
    if (rasToTableTopMiddleTransformNode)
    {
      rasToTableTopMiddleTransformNode->SetAndObserveTransformToParent(rasToTableTopMiddleTransform);
    }

    vtkMRMLModelNode* tableTopMiddleModel = vtkMRMLModelNode::SafeDownCast(
      this->GetMRMLScene()->GetFirstNodeByName(TABLE_MIDDLE_MODEL_NAME) );
    if (!tableTopMiddleModel)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to access table middle model");
      return;
    }
    if (rasToTableTopMiddleTransformNode)
    {
      tableTopMiddleModel->SetAndObserveTransformNodeID(rasToTableTopMiddleTransformNode->GetID());
      tableTopMiddleModel->CreateDefaultDisplayNodes();
      tableTopMiddleModel->GetDisplayNode()->SetColor(0.3, 0.3, 0.3);
    }
  }

  // Table top support movement model - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableTopOrigin -> TableTopSupport
  vtkNew<vtkTransform> rasToTableTopSupportTransform;
  if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::TableTopSupport, 
    rasToTableTopSupportTransform, false))
  {
    // Transform to RAS, set transform to node, transform the model
    rasToTableTopSupportTransform->Concatenate(patientToRasTransform);

    // Find RasToTableTopInferiorSuperiorTransform or create it
    vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableTopSupportTransformNode;
    if (scene->GetFirstNodeByName("RasToTableTopSupportTransform"))
    {
      rasToTableTopSupportTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
        scene->GetFirstNodeByName("RasToTableTopSupportTransform"));
    }
    else
    {
      rasToTableTopSupportTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      rasToTableTopSupportTransformNode->SetName("RasToTableTopSupportTransform");
//      rasToTableLateralTransformNode->SetHideFromEditors(1);
      std::string singletonTag = std::string("IHEP_") + "RasToTableTopSupportTransform";
      rasToTableTopSupportTransformNode->SetSingletonTag(singletonTag.c_str());
      scene->AddNode(rasToTableTopSupportTransformNode);
    }

    vtkMRMLModelNode* tableTopSupportModel = vtkMRMLModelNode::SafeDownCast(
      scene->GetFirstNodeByName(TABLE_SUPPORT_MODEL_NAME) );
    if (!tableTopSupportModel)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to access table top support model");
      return;
    }
    if (rasToTableTopSupportTransformNode)
    {
      rasToTableTopSupportTransformNode->SetAndObserveTransformToParent(rasToTableTopSupportTransform);
      tableTopSupportModel->SetAndObserveTransformNodeID(rasToTableTopSupportTransformNode->GetID());
      tableTopSupportModel->CreateDefaultDisplayNodes();
      tableTopSupportModel->GetDisplayNode()->SetColor(0.95, 0.95, 0.95);
    }
  }

  // Table platform model - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableTopOrigin -> TableTopSupport -> TablePlatform
  vtkNew<vtkTransform> rasToTablePlatformTransform;
  if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::TablePlatform, 
    rasToTablePlatformTransform, false))
  {
    // Transform to RAS, set transform to node, transform the model
    rasToTablePlatformTransform->Concatenate(patientToRasTransform);

    // Find RasToTableTopInferiorSuperiorTransform or create it
    vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTablePlatformTransformNode;
    if (scene->GetFirstNodeByName("RasToTablePlatformTransform"))
    {
      rasToTablePlatformTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
        scene->GetFirstNodeByName("RasToTablePlatformTransform"));
    }
    else
    {
      rasToTablePlatformTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      rasToTablePlatformTransformNode->SetName("RasToTablePlatformTransform");
//      rasToTablePlatformTransformNode->SetHideFromEditors(1);
      std::string singletonTag = std::string("IHEP_") + "RasToTablePlatformTransform";
      rasToTablePlatformTransformNode->SetSingletonTag(singletonTag.c_str());
      scene->AddNode(rasToTablePlatformTransformNode);
    }

    vtkMRMLModelNode* tablePlatformModel = vtkMRMLModelNode::SafeDownCast(
      scene->GetFirstNodeByName(TABLE_PLATFORM_MODEL_NAME) );
    if (!tablePlatformModel)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to access table platform model");
      return;
    }
    if (rasToTablePlatformTransformNode)
    {
      rasToTablePlatformTransformNode->SetAndObserveTransformToParent(rasToTablePlatformTransform);
      tablePlatformModel->SetAndObserveTransformNodeID(rasToTablePlatformTransformNode->GetID());
      tablePlatformModel->CreateDefaultDisplayNodes();
      tablePlatformModel->GetDisplayNode()->SetColor(0.75, 0.75, 0.75);
    }
  }

  // Patient support - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableTopOrigin -> TableTopSupport -> TablePlatform -> PatientSupport
  vtkNew<vtkTransform> rasToPatientSupportTransform;
  if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::PatientSupport, 
    rasToPatientSupportTransform, false))
  {
    // Transform to RAS, set transform to node, transform the model
    rasToPatientSupportTransform->Concatenate(patientToRasTransform);

    // Find RasToFixedReferenceTransform or create it
    vtkSmartPointer<vtkMRMLLinearTransformNode> rasToPatientSupportTransformNode;
    if (scene->GetFirstNodeByName("RasToPatientSupportTransform"))
    {
      rasToPatientSupportTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
        scene->GetFirstNodeByName("RasToPatientSupportTransform"));
    }
    else
    {
      rasToPatientSupportTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      rasToPatientSupportTransformNode->SetName("RasToPatientSupportTransform");
//      rasToPatientSupportTransformNode->SetHideFromEditors(1);
      std::string singletonTag = std::string("IHEP_") + "RasToPatientSupportTransform";
      rasToPatientSupportTransformNode->SetSingletonTag(singletonTag.c_str());
      scene->AddNode(rasToPatientSupportTransformNode);
    }
    if (rasToPatientSupportTransformNode)
    {
      rasToPatientSupportTransformNode->SetAndObserveTransformToParent(rasToPatientSupportTransform);
    }

    vtkMRMLModelNode* patientSupportModel = vtkMRMLModelNode::SafeDownCast(
      scene->GetFirstNodeByName(PATIENTSUPPORT_MODEL_NAME) );
    if (!patientSupportModel)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to access patient support model");
      return;
    }
    if (rasToPatientSupportTransformNode)
    {
      rasToPatientSupportTransformNode->SetAndObserveTransformToParent(rasToPatientSupportTransform);
      patientSupportModel->SetAndObserveTransformNodeID(rasToPatientSupportTransformNode->GetID());
      patientSupportModel->CreateDefaultDisplayNodes();
      patientSupportModel->GetDisplayNode()->SetColor(0.55, 0.55, 0.55);
    }
  }

  // Fixed Reference - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableTopOrigin -> TableTopSupport -> TablePlatform -> PatientSupport -> FixedReference
  vtkNew<vtkTransform> rasToFixedReferenceTransform;
  if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::FixedReference, 
    rasToFixedReferenceTransform, false))
  {
    // Transform to RAS, set transform to node, transform the model
    rasToFixedReferenceTransform->Concatenate(patientToRasTransform);

    // Find RasToFixedReferenceTransform or create it
    vtkSmartPointer<vtkMRMLLinearTransformNode> rasToFixedReferenceTransformNode;
    if (scene->GetFirstNodeByName("RasToFixedReferenceTransform"))
    {
      rasToFixedReferenceTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
        scene->GetFirstNodeByName("RasToFixedReferenceTransform"));
    }
    else
    {
      rasToFixedReferenceTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      rasToFixedReferenceTransformNode->SetName("RasToFixedReferenceTransform");
//      rasToFixedReferenceTransformNode->SetHideFromEditors(1);
      std::string singletonTag = std::string("IHEP_") + "RasToFixedReferenceTransform";
      rasToFixedReferenceTransformNode->SetSingletonTag(singletonTag.c_str());
      scene->AddNode(rasToFixedReferenceTransformNode);
    }
    if (rasToFixedReferenceTransformNode)
    {
      rasToFixedReferenceTransformNode->SetAndObserveTransformToParent(rasToFixedReferenceTransform);
    }

    vtkMRMLModelNode* fixedReferenceModel = vtkMRMLModelNode::SafeDownCast(
      this->GetMRMLScene()->GetFirstNodeByName(FIXEDREFERENCE_MODEL_NAME) );
    if (!fixedReferenceModel)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to access fixed reference model");
      return;
    }
    if (rasToFixedReferenceTransformNode)
    {
      fixedReferenceModel->SetAndObserveTransformNodeID(rasToFixedReferenceTransformNode->GetID());
      fixedReferenceModel->CreateDefaultDisplayNodes();
      fixedReferenceModel->GetDisplayNode()->SetColor(0.7, 0.65, 0.65);
    }
  }

  // Update markups (TableTop fiducials, plane node, FixedReference line node)
  this->UpdateTableOriginFiducialNode(parameterNode);
  this->UpdateTableMirrorFiducialNode(parameterNode);
  this->UpdateTableMiddleFiducialNode(parameterNode);
  this->UpdateTableTopPlaneNode(parameterNode);
  this->UpdateFixedReferenceLineNode(parameterNode);
  double tmp[3];
  this->GetTableTopCenterToFixedIsocenterTranslate(parameterNode, tmp);
  this->GetPatientIsocenterToFixedIsocenterTranslate(parameterNode, tmp);
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateTableTopToTableOriginTransform(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableTopToTableOriginTransform: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("UpdateTableTopToTableOriginTransform: Invalid parameter node");
    return;
  }

  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* tableTopToTableOriginTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableTop, IHEP::TableTopOrigin);

  if (tableTopToTableOriginTransformNode)
  {
    double PatientTableTopTranslation[3] = {};
//    double originTranslation[3] = { 265.5, 1116.6, -352. }; // translation of table top origin to origin of fixed reference (RAS)
    const double* originTranslation = TableTopOriginFixedReference; // translation of table top origin to origin of fixed reference (RAS)
    
    parameterNode->GetPatientToTableTopTranslation(PatientTableTopTranslation);
    PatientTableTopTranslation[0] *= -1.;
    PatientTableTopTranslation[1] *= -1.;
    PatientTableTopTranslation[2] *= -1.;

    vtkNew<vtkTransform> tableTopToTableOriginTransform;
    tableTopToTableOriginTransform->PreMultiply();

    // Move TableTop model to RAS origin
    tableTopToTableOriginTransform->Translate(originTranslation);
    tableTopToTableOriginTransform->Translate(PatientTableTopTranslation);
    // Apply transform (rotation around X and Y axis)
    tableTopToTableOriginTransform->RotateX(parameterNode->GetTableTopLongitudinalAngle());
    tableTopToTableOriginTransform->RotateY(parameterNode->GetTableTopLateralAngle());
    // Move back
    tableTopToTableOriginTransform->Translate(-1. * originTranslation[0], -1. * originTranslation[1], -1. * originTranslation[2]);
    tableTopToTableOriginTransform->Translate(-1. * PatientTableTopTranslation[0], -1. * PatientTableTopTranslation[1], -1. * PatientTableTopTranslation[2]);
    // Inverse
    tableTopToTableOriginTransform->Inverse();

    tableTopToTableOriginTransformNode->SetAndObserveTransformToParent(tableTopToTableOriginTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateTableOriginToTableSupportTransform(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableOriginToTableSupportTransform: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("UpdateTableOriginToTableSupportTransform: Invalid parameter node");
    return;
  }

  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* tableOriginToTableSupportTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableTopOrigin, IHEP::TableTopSupport);

  if (tableOriginToTableSupportTransformNode)
  {
    // TableTop to TableOrigin rotation matrix
    vtkNew<vtkTransform> tableTopToTableOriginTransform;
    vtkMRMLLinearTransformNode* tableTopToTableOriginTransformNode =
      this->IhepLogic->GetTransformNodeBetween(IHEP::TableTop, IHEP::TableTopOrigin);
    if (tableTopToTableOriginTransformNode)
    {
      vtkNew<vtkMatrix4x4> tableTopToTableOriginMatrix;
      tableTopToTableOriginTransformNode->GetMatrixTransformToParent(tableTopToTableOriginMatrix);
      // Ignore translation
      tableTopToTableOriginMatrix->SetElement( 0, 3, 0.);
      tableTopToTableOriginMatrix->SetElement( 1, 3, 0.);
      tableTopToTableOriginMatrix->SetElement( 2, 3, 0.);
      tableTopToTableOriginTransform->Concatenate(tableTopToTableOriginMatrix);
    }

    vtkNew<vtkTransform> tableOriginToTableSupportTransform;
    // Vertical translation of the TableOrigin in TableTopSupport system (in table top support local system)
    double originPosTableTop[4] = { 0., 0., -1. * parameterNode->GetTableTopVerticalPositionOrigin(), 1. }; // Translation of table origin in table top support system
    double originPosTableLateral[4];
    // Get translation of the origin in a new table top coordinate
    tableTopToTableOriginTransform->MultiplyPoint( originPosTableTop, originPosTableLateral);
    // Apply translation to TableTopSupport system
    tableOriginToTableSupportTransform->Translate(originPosTableLateral);
    tableOriginToTableSupportTransformNode->SetAndObserveTransformToParent(tableOriginToTableSupportTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateTableMirrorToTableSupportTransform(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableMirrorToTableSupportTransform: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("UpdateTableMirrorToTableSupportTransform: Invalid parameter node");
    return;
  }

  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* tableMirrorToTableSupportTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableTopMirror, IHEP::TableTopSupport);

  if (tableMirrorToTableSupportTransformNode)
  {
    // TableTop to TableOrigin rotation matrix
    vtkNew<vtkTransform> tableTopToTableOriginTransform;
    vtkMRMLLinearTransformNode* tableTopToTableOriginTransformNode =
      this->IhepLogic->GetTransformNodeBetween(IHEP::TableTop, IHEP::TableTopOrigin);
    if (tableTopToTableOriginTransformNode)
    {
      vtkNew<vtkMatrix4x4> tableTopToTableOriginMatrix;
      tableTopToTableOriginTransformNode->GetMatrixTransformToParent(tableTopToTableOriginMatrix);
      // Ignore translation
      tableTopToTableOriginMatrix->SetElement( 0, 3, 0.);
      tableTopToTableOriginMatrix->SetElement( 1, 3, 0.);
      tableTopToTableOriginMatrix->SetElement( 2, 3, 0.);
      tableTopToTableOriginTransform->Concatenate(tableTopToTableOriginMatrix);
    }

    vtkNew<vtkTransform> tableMirrorToTableSupportTransform;
    // Vertical translation of the TableMirror in TableTopSupport system (in table top support local system)
    double mirrorPosTableTop[4] = { 0., 0., -1. * parameterNode->GetTableTopVerticalPositionMirror(), 1. }; // Translation of table mirror in table top support system
    double mirrorPosTableLateral[4];
    // Get translation of the mirror in a new table top coordinate
    tableTopToTableOriginTransform->MultiplyPoint( mirrorPosTableTop, mirrorPosTableLateral);
    // Apply translation to TableTopSupport system
    tableMirrorToTableSupportTransform->Translate(mirrorPosTableLateral);

    tableMirrorToTableSupportTransformNode->SetAndObserveTransformToParent(tableMirrorToTableSupportTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateTableMiddleToTableSupportTransform(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableMiddleToTableSupportTransform: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("UpdateTableMiddleToTableSupportTransform: Invalid parameter node");
    return;
  }

  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* tableMiddleToTableSupportTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableTopMiddle, IHEP::TableTopSupport);

  if (tableMiddleToTableSupportTransformNode)
  {
    // TableTop to TableOrigin rotation matrix
    vtkNew<vtkTransform> tableTopToTableOriginTransform;
    vtkMRMLLinearTransformNode* tableTopToTableOriginTransformNode =
      this->IhepLogic->GetTransformNodeBetween(IHEP::TableTop, IHEP::TableTopOrigin);
    if (tableTopToTableOriginTransformNode)
    {
      vtkNew<vtkMatrix4x4> tableTopToTableOriginMatrix;
      tableTopToTableOriginTransformNode->GetMatrixTransformToParent(tableTopToTableOriginMatrix);
      // Ignore translation
      tableTopToTableOriginMatrix->SetElement( 0, 3, 0.);
      tableTopToTableOriginMatrix->SetElement( 1, 3, 0.);
      tableTopToTableOriginMatrix->SetElement( 2, 3, 0.);
      tableTopToTableOriginTransform->Concatenate(tableTopToTableOriginMatrix);
    }
    vtkNew<vtkTransform> tableMiddleToTableSupportTransform;
    // Vertical translation of the TableMiddle in TableTopSupport system (in table top support local system)
    double middlePosTableTop[4] = { 0., 0., -1. * parameterNode->GetTableTopVerticalPositionMiddle(), 1. }; // Translation of table middle in table top support system
    double middlePosTableLateral[4];
    // Get translation of the middle in a new table top coordinate
    tableTopToTableOriginTransform->MultiplyPoint( middlePosTableTop, middlePosTableLateral);
    // Apply translation to TableTopSupport system
    tableMiddleToTableSupportTransform->Translate(middlePosTableLateral);

    tableMiddleToTableSupportTransformNode->SetAndObserveTransformToParent(tableMiddleToTableSupportTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdatePatientSupportToFixedReferenceTransform(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdatePatientSupportToFixedReferenceTransform: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("UpdatePatientSupportToFixedReferenceTransform: Invalid parameter node");
    return;
  }

  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* patientSupportToFixedReferenceTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::PatientSupport, IHEP::FixedReference);

  if (patientSupportToFixedReferenceTransformNode)
  {
    // TableOrigin to TableTop rotation matrix (inverse of TableTop to TableOrigin rotation matrix)
    vtkNew<vtkTransform> tableOriginToTableTopTransform;
    // TableTop to TableOrigin rotation matrix
    vtkNew<vtkTransform> tableTopToTableOriginTransform;
    vtkMRMLLinearTransformNode* tableTopToTableOriginTransformNode =
      this->IhepLogic->GetTransformNodeBetween(IHEP::TableTop, IHEP::TableTopOrigin);
    if (tableTopToTableOriginTransformNode)
    {
      // Get matrix
      vtkNew<vtkMatrix4x4> tableTopToTableOriginMatrix;
      tableTopToTableOriginTransformNode->GetMatrixTransformToParent(tableTopToTableOriginMatrix);
      tableOriginToTableTopTransform->Concatenate(tableTopToTableOriginMatrix);
      tableTopToTableOriginTransform->Concatenate(tableTopToTableOriginMatrix);
    }

    // Translation and rotation around Z-axis of FixedReference object
    vtkNew<vtkTransform> fixedReferenceToRasTransform;
    fixedReferenceToRasTransform->PreMultiply();
    // Vertical translation of the TableOrigin in TableTopSupport system (in table top support local system)
    double originPosTableTop[4] = { 0., 0., -1. * parameterNode->GetTableTopVerticalPositionOrigin(), 1. }; // Translation of table origin in table top support system
    double originPosTableLateral[4];
    // Get translation of the origin in a new table top coordinate
    tableOriginToTableTopTransform->MultiplyPoint( originPosTableTop, originPosTableLateral);

    double PatientTableTopTranslation[3] = {};
    // Translation to the origin of fixed reference and patient
    double originTranslation[3] = { -1. * parameterNode->GetTableTopLateralPosition(), parameterNode->GetTableTopLongitudinalPosition(), 0 }; // translation of fixed reference to RAS
    parameterNode->GetPatientToTableTopTranslation(PatientTableTopTranslation);
    PatientTableTopTranslation[0] *= -1.;
    PatientTableTopTranslation[1] *= -1.;
    PatientTableTopTranslation[2] *= -1.;

    tableOriginToTableTopTransform->Inverse();

    // Move FixedReference model to RAS origin
    fixedReferenceToRasTransform->Translate(originTranslation);
    fixedReferenceToRasTransform->Translate(PatientTableTopTranslation);
    // Apply transform (rotation)
    fixedReferenceToRasTransform->RotateZ(-parameterNode->GetPatientSupportRotationAngle());
    // Move back
    fixedReferenceToRasTransform->Translate(-1. * originTranslation[0], -1. * originTranslation[1], -1. * originTranslation[2]);
    fixedReferenceToRasTransform->Translate(-1. * PatientTableTopTranslation[0], -1. * PatientTableTopTranslation[1], -1. * PatientTableTopTranslation[2]);

    // apply table origin to table top transform to setup fixed reference into initial position
    // apply translation and rotation to FixedReference to update rotation position
    fixedReferenceToRasTransform->Concatenate(tableOriginToTableTopTransform);
    // rotate fixed reference in the same order as table top origin
    tableTopToTableOriginTransform->Concatenate(fixedReferenceToRasTransform);

    // apply transform
    patientSupportToFixedReferenceTransformNode->SetAndObserveTransformToParent(tableTopToTableOriginTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateTableSupportToTablePlatformTransform(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableSupportToTablePlatformTransform: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("UpdateTableSupportToTablePlatformTransform: Invalid parameter node");
    return;
  }

  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* tableSupportToTablePlatformTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableTopSupport, IHEP::TablePlatform);

  if (tableSupportToTablePlatformTransformNode)
  {

    // TableTop to TableOrigin rotation matrix
    vtkNew<vtkTransform> tableTopToTableOriginTransform;
    vtkMRMLLinearTransformNode* tableTopToTableOriginTransformNode =
      this->IhepLogic->GetTransformNodeBetween(IHEP::TableTop, IHEP::TableTopOrigin);
    if (tableTopToTableOriginTransformNode)
    {
      vtkNew<vtkMatrix4x4> tableTopToTableOriginMatrix;
      tableTopToTableOriginTransformNode->GetMatrixTransformToParent(tableTopToTableOriginMatrix);
      tableTopToTableOriginMatrix->SetElement( 0, 3, 0.);
      tableTopToTableOriginMatrix->SetElement( 1, 3, 0.);
      tableTopToTableOriginMatrix->SetElement( 2, 3, 0.);
      tableTopToTableOriginTransform->Concatenate(tableTopToTableOriginMatrix);
    }

    vtkNew<vtkTransform> tableSupportToTablePlatformTransform;
    // Horizontal translation of the TableTopSupport in TablePlatfrom system (Table platform)
    double supportPosTableTop[4] = { -1. * parameterNode->GetTableTopLateralPosition(), 0., 0., 1. }; // origin in FixedReference transform
    double supportPosTablePlatform[4];
    // Get translation of the origin in a new table top coordinate
    tableTopToTableOriginTransform->MultiplyPoint( supportPosTableTop, supportPosTablePlatform);
    // Apply translation to TablePlatform system
    tableSupportToTablePlatformTransform->Translate(supportPosTablePlatform);

    tableSupportToTablePlatformTransformNode->SetAndObserveTransformToParent(tableSupportToTablePlatformTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateTablePlatformToPatientSupportTransform(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTablePlatformToPatientSupportTransform: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("UpdateTablePlatformToPatientSupportTransform: Invalid parameter node");
    return;
  }

  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* tablePlatformToPatientSupportTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TablePlatform, IHEP::PatientSupport);

  if (tablePlatformToPatientSupportTransformNode)
  {

    // TableTop to TableOrigin rotation matrix
    vtkNew<vtkTransform> tableTopToTableOriginTransform;
    vtkMRMLLinearTransformNode* tableTopToTableOriginTransformNode =
      this->IhepLogic->GetTransformNodeBetween(IHEP::TableTop, IHEP::TableTopOrigin);
    if (tableTopToTableOriginTransformNode)
    {
      vtkNew<vtkMatrix4x4> tableTopToTableOriginMatrix;
      tableTopToTableOriginTransformNode->GetMatrixTransformToParent(tableTopToTableOriginMatrix);
      tableTopToTableOriginMatrix->SetElement( 0, 3, 0.);
      tableTopToTableOriginMatrix->SetElement( 1, 3, 0.);
      tableTopToTableOriginMatrix->SetElement( 2, 3, 0.);
      tableTopToTableOriginTransform->Concatenate(tableTopToTableOriginMatrix);
    }

    vtkNew<vtkTransform> tablePlatformToPatientSupportTransform;
    // Horizontal translation of the TablePlatform in PatientSupport system (PatientSupport)
    double platformPosTableTop[4] = {  0., parameterNode->GetTableTopLongitudinalPosition(), 0., 1. }; // origin in FixedReference transform
    double platformPosPatientSupport[4];
    // Get translation of the origin in a new table top coordinate
    tableTopToTableOriginTransform->MultiplyPoint( platformPosTableTop, platformPosPatientSupport);
    // Apply translation to TablePlatform system
    tablePlatformToPatientSupportTransform->Translate(platformPosPatientSupport);

    tablePlatformToPatientSupportTransformNode->SetAndObserveTransformToParent(tablePlatformToPatientSupportTransform);
  }
  
  this->UpdatePatientSupportToFixedReferenceTransform(parameterNode);
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdatePatientToTableTopTransform(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdatePatientToTableTopTransform: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("UpdatePatientToTableTopTransform: Invalid parameter node");
    return;
  }

  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* patientToTableTopTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::Patient, IHEP::TableTop);

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

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerIhepStandGeometryLogic::UpdateTableOriginMarkupsTransform(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateTableOriginMarkupsTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableOriginMarkupsTransform: Invalid MRML scene");
    return nullptr;
  }

  // Display all pieces of the treatment room and sets each piece a color to provide realistic representation
  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;

  // Transform IHEP stand models (IEC Patient) to RAS
  vtkNew<vtkTransform> patientToRasTransform;
  patientToRasTransform->Identity();
  patientToRasTransform->RotateX(-90.);
  if (!parameterNode->GetPatientHeadFeetRotation())
  {
    patientToRasTransform->RotateZ(180.);
  }

  // Transform path: RAS -> Patient -> TableTop -> TableTopOrigin

  // Find RasToTableTopOriginTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableTopOriginTransformNode;
  if (scene->GetFirstNodeByName("RasToTableTopOriginTransform"))
  {
    rasToTableTopOriginTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
      scene->GetFirstNodeByName("RasToTableTopOriginTransform"));
  }
  else
  {
    vtkNew<vtkTransform> rasToTableTopOriginTransform;
    if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::TableTopOrigin, 
      rasToTableTopOriginTransform, false))
    {
      // Transform to RAS, set transform to node, transform the model
      rasToTableTopOriginTransform->Concatenate(patientToRasTransform);

      rasToTableTopOriginTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      rasToTableTopOriginTransformNode->SetName("RasToTableTopOriginTransform");
//      rasToTableTopOriginTransformNode->SetHideFromEditors(1);
      std::string singletonTag = std::string("IHEP_") + "RasToTableTopOriginTransform";
      rasToTableTopOriginTransformNode->SetSingletonTag(singletonTag.c_str());

      scene->AddNode(rasToTableTopOriginTransformNode);
    }
    if (rasToTableTopOriginTransformNode)
    {
      rasToTableTopOriginTransformNode->SetAndObserveTransformToParent(rasToTableTopOriginTransform);
    }
  }
  return rasToTableTopOriginTransformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerIhepStandGeometryLogic::UpdateTableMirrorMarkupsTransform(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateTableMirrorMarkupsTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableMirrorMarkupsTransform: Invalid MRML scene");
    return nullptr;
  }

  // Display all pieces of the treatment room and sets each piece a color to provide realistic representation
  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;

  // Transform IHEP stand models (IEC Patient) to RAS
  vtkNew<vtkTransform> patientToRasTransform;
  patientToRasTransform->Identity();
  patientToRasTransform->RotateX(-90.);
  if (!parameterNode->GetPatientHeadFeetRotation())
  {
    patientToRasTransform->RotateZ(180.);
  }

  // Transform path: RAS -> Patient -> TableTop -> TableTopMirror

  // Find RasToTableTopMirrorTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableTopMirrorTransformNode;
  if (scene->GetFirstNodeByName("RasToTableTopMirrorTransform"))
  {
    rasToTableTopMirrorTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
      scene->GetFirstNodeByName("RasToTableTopMirrorTransform"));
  }
  else
  {
    vtkNew<vtkTransform> rasToTableTopMirrorTransform;
    if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::TableTopMirror, 
      rasToTableTopMirrorTransform, false))
    {
      // Transform to RAS, set transform to node, transform the model
      rasToTableTopMirrorTransform->Concatenate(patientToRasTransform);

      rasToTableTopMirrorTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      rasToTableTopMirrorTransformNode->SetName("RasToTableTopMirrorTransform");
//      rasToTableTopMirrorTransformNode->SetHideFromEditors(1);
      std::string singletonTag = std::string("IHEP_") + "RasToTableTopMirrorTransform";
      rasToTableTopMirrorTransformNode->SetSingletonTag(singletonTag.c_str());

      scene->AddNode(rasToTableTopMirrorTransformNode);
    }
    if (rasToTableTopMirrorTransformNode)
    {
      rasToTableTopMirrorTransformNode->SetAndObserveTransformToParent(rasToTableTopMirrorTransform);
    }
  }
  return rasToTableTopMirrorTransformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerIhepStandGeometryLogic::UpdateTableMiddleMarkupsTransform(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateTableMiddleMarkupsTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableMiddleMarkupsTransform: Invalid MRML scene");
    return nullptr;
  }

  // Display all pieces of the treatment room and sets each piece a color to provide realistic representation
  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;

  // Transform IHEP stand models (IEC Patient) to RAS
  vtkNew<vtkTransform> patientToRasTransform;
  patientToRasTransform->Identity();
  patientToRasTransform->RotateX(-90.);
  if (!parameterNode->GetPatientHeadFeetRotation())
  {
    patientToRasTransform->RotateZ(180.);
  }

  // Transform path: RAS -> Patient -> TableTop -> TableTopMiddle

  // Find RasToTableTopMiddleTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableTopMiddleTransformNode;
  if (scene->GetFirstNodeByName("RasToTableTopMiddleTransform"))
  {
    rasToTableTopMiddleTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
      scene->GetFirstNodeByName("RasToTableTopMiddleTransform"));
  }
  else
  {
    vtkNew<vtkTransform> rasToTableTopMiddleTransform;
    if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::TableTopMiddle, 
      rasToTableTopMiddleTransform, false))
    {
      // Transform to RAS, set transform to node, transform the model
      rasToTableTopMiddleTransform->Concatenate(patientToRasTransform);

      rasToTableTopMiddleTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      rasToTableTopMiddleTransformNode->SetName("RasToTableTopMiddleTransform");
//      rasToTableTopMiddleTransformNode->SetHideFromEditors(1);
      std::string singletonTag = std::string("IHEP_") + "RasToTableTopMiddleTransform";
      rasToTableTopMiddleTransformNode->SetSingletonTag(singletonTag.c_str());

      scene->AddNode(rasToTableTopMiddleTransformNode);
    }
    if (rasToTableTopMiddleTransformNode)
    {
      rasToTableTopMiddleTransformNode->SetAndObserveTransformToParent(rasToTableTopMiddleTransform);
    }
  }
  return rasToTableTopMiddleTransformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerIhepStandGeometryLogic::UpdateTableTopPlaneTransform(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateTableTopPlaneTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableTopPlaneTransform: Invalid MRML scene");
    return nullptr;
  }

  // Display all pieces of the treatment room and sets each piece a color to provide realistic representation
  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;

  // Transform IHEP stand models (IEC Patient) to RAS
  vtkNew<vtkTransform> patientToRasTransform;
  patientToRasTransform->Identity();
  patientToRasTransform->RotateX(-90.);
  if (!parameterNode->GetPatientHeadFeetRotation())
  {
    patientToRasTransform->RotateZ(180.);
  }

  // Transform path: RAS -> Patient -> TableTop

  // Find RasToTableTopMiddleTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableTopTransformNode;
  if (scene->GetFirstNodeByName("RasToTableTopTransform"))
  {
    rasToTableTopTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
      scene->GetFirstNodeByName("RasToTableTopTransform"));
  }
  else
  {
    vtkNew<vtkTransform> rasToTableTopTransform;
    if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::TableTop, 
      rasToTableTopTransform, false))
    {
      // Transform to RAS, set transform to node, transform the model
      rasToTableTopTransform->Concatenate(patientToRasTransform);

      rasToTableTopTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      rasToTableTopTransformNode->SetName("RasToTableTopTransform");
//      rasToTableTopMiddleTransformNode->SetHideFromEditors(1);
      std::string singletonTag = std::string("IHEP_") + "RasToTableTopTransform";
      rasToTableTopTransformNode->SetSingletonTag(singletonTag.c_str());

      scene->AddNode(rasToTableTopTransformNode);
    }
    if (rasToTableTopTransformNode)
    {
      rasToTableTopTransformNode->SetAndObserveTransformToParent(rasToTableTopTransform);
    }
  }
  return rasToTableTopTransformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerIhepStandGeometryLogic::UpdateFixedReferenceMarkupsTransform(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateFixedReferenceMarkupsTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateFixedReferenceMarkupsTransform: Invalid MRML scene");
    return nullptr;
  }

  // Display all pieces of the treatment room and sets each piece a color to provide realistic representation
  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;

  // Transform IHEP stand models (IEC Patient) to RAS
  vtkNew<vtkTransform> patientToRasTransform;
  patientToRasTransform->Identity();
  patientToRasTransform->RotateX(-90.);
  if (!parameterNode->GetPatientHeadFeetRotation())
  {
    patientToRasTransform->RotateZ(180.);
  }

  // Transformation path: RAS -> Patient -> TableTop -> TableTopOrigin -> TableTopSupport -> TablePlatform -> PatientSupport -> FixedReference

  // Find RasToFixedReferenceTransform or create it
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToFixedReferenceTransformNode;
  if (scene->GetFirstNodeByName("RasToFixedReferenceTransform"))
  {
    rasToFixedReferenceTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
      scene->GetFirstNodeByName("RasToFixedReferenceTransform"));
  }
  else
  {
    vtkNew<vtkTransform> rasToFixedReferenceTransform;
    if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::FixedReference, 
      rasToFixedReferenceTransform, false))
    {
      // Transform to RAS, set transform to node, transform the model
      rasToFixedReferenceTransform->Concatenate(patientToRasTransform);

      rasToFixedReferenceTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      rasToFixedReferenceTransformNode->SetName("RasToFixedReferenceTransform");
//      rasToFixedReferenceMiddleTransformNode->SetHideFromEditors(1);
      std::string singletonTag = std::string("IHEP_") + "RasToFixedReferenceTransform";
      rasToFixedReferenceTransformNode->SetSingletonTag(singletonTag.c_str());

      scene->AddNode(rasToFixedReferenceTransformNode);
    }
    if (rasToFixedReferenceTransformNode)
    {
      rasToFixedReferenceTransformNode->SetAndObserveTransformToParent(rasToFixedReferenceTransform);
    }
  }
  return rasToFixedReferenceTransformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerIhepStandGeometryLogic::GetFixedReferenceTransform()
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
vtkMRMLLinearTransformNode* vtkSlicerIhepStandGeometryLogic::GetPatientTransform()
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
vtkMRMLLinearTransformNode* vtkSlicerIhepStandGeometryLogic::GetTableTopTransform()
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
vtkMRMLLinearTransformNode* vtkSlicerIhepStandGeometryLogic::GetTableTopOriginTransform()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("GetTableTopOriginTransform: Invalid MRML scene");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToTableTopOriginTransform"))
  {
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }

  return transformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerIhepStandGeometryLogic::GetTableTopMiddleTransform()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("GetTableTopMiddleTransform: Invalid MRML scene");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToTableTopMiddleTransform"))
  {
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }

  return transformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerIhepStandGeometryLogic::GetTableTopMirrorTransform()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("GetTableTopMirrorTransform: Invalid MRML scene");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToTableTopMirrorTransform"))
  {
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }

  return transformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerIhepStandGeometryLogic::GetTableTopSupportTransform()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("GetTableTopSupportTransform: Invalid MRML scene");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToTableTopSupportTransform"))
  {
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }

  return transformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerIhepStandGeometryLogic::GetTablePlatformTransform()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("GetTablePlatformTransform: Invalid MRML scene");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToTablePlatformTransform"))
  {
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }

  return transformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerIhepStandGeometryLogic::GetPatientSupportTransform()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("GetPatientSupportTransform: Invalid MRML scene");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName("RasToPatientSupportTransform"))
  {
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  }

  return transformNode;
}

//------------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateFixedReferenceCamera(vtkMRMLIhepStandGeometryNode* parameterNode, 
  vtkMRMLCameraNode* cameraNode /* = nullptr */)
{
  if (!cameraNode && !this->Camera3DViewNode)
  {
    vtkErrorMacro("UpdateFixedReferenceCamera: Camera nodes are invalid");
    return;
  }

  if (cameraNode)
  {
    this->Camera3DViewNode = cameraNode;
  }

  if (!cameraNode && this->Camera3DViewNode)
  {
    vtkDebugMacro("UpdateFixedReferenceCamera: Use member camera node");
  }

  vtkMRMLTransformNode* fixedReferenceTransformNode = this->GetFixedReferenceTransform();
  if (fixedReferenceTransformNode)
  {
    vtkTransform* fixedReferenceTransform = nullptr;
    vtkNew<vtkMatrix4x4> mat;
    mat->Identity();

    fixedReferenceTransform = vtkTransform::SafeDownCast(fixedReferenceTransformNode->GetTransformToParent());
    if (fixedReferenceTransform)
    {
      fixedReferenceTransform->GetMatrix(mat);

      double viewUpVector[4] = { 0., 1., 0., 0. }; // positive Y-axis
      double vup[4];
  
      mat->MultiplyPoint( viewUpVector, vup);

      double fixedIsocenter[4] = { 0., 0., 0., 1. }; // origin in FixedReference transform
      double isocenterWorld[4];

      double sourcePosition[4] = { 0., 0., -4000., 1. }; // origin in FixedReference transform
      double sourcePositionWorld[4];

      mat->MultiplyPoint( fixedIsocenter, isocenterWorld);
      mat->MultiplyPoint( sourcePosition, sourcePositionWorld);

      this->Camera3DViewNode->GetCamera()->SetPosition(sourcePositionWorld);
      this->Camera3DViewNode->GetCamera()->SetFocalPoint(isocenterWorld);

      this->Camera3DViewNode->SetViewUp(vup);
    
      this->Camera3DViewNode->GetCamera()->Elevation(30.);
      this->Camera3DViewNode->GetCamera()->Azimuth(-45.);
    }
  }
}

//------------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::SetFixedReferenceCamera(vtkMRMLCameraNode* cameraNode)
{
  if (!cameraNode && !this->Camera3DViewNode)
  {
    vtkErrorMacro("SetFixedReferenceCamera: Both argument pointer and member pointer to camera node are invalid");
    return;
  }

  if (cameraNode)
  {
    vtkDebugMacro("SetFixedReferenceCamera: Set member camera node");
    this->Camera3DViewNode = cameraNode;
  }
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::CalculateMovementsForBeam(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();

  if (!scene)
  {
    vtkErrorMacro("SetupTreatmentMachineModels: Invalid scene");
    return;
  }
  
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateFixedReferenceMarkupsTransform: Invalid parameter node");
    return;
  }
  
  vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode();


  // Display all pieces of the treatment room and sets each piece a color to provide realistic representation
  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;


  vtkMRMLMarkupsLineNode* fixedReferenceLineNode = nullptr;

  // origin fiducial markups node
  if (scene->GetFirstNodeByName(FIXEDREFERENCE_MARKUPS_LINE_NODE_NAME))
  {
    fixedReferenceLineNode = vtkMRMLMarkupsLineNode::SafeDownCast(scene->GetFirstNodeByName(FIXEDREFERENCE_MARKUPS_LINE_NODE_NAME));
  }

  double* posBegin = fixedReferenceLineNode->GetNthControlPointPosition(0);
  double* posEnd = fixedReferenceLineNode->GetNthControlPointPosition(1);

  double posFixedReferenceBegin[4] = { posBegin[0], posBegin[1], posBegin[2], 1. };
  double posFixedReferenceEnd[4] = { posEnd[0], posEnd[1], posEnd[2], 1. };

  double posTableTopBegin[4] = {};
  double posTableTopEnd[4] = {};

  // table top plane origin point in table top support frame
  this->IhepLogic->GetTransformForPointThroughtRAS( IHEP::FixedReference, 
    IHEP::TableTop, posFixedReferenceBegin, posTableTopBegin);

  // table top plane point-1 in table top support frame
  this->IhepLogic->GetTransformForPointThroughtRAS( IHEP::FixedReference, 
    IHEP::TableTop, posFixedReferenceEnd, posTableTopEnd);

  // Get isocenter position in table top frame
  
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::CreateFixedBeamPlanAndNode(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();

  if (!scene)
  {
    vtkErrorMacro("CreateFixedBeamPlanAndNode: Invalid scene");
    return;
  }

  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(scene);
  if (!shNode)
  {
    vtkErrorMacro("CreateFixedBeamPlanAndNode: Failed to access subject hierarchy node");
    return;
  }

  if (!parameterNode)
  {
    vtkErrorMacro("CreateFixedBeamPlanAndNode: Invalid parameter node");
    return;
  }
  vtkMRMLRTPlanNode* fixedPlanNode = vtkMRMLRTPlanNode::SafeDownCast(scene->AddNewNodeByClass( "vtkMRMLRTPlanNode", "FixedPlan"));
  fixedPlanNode->SetIonPlanFlag(true);
  // Create beam and add to scene
  vtkSmartPointer<vtkMRMLRTBeamNode> beamNode;
  if (fixedPlanNode->GetIonPlanFlag())
  {
    beamNode = vtkSmartPointer<vtkMRMLRTFixedIonBeamNode>::New();
  }
  else
  {
    beamNode = vtkSmartPointer<vtkMRMLRTBeamNode>::New();
  }

  vtkMRMLMarkupsFiducialNode* fixedIsocenterNode = nullptr;

  // fixed isocenter fiducial markups node
  if (scene->GetFirstNodeByName(FIXEDISOCENTER_MARKUPS_FIDUCIAL_NODE_NAME))
  {
    fixedIsocenterNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(scene->GetFirstNodeByName(FIXEDISOCENTER_MARKUPS_FIDUCIAL_NODE_NAME));
  }
  beamNode->SetName(fixedPlanNode->GenerateNewBeamName().c_str());
  fixedPlanNode->GetScene()->AddNode(beamNode);
  fixedPlanNode->AddBeam(beamNode);
  if (fixedIsocenterNode)
  {
    // Get fixed plan and isocenter Subject Hierarchy ID
    vtkIdType fixedIsocenterShId = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    vtkIdType fixedPlanShId = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    // set fixed plan ID as a parent of fixed isocenter
    fixedPlanShId = shNode->GetItemByDataNode(fixedPlanNode);
    fixedIsocenterShId = shNode->GetItemByDataNode(fixedIsocenterNode);
    if (fixedPlanShId != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID && 
      fixedIsocenterShId != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
      shNode->SetItemParent( fixedIsocenterShId, fixedPlanShId);
    }

    fixedPlanNode->SetAndObservePoisMarkupsFiducialNode(fixedIsocenterNode);
  }

  vtkMRMLTransformNode* beamTranfsormNode = beamNode->GetParentTransformNode();
  // Find RasToFixedReferenceTransform or create it
  vtkMRMLLinearTransformNode* rasToFixedReferenceTransformNode = nullptr;
  if (scene->GetFirstNodeByName("RasToFixedReferenceTransform"))
  {
    rasToFixedReferenceTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
      scene->GetFirstNodeByName("RasToFixedReferenceTransform"));
  }
  if (beamTranfsormNode && rasToFixedReferenceTransformNode)
  {
    beamTranfsormNode->SetAndObserveTransformNodeID( rasToFixedReferenceTransformNode->GetID() );
  }
}

//----------------------------------------------------------------------------
bool vtkSlicerIhepStandGeometryLogic::GetTableTopCenterToFixedIsocenterTranslate( vtkMRMLIhepStandGeometryNode* parameterNode, double* translateTableTopFrame)
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("GetTableTopCenterToFixedIsocenterTranslate: Invalid MRML scene");
    return false;
  }

  if (!parameterNode)
  {
    vtkErrorMacro("GetTableTopCenterToFixedIsocenterTranslate: Invalid parameter node");
    return false;
  }

//  double PatientTableTopTranslation[3] = {};
  // Translation to the origin of fixed reference and patient
//  double originTranslation[3] = { -1. * parameterNode->GetTableTopLateralPosition(), parameterNode->GetTableTopLongitudinalPosition(), parameterNode->GetTableTopVerticalPosition() }; // translation of fixed reference to RAS
//  parameterNode->GetPatientToTableTopTranslation(PatientTableTopTranslation);

  // Transform IHEP stand models (IEC Patient) to RAS
  vtkNew<vtkTransform> patientToRasTransform;
  patientToRasTransform->Identity();
  patientToRasTransform->RotateX(-90.);
  if (!parameterNode->GetPatientHeadFeetRotation())
  {
    patientToRasTransform->RotateZ(180.);
  }

  double tableTopCenter[4] = { TableTopCenterFixedReference[0], TableTopCenterFixedReference[1], TableTopCenterFixedReference[2], 1. };
  double tableTopCenterRAS[4] = { 0., 0., 0., 1. };
//  double tableTopCenterTableTop[4] = { 0., 0., 0., 1. };
//  double tableTopCenterFixedReference[4] = { 0., 0., 0., 1. };
  
  patientToRasTransform->MultiplyPoint( tableTopCenter, tableTopCenterRAS);

  vtkWarningMacro("GetTableTopCenterToFixedIsocenterTranslate: TableTop center in TableTop " << tableTopCenterRAS[0] << " " << tableTopCenterRAS[1] << " " << tableTopCenterRAS[2]);
/*
  vtkMRMLTransformNode* tableTopTransformNode = this->GetTableTopTransform();
  if (tableTopTransformNode)
  {
    vtkTransform* tableTopTransform = nullptr;
    vtkNew<vtkMatrix4x4> mat;

    tableTopTransform = vtkTransform::SafeDownCast(tableTopTransformNode->GetTransformToParent());
    if (tableTopTransform)
    {
      tableTopTransform->GetMatrix(mat);

      mat->Invert(); // table top to ras
      mat->MultiplyPoint( tableTopCenterRAS, tableTopCenter);
      vtkWarningMacro("GetTableTopCenterToFixedIsocenterTranslate: " << tableTopCenter[0] << " " << tableTopCenter[1] << " " << tableTopCenter[2]);
    }
  }
*/
/*
  double FixedReferenceOrigin[3] = { 0., 0., -1. * parameterNode->GetTableTopLongitudinalPosition() + tableTopCenterRAS[2] };
  double FixedReferenceTableTopNoOffset[3] = {};
  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;
  this->IhepLogic->GetTransformForPointThroughtRAS( IHEP::FixedReference, IHEP::RAS, FixedReferenceOrigin, FixedReferenceTableTopNoOffset);
  vtkWarningMacro("GetTableTopCenterToFixedIsocenterTranslate: FixedReference in TableTop no offset " << FixedReferenceTableTopNoOffset[0] << " " << FixedReferenceTableTopNoOffset[1] << " " << FixedReferenceTableTopNoOffset[2]);
  double fixedInTableTop1[4] = { FixedReferenceTableTopNoOffset[0], FixedReferenceTableTopNoOffset[1], FixedReferenceTableTopNoOffset[2], 1. };
  double fixedInTableTop2[4] = { };
  patientToRasTransform->MultiplyPoint( fixedInTableTop1, fixedInTableTop2);
  vtkWarningMacro("GetTableTopCenterToFixedIsocenterTranslate: FixedReference in TableTop no offset (RAS) " << fixedInTableTop2[0] << " " << fixedInTableTop2[1] << " " << fixedInTableTop2[2]);
*/

  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;

  vtkNew<vtkTransform> rasToFixedReferenceTransform;
  double rasOrigin[4] = { 0., 0., 0., 1. };
  if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::FixedReference, rasToFixedReferenceTransform, false))
  {
    double rasFixedReferenceNoOffset[4] = {};
    rasToFixedReferenceTransform->MultiplyPoint( rasOrigin, rasFixedReferenceNoOffset);
    vtkWarningMacro("GetTableTopCenterToFixedIsocenterTranslate: RAS->FixedReference " << rasFixedReferenceNoOffset[0] << " " << rasFixedReferenceNoOffset[1] << " " << rasFixedReferenceNoOffset[2]);
  }
/*
  vtkNew<vtkTransform> rasToTableTopTransform;
  if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::TableTop, rasToTableTopTransform, false))
  {
    double rasTableTopNoOffset[4] = {};
    rasToTableTopTransform->MultiplyPoint( rasOrigin, rasTableTopNoOffset);
    vtkWarningMacro("GetTableTopCenterToFixedIsocenterTranslate: RAS->TableTop " << rasTableTopNoOffset[0] << " " << rasTableTopNoOffset[1] << " " << rasTableTopNoOffset[2]);
  }

  vtkNew<vtkTransform> fixedReferenceToTableTopTransform;
  double fixedReferenceToTableTopNoOffset[4] = {};
  if (this->IhepLogic->GetTransformBetween( IHEP::FixedReference, IHEP::TableTop, fixedReferenceToTableTopTransform, false))
  {
    fixedReferenceToTableTopTransform->MultiplyPoint( rasOrigin, fixedReferenceToTableTopNoOffset);
    vtkWarningMacro("GetTableTopCenterToFixedIsocenterTranslate: FixedReference->TableTop " << fixedReferenceToTableTopNoOffset[0] << " " << fixedReferenceToTableTopNoOffset[1] << " " << fixedReferenceToTableTopNoOffset[2]);
  }
*/

  double fixedReferenceToTableTopNoOffset[4] = {};
  if (this->IhepLogic->GetTransformPointThroughtRAS( IHEP::FixedReference, IHEP::TableTop, rasOrigin, fixedReferenceToTableTopNoOffset, false))
  {
    vtkWarningMacro("GetTableTopCenterToFixedIsocenterTranslate: FixedReference->TableTop on offset " << fixedReferenceToTableTopNoOffset[0] << " " << fixedReferenceToTableTopNoOffset[1] << " " << fixedReferenceToTableTopNoOffset[2]);
  }
  fixedReferenceToTableTopNoOffset[0] -= tableTopCenterRAS[0];
  fixedReferenceToTableTopNoOffset[1] -= tableTopCenterRAS[1];
  fixedReferenceToTableTopNoOffset[2] -= tableTopCenterRAS[2];
  vtkWarningMacro("GetTableTopCenterToFixedIsocenterTranslate: FixedReference->TableTop with offset " << fixedReferenceToTableTopNoOffset[0] << " " << fixedReferenceToTableTopNoOffset[1] << " " << fixedReferenceToTableTopNoOffset[2]);

  translateTableTopFrame[0] = fixedReferenceToTableTopNoOffset[0];
  translateTableTopFrame[1] = fixedReferenceToTableTopNoOffset[1];
  translateTableTopFrame[2] = fixedReferenceToTableTopNoOffset[2];

/*
  double fixedReferenceToPatientNoOffset[4] = {};
  if (this->IhepLogic->GetTransformPointThroughtRAS( IHEP::FixedReference, IHEP::Patient, rasOrigin, fixedReferenceToPatientNoOffset, false))
  {
    vtkWarningMacro("GetTableTopCenterToFixedIsocenterTranslate: FixedReference->Patient on offset " << fixedReferenceToPatientNoOffset[0] << " " << fixedReferenceToPatientNoOffset[1] << " " << fixedReferenceToPatientNoOffset[2]);
  }
*/
/*
  vtkWarningMacro("GetTableTopCenterToFixedIsocenterTranslate: TableTopCenter in FixedReference " << tableTopCenterFixedReference[0] << " " << tableTopCenterFixedReference[1] << " " << tableTopCenterFixedReference[2]);

  this->IhepLogic->GetTransformForPointThroughtRAS( IHEP::TableTop, 
    IHEP::RAS, tableTopCenterRAS, tableTopCenterTableTop);
  vtkWarningMacro("GetTableTopCenterToFixedIsocenterTranslate: TableTopCenter in TableTop " << tableTopCenterTableTop[0] << " " << tableTopCenterTableTop[1] << " " << tableTopCenterTableTop[2]);
*/
/*
  double fixedReferenceCenter[4] = { 0., 0., 0., 1. };
  double fixedReferenceRAS[4] = { };
  vtkMRMLTransformNode* fixedReferenceTransformNode = this->GetFixedReferenceTransform();
  if (fixedReferenceTransformNode)
  {
    vtkTransform* fixedReferenceTransform = nullptr;
    vtkNew<vtkMatrix4x4> mat;

    fixedReferenceTransform = vtkTransform::SafeDownCast(fixedReferenceTransformNode->GetTransformToParent());
    if (fixedReferenceTransform)
    {
      fixedReferenceTransform->GetMatrix(mat);

      mat->Invert(); // fixed reference to ras
      mat->MultiplyPoint( fixedReferenceCenter, fixedReferenceRAS);
      vtkWarningMacro("GetTableTopCenterToFixedIsocenterTranslate: FixedReferenceOrigin in RAS " << fixedReferenceRAS[0] << " " << fixedReferenceRAS[1] << " " << fixedReferenceRAS[2]);
    }
  }

  vtkMRMLTransformNode* tableTopTransformNode = this->GetTableTopTransform();
  if (tableTopTransformNode)
  {
    vtkTransform* tableTopTransform = nullptr;
    vtkNew<vtkMatrix4x4> mat;

    tableTopTransform = vtkTransform::SafeDownCast(tableTopTransformNode->GetTransformToParent());
    if (tableTopTransform)
    {
      tableTopTransform->GetMatrix(mat);

      double fixedReferenceTableTop[4] = { };
      mat->MultiplyPoint( fixedReferenceRAS, fixedReferenceTableTop);
      vtkWarningMacro("GetTableTopCenterToFixedIsocenterTranslate: FixedReferenceOrigin in TableTop " << fixedReferenceTableTop[0] << " " << fixedReferenceTableTop[1] << " " << fixedReferenceTableTop[2]);
    }
  }
*/
/*
  // Transform IHEP stand models (IEC Patient) to RAS
  vtkNew<vtkTransform> patientToRasTransform;
  patientToRasTransform->Identity();
  patientToRasTransform->RotateX(-90.);
  if (!parameterNode->GetPatientHeadFeetRotation())
  {
    patientToRasTransform->RotateZ(180.);
  }

  double tableTopCenter[4] = { TableTopCenterFixedReference[0], TableTopCenterFixedReference[1], TableTopCenterFixedReference[2], 1. };
  double tableTopCenterRAS[4] = { };
  patientToRasTransform->MultiplyPoint( tableTopCenter, tableTopCenterRAS);

  vtkWarningMacro("GetTableTopCenterToFixedIsocenterTranslate: TableTop center in TableTop " << tableTopCenterRAS[0] << " " << tableTopCenterRAS[1] << " " << tableTopCenterRAS[2]);

  vtkMRMLTransformNode* tableTopTransformNode = this->GetTableTopTransform();
  if (tableTopTransformNode)
  {
    vtkTransform* tableTopTransform = nullptr;
    vtkNew<vtkMatrix4x4> mat;

    tableTopTransform = vtkTransform::SafeDownCast(tableTopTransformNode->GetTransformToParent());
    if (tableTopTransform)
    {
      tableTopTransform->GetMatrix(mat);

      mat->Invert(); // table top to ras
      mat->MultiplyPoint( tableTopCenterRAS, tableTopCenter);
      vtkWarningMacro("GetTableTopCenterToFixedIsocenterTranslate: " << tableTopCenter[0] << " " << tableTopCenter[1] << " " << tableTopCenter[2]);
    }
  }
*/

/*
  double fixedReferenceIsocenter[4] = { 0., 0., 0., 1. };
  double fixedReferenceIsocenterTableTop[4] = { };

  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;

  // table top center point in table top frame
  this->IhepLogic->GetTransformForPointThroughtRAS( IHEP::FixedReference, 
    IHEP::TableTop, fixedReferenceIsocenter, fixedReferenceIsocenterTableTop);

  vtkWarningMacro("GetTableTopCenterToFixedIsocenterTranslate: FixedReference isocenter in TableTop " << fixedReferenceIsocenterTableTop[0] << " " << fixedReferenceIsocenterTableTop[1] << " " << fixedReferenceIsocenterTableTop[2]);
*/
/*
  double translate[3] = {
    fixedReferenceIsocenterTableTop[0] - tableTopCenterRAS[0],
    fixedReferenceIsocenterTableTop[1] - tableTopCenterRAS[1],
    fixedReferenceIsocenterTableTop[2] - tableTopCenterRAS[2] };

  vtkWarningMacro("GetTableTopCenterToFixedIsocenterTranslate: " << translate[0] << " " << translate[1] << " " << translate[2]);
*/
//  translateTableTopFrame[0] = translate[0];
//  translateTableTopFrame[1] = translate[1];
//  translateTableTopFrame[2] = translate[2];

/*
  vtkMRMLMarkupsPlaneNode* tableTopPlaneNode = nullptr;

  // table top plane markups node
  if (scene->GetFirstNodeByName(TABLETOP_MARKUPS_PLANE_NODE_NAME))
  {
    tableTopPlaneNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(scene->GetFirstNodeByName(TABLETOP_MARKUPS_PLANE_NODE_NAME));
  }

  double tableTopOrigin[4] = { 0., 0., 0., 1. };
  double tableTopRAS[4] = { };
  double tableTopFixedReference[4] = { };

  if (tableTopPlaneNode)
  {
    double* planeOrigin = tableTopPlaneNode->GetOrigin();
    double planeOriginTableTop[4] = { planeOrigin[0], planeOrigin[1], planeOrigin[2], 1. };
    double planeOriginFixedReference[4] = {};

    using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;
    // table top center point in table top support frame
    this->IhepLogic->GetTransformForPointThroughtRAS( IHEP::TableTop, 
      IHEP::RAS, planeOriginTableTop, planeOriginFixedReference);
    
//    vtkWarningMacro("GetTableTopCenterToFixedIsocenterTranslate: " << planeOriginTableTop[0] << " " << planeOriginTableTop[1] << " " << planeOriginTableTop[2]);
//    vtkWarningMacro("GetTableTopCenterToFixedIsocenterTranslate: " << planeOriginFixedReference[0] << " " << planeOriginFixedReference[1] << " " << planeOriginFixedReference[2]);

    vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableTopTransformNode;
    vtkTransform* rasToTableTopTransform = nullptr;
    vtkNew<vtkMatrix4x4> rasToTableTopMatrix;
    if (scene->GetFirstNodeByName("RasToTableTopTransform"))
    {
      rasToTableTopTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
        scene->GetFirstNodeByName("RasToTableTopTransform"));
      rasToTableTopTransform = vtkTransform::SafeDownCast(rasToTableTopTransformNode->GetTransformToParent());
    }
    if (rasToTableTopTransform)
    {
      rasToTableTopTransform->GetMatrix(rasToTableTopMatrix);
    }
    rasToTableTopMatrix->Invert();
    rasToTableTopMatrix->MultiplyPoint( tableTopOrigin, tableTopRAS);
//    vtkWarningMacro("GetTableTopCenterToFixedIsocenterTranslate: tableTopCenterRAS " << tableTopRAS[0] << " " << tableTopRAS[1] << " " << tableTopRAS[2]);
  }
*/
/*
  vtkMRMLMarkupsPlaneNode* tableTopPlaneNode = nullptr;

  // table top plane markups node
  if (scene->GetFirstNodeByName(TABLETOP_MARKUPS_PLANE_NODE_NAME))
  {
    tableTopPlaneNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(scene->GetFirstNodeByName(TABLETOP_MARKUPS_PLANE_NODE_NAME));
  }

  double tableTopOriginWorld[4] = { 0., 0., 0., 1. };
  double tableTopOriginTableTop[4] = { 0., 0., 0., 1. };
  if (tableTopPlaneNode)
  {
    tableTopPlaneNode->GetOriginWorld(tableTopOriginWorld);
  }

  double fixedReferenceLocal[4] = { 0., 0., 0., 1. };
  double fixedReferenceWorld[4] = { };
  double fixedReferenceTableTop[4] = { };

  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToFixedReferenceTransformNode;
  vtkTransform* rasToFixedReferenceTransform = nullptr;
  vtkNew<vtkMatrix4x4> rasToFixedReferenceMatrix;
  if (scene->GetFirstNodeByName("RasToFixedReferenceTransform"))
  {
    rasToFixedReferenceTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
      scene->GetFirstNodeByName("RasToFixedReferenceTransform"));
//    rasToFixedReferenceTransform = vtkTransform::SafeDownCast(rasToFixedReferenceTransformNode->GetTransformToWorld(rasToFixedReferenceMatrix));
  }
  if (rasToFixedReferenceTransformNode)
  {
    rasToFixedReferenceTransformNode->GetMatrixTransformToWorld(rasToFixedReferenceMatrix);
  }
  rasToFixedReferenceMatrix->MultiplyPoint( fixedReferenceLocal, fixedReferenceWorld);
  vtkWarningMacro("GetTableTopCenterToFixedIsocenterTranslate: FixedReference in World " << fixedReferenceWorld[0] << " " << fixedReferenceWorld[1] << " " << fixedReferenceWorld[2]);

  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableTopTransformNode;
  vtkTransform* rasToTableTopTransform = nullptr;
  vtkNew<vtkMatrix4x4> rasToTableTopMatrix;
  if (scene->GetFirstNodeByName("RasToTableTopTransform"))
  {
    vtkErrorMacro("GetTableTopCenterToFixedIsocenterTranslate: 1");
    rasToTableTopTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
      scene->GetFirstNodeByName("RasToTableTopTransform"));
//    rasToTableTopTransform = vtkTransform::SafeDownCast(rasToTableTopTransformNode->GetTransformFromWorld());
  }
  if (rasToTableTopTransformNode)
  {
    vtkErrorMacro("GetTableTopCenterToFixedIsocenterTranslate: 2");
    rasToTableTopTransformNode->GetMatrixTransformToWorld(rasToTableTopMatrix);
  }
  rasToTableTopMatrix->Invert();
  rasToTableTopMatrix->MultiplyPoint( tableTopOriginWorld, tableTopOriginTableTop);
  vtkWarningMacro("GetTableTopCenterToFixedIsocenterTranslate: TableTopOrigin in TableTop " << tableTopOriginTableTop[0] << " " << tableTopOriginTableTop[1] << " " << tableTopOriginTableTop[2]);
*/
//  rasToFixedReferenceMatrix->Invert(); // World to fixed reference
//  rasToFixedReferenceMatrix->MultiplyPoint( tableTopOriginWorld, fixedReferenceWorld);
/*
  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;

  // Transform IHEP stand models (IEC Patient) to RAS
  vtkNew<vtkMatrix4x4> patientToRasMatrix;
  vtkNew<vtkTransform> patientToRasTransform;
  patientToRasTransform->Identity();
  patientToRasTransform->RotateX(-90.);
  if (!parameterNode->GetPatientHeadFeetRotation())
  {
    patientToRasTransform->RotateZ(180.);
  }
  patientToRasTransform->GetMatrix(patientToRasMatrix);

  double tableTopCenter[4] = { TableTopCenterFixedReference[0], TableTopCenterFixedReference[1], TableTopCenterFixedReference[2], 1. };
  double tableTopCenterRAS[4] = { };

//  patientToRasMatrix->MultiplyPoint( tableTopCenter, tableTopCenterRAS);

  double tableTopCenterRAS1[4] = { };
  this->IhepLogic->GetTransformForPointThroughtRAS( IHEP::FixedReference,
    IHEP::TableTop, tableTopCenter, tableTopCenterRAS);

  vtkWarningMacro("GetTableTopCenterToFixedIsocenterTranslate: " << tableTopCenterRAS[0] << " " << tableTopCenterRAS[1] << " " << tableTopCenterRAS[2]);
//  vtkWarningMacro("GetTableTopCenterToFixedIsocenterTranslate: " << tableTopCenterRAS1[0] << " " << tableTopCenterRAS1[1] << " " << tableTopCenterRAS1[2]);
*/
/*
  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableTopTransformNode;
  vtkTransform* rasToTableTopTransform = nullptr;
  vtkNew<vtkMatrix4x4> rasToTableMatrix;
  if (scene->GetFirstNodeByName("RasToTableTopTransform"))
  {
    rasToTableTopTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
      scene->GetFirstNodeByName("RasToTableTopTransform"));
    rasToTableTopTransform = vtkTransform::SafeDownCast(rasToTableTopTransformNode->GetTransformToParent());
  }
  if (rasToTableTopTransform)
  {
    rasToTableTopTransform->GetMatrix(rasToTableMatrix);
  }
  double tableTopCenter[4] = { 0., 0., 0., 1. };
  double tableTopCenterRAS[4] = { };
  rasToTableTopTransform->MultiplyPoint( tableTopCenter, tableTopCenterRAS);
  vtkWarningMacro("GetTableTopCenterToFixedIsocenterTranslate: tableTopCenterRAS " << tableTopCenterRAS[0] << " " << tableTopCenterRAS[1] << " " << tableTopCenterRAS[2]);

  vtkSmartPointer<vtkMRMLLinearTransformNode> rasToFixedReferenceTransformNode;
  vtkTransform* rasToFixedReferenceTransform = nullptr;
  vtkNew<vtkMatrix4x4> rasToFixedReferenceMatrix;
  if (scene->GetFirstNodeByName("RasToFixedReferenceTransform"))
  {
    rasToFixedReferenceTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
      scene->GetFirstNodeByName("RasToFixedReferenceTransform"));
    rasToFixedReferenceTransform = vtkTransform::SafeDownCast(rasToFixedReferenceTransformNode->GetTransformToParent());
  }
  if (rasToFixedReferenceTransform)
  {
    rasToFixedReferenceTransform->GetMatrix(rasToFixedReferenceMatrix);
  }
  double fixedIsocenter[4] = { 0., 0., 0., 1. };
  double fixedIsocenterRAS[4] = { };
  rasToFixedReferenceTransform->MultiplyPoint( fixedIsocenter, fixedIsocenterRAS);
  vtkWarningMacro("GetTableTopCenterToFixedIsocenterTranslate: fixedIsocenterRAS " << fixedIsocenterRAS[0] << " " << fixedIsocenterRAS[1] << " " << fixedIsocenterRAS[2]);
*/
/*
  // Transform IHEP stand models (IEC Patient) to RAS
  vtkNew<vtkMatrix4x4> patientToRasMatrix;
  vtkNew<vtkTransform> patientToRasTransform;
  patientToRasTransform->Identity();
  patientToRasTransform->RotateX(-90.);
  if (!parameterNode->GetPatientHeadFeetRotation())
  {
    patientToRasTransform->RotateZ(180.);
  }
  patientToRasTransform->GetMatrix(patientToRasMatrix);

  double tableTopCenter[4] = { TableTopCenterFixedReference[0], TableTopCenterFixedReference[1], TableTopCenterFixedReference[2], 1. };
  double tableTopCenterRAS[4] = { };
  patientToRasMatrix->MultiplyPoint( tableTopCenter, tableTopCenterRAS);

  vtkMRMLMarkupsPlaneNode* tableTopPlaneNode = nullptr;

  // table top plane markups node
  if (scene->GetFirstNodeByName(TABLETOP_MARKUPS_PLANE_NODE_NAME))
  {
    tableTopPlaneNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(scene->GetFirstNodeByName(TABLETOP_MARKUPS_PLANE_NODE_NAME));
  }

  // Display all pieces of the treatment room and sets each piece a color to provide realistic representation
  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;

  vtkMRMLMarkupsLineNode* fixedReferenceLineNode = nullptr;

  // origin fiducial markups node
  if (scene->GetFirstNodeByName(FIXEDREFERENCE_MARKUPS_LINE_NODE_NAME))
  {
    fixedReferenceLineNode = vtkMRMLMarkupsLineNode::SafeDownCast(scene->GetFirstNodeByName(FIXEDREFERENCE_MARKUPS_LINE_NODE_NAME));
  }

  double fixedIsocenter[4] = { 0., 0., 0., 1. };
  double fixedIsocenterRAS[4] = { };
  double fixedIsocenterRAS1[4] = { };

  vtkMRMLTransformNode* fixedReferenceTransformNode = this->GetFixedReferenceTransform();
  if (fixedReferenceTransformNode)
  {
    vtkTransform* fixedReferenceTransform = nullptr;
    vtkNew<vtkMatrix4x4> mat;

    fixedReferenceTransform = vtkTransform::SafeDownCast(fixedReferenceTransformNode->GetTransformToParent());
    if (fixedReferenceTransform)
    {
      fixedReferenceTransform->GetMatrix(mat);

      mat->MultiplyPoint( fixedIsocenter, fixedIsocenterRAS);
//      vtkWarningMacro("GetTableTopCenterToFixedIsocenterTranslate: " << fixedIsocenterRAS[0] << " " << fixedIsocenterRAS[1] << " " << fixedIsocenterRAS[2]);
    }
  }

  // fixed isocenter point in table top frame
  this->IhepLogic->GetTransformForPointThroughtRAS( IHEP::FixedReference, 
    IHEP::TableTop, fixedIsocenter, fixedIsocenterRAS);

  this->IhepLogic->GetTransformForPointThroughtRAS( IHEP::RAS, 
    IHEP::TableTop, fixedIsocenter, fixedIsocenterRAS);

//  double tableTopCenter[4] = { TableTopCenterFixedReference[0], TableTopCenterFixedReference[1], TableTopCenterFixedReference[2], 1. };
//  double tableTopCenterRAS[4] = { };

  // table top center point in table top frame
//  this->IhepLogic->GetTransformForPointThroughtRAS( IHEP::RAS, 
//    IHEP::TableTop, tableTopCenter, tableTopCenterRAS);

  double translate[3] = {
    fixedIsocenterRAS[0] - tableTopCenterRAS[0],
    fixedIsocenterRAS[1] - tableTopCenterRAS[1],
    fixedIsocenterRAS[2] - tableTopCenterRAS[2] };

  vtkWarningMacro("GetTableTopCenterToFixedIsocenterTranslate: " << fixedIsocenterRAS[0] << " " << fixedIsocenterRAS[1] << " " << fixedIsocenterRAS[2]);
//  vtkWarningMacro("GetTableTopCenterToFixedIsocenterTranslate: " << translate[0] << " " << translate[1] << " " << translate[2]);

//  translateTableTopFrame[0] = translate[0];
//  translateTableTopFrame[1] = translate[1];
//  translateTableTopFrame[2] = translate[2];
*/
  return true;
}

//----------------------------------------------------------------------------
bool vtkSlicerIhepStandGeometryLogic::GetPatientIsocenterToFixedIsocenterTranslate(vtkMRMLIhepStandGeometryNode* parameterNode, double* translatePatientFrame)
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("GetPatientIsocenterToFixedIsocenterTranslate: Invalid MRML scene");
    return false;
  }

  if (!parameterNode)
  {
    vtkErrorMacro("GetPatientIsocenterToFixedIsocenterTranslate: Invalid parameter node");
    return false;
  }

  vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode();

  if (!beamNode)
  {
    vtkErrorMacro("GetPatientIsocenterToFixedIsocenterTranslate: Invalid beam node");
    return false;
  }

  double planIsocenterRAS[3];
  if (!beamNode->GetPlanIsocenterPosition(planIsocenterRAS))
  {
    return false;
  }

  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;

  double fixedReferenceOrigin[4] = { 0., 0., 0., 1. };
  double fixedReferenceToPatientNoOffset[4] = {};
  if (this->IhepLogic->GetTransformPointThroughtRAS( IHEP::FixedReference, IHEP::Patient, fixedReferenceOrigin, fixedReferenceToPatientNoOffset, false))
  {
    vtkWarningMacro("GetPatientIsocenterToFixedIsocenterTranslate: FixedReference->Patient on offset " << fixedReferenceToPatientNoOffset[0] << " " << fixedReferenceToPatientNoOffset[1] << " " << fixedReferenceToPatientNoOffset[2]);
  }
  fixedReferenceToPatientNoOffset[0] -= planIsocenterRAS[0];
  fixedReferenceToPatientNoOffset[1] -= planIsocenterRAS[1];
  fixedReferenceToPatientNoOffset[2] -= planIsocenterRAS[2];
  vtkWarningMacro("GetPatientIsocenterToFixedIsocenterTranslate: FixedReference->Patient with offset " << fixedReferenceToPatientNoOffset[0] << " " << fixedReferenceToPatientNoOffset[1] << " " << fixedReferenceToPatientNoOffset[2]);

  translatePatientFrame[0] = fixedReferenceToPatientNoOffset[0];
  translatePatientFrame[1] = fixedReferenceToPatientNoOffset[1];
  translatePatientFrame[2] = fixedReferenceToPatientNoOffset[2];

  return true;
}

//----------------------------------------------------------------------------
bool vtkSlicerIhepStandGeometryLogic::GetPatientBeamToFixedBeamTransform(vtkMRMLIhepStandGeometryNode* parameterNode, vtkMRMLRTBeamNode* drrBeam, vtkMRMLRTFixedIonBeamNode* fixedBeam, vtkTransform* transform)
{
  return true;
}


//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::CalculateTableTopTranslation( vtkMRMLIhepStandGeometryNode* parameterNode, const double patientIsocenter[3], const double fixedIsocenter[3])
{
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::CalculateTransformBetweenBeams( vtkMRMLIhepStandGeometryNode* parameterNode, vtkMRMLRTBeamNode* patientBeam, vtkMRMLRTBeamNode* fixedBeam)
{
}
