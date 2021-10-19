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

//----------------------------------------------------------------------------
// Treatment machine component names
const char* vtkSlicerIhepStandGeometryLogic::FIXEDREFERENCE_MODEL_NAME = "FixedReference";
const char* vtkSlicerIhepStandGeometryLogic::PATIENTSUPPORT_MODEL_NAME = "PatientSupportRotationNew";

const char* vtkSlicerIhepStandGeometryLogic::TABLE_LONGITUDINAL_MODEL_NAME = "PatientSupportStandY";
const char* vtkSlicerIhepStandGeometryLogic::TABLE_LATERAL_MODEL_NAME = "TableTopStand";
const char* vtkSlicerIhepStandGeometryLogic::TABLE_ORIGIN_MODEL_NAME = "TableTopStandOrigin";
const char* vtkSlicerIhepStandGeometryLogic::TABLE_MIRROR_MODEL_NAME = "TableTopStandMirror";
const char* vtkSlicerIhepStandGeometryLogic::TABLE_MIDDLE_MODEL_NAME = "TableTopStandMiddle";
  
const char* vtkSlicerIhepStandGeometryLogic::TABLETOP_MODEL_NAME = "TableTop";
const char* vtkSlicerIhepStandGeometryLogic::TABLETOP_PLANE_MARKUPS_NODE_NAME = "TableTopMarkupsPlane";

const char* vtkSlicerIhepStandGeometryLogic::TABLE_FIDUCIALS_MARKUPS_NODE_NAME = "TableMarkupsFiducials";
const char* vtkSlicerIhepStandGeometryLogic::TABLE_FIDUCIALS_TRANSFORM_NODE_NAME = "TableMarkupsFiducialsTransform";

const char* vtkSlicerIhepStandGeometryLogic::FIXEDREFERENCE_LINE_MARKUPS_NODE_NAME = "FixedReferenceMarkupsLine";
const char* vtkSlicerIhepStandGeometryLogic::FIXEDREFERENCE_LINE_TRANSFORM_NODE_NAME = "FixedReferenceMarkupsLineTransform";

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

  // Update TableTop -> TableOrigin
  // Transformation (translation) of the TableTop to TableOrigin
  vtkMRMLLinearTransformNode* tableTopToTableOriginTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableTop, IHEP::TableOriginVerticalMovement);

  vtkTransform* tableTopToTableOriginTransform = vtkTransform::SafeDownCast(
    tableTopToTableOriginTransformNode->GetTransformToParent() );
  tableTopToTableOriginTransform->Identity();
//  tableTopToTableOriginTransform->Translate( 0.,-1400., -675.);
//  tableTopToTableOriginTransform->Concatenate(rotateYTransform);
  tableTopToTableOriginTransform->Modified();

  // Update TableMiddle -> TableLateral
  // Translation of the TableMiddle to TableLateral
  vtkMRMLLinearTransformNode* tableMiddleToTableLateralTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableMiddleVerticalMovement, IHEP::TableLateralMovement);

  vtkTransform* tableMiddleToTableLateralTransform = vtkTransform::SafeDownCast(
    tableMiddleToTableLateralTransformNode->GetTransformToParent() );
  tableMiddleToTableLateralTransform->Identity();
//  tableMiddleToTableLateralTransform->Translate( 0.,-1400., -675.);
//  tableMiddleToTableLateralTransform->Concatenate(rotateYTransform);
  tableMiddleToTableLateralTransform->Modified();

  // Update TableMirror -> TableLateral
  // Translation of the TableMirror to TableLateral
  vtkMRMLLinearTransformNode* tableMirrorToTableLateralTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableMirrorVerticalMovement, IHEP::TableLateralMovement);

  vtkTransform* tableMirrorToTableLateralTransform = vtkTransform::SafeDownCast(
    tableMirrorToTableLateralTransformNode->GetTransformToParent() );
  tableMirrorToTableLateralTransform->Identity();
//  tableMirrorToTableLateralTransform->Translate( 0.,-1400., -675.);
//  tableMirrorToTableLateralTransform->Concatenate(rotateYTransform);
  tableMirrorToTableLateralTransform->Modified();

  // Update TableOrigin -> TableLateral
  // Translation of the TableOrigin to TableLateral
  vtkMRMLLinearTransformNode* tableOriginToTableLateralTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableOriginVerticalMovement, IHEP::TableLateralMovement);

  vtkTransform* tableOriginToTableLateralTransform = vtkTransform::SafeDownCast(
    tableOriginToTableLateralTransformNode->GetTransformToParent() );
  tableOriginToTableLateralTransform->Identity();
//  tableOriginToTableLateralTransform->Translate( 0.,-1400., -675.);
//  tableOriginToTableLateralTransform->Concatenate(rotateYTransform);
  tableOriginToTableLateralTransform->Modified();

  // Update TableLateralMovement -> TableLongitudinalMovement
  // Transformation (translation) of the TableLateral to TableLongitudinal
  vtkMRMLLinearTransformNode* tableLateralToTableLongitudinalTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableLateralMovement, IHEP::TableLongitudinalMovement);
  vtkTransform* tableLateralToTableLongitudinalTransform = vtkTransform::SafeDownCast(
    tableLateralToTableLongitudinalTransformNode->GetTransformToParent() );
  tableLateralToTableLongitudinalTransform->Identity();
//  tableLateralToTableLongitudinalTransform->Translate( 0., 1400., -1550.);
//  tableLateralToTableLongitudinalTransform->Concatenate(rotateYTransform);
  tableLateralToTableLongitudinalTransform->Modified();

  // Update TableLongitudinalMovement -> PatientSupportRotation
  // Transformation (translation) of the TableLongitudinal to PatientSupport
  vtkMRMLLinearTransformNode* tableLongitudinalToPatientSupportTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableLongitudinalMovement, IHEP::TableSupportRotation);
  vtkTransform* tableLongitudinalToPatientSupportTransform = vtkTransform::SafeDownCast(
    tableLongitudinalToPatientSupportTransformNode->GetTransformToParent() );
  tableLongitudinalToPatientSupportTransform->Identity();
//  tableLongitudinalToPatientSupportTransform->Translate( 0., -1400., -1550.);
//  tableLongitudinalToPatientSupportTransform->Translate( 0., 0., -1850.);
//  tableLongitudinalToPatientSupportTransform->Concatenate(rotateYTransform);
  tableLongitudinalToPatientSupportTransform->Modified();

  // Update PatientSupportRotation -> FixedReference
  // Translation of the PatientSupport to FixedReference
  vtkMRMLLinearTransformNode* patientSupportToFixedReferenceTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableSupportRotation, IHEP::FixedReferenceCanyon);
  vtkTransform* patientSupportToFixedReferenceTransform = vtkTransform::SafeDownCast(
    patientSupportToFixedReferenceTransformNode->GetTransformToParent() );
  patientSupportToFixedReferenceTransform->Identity();
//  patientSupportToFixedReferenceTransform->Translate( 0., 0., -1850.);
 // patientSupportToFixedReferenceTransform->Concatenate(rotateYTransform);
  patientSupportToFixedReferenceTransform->Modified();
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialNode* vtkSlicerIhepStandGeometryLogic::CreateTableFiducialNode(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLMarkupsFiducialNode* pointsMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(this->GetMRMLScene()->AddNewNodeByClass("vtkMRMLMarkupsFiducialNode"));
  pointsMarkupsNode->SetName(TABLE_FIDUCIALS_MARKUPS_NODE_NAME);
//  pointsMarkupsNode->SetHideFromEditors(1);
  std::string singletonTag = std::string("IHEP_") + TABLE_FIDUCIALS_MARKUPS_NODE_NAME;
  pointsMarkupsNode->SetSingletonTag(singletonTag.c_str());

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("CreateFiducialNode: Invalid MRML scene");
    return nullptr;
  }

  if (parameterNode)
  {
    // add points to fiducial node
    vtkVector3d p0( 265.5, 1116.6, -352.); // Origin
    vtkVector3d p1( -264.5, 1116.6, -352.); // Mirror
    vtkVector3d p2( 0.5, 1771.6, -352.); // Middle

    pointsMarkupsNode->AddControlPoint( p0, "Origin");
    pointsMarkupsNode->AddControlPoint( p1, "Mirror");
    pointsMarkupsNode->AddControlPoint( p2, "Middle");

    vtkMRMLTransformNode* transformNode = this->UpdateTableMarkupsTransform(parameterNode);

    // add transform to fiducial node
    if (transformNode)
    {
      pointsMarkupsNode->SetAndObserveTransformNodeID(transformNode->GetID());
    }
  }
  return pointsMarkupsNode;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsPlaneNode* vtkSlicerIhepStandGeometryLogic::CreateTableTopPlaneNode(
  vtkMRMLIhepStandGeometryNode* parameterNode, vtkMRMLMarkupsFiducialNode* pointsMarkupsNode)
{
  vtkNew<vtkMRMLMarkupsPlaneNode> tableTopPlaneNode;
  this->GetMRMLScene()->AddNode(tableTopPlaneNode);
  tableTopPlaneNode->SetName(TABLETOP_PLANE_MARKUPS_NODE_NAME);
//  tableTopPlaneNode->SetHideFromEditors(1);
  std::string singletonTag = std::string("IHEP_") + TABLETOP_PLANE_MARKUPS_NODE_NAME;
  tableTopPlaneNode->SetSingletonTag(singletonTag.c_str());

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("CreateTableTopStandPlaneNode: Invalid MRML scene");
    return nullptr;
  }

  if (parameterNode)
  {
    // add points from markups fiducial node
    double originWorld[3] = {};
    double mirrorWorld[3] = {};
    double middleWorld[3] = {};
    pointsMarkupsNode->GetNthControlPointPositionWorld( 0, originWorld);
    pointsMarkupsNode->GetNthControlPointPositionWorld( 1, mirrorWorld);
    pointsMarkupsNode->GetNthControlPointPositionWorld( 2, middleWorld);

    originWorld[1] += 142.;
    mirrorWorld[1] += 142.;
    middleWorld[1] += 142.;

//    vtkVector3d plane0( originWorld[0], originWorld[1], originWorld[2]); // Origin
    vtkVector3d plane1( mirrorWorld[0], mirrorWorld[1], mirrorWorld[2]); // Mirror
    vtkVector3d plane2( middleWorld[0], middleWorld[1], middleWorld[2]); // Middle

    tableTopPlaneNode->SetOrigin(originWorld);
    tableTopPlaneNode->AddControlPoint( plane1, "MirrorPlane");
    tableTopPlaneNode->AddControlPoint( plane2, "MiddlePlane");
    
    this->UpdateTableTopToTableLateralTransform(originWorld, mirrorWorld, middleWorld);
  }
  return tableTopPlaneNode;
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateTableTopToTableLateralTransform( double posOrigin[3], 
  double posMirror[3], double posMiddle[3])
{
  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;
/*
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
  vtkWarningMacro("UpdateTableTopToTableLateralTransform: Plane source normal " << norm[0] << " " << norm[1] << " " << norm[2]);
  // Update TableTop -> TableLateralMovement
  // Tranformation of the TableTop to TableLateral
  vtkMRMLLinearTransformNode* tableTopToTableLateralTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableTop, IHEP::TableLateralMovement);
  vtkTransform* tableTopToTableLateralTransform = vtkTransform::SafeDownCast(
    tableTopToTableLateralTransformNode->GetTransformToParent() );

  // Calculate transform from tree points
//  tableTopToTableLateralTransform->Identity();
//  tableTopToTableLateralTransform->Modified();
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsLineNode* vtkSlicerIhepStandGeometryLogic::CreateFixedReferenceLineNode(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkNew<vtkMRMLMarkupsLineNode> lineMarkupsNode;
  this->GetMRMLScene()->AddNode(lineMarkupsNode);
  lineMarkupsNode->SetName(FIXEDREFERENCE_LINE_MARKUPS_NODE_NAME);
//  pointsMarkupsNode->SetHideFromEditors(1);
  std::string singletonTag = std::string("IHEP_") + FIXEDREFERENCE_LINE_MARKUPS_NODE_NAME;
  lineMarkupsNode->SetSingletonTag(singletonTag.c_str());


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

    vtkMRMLTransformNode* transformNode = this->UpdateFixedReferenceMarkupsTransform(parameterNode);

    // add transform to fiducial node
    if (transformNode)
    {
      lineMarkupsNode->SetAndObserveTransformNodeID(transformNode->GetID());
    }
  }
  return lineMarkupsNode;
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateTableFiducialNode(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("UpdateTableFiducialNode: Invalid MRML scene");
    return;
  }

  if (!parameterNode)
  {
    vtkErrorMacro("UpdateTableFiducialNode: Invalid parameter node");
    return;
  }

  vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode();
  if (!beamNode)
  {
    vtkErrorMacro("UpdateTableFiducialNode: Invalid beam node");
    return;
  }

  // fiducial markups node
  if (scene->GetFirstNodeByName(TABLE_FIDUCIALS_MARKUPS_NODE_NAME))
  {
    vtkMRMLMarkupsFiducialNode* pointsMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
      scene->GetFirstNodeByName(TABLE_FIDUCIALS_MARKUPS_NODE_NAME));

    vtkVector3d p0( 265.5, 1116.6,
      -352. + parameterNode->GetTableTopVerticalPositionOrigin()); // Origin
    vtkVector3d p1( -264.5, 1116.6, 
      -352. + parameterNode->GetTableTopVerticalPositionMirror()); // Mirror
    vtkVector3d p2( 0.5, 1771.6, 
      -352. + parameterNode->GetTableTopVerticalPositionMiddle()); // Middle

    // update fiducials
    double* p = pointsMarkupsNode->GetNthControlPointPosition(0);
    if (p)
    {
      pointsMarkupsNode->SetNthControlPointPosition( 0, p0.GetX(), p0.GetY(), p0.GetZ());
    }
    else
    {
      pointsMarkupsNode->AddControlPoint(p0);
    }
    
    p = pointsMarkupsNode->GetNthControlPointPosition(1);
    if (p)
    {
      pointsMarkupsNode->SetNthControlPointPosition( 1, p1.GetX(), p1.GetY(), p1.GetZ());
    }
    else
    {
      pointsMarkupsNode->AddControlPoint(p1);
    }
    
    p = pointsMarkupsNode->GetNthControlPointPosition(2);
    if (p)
    {
      pointsMarkupsNode->SetNthControlPointPosition( 2, p2.GetX(), p2.GetY(), p2.GetZ());
    }
    else
    {
      pointsMarkupsNode->AddControlPoint(p2);
    }

    // Update fiducials markups transform node if it's changed    
    vtkMRMLTransformNode* markupsTransformNode = this->UpdateTableMarkupsTransform(parameterNode);

    if (markupsTransformNode)
    {
      pointsMarkupsNode->SetAndObserveTransformNodeID(markupsTransformNode->GetID());
    }
  }
  else
  {
    this->CreateTableFiducialNode(parameterNode);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateTableTopPlaneNode( 
  vtkMRMLIhepStandGeometryNode* parameterNode, vtkMRMLMarkupsFiducialNode* pointsMarkupsNode)
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

  if (scene->GetFirstNodeByName(TABLETOP_PLANE_MARKUPS_NODE_NAME))
  {
    vtkMRMLMarkupsPlaneNode* tableTopPlaneNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(
      scene->GetFirstNodeByName(TABLETOP_PLANE_MARKUPS_NODE_NAME));

    // update table top plane using fiducials
    double originWorld[3] = {};
    double mirrorWorld[3] = {};
    double middleWorld[3] = {};
    pointsMarkupsNode->GetNthControlPointPositionWorld( 0, originWorld);
    pointsMarkupsNode->GetNthControlPointPositionWorld( 1, mirrorWorld);
    pointsMarkupsNode->GetNthControlPointPositionWorld( 2, middleWorld);

    originWorld[1] += 142.;
    mirrorWorld[1] += 142.;
    middleWorld[1] += 142.;
    
    vtkVector3d plane1( mirrorWorld[0], mirrorWorld[1], mirrorWorld[2]); // Mirror
    vtkVector3d plane2( middleWorld[0], middleWorld[1], middleWorld[2]); // Middle

    tableTopPlaneNode->SetOrigin(originWorld);
    tableTopPlaneNode->SetNthControlPointPosition( 1, plane1.GetX(), plane1.GetY(), plane1.GetZ());
    tableTopPlaneNode->SetNthControlPointPosition( 2, plane2.GetX(), plane2.GetY(), plane2.GetZ());
    
    this->UpdateTableTopToTableLateralTransform(originWorld, mirrorWorld, middleWorld);
  }
  else
  {
    this->CreateTableTopPlaneNode( parameterNode, pointsMarkupsNode);
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
  if (scene->GetFirstNodeByName(FIXEDREFERENCE_LINE_MARKUPS_NODE_NAME))
  {
    vtkMRMLMarkupsLineNode* lineMarkupsNode = vtkMRMLMarkupsLineNode::SafeDownCast(
      scene->GetFirstNodeByName(FIXEDREFERENCE_LINE_MARKUPS_NODE_NAME));

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

    // Update fiducials markups transform node if it's changed    
    vtkMRMLTransformNode* markupsTransformNode = this->UpdateFixedReferenceMarkupsTransform(parameterNode);

    if (markupsTransformNode)
    {
      lineMarkupsNode->SetAndObserveTransformNodeID(markupsTransformNode->GetID());
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
  if (scene->GetFirstNodeByName(TABLE_FIDUCIALS_MARKUPS_NODE_NAME))
  {
    vtkMRMLMarkupsFiducialNode* pointsMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
      scene->GetFirstNodeByName(TABLE_FIDUCIALS_MARKUPS_NODE_NAME));
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

  // Table platform for longitudinal movement - optional
  std::string tablePlatformModelSingletonTag = machineType + "_" + TABLE_LONGITUDINAL_MODEL_NAME;
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
    std::string tablePlatformModelFilePath = treatmentMachineModelsDirectory + "/" + TABLE_LONGITUDINAL_MODEL_NAME + ".stl";
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

  // Table support for lateral movement  - mandatory
  std::string tableSupportModelSingletonTag = machineType + "_" + TABLE_LATERAL_MODEL_NAME;
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
    std::string tableSupportModelFilePath = treatmentMachineModelsDirectory + "/" + TABLE_LATERAL_MODEL_NAME + ".stl";
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
  // Update markups (Table fiducials and plane nodes and FixedReference line node)
//  vtkMRMLMarkupsFiducialNode* pointsMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
//    scene->GetFirstNodeByName(TABLE_FIDUCIALS_MARKUPS_NODE_NAME));

//  this->UpdateTableFiducialNode(parameterNode);

//  if (pointsMarkupsNode)
//  {
//    this->UpdateTableTopPlaneNode(parameterNode, pointsMarkupsNode);
//  }
  this->UpdateFixedReferenceLineNode(parameterNode);
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
  
  vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode();
  //TODO: Store treatment machine component color and other properties in JSON

  // Display all pieces of the treatment room and sets each piece a color to provide realistic representation
  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;

  // Transform IHEP stand models (IEC Patient) to RAS
  vtkNew<vtkTransform> patientToRasTransform;
  patientToRasTransform->Identity();
  patientToRasTransform->RotateX(-90.);
  patientToRasTransform->RotateZ(180.);

  // Table top - mandatory
  // Transform path: RAS -> Patient -> TableTop
  vtkNew<vtkTransform> rasToTableTopLinearTransform;
  if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::TableTop, 
    rasToTableTopLinearTransform, false))
  {
    // Transform to RAS, set transform to node, transform the model
    rasToTableTopLinearTransform->Concatenate(patientToRasTransform);

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
//      rasToTableTopTransformNode->SetSingletonTag("IHEP_");
      scene->AddNode(rasToTableTopTransformNode);
    }
    if (rasToTableTopTransformNode)
    {
      rasToTableTopTransformNode->SetAndObserveTransformToParent(rasToTableTopLinearTransform);
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

  // Table origin - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableOriginVerticalMovement
  vtkNew<vtkTransform> rasToTableOriginLinearTransform;
  if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::TableOriginVerticalMovement, 
    rasToTableOriginLinearTransform, false))
  {
    // Transform to RAS, set transform to node, transform the model
    rasToTableOriginLinearTransform->Concatenate(patientToRasTransform);

    // Find RasToTableOriginTransform or create it
    vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableOriginTransformNode;
    if (scene->GetFirstNodeByName("RasToTableOriginTransform"))
    {
      rasToTableOriginTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
        scene->GetFirstNodeByName("RasToTableOriginTransform"));
    }
    else
    {
      rasToTableOriginTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      rasToTableOriginTransformNode->SetName("RasToTableOriginTransform");
//      rasToTableOriginTransformNode->SetHideFromEditors(1);
//      rasToTableOriginTransformNode->SetSingletonTag("IHEP_");
      scene->AddNode(rasToTableOriginTransformNode);
    }
    if (rasToTableOriginTransformNode)
    {
      rasToTableOriginTransformNode->SetAndObserveTransformToParent(rasToTableOriginLinearTransform);
    }

    vtkMRMLModelNode* tableOriginModel = vtkMRMLModelNode::SafeDownCast(
      this->GetMRMLScene()->GetFirstNodeByName(TABLE_ORIGIN_MODEL_NAME) );
    if (!tableOriginModel)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to access table origin model");
      return;
    }
    if (rasToTableOriginTransformNode)
    {
      tableOriginModel->SetAndObserveTransformNodeID(rasToTableOriginTransformNode->GetID());
      tableOriginModel->CreateDefaultDisplayNodes();
      tableOriginModel->GetDisplayNode()->SetColor(0.3, 0.3, 0.3);
    }
  }

  // Table mirror - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableMirrorVerticalMovement
  vtkNew<vtkTransform> rasToTableMirrorLinearTransform;
  if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::TableMirrorVerticalMovement, 
    rasToTableMirrorLinearTransform, false))
  {
    // Transform to RAS, set transform to node, transform the model
    rasToTableMirrorLinearTransform->Concatenate(patientToRasTransform);

    // Find RasToTableMirrorTransform or create it
    vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableMirrorTransformNode;
    if (scene->GetFirstNodeByName("RasToTableMirrorTransform"))
    {
      rasToTableMirrorTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
        scene->GetFirstNodeByName("RasToTableMirrorTransform"));
    }
    else
    {
      rasToTableMirrorTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      rasToTableMirrorTransformNode->SetName("RasToTableMirrorTransform");
//      rasToTableMirrorTransformNode->SetHideFromEditors(1);
//      rasToTableMirrorTransformNode->SetSingletonTag("IHEP_");
      scene->AddNode(rasToTableMirrorTransformNode);
    }
    if (rasToTableMirrorTransformNode)
    {
      rasToTableMirrorTransformNode->SetAndObserveTransformToParent(rasToTableMirrorLinearTransform);
    }

    vtkMRMLModelNode* tableMirrorModel = vtkMRMLModelNode::SafeDownCast(
      this->GetMRMLScene()->GetFirstNodeByName(TABLE_MIRROR_MODEL_NAME) );
    if (!tableMirrorModel)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to access table mirror model");
      return;
    }
    if (rasToTableMirrorTransformNode)
    {
      tableMirrorModel->SetAndObserveTransformNodeID(rasToTableMirrorTransformNode->GetID());
      tableMirrorModel->CreateDefaultDisplayNodes();
      tableMirrorModel->GetDisplayNode()->SetColor(0.3, 0.3, 0.3);
    }
  }

  // Table middle - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableMiddleVerticalMovement
  vtkNew<vtkTransform> rasToTableMiddleLinearTransform;
  if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::TableMiddleVerticalMovement, 
    rasToTableMiddleLinearTransform, false))
  {
    // Transform to RAS, set transform to node, transform the model
    rasToTableMiddleLinearTransform->Concatenate(patientToRasTransform);

    // Find RasToTableMiddleTransform or create it
    vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableMiddleTransformNode;
    if (scene->GetFirstNodeByName("RasToTableMiddleTransform"))
    {
      rasToTableMiddleTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
        scene->GetFirstNodeByName("RasToTableMiddleTransform"));
    }
    else
    {
      rasToTableMiddleTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      rasToTableMiddleTransformNode->SetName("RasToTableMiddleTransform");
//      rasToTableMiddleTransformNode->SetHideFromEditors(1);
//      rasToTableMiddleTransformNode->SetSingletonTag("IHEP_");
      scene->AddNode(rasToTableMiddleTransformNode);
    }
    if (rasToTableMiddleTransformNode)
    {
      rasToTableMiddleTransformNode->SetAndObserveTransformToParent(rasToTableMiddleLinearTransform);
    }

    vtkMRMLModelNode* tableMiddleModel = vtkMRMLModelNode::SafeDownCast(
      this->GetMRMLScene()->GetFirstNodeByName(TABLE_MIDDLE_MODEL_NAME) );
    if (!tableMiddleModel)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to access table middle model");
      return;
    }
    if (rasToTableMiddleTransformNode)
    {
      tableMiddleModel->SetAndObserveTransformNodeID(rasToTableMiddleTransformNode->GetID());
      tableMiddleModel->CreateDefaultDisplayNodes();
      tableMiddleModel->GetDisplayNode()->SetColor(0.3, 0.3, 0.3);
    }
  }

  // Table support lateral movement model - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableOriginVerticalMovement -> TableLateralMovement
  vtkNew<vtkTransform> rasToTableLateralLinearTransform;
  if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::TableLateralMovement, 
    rasToTableLateralLinearTransform, false))
  {
    // Transform to RAS, set transform to node, transform the model
    rasToTableLateralLinearTransform->Concatenate(patientToRasTransform);

    // Find RasToTableTopInferiorSuperiorTransform or create it
    vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableLateralTransformNode;
    if (scene->GetFirstNodeByName("RasToTableLateralTransform"))
    {
      rasToTableLateralTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
        scene->GetFirstNodeByName("RasToTableLateralTransform"));
    }
    else
    {
      rasToTableLateralTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      rasToTableLateralTransformNode->SetName("RasToTableLateralTransform");
//      rasToTableLateralTransformNode->SetHideFromEditors(1);
//      rasToTableLateralTransformNode->SetSingletonTag("IHEP_");
      scene->AddNode(rasToTableLateralTransformNode);
    }

    vtkMRMLModelNode* tableLateralModel = vtkMRMLModelNode::SafeDownCast(
      scene->GetFirstNodeByName(TABLE_LATERAL_MODEL_NAME) );
    if (!tableLateralModel)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to access table lateral model");
      return;
    }
    if (rasToTableLateralTransformNode)
    {
      rasToTableLateralTransformNode->SetAndObserveTransformToParent(rasToTableLateralLinearTransform);
      tableLateralModel->SetAndObserveTransformNodeID(rasToTableLateralTransformNode->GetID());
      tableLateralModel->CreateDefaultDisplayNodes();
      tableLateralModel->GetDisplayNode()->SetColor(0.95, 0.95, 0.95);
    }
  }

  // Table platform longitudinal movement model - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableOriginVerticalMovement -> TableLateralMovement -> TableLongitudinalMovement
  vtkNew<vtkTransform> rasToTableLongitudinalLinearTransform;
  if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::TableLongitudinalMovement, 
    rasToTableLongitudinalLinearTransform, false))
  {
    // Transform to RAS, set transform to node, transform the model
    rasToTableLongitudinalLinearTransform->Concatenate(patientToRasTransform);

    // Find RasToTableTopInferiorSuperiorTransform or create it
    vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableLongitudinalTransformNode;
    if (scene->GetFirstNodeByName("RasToTableLongitudinalTransform"))
    {
      rasToTableLongitudinalTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
        scene->GetFirstNodeByName("RasToTableLongitudinalTransform"));
    }
    else
    {
      rasToTableLongitudinalTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      rasToTableLongitudinalTransformNode->SetName("RasToTableLongitudinalTransform");
//      rasToTableLongitudinalTransformNode->SetHideFromEditors(1);
//      rasToTableLongitudinalTransformNode->SetSingletonTag("IHEP_");
      scene->AddNode(rasToTableLongitudinalTransformNode);
    }

    vtkMRMLModelNode* tableLongitudinalModel = vtkMRMLModelNode::SafeDownCast(
      scene->GetFirstNodeByName(TABLE_LONGITUDINAL_MODEL_NAME) );
    if (!tableLongitudinalModel)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to access table longitudinal model");
      return;
    }
    if (rasToTableLongitudinalTransformNode)
    {
      rasToTableLongitudinalTransformNode->SetAndObserveTransformToParent(rasToTableLongitudinalLinearTransform);
      tableLongitudinalModel->SetAndObserveTransformNodeID(rasToTableLongitudinalTransformNode->GetID());
      tableLongitudinalModel->CreateDefaultDisplayNodes();
      tableLongitudinalModel->GetDisplayNode()->SetColor(0.75, 0.75, 0.75);
    }
  }

  // Patient support - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableOriginVerticalMovement -> TableLateralMovement -> TableLongitudinalMovement -> PatientSupportRotation
  vtkNew<vtkTransform> rasToPatientSupportLinearTransform;
  if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::TableSupportRotation, 
    rasToPatientSupportLinearTransform, false))
  {
    // Transform to RAS, set transform to node, transform the model
    rasToPatientSupportLinearTransform->Concatenate(patientToRasTransform);

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
//      rasToPatientSupportTransformNode->SetSingletonTag("IHEP_");
      scene->AddNode(rasToPatientSupportTransformNode);
    }
    if (rasToPatientSupportTransformNode)
    {
      rasToPatientSupportTransformNode->SetAndObserveTransformToParent(rasToPatientSupportLinearTransform);
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
      rasToPatientSupportTransformNode->SetAndObserveTransformToParent(rasToPatientSupportLinearTransform);
      patientSupportModel->SetAndObserveTransformNodeID(rasToPatientSupportTransformNode->GetID());
      patientSupportModel->CreateDefaultDisplayNodes();
      patientSupportModel->GetDisplayNode()->SetColor(0.55, 0.55, 0.55);
    }
  }

  // Fixed Reference - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableOriginVerticalMovement -> TableLateralMovement -> TableLongitudinalMovement -> PatientSupportRotation -> FixedReference
  vtkNew<vtkTransform> rasToFixedReferenceLinearTransform;
  if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::FixedReferenceCanyon, 
    rasToFixedReferenceLinearTransform, false))
  {
    // Transform to RAS, set transform to node, transform the model
    vtkNew<vtkTransform> patientSupportToFixedReferenceTransform;

    rasToFixedReferenceLinearTransform->Concatenate(patientToRasTransform);

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
//      rasToFixedReferenceTransformNode->SetSingletonTag("IHEP_");
      scene->AddNode(rasToFixedReferenceTransformNode);
    }
    if (rasToFixedReferenceTransformNode)
    {
      rasToFixedReferenceTransformNode->SetAndObserveTransformToParent(rasToFixedReferenceLinearTransform);
    }

    vtkMRMLModelNode* fixedReferenceModel = vtkMRMLModelNode::SafeDownCast(
      this->GetMRMLScene()->GetFirstNodeByName(FIXEDREFERENCE_MODEL_NAME) );
    if (!fixedReferenceModel)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to access table top model");
      return;
    }
    if (rasToFixedReferenceTransformNode)
    {
      fixedReferenceModel->SetAndObserveTransformNodeID(rasToFixedReferenceTransformNode->GetID());
      fixedReferenceModel->CreateDefaultDisplayNodes();
      fixedReferenceModel->GetDisplayNode()->SetColor(0.7, 0.65, 0.65);
    }
  }

  // Update markups (TableTop fiducials and plane nodes and FixedReference line node)
//  vtkMRMLMarkupsFiducialNode* pointsMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
//    scene->GetFirstNodeByName(TABLE_FIDUCIALS_MARKUPS_NODE_NAME));

//  this->UpdateTableFiducialNode(parameterNode);

//  if (pointsMarkupsNode)
//  {
//    this->UpdateTableTopPlaneNode(parameterNode, pointsMarkupsNode);
//  }
  this->UpdateFixedReferenceLineNode(parameterNode);
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
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableTop, IHEP::TableOriginVerticalMovement);

  if (tableTopToTableOriginTransformNode)
  {
    double PatientTableTopTranslation[3] = {};
    double originTranslation[3] = { 265.5, 1116.6, -352. }; // translation of table top origin to origin of fixed reference (RAS)
    parameterNode->GetPatientToTableTopTranslation(PatientTableTopTranslation);
    PatientTableTopTranslation[0] *= -1.;
    PatientTableTopTranslation[1] *= -1.;
    PatientTableTopTranslation[2] *= -1.;

//    double originWorld[3] = { 265.5 + PatientTableTopTranslation[0], 1116.6 + PatientTableTopTranslation[1], -352. + PatientTableTopTranslation[2] };

    vtkNew<vtkTransform> tableTopToTableOriginTransform;
    // Move TableTop model to RAS origin
    tableTopToTableOriginTransform->Translate(originTranslation);
    tableTopToTableOriginTransform->Translate(PatientTableTopTranslation);
    // Apply transform (rotation)
    tableTopToTableOriginTransform->RotateX(parameterNode->GetTableTopLongitudinalAngle());
    tableTopToTableOriginTransform->RotateY(parameterNode->GetTableTopLateralAngle());
    // Move back
    tableTopToTableOriginTransform->Translate(-1. * originTranslation[0], -1. * originTranslation[1], -1. * originTranslation[2]);
    tableTopToTableOriginTransform->Translate(-1. * PatientTableTopTranslation[0], -1. * PatientTableTopTranslation[1], -1. * PatientTableTopTranslation[2]);
    tableTopToTableOriginTransform->Inverse();

    tableTopToTableOriginTransformNode->SetAndObserveTransformToParent(tableTopToTableOriginTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateTableOriginToTableLateralTransform(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableOriginToTableLateralTransform: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("UpdateTableOriginToTableLateralTransform: Invalid parameter node");
    return;
  }

  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* tableOriginToTableLateralTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableOriginVerticalMovement, IHEP::TableLateralMovement);

  if (tableOriginToTableLateralTransformNode)
  {
    // TableTop to TableOrigin rotation matrix
    vtkNew<vtkTransform> tableTopToTableOriginTransform;
    vtkMRMLLinearTransformNode* tableTopToTableOriginTransformNode =
      this->IhepLogic->GetTransformNodeBetween(IHEP::TableTop, IHEP::TableOriginVerticalMovement);
    if (tableTopToTableOriginTransformNode)
    {
      vtkNew<vtkMatrix4x4> tableTopToTableOriginMatrix;
      tableTopToTableOriginTransformNode->GetMatrixTransformToParent(tableTopToTableOriginMatrix);
      tableTopToTableOriginMatrix->SetElement( 0, 3, 0.);
      tableTopToTableOriginMatrix->SetElement( 1, 3, 0.);
      tableTopToTableOriginMatrix->SetElement( 2, 3, 0.);
      tableTopToTableOriginTransform->Concatenate(tableTopToTableOriginMatrix);
    }

    vtkNew<vtkTransform> tableOriginToTableLateralTransform;
    // Vertical translation of the TableOrigin in TableLateralMovement system (Table stand)
    double originPosTableTop[4] = { 0., 0., -1. * parameterNode->GetTableTopVerticalPositionOrigin(), 1. }; // origin in FixedReference transform
    double originPosTableLateral[4];
    // Get translation of the origin in a new table top coordinate
    tableTopToTableOriginTransform->MultiplyPoint( originPosTableTop, originPosTableLateral);
    // Apply translation to TableLateralMovement system
    tableOriginToTableLateralTransform->Translate(originPosTableLateral);

    tableOriginToTableLateralTransformNode->SetAndObserveTransformToParent(tableOriginToTableLateralTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateTableMirrorToTableLateralTransform(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableMirrorToTableLateralTransform: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("UpdateTableMirrorToTableLateralTransform: Invalid parameter node");
    return;
  }

  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* tableMirrorToTableLateralTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableMirrorVerticalMovement, IHEP::TableLateralMovement);

  if (tableMirrorToTableLateralTransformNode)
  {
    // TableTop to TableOrigin rotation matrix
    vtkNew<vtkTransform> tableTopToTableOriginTransform;
    vtkMRMLLinearTransformNode* tableTopToTableOriginTransformNode =
      this->IhepLogic->GetTransformNodeBetween(IHEP::TableTop, IHEP::TableOriginVerticalMovement);
    if (tableTopToTableOriginTransformNode)
    {
      vtkNew<vtkMatrix4x4> tableTopToTableOriginMatrix;
      tableTopToTableOriginTransformNode->GetMatrixTransformToParent(tableTopToTableOriginMatrix);
      tableTopToTableOriginMatrix->SetElement( 0, 3, 0.);
      tableTopToTableOriginMatrix->SetElement( 1, 3, 0.);
      tableTopToTableOriginMatrix->SetElement( 2, 3, 0.);
      tableTopToTableOriginTransform->Concatenate(tableTopToTableOriginMatrix);
    }

    vtkNew<vtkTransform> tableMirrorToTableLateralTransform;
    // Vertical translation of the TableMirror in TableLateralMovement system (Table stand)
    double mirrorPosTableTop[4] = { 0., 0., -1. * parameterNode->GetTableTopVerticalPositionMirror(), 1. }; // origin in FixedReference transform
    double mirrorPosTableLateral[4];
    // Get translation of the origin in a new table top coordinate
    tableTopToTableOriginTransform->MultiplyPoint( mirrorPosTableTop, mirrorPosTableLateral);
    // Apply translation to TableLateralMovement system
    tableMirrorToTableLateralTransform->Translate(mirrorPosTableLateral);

    tableMirrorToTableLateralTransformNode->SetAndObserveTransformToParent(tableMirrorToTableLateralTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateTableMiddleToTableLateralTransform(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableMiddleToTableLateralTransform: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("UpdateTableMiddleToTableLateralTransform: Invalid parameter node");
    return;
  }

  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* tableMiddleToTableLateralTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableMiddleVerticalMovement, IHEP::TableLateralMovement);

  if (tableMiddleToTableLateralTransformNode)
  {
    // TableTop to TableOrigin rotation matrix
    vtkNew<vtkTransform> tableTopToTableOriginTransform;
    vtkMRMLLinearTransformNode* tableTopToTableOriginTransformNode =
      this->IhepLogic->GetTransformNodeBetween(IHEP::TableTop, IHEP::TableOriginVerticalMovement);
    if (tableTopToTableOriginTransformNode)
    {
      vtkNew<vtkMatrix4x4> tableTopToTableOriginMatrix;
      tableTopToTableOriginTransformNode->GetMatrixTransformToParent(tableTopToTableOriginMatrix);
      tableTopToTableOriginMatrix->SetElement( 0, 3, 0.);
      tableTopToTableOriginMatrix->SetElement( 1, 3, 0.);
      tableTopToTableOriginMatrix->SetElement( 2, 3, 0.);
      tableTopToTableOriginTransform->Concatenate(tableTopToTableOriginMatrix);
    }
    vtkNew<vtkTransform> tableMiddleToTableLateralTransform;
    // Vertical translation of the TableMiddle in TableLateralMovement system (Table stand)
    double middlePosTableTop[4] = { 0., 0., -1. * parameterNode->GetTableTopVerticalPositionMiddle(), 1. }; // origin in FixedReference transform
    double middlePosTableLateral[4];
    // Get translation of the origin in a new table top coordinate
    tableTopToTableOriginTransform->MultiplyPoint( middlePosTableTop, middlePosTableLateral);
    // Apply translation to TableLateralMovement system
    tableMiddleToTableLateralTransform->Translate(middlePosTableLateral);

    tableMiddleToTableLateralTransformNode->SetAndObserveTransformToParent(tableMiddleToTableLateralTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdatePatientSupportToFixedReferenceTransform(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdatePatientSupportToFixedReferenceTransfrom: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("UpdatePatientSupportToFixedReferenceTransfrom: Invalid parameter node");
    return;
  }

  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* patientSupportToFixedReferenceTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableSupportRotation, IHEP::FixedReferenceCanyon);

  if (patientSupportToFixedReferenceTransformNode)
  {

    // TableTop to TableOrigin rotation matrix
    vtkNew<vtkTransform> tableTopToTableOriginTransform;
    vtkMRMLLinearTransformNode* tableTopToTableOriginTransformNode =
      this->IhepLogic->GetTransformNodeBetween(IHEP::TableTop, IHEP::TableOriginVerticalMovement);
    if (tableTopToTableOriginTransformNode)
    {
      vtkNew<vtkMatrix4x4> tableTopToTableOriginMatrix;
      tableTopToTableOriginTransformNode->GetMatrixTransformToParent(tableTopToTableOriginMatrix);
      tableTopToTableOriginMatrix->SetElement( 0, 3, 0.);
      tableTopToTableOriginMatrix->SetElement( 1, 3, 0.);
      tableTopToTableOriginMatrix->SetElement( 2, 3, 0.);
      tableTopToTableOriginTransform->Concatenate(tableTopToTableOriginMatrix);
//      tableTopToTableOriginTransform->Inverse();
    }

    vtkNew<vtkTransform> patientSupportToFixedReferenceTransform;
//    patientSupportToFixedReferenceTransform->PreMultiply();
    // Apply rotation
//    patientSupportToFixedReferenceTransform->Concatenate(tableTopToTableOriginTransform);    
//    tableTopToTableOriginTransform->RotateZ(-1. * parameterNode->GetPatientSupportRotationAngle());
    patientSupportToFixedReferenceTransform->RotateZ(-1. * parameterNode->GetPatientSupportRotationAngle());
//    patientSupportToFixedReferenceTransform->Concatenate(tableTopToTableOriginTransform);
///    tableTopToTableOriginTransform->Concatenate(patientSupportToFixedReferenceTransform);
///    tableTopToTableOriginTransform->Inverse();
    patientSupportToFixedReferenceTransformNode->SetAndObserveTransformToParent(patientSupportToFixedReferenceTransform);
//    patientSupportToFixedReferenceTransformNode->SetAndObserveTransformToParent(tableTopToTableOriginTransform);

/*
    // TableTop to TableOrigin rotation matrix
    vtkNew<vtkTransform> tableTopToTableOriginTransform;
    vtkMRMLLinearTransformNode* tableTopToTableOriginTransformNode =
      this->IhepLogic->GetTransformNodeBetween(IHEP::TableTop, IHEP::TableOriginVerticalMovement);
    if (tableTopToTableOriginTransformNode)
    {
      vtkNew<vtkMatrix4x4> tableTopToTableOriginMatrix;
      tableTopToTableOriginTransformNode->GetMatrixTransformToParent(tableTopToTableOriginMatrix);
      tableTopToTableOriginMatrix->SetElement( 0, 3, 0.);
      tableTopToTableOriginMatrix->SetElement( 1, 3, 0.);
      tableTopToTableOriginMatrix->SetElement( 2, 3, 0.);
      tableTopToTableOriginTransform->Concatenate(tableTopToTableOriginMatrix);
    }

    vtkNew<vtkTransform> tableLateralToTableLongitudinalTransform;
    // Horizontal translation of the TableLateral in TableLongitudinalMovement system (Table platform)
    double lateralPosTableTop[4] = { -1. * parameterNode->GetTableTopLateralPosition(), 0., 0., 1. }; // origin in FixedReference transform
    double lateralPosTableLongitudinal[4];
    // Get translation of the origin in a new table top coordinate
    tableTopToTableOriginTransform->MultiplyPoint( lateralPosTableTop, lateralPosTableLongitudinal);
    // Apply translation to TableLongitudinalMovement system
    tableLateralToTableLongitudinalTransform->Translate(lateralPosTableLongitudinal);
*/
    // Patient Support To Fixed Reference rotation matrix
///    vtkNew<vtkTransform> patientSupportToFixedReferenceTransform;

//    patientSupportToFixedReferenceTransform->Translate(pos);
    // Apply rotation
///    patientSupportToFixedReferenceTransform->RotateZ(-1. * parameterNode->GetPatientSupportRotationAngle());
    
//    tableLateralToTableLongitudinalTransform->Concatenate(patientSupportToFixedReferenceTransform);
    // Apply rotation
//    patientSupportToFixedReferenceTransform->RotateZ(-1. * parameterNode->GetPatientSupportRotationAngle());

//    patientSupportToFixedReferenceTransform->Concatenate(tableTopToTableOriginTransform);

/*
    // Patient Support To Fixed Reference rotation matrix
    vtkNew<vtkTransform> patientSupportToFixedReferenceTransform;
//    patientSupportToFixedReferenceTransform->Concatenate(tableTopToTableOriginTransform);

    double PatientTableTopTranslation[3] = {};
    parameterNode->GetPatientToTableTopTranslation(PatientTableTopTranslation);
    double pos[4] = { -1. * parameterNode->GetTableTopLateralPosition() + PatientTableTopTranslation[0], 
      parameterNode->GetTableTopLongitudinalPosition() + PatientTableTopTranslation[1], 
      -1. * parameterNode->GetTableTopVerticalPosition() + PatientTableTopTranslation[2], 1. };

//    patientSupportToFixedReferenceTransform->Translate(pos);
    // Apply rotation
    patientSupportToFixedReferenceTransform->RotateZ(-1. * parameterNode->GetPatientSupportRotationAngle());
    // Move back
//    patientSupportToFixedReferenceTransform->Translate( -1. * pos[0], -1. * pos[1], -1. * pos[2]);

//    double lateralPosTableLongitudinal[4];
    // Get translation of the origin in a new table top coordinate
//    tableTopToTableOriginTransform->MultiplyPoint( pos, lateralPosTableLongitudinal);
    // Move to Origin
//   patientSupportToFixedReferenceTransform->Translate( lateralPosTableLongitudinal[0],
//      lateralPosTableLongitudinal[1], 
//      lateralPosTableLongitudinal[2]);
    // Apply rotation
//    patientSupportToFixedReferenceTransform->RotateZ(-1. * parameterNode->GetPatientSupportRotationAngle());
    // Move back
//    patientSupportToFixedReferenceTransform->Translate( -lateralPosTableLongitudinal[0],
//      -lateralPosTableLongitudinal[1], 
//      -lateralPosTableLongitudinal[2]);

//    patientSupportToFixedReferenceTransform->Concatenate(tableTopToTableOriginTransform);
*/
    // Observe transform
///    patientSupportToFixedReferenceTransformNode->SetAndObserveTransformToParent(patientSupportToFixedReferenceTransform);
//    patientSupportToFixedReferenceTransformNode->SetAndObserveTransformToParent(tableLateralToTableLongitudinalTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateTableLateralToTableLongitudinalTransform(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableLateralToTableLongitudinalTransform: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("UpdateTableLateralToTableLongitudinalTransform: Invalid parameter node");
    return;
  }

  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* tableLateralToTableLongitudinalTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableLateralMovement, IHEP::TableLongitudinalMovement);

  if (tableLateralToTableLongitudinalTransformNode)
  {

    // TableTop to TableOrigin rotation matrix
    vtkNew<vtkTransform> tableTopToTableOriginTransform;
    vtkMRMLLinearTransformNode* tableTopToTableOriginTransformNode =
      this->IhepLogic->GetTransformNodeBetween(IHEP::TableTop, IHEP::TableOriginVerticalMovement);
    if (tableTopToTableOriginTransformNode)
    {
      vtkNew<vtkMatrix4x4> tableTopToTableOriginMatrix;
      tableTopToTableOriginTransformNode->GetMatrixTransformToParent(tableTopToTableOriginMatrix);
      tableTopToTableOriginMatrix->SetElement( 0, 3, 0.);
      tableTopToTableOriginMatrix->SetElement( 1, 3, 0.);
      tableTopToTableOriginMatrix->SetElement( 2, 3, 0.);
      tableTopToTableOriginTransform->Concatenate(tableTopToTableOriginMatrix);
    }

    vtkNew<vtkTransform> tableLateralToTableLongitudinalTransform;
    // Horizontal translation of the TableLateral in TableLongitudinalMovement system (Table platform)
    double lateralPosTableTop[4] = { -1. * parameterNode->GetTableTopLateralPosition(), 0., 0., 1. }; // origin in FixedReference transform
    double lateralPosTableLongitudinal[4];
    // Get translation of the origin in a new table top coordinate
    tableTopToTableOriginTransform->MultiplyPoint( lateralPosTableTop, lateralPosTableLongitudinal);
    // Apply translation to TableLongitudinalMovement system
    tableLateralToTableLongitudinalTransform->Translate(lateralPosTableLongitudinal);

    tableLateralToTableLongitudinalTransformNode->SetAndObserveTransformToParent(tableLateralToTableLongitudinalTransform);
  }

//  this->UpdatePatientSupportToFixedReferenceTransform(parameterNode);
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateTableLongitudinalToPatientSupportTransform(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableLongitudinalToPatientSupportTransform: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("UpdateTableLongitudinalToPatientSupportTransform: Invalid parameter node");
    return;
  }

  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* tableLongitudinalToPatientSupportTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableLongitudinalMovement, IHEP::TableSupportRotation);

  if (tableLongitudinalToPatientSupportTransformNode)
  {

    // TableTop to TableOrigin rotation matrix
    vtkNew<vtkTransform> tableTopToTableOriginTransform;
    vtkMRMLLinearTransformNode* tableTopToTableOriginTransformNode =
      this->IhepLogic->GetTransformNodeBetween(IHEP::TableTop, IHEP::TableOriginVerticalMovement);
    if (tableTopToTableOriginTransformNode)
    {
      vtkNew<vtkMatrix4x4> tableTopToTableOriginMatrix;
      tableTopToTableOriginTransformNode->GetMatrixTransformToParent(tableTopToTableOriginMatrix);
      tableTopToTableOriginMatrix->SetElement( 0, 3, 0.);
      tableTopToTableOriginMatrix->SetElement( 1, 3, 0.);
      tableTopToTableOriginMatrix->SetElement( 2, 3, 0.);
      tableTopToTableOriginTransform->Concatenate(tableTopToTableOriginMatrix);
    }

    vtkNew<vtkTransform> tableLongitudinalToPatientSupportTransform;
    // Horizontal translation of the TableLongitudinal in PatientSupportRotation system (PatientSupport)
    double longitudinalPosTableTop[4] = {  0., parameterNode->GetTableTopLongitudinalPosition(), 0., 1. }; // origin in FixedReference transform
    double longitudinalPosPatientSupport[4];
    // Get translation of the origin in a new table top coordinate
    tableTopToTableOriginTransform->MultiplyPoint( longitudinalPosTableTop, longitudinalPosPatientSupport);
    // Apply translation to TableLateralMovement system
    tableLongitudinalToPatientSupportTransform->Translate(longitudinalPosPatientSupport);

    tableLongitudinalToPatientSupportTransformNode->SetAndObserveTransformToParent(tableLongitudinalToPatientSupportTransform);
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
vtkMRMLLinearTransformNode* vtkSlicerIhepStandGeometryLogic::UpdateTableMarkupsTransform(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateTableMarkupsTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableMarkupsTransform: Invalid MRML scene");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (!scene->GetFirstNodeByName(TABLE_FIDUCIALS_TRANSFORM_NODE_NAME))
  {
    transformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    transformNode->SetName(TABLE_FIDUCIALS_TRANSFORM_NODE_NAME);
//    transformNode->SetHideFromEditors(1);
    transformNode->SetSingletonTag("TABLE_FIDUCIALS_Transform");
    scene->AddNode(transformNode);
  }
  else
  {
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast(
      scene->GetFirstNodeByName(TABLE_FIDUCIALS_TRANSFORM_NODE_NAME));
  }

  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;
  // Dynamic transform from RAS to TableLateralMovement
  // Transformation path: RAS -> Patient -> TableTop -> TableLateralMovement
  vtkNew<vtkGeneralTransform> generalTransform;
  if (transformNode && this->IhepLogic->GetTransformBetween( IHEP::RAS, 
    IHEP::TableLateralMovement, generalTransform, false))
  {
    // Convert general transform to linear
    // This call also makes hard copy of the transform so that it doesn't change when other beam transforms change
    vtkNew<vtkTransform> rasToTableLateralTransform;
    if (!vtkMRMLTransformNode::IsGeneralTransformLinear(generalTransform, rasToTableLateralTransform))
    {
      vtkErrorMacro("UpdateTableTopStandMarkupsTransform: Unable to set transform with non-linear components");
      return nullptr;
    }

    // Update to new origin because of Patient to TableTop translate vector
    double PatientTableTopTranslation[3] = {};
    parameterNode->GetPatientToTableTopTranslation(PatientTableTopTranslation);
    vtkNew<vtkTransform> linearTransform;
    // Move to RAS origin
    linearTransform->Translate( -1. * PatientTableTopTranslation[0], 
      -1. * PatientTableTopTranslation[1], -1. * PatientTableTopTranslation[2]);
    // Apply transform
    linearTransform->Concatenate(rasToTableLateralTransform);
    // Move back
    linearTransform->Translate(PatientTableTopTranslation);

    // Set transform to node
    transformNode->SetAndObserveTransformToParent(linearTransform);
  }
  return transformNode;
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

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (!scene->GetFirstNodeByName(FIXEDREFERENCE_LINE_TRANSFORM_NODE_NAME))
  {
    transformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    transformNode->SetName(FIXEDREFERENCE_LINE_TRANSFORM_NODE_NAME);
//    transformNode->SetHideFromEditors(1);
    transformNode->SetSingletonTag("FIXEDREFERENCE_LINE_Transform");
    scene->AddNode(transformNode);
  }
  else
  {
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast(
      scene->GetFirstNodeByName(FIXEDREFERENCE_LINE_TRANSFORM_NODE_NAME));
  }

  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;
  // Dynamic transform from RAS to FixedReference
  // Transformation path: RAS -> Patient -> TableTop -> TableLateralMovement -> TableLongitudinalMovement -> PatientSupportRotation -> FixedReference
  vtkNew<vtkGeneralTransform> rasToFixedReferenceTransform;
  if (transformNode && this->IhepLogic->GetTransformBetween( IHEP::RAS, 
    IHEP::FixedReferenceCanyon, rasToFixedReferenceTransform, false))
  {
    // Convert general transform to linear
    // This call also makes hard copy of the transform so that it doesn't change when other beam transforms change
//    vtkNew<vtkTransform> rasToFixedReferenceTransform;
//    if (!vtkMRMLTransformNode::IsGeneralTransformLinear(generalTransform, rasToFixedReferenceTransform))
//    {
//      vtkErrorMacro("UpdateFixedReferenceMarkupsTransform: Unable to set transform with non-linear components");
//      return nullptr;
//    }

    // Transform IHEP stand models (IEC Patient) to RAS
    vtkNew<vtkTransform> rasToPatientTransform;
    rasToPatientTransform->Identity();
    rasToPatientTransform->RotateX(-90.);
    rasToPatientTransform->RotateZ(180.);

    // Apply transform
    rasToFixedReferenceTransform->Concatenate(rasToPatientTransform);
    // Set transform to node
    transformNode->SetAndObserveTransformToParent(rasToFixedReferenceTransform);
  }
  return transformNode;
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerIhepStandGeometryLogic::GetFixedReferenceMarkupsTransform()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateFixedReferenceMarkupsTransform: Invalid MRML scene");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (vtkMRMLNode* node = scene->GetFirstNodeByName(FIXEDREFERENCE_LINE_TRANSFORM_NODE_NAME))
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

  vtkMRMLTransformNode* fixedReferenceTransformNode = this->GetFixedReferenceMarkupsTransform();
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
