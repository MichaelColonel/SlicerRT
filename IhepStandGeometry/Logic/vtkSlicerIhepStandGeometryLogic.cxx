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

//----------------------------------------------------------------------------
// Treatment machine component names
const char* vtkSlicerIhepStandGeometryLogic::CANYON_MODEL_NAME = "Canyon";
const char* vtkSlicerIhepStandGeometryLogic::PATIENTSUPPORT_MODEL_NAME = "PatientSupportRotation";
const char* vtkSlicerIhepStandGeometryLogic::TABLETOPSTAND_MODEL_NAME = "TableTopStand";
const char* vtkSlicerIhepStandGeometryLogic::TABLETOP_MODEL_NAME = "TableTop";
const char* vtkSlicerIhepStandGeometryLogic::TABLETOP_PLANE_MARKUPS_NODE_NAME = "TableTopMarkupsPlane";
const char* vtkSlicerIhepStandGeometryLogic::TABLETOPSTAND_FIDUCIALS_MARKUPS_NODE_NAME = "TableTopMarkupsFiducials";
const char* vtkSlicerIhepStandGeometryLogic::TABLETOPSTAND_FIDUCIALS_TRANSFORM_NODE_NAME = "TableTopMarkupsFiducialsTransform";

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

  // Update TableTop -> TableTopVertical
  // Translation of the TableTop from TableTopStand
  vtkMRMLLinearTransformNode* tableTopToTableTopVerticalTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableTop, IHEP::TableTopVertical);
  vtkTransform* tableTopToTableTopVerticalTransform = vtkTransform::SafeDownCast(
    tableTopToTableTopVerticalTransformNode->GetTransformToParent() );
  tableTopToTableTopVerticalTransform->Identity();
//  tableTopToTableTopVerticalTransform->Translate( 0.,-1400., -675.);
//  tableTopToTableTopVerticalTransform->Concatenate(rotateYTransform);
  tableTopToTableTopVerticalTransform->Modified();

  // Update TableTopVertical -> TableTopStand
  // Translation of the TableTop from TableTopStand
  vtkMRMLLinearTransformNode* tableTopVerticalToTableTopStandTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableTopVertical, IHEP::TableTopStand);
  vtkTransform* tableTopVerticalToTableTopStandTransform = vtkTransform::SafeDownCast(
    tableTopVerticalToTableTopStandTransformNode->GetTransformToParent() );
  tableTopVerticalToTableTopStandTransform->Identity();
//  tableTopVerticalToTableTopStandTransform->Translate( 0., 1400., -1550.);
//  tableTopVerticalToTableTopStandTransform->Concatenate(rotateYTransform);
  tableTopVerticalToTableTopStandTransform->Modified();

  // Update TableTopStand -> PatientSupport
  // Translation of the TableTopStand from PatientSupport
  vtkMRMLLinearTransformNode* tableTopStandToPatientSupportTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableTopStand, IHEP::PatientSupport);
  vtkTransform* tableTopStandToPatientSupportTransform = vtkTransform::SafeDownCast(
    tableTopStandToPatientSupportTransformNode->GetTransformToParent() );
  tableTopStandToPatientSupportTransform->Identity();
//  tableTopStandToPatientSupportTransform->Translate( 0., -1400., -1550.);
//  tableTopStandToPatientSupportTransform->Translate( 0., 0., -1850.);
//  tableTopStandToPatientSupportTransform->Concatenate(rotateYTransform);
  tableTopStandToPatientSupportTransform->Modified();

  // Update PatientSupport -> FixedReference
  // Translation of the PatientSupport from FixedReference
  vtkMRMLLinearTransformNode* patientSupportToFixedReferenceTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::PatientSupport, IHEP::FixedReference);
  vtkTransform* patientSupportToFixedReferenceTransform = vtkTransform::SafeDownCast(
    patientSupportToFixedReferenceTransformNode->GetTransformToParent() );
  patientSupportToFixedReferenceTransform->Identity();
//  patientSupportToFixedReferenceTransform->Translate( 0., 0., -1850.);
 // patientSupportToFixedReferenceTransform->Concatenate(rotateYTransform);
  patientSupportToFixedReferenceTransform->Modified();
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialNode* vtkSlicerIhepStandGeometryLogic::CreateTableTopStandFiducialNode(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkNew<vtkMRMLMarkupsFiducialNode> pointsMarkupsNode;
  this->GetMRMLScene()->AddNode(pointsMarkupsNode);
  pointsMarkupsNode->SetName(TABLETOPSTAND_FIDUCIALS_MARKUPS_NODE_NAME);
//  pointsMarkupsNode->SetHideFromEditors(1);
//  std::string singletonTag = std::string("IHEP_") + TABLETOPSTAND_FIDUCIALS_MARKUPS_NODE_NAME;
//  pointsMarkupsNode->SetSingletonTag(singletonTag.c_str());

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("CreateTableTopFiducialNode: Invalid MRML scene");
    return nullptr;
  }

  if (parameterNode)
  {
    // add points to fiducial node
    vtkVector3d p0( 250., 1450., -120. - 650.); // Origin
    vtkVector3d p1( -250., 1450., -120. - 650.); // Mirror
    vtkVector3d p2( 0., 2050., -120. - 650.); // Middle

    pointsMarkupsNode->AddControlPoint( p0, "Origin");
    pointsMarkupsNode->AddControlPoint( p1, "Mirror");
    pointsMarkupsNode->AddControlPoint( p2, "Middle");

    vtkMRMLTransformNode* transformNode = this->UpdateTableTopStandMarkupsTransform(parameterNode);

    // add transform to fiducial node
    if (transformNode)
    {
      pointsMarkupsNode->SetAndObserveTransformNodeID(transformNode->GetID());
    }
    
  }
  return pointsMarkupsNode;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsPlaneNode* vtkSlicerIhepStandGeometryLogic::CreateTableTopStandPlaneNode(
  vtkMRMLIhepStandGeometryNode* parameterNode, vtkMRMLMarkupsFiducialNode* pointsMarkupsNode)
{
  vtkNew<vtkMRMLMarkupsPlaneNode> tableTopPlaneNode;
  this->GetMRMLScene()->AddNode(tableTopPlaneNode);
  tableTopPlaneNode->SetName(TABLETOP_PLANE_MARKUPS_NODE_NAME);
//  tableTopPlaneNode->SetHideFromEditors(1);
//  singletonTag = std::string("IHEP_") + TABLETOP_PLANE_MARKUPS_NODE_NAME;
//  tableTopPlaneNode->SetSingletonTag(singletonTag.c_str());

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

//    vtkVector3d plane0( originWorld[0], originWorld[1], originWorld[2]); // Origin
    vtkVector3d plane1( mirrorWorld[0], mirrorWorld[1], mirrorWorld[2]); // Mirror
    vtkVector3d plane2( middleWorld[0], middleWorld[1], middleWorld[2]); // Middle

    tableTopPlaneNode->SetOrigin(originWorld);
    tableTopPlaneNode->AddControlPoint( plane1, "MirrorPlane");
    tableTopPlaneNode->AddControlPoint( plane2, "OriginPlane");
  }
  return tableTopPlaneNode;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsLineNode* vtkSlicerIhepStandGeometryLogic::CreateFixedReferenceLineNode(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkNew<vtkMRMLMarkupsLineNode> lineMarkupsNode;
  this->GetMRMLScene()->AddNode(lineMarkupsNode);
  lineMarkupsNode->SetName(FIXEDREFERENCE_LINE_MARKUPS_NODE_NAME);
//  pointsMarkupsNode->SetHideFromEditors(1);
//  std::string singletonTag = std::string("IHEP_") + TABLETOPSTAND_FIDUCIALS_MARKUPS_NODE_NAME;
//  pointsMarkupsNode->SetSingletonTag(singletonTag.c_str());


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
void vtkSlicerIhepStandGeometryLogic::UpdateTableTopStandFiducialNode(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("UpdateTableTopStandFiducialNode: Invalid MRML scene");
    return;
  }

  if (!parameterNode)
  {
    vtkErrorMacro("UpdateTableTopStandFiducialNode: Invalid parameter node");
    return;
  }

  vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode();
  if (!beamNode)
  {
    vtkErrorMacro("UpdateTableTopStandFiducialNode: Invalid beam node");
    return;
  }

  // fiducial markups node
  if (scene->GetFirstNodeByName(TABLETOPSTAND_FIDUCIALS_MARKUPS_NODE_NAME))
  {
    vtkMRMLMarkupsFiducialNode* pointsMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
      scene->GetFirstNodeByName(TABLETOPSTAND_FIDUCIALS_MARKUPS_NODE_NAME));

    vtkVector3d p0( 250., 
      1450. + parameterNode->GetTableTopVerticalPosition(),
      -120. - 650. + parameterNode->GetTableTopVerticalPositionOrigin()); // Origin
    vtkVector3d p1( -250., 
      1450. + parameterNode->GetTableTopVerticalPosition(), 
      -120. - 650. + parameterNode->GetTableTopVerticalPositionMirror()); // Mirror
    vtkVector3d p2( 0., 
      2050. + parameterNode->GetTableTopVerticalPosition(), 
      -120. - 650. + parameterNode->GetTableTopVerticalPositionMiddle()); // Middle
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
    vtkMRMLTransformNode* markupsTransformNode = this->UpdateTableTopStandMarkupsTransform(parameterNode);

    if (markupsTransformNode)
    {
      pointsMarkupsNode->SetAndObserveTransformNodeID(markupsTransformNode->GetID());
    }
  }
  else
  {
    this->CreateTableTopStandFiducialNode(parameterNode);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateTableTopStandPlaneNode( 
  vtkMRMLIhepStandGeometryNode* parameterNode, vtkMRMLMarkupsFiducialNode* pointsMarkupsNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("UpdateTableTopStandMarkupsNodes: Invalid MRML scene");
    return;
  }

  if (!parameterNode)
  {
    vtkErrorMacro("UpdateTableTopStandMarkupsNodes: Invalid parameter node");
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

    vtkVector3d plane1( mirrorWorld[0], mirrorWorld[1], mirrorWorld[2]); // Mirror
    vtkVector3d plane2( middleWorld[0], middleWorld[1], middleWorld[2]); // Middle

    tableTopPlaneNode->SetOrigin(originWorld);
    tableTopPlaneNode->SetNthControlPointPosition( 1, plane1.GetX(), plane1.GetY(), plane1.GetZ());
    tableTopPlaneNode->SetNthControlPointPosition( 2, plane2.GetX(), plane2.GetY(), plane2.GetZ());
  }
  else
  {
    this->CreateTableTopStandPlaneNode( parameterNode, pointsMarkupsNode);
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
  if (scene->GetFirstNodeByName(TABLETOPSTAND_FIDUCIALS_MARKUPS_NODE_NAME))
  {
    vtkMRMLMarkupsFiducialNode* pointsMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
      scene->GetFirstNodeByName(TABLETOPSTAND_FIDUCIALS_MARKUPS_NODE_NAME));
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

  // Canyon - mandatory
  std::string canyonModelSingletonTag = machineType + "_" + CANYON_MODEL_NAME;
  vtkMRMLModelNode* canyonModelNode = vtkMRMLModelNode::SafeDownCast(
    scene->GetSingletonNode(canyonModelSingletonTag.c_str(), "vtkMRMLModelNode") );
  if (canyonModelNode && !canyonModelNode->GetPolyData())
  {
    // Remove node if contains empty polydata (e.g. after closing scene), so that it can be loaded again
    scene->RemoveNode(canyonModelNode);
    canyonModelNode = nullptr;
  }
  if (!canyonModelNode)
  {
    std::string canyonModelFilePath = treatmentMachineModelsDirectory + "/" + CANYON_MODEL_NAME + ".stl";
    if (vtksys::SystemTools::FileExists(canyonModelFilePath))
    {
      canyonModelNode = modelsLogic->AddModel(canyonModelFilePath.c_str());
    }
    if (canyonModelNode)
    {
      canyonModelNode->SetSingletonTag(canyonModelSingletonTag.c_str());
      vtkNew<vtkMRMLModelHierarchyNode> canyonModelHierarchyNode;
      scene->AddNode(canyonModelHierarchyNode);
      canyonModelHierarchyNode->SetModelNodeID(canyonModelNode->GetID());
      canyonModelHierarchyNode->SetParentNodeID(rootModelHierarchyNode->GetID());
      canyonModelHierarchyNode->HideFromEditorsOn();
    }
    else
    {
      vtkErrorMacro("LoadTreatmentMachineModels: Failed to load canyon model");
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

  // Table top stand - mandatory
  std::string tableTopStandModelSingletonTag = machineType + "_" + TABLETOPSTAND_MODEL_NAME;
  vtkMRMLModelNode* tableTopStandModelNode = vtkMRMLModelNode::SafeDownCast(
    scene->GetSingletonNode(tableTopStandModelSingletonTag.c_str(), "vtkMRMLModelNode") );
  if (tableTopStandModelNode && !tableTopStandModelNode->GetPolyData())
  {
    // Remove node if contains empty polydata (e.g. after closing scene), so that it can be loaded again
    scene->RemoveNode(tableTopStandModelNode);
    tableTopStandModelNode = nullptr;
  }
  if (!tableTopStandModelNode)
  {
    std::string tableTopStandModelFilePath = treatmentMachineModelsDirectory + "/" + TABLETOPSTAND_MODEL_NAME + ".stl";
    if (vtksys::SystemTools::FileExists(tableTopStandModelFilePath))
    {
      tableTopStandModelNode = modelsLogic->AddModel(tableTopStandModelFilePath.c_str());
    }
    if (tableTopStandModelNode)
    {
      tableTopStandModelNode->SetSingletonTag(tableTopStandModelSingletonTag.c_str());
      vtkNew<vtkMRMLModelHierarchyNode> tableTopStandModelHierarchyNode;
      scene->AddNode(tableTopStandModelHierarchyNode);
      tableTopStandModelHierarchyNode->SetModelNodeID(tableTopStandModelNode->GetID());
      tableTopStandModelHierarchyNode->SetParentNodeID(rootModelHierarchyNode->GetID());
      tableTopStandModelHierarchyNode->HideFromEditorsOn();
    }
    else
    {
      vtkErrorMacro("LoadTreatmentMachineModels: Failed to load table top stand model");
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

  if (!canyonModelNode || !canyonModelNode->GetPolyData() 
    || !tableTopStandModelNode || !tableTopStandModelNode->GetPolyData() 
    || !patientSupportModelNode || !patientSupportModelNode->GetPolyData() 
    || !tableTopModelNode || !tableTopModelNode->GetPolyData())
  {
    vtkErrorMacro("LoadTreatmentMachineModels: Failed to load every mandatory treatment machine component");
    return;
  }

  // Create / Update markups nodes if they already exists 
  // Update markups (TableTop fiducials and plane nodes and FixedReference line node)
  vtkMRMLMarkupsFiducialNode* pointsMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
    scene->GetFirstNodeByName(TABLETOPSTAND_FIDUCIALS_MARKUPS_NODE_NAME));

  this->UpdateTableTopStandFiducialNode(parameterNode);

  if (pointsMarkupsNode)
  {
    this->UpdateTableTopStandPlaneNode(parameterNode, pointsMarkupsNode);
  }
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

  // Table top stand model - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableTopVertical -> TableTopStand
  vtkNew<vtkGeneralTransform> rasToTableTopStandGeneralTransform;
  vtkNew<vtkTransform> rasToTableTopStandLinearTransform;
  if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::TableTopStand, 
    rasToTableTopStandGeneralTransform, false))
  {
    // Convert general transform to linear
    // This call also makes hard copy of the transform so that it doesn't change when other beam transforms change
    if (!vtkMRMLTransformNode::IsGeneralTransformLinear( rasToTableTopStandGeneralTransform, 
      rasToTableTopStandLinearTransform))
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to get transform hierarchy from RAS to TableTopStand");
      return;
    }
    // Transform to RAS, set transform to node, transform the model
    rasToTableTopStandLinearTransform->Concatenate(patientToRasTransform);

    // Find RasToTableTopInferiorSuperiorTransform or create it
    vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableTopStandTransformNode;
    if (scene->GetFirstNodeByName("RasToTableTopStandTransform"))
    {
      rasToTableTopStandTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
        scene->GetFirstNodeByName("RasToTableTopStandTransform"));
    }
    else
    {
      rasToTableTopStandTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      rasToTableTopStandTransformNode->SetName("RasToTableTopStandTransform");
//      rasToTableTopStandTransformNode->SetHideFromEditors(1);
//      rasToTableTopStandTransformNode->SetSingletonTag("IHEP_");
      scene->AddNode(rasToTableTopStandTransformNode);
    }

    vtkMRMLModelNode* tableTopStandModel = vtkMRMLModelNode::SafeDownCast(
      scene->GetFirstNodeByName(TABLETOPSTAND_MODEL_NAME) );
    if (!tableTopStandModel)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to access table top stand model");
      return;
    }
    if (rasToTableTopStandTransformNode)
    {
      rasToTableTopStandTransformNode->SetAndObserveTransformToParent(rasToTableTopStandLinearTransform);
      tableTopStandModel->SetAndObserveTransformNodeID(rasToTableTopStandTransformNode->GetID());
      tableTopStandModel->CreateDefaultDisplayNodes();
      tableTopStandModel->GetDisplayNode()->SetColor(0.95, 0.95, 0.95);
    }
  }

  // Table top - mandatory
  // Transform path: RAS -> Patient -> TableTop
  vtkNew<vtkGeneralTransform> rasToTableTopGeneralTransform;
  vtkNew<vtkTransform> rasToTableTopLinearTransform;
  if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::TableTop, 
    rasToTableTopGeneralTransform, false))
  {
    // Convert general transform to linear
    // This call also makes hard copy of the transform so that it doesn't change when other beam transforms change
    if (!vtkMRMLTransformNode::IsGeneralTransformLinear( rasToTableTopGeneralTransform, 
      rasToTableTopLinearTransform))
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to get transform hierarchy from RAS to TableTop");
      return;
    }
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

  // Patient support - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableTopVectical -> TableTopStand -> PatientSupport
  vtkNew<vtkGeneralTransform> rasToPatientSupportGeneralTransform;
  vtkNew<vtkTransform> rasToPatientSupportLinearTransform;
  if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::PatientSupport, 
    rasToPatientSupportGeneralTransform, false))
  {
    // Convert general transform to linear
    // This call also makes hard copy of the transform so that it doesn't change when other beam transforms change
    if (!vtkMRMLTransformNode::IsGeneralTransformLinear( rasToPatientSupportGeneralTransform, 
      rasToPatientSupportLinearTransform))
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to get transform hierarchy from RAS to PatientSupport");
      return;
    }

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
      patientSupportModel->GetDisplayNode()->SetColor(0.85, 0.85, 0.85);
    }
  }

  // Fixed Reference (canyon) - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableTopStand -> PatientSupport -> FixedReference
  vtkNew<vtkGeneralTransform> rasToFixedReferenceGeneralTransform;
  vtkNew<vtkTransform> rasToFixedReferenceLinearTransform;
  if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::FixedReference, 
    rasToFixedReferenceGeneralTransform, false))
  {
    // Convert general transform to linear
    // This call also makes hard copy of the transform so that it doesn't change when other beam transforms change
    if (!vtkMRMLTransformNode::IsGeneralTransformLinear( rasToFixedReferenceGeneralTransform, 
      rasToFixedReferenceLinearTransform))
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to get transform hierarchy from RAS to FixedReference");
      return;
    }
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

    vtkMRMLModelNode* canyonModel = vtkMRMLModelNode::SafeDownCast(
      this->GetMRMLScene()->GetFirstNodeByName(CANYON_MODEL_NAME) );
    if (!canyonModel)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to access table top model");
      return;
    }
    if (rasToFixedReferenceTransformNode)
    {
      canyonModel->SetAndObserveTransformNodeID(rasToFixedReferenceTransformNode->GetID());
      canyonModel->CreateDefaultDisplayNodes();
      canyonModel->GetDisplayNode()->SetColor(0.7, 0.65, 0.65);
    }
  }

  // Update markups (TableTop fiducials and plane nodes and FixedReference line node)
  vtkMRMLMarkupsFiducialNode* pointsMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
    scene->GetFirstNodeByName(TABLETOPSTAND_FIDUCIALS_MARKUPS_NODE_NAME));

  this->UpdateTableTopStandFiducialNode(parameterNode);

  if (pointsMarkupsNode)
  {
    this->UpdateTableTopStandPlaneNode(parameterNode, pointsMarkupsNode);
  }
  this->UpdateFixedReferenceLineNode(parameterNode);
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateTableTopVerticalToTableTopStandTransform(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableTopVecticalToTableTopStandTransform: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("UpdateTableTopVecticalToTableTopStandTransform: Invalid parameter node");
    return;
  }

  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* tableTopVerticalToTableTopStandMovementTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableTopVertical, IHEP::TableTopStand);

  if (tableTopVerticalToTableTopStandMovementTransformNode)
  {
    vtkNew<vtkTransform> tableTopVerticalToTableTopStandMovementTransform;
    tableTopVerticalToTableTopStandMovementTransform->Translate( 0., 0., -1. * parameterNode->GetTableTopVerticalPosition());
    tableTopVerticalToTableTopStandMovementTransformNode->SetAndObserveTransformToParent(tableTopVerticalToTableTopStandMovementTransform);
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
    this->IhepLogic->GetTransformNodeBetween(IHEP::PatientSupport, IHEP::FixedReference);

  if (patientSupportToFixedReferenceTransformNode)
  {
    // Patient Support To Fixed Reference rotation matrix
    vtkNew<vtkTransform> patientSupportToFixedReferenceTransform;
    double PatientTableTopTranslation[3] = {};
    parameterNode->GetPatientToTableTopTranslation(PatientTableTopTranslation);
    // Move to Origin
    patientSupportToFixedReferenceTransform->Translate( -1. * parameterNode->GetTableTopLateralPosition() + PatientTableTopTranslation[0], 
      parameterNode->GetTableTopLongitudinalPosition() + PatientTableTopTranslation[1], 
      -1. * parameterNode->GetTableTopVerticalPosition() + PatientTableTopTranslation[2]);
    // Apply rotation
    patientSupportToFixedReferenceTransform->RotateZ(-1. * parameterNode->GetPatientSupportRotationAngle());
    // Move back
    patientSupportToFixedReferenceTransform->Translate( parameterNode->GetTableTopLateralPosition() - PatientTableTopTranslation[0], 
      -1. * parameterNode->GetTableTopLongitudinalPosition() - PatientTableTopTranslation[1], 
      parameterNode->GetTableTopVerticalPosition() - PatientTableTopTranslation[2]);
    // Observe transform
    patientSupportToFixedReferenceTransformNode->SetAndObserveTransformToParent(patientSupportToFixedReferenceTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateTableTopStandToPatientSupportTransform(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableTopStandToPatientSupportTransfrom: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("UpdateTableTopStandToPatientSupportTransfrom: Invalid parameter node");
    return;
  }

  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* tableTopStandToPatientSupportMovementTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableTopStand, IHEP::PatientSupport);

  if (tableTopStandToPatientSupportMovementTransformNode)
  {
    vtkNew<vtkTransform> tableTopStandToPatientSupportMovementTransform;
    
    tableTopStandToPatientSupportMovementTransform->Translate( -1. * parameterNode->GetTableTopLateralPosition(), 
      parameterNode->GetTableTopLongitudinalPosition(), 0.);

    tableTopStandToPatientSupportMovementTransformNode->SetAndObserveTransformToParent(tableTopStandToPatientSupportMovementTransform);
  }
  UpdatePatientSupportToFixedReferenceTransform(parameterNode);
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

    vtkNew<vtkTransform> patientToTableTopTransform;
    patientToTableTopTransform->Translate(patientToTableTopTranslation);
    patientToTableTopTransformNode->SetAndObserveTransformToParent(patientToTableTopTransform);
  }
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerIhepStandGeometryLogic::UpdateTableTopStandMarkupsTransform(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateTableTopStandMarkupsTransform: Invalid parameter node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableTopStandMarkupsTransform: Invalid MRML scene");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (!scene->GetFirstNodeByName(TABLETOPSTAND_FIDUCIALS_TRANSFORM_NODE_NAME))
  {
    transformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    transformNode->SetName(TABLETOPSTAND_FIDUCIALS_TRANSFORM_NODE_NAME);
//    transformNode->SetHideFromEditors(1);
    transformNode->SetSingletonTag("TABLETOPSTAND_FIDUCIAL_Transform");
    scene->AddNode(transformNode);
  }
  else
  {
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast(
      scene->GetFirstNodeByName(TABLETOPSTAND_FIDUCIALS_TRANSFORM_NODE_NAME));
  }

  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;
  // Dynamic transform from Gantry to RAS
  // Transformation path: RAS -> Patient -> TableTop -> TableTopVertical -> TableTopStand
  vtkNew<vtkGeneralTransform> generalTransform;
  if (transformNode && this->IhepLogic->GetTransformBetween( IHEP::RAS, 
    IHEP::TableTopStand, generalTransform, false))
  {
    // Convert general transform to linear
    // This call also makes hard copy of the transform so that it doesn't change when other beam transforms change
    vtkNew<vtkTransform> rasToTableTopStandTransform;
    if (!vtkMRMLTransformNode::IsGeneralTransformLinear(generalTransform, rasToTableTopStandTransform))
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
    linearTransform->Concatenate(rasToTableTopStandTransform);
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
  // Transformation path: RAS -> Patient -> TableTop -> TableTopVertical -> TableTopStand -> FixedReference
  vtkNew<vtkGeneralTransform> generalTransform;
  if (transformNode && this->IhepLogic->GetTransformBetween( IHEP::RAS, 
    IHEP::FixedReference, generalTransform, false))
  {
    // Convert general transform to linear
    // This call also makes hard copy of the transform so that it doesn't change when other beam transforms change
    vtkNew<vtkTransform> rasToFixedReferenceTransform;
    if (!vtkMRMLTransformNode::IsGeneralTransformLinear(generalTransform, rasToFixedReferenceTransform))
    {
      vtkErrorMacro("UpdateFixedReferenceMarkupsTransform: Unable to set transform with non-linear components");
      return nullptr;
    }

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
