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

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLLinearTransformNode.h>

//#include <vtkMRMLMarkupsPlaneNode.h>
#include <vtkMRMLMarkupsLineNode.h>

#include <vtkMRMLScene.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLModelHierarchyNode.h>
#include <vtkMRMLModelDisplayNode.h>

// Slicer includes
#include <vtkSlicerModelsLogic.h>
#include <vtkSlicerSegmentationsModuleLogic.h>

// vtkSegmentationCore includes
#include <vtkSegmentationConverter.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>
#include <vtkTransform.h>
#include <vtkAppendPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtksys/SystemTools.hxx>
#include <vtkTransformPolyDataFilter.h>
#include <vtkGeneralTransform.h>
#include <vtkTransformFilter.h>

namespace
{
// Initial positions
const double PatientSupportToTableTopSurfaceDistanceAxisZ = 925.; // mm
const double TableTopFromIsocenterDistanceAxisY = 1400.; // mm

}

//----------------------------------------------------------------------------
// Treatment machine component names
const char* vtkSlicerIhepStandGeometryLogic::CANYON_MODEL_NAME = "Canyon";
const char* vtkSlicerIhepStandGeometryLogic::PATIENTSUPPORT_MODEL_NAME = "PatientSupportRotation";
const char* vtkSlicerIhepStandGeometryLogic::TABLETOPSTAND_MODEL_NAME = "TableTopStand";
const char* vtkSlicerIhepStandGeometryLogic::TABLETOP_MODEL_NAME = "TableTop";
const char* vtkSlicerIhepStandGeometryLogic::BEAMLINE_MARKUPS_NODE_NAME = "BeamLine";

const char* vtkSlicerIhepStandGeometryLogic::BEAMLINE_TRANSFORM_NODE_NAME = "IhepStandGeometryBeamlineTransform";

const char* vtkSlicerIhepStandGeometryLogic::ORIENTATION_MARKER_MODEL_NODE_NAME = "IhepStandGeometryOrientationMarker";

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerIhepStandGeometryLogic);

//----------------------------------------------------------------------------
vtkSlicerIhepStandGeometryLogic::vtkSlicerIhepStandGeometryLogic()
  :
  IhepLogic(vtkSlicerIhepStandGeometryTransformLogic::New())
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


//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::CreateMarkupsNodes(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("CreateMarkupsNodes: Invalid MRML scene");
    return;
  }

  if (!parameterNode)
  {
    vtkErrorMacro("CreateMarkupsNodes: Invalid parameter node");
    return;
  }

  vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode();
  vtkMRMLTransformNode* transformNode = nullptr;
  if (beamNode)
  {
    transformNode = this->UpdateBeamlineTransformFromBeam(beamNode);
  }

  // Create markups nodes if they don't exist

  // Beamline markups node
  vtkSmartPointer<vtkMRMLMarkupsLineNode> beamlineMarkupsNode;
  if (!scene->GetFirstNodeByName(BEAMLINE_MARKUPS_NODE_NAME))
  {
    beamlineMarkupsNode = this->CreateBeamline(parameterNode);
  }
  else
  {
    beamlineMarkupsNode = vtkMRMLMarkupsLineNode::SafeDownCast(
      scene->GetFirstNodeByName(BEAMLINE_MARKUPS_NODE_NAME));
    // Update beamline points using IhepStandGeometry node data
    if (transformNode)
    {
      beamlineMarkupsNode->SetAndObserveTransformNodeID(transformNode->GetID());
    }
  }
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateMarkupsNodes(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("UpdateMarkupsNodes: Invalid MRML scene");
    return;
  }

  if (!parameterNode)
  {
    vtkErrorMacro("UpdateMarkupsNodes: Invalid parameter node");
    return;
  }

  vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode();
  if (!beamNode)
  {
    vtkErrorMacro("UpdateMarkupsNodes: Invalid beam node");
    return;
  }

  vtkMRMLTransformNode* transformNode = this->UpdateBeamlineTransformFromBeam(beamNode);

  // beamline markups line node
  if (scene->GetFirstNodeByName(BEAMLINE_MARKUPS_NODE_NAME))
  {
    vtkMRMLMarkupsLineNode* beamlineMarkupsNode = vtkMRMLMarkupsLineNode::SafeDownCast(
      scene->GetFirstNodeByName(BEAMLINE_MARKUPS_NODE_NAME));

    // update points
    vtkVector3d p0( 0., 0., -5000.);
    vtkVector3d p1( 0., 0., 5000.);

    double* p = beamlineMarkupsNode->GetNthControlPointPosition(0);
    if (p)
    {
      beamlineMarkupsNode->SetNthControlPointPosition( 0, p0.GetX(), p0.GetY(), p0.GetZ());
    }
    else
    {
      beamlineMarkupsNode->AddControlPoint(p0);
    }
    
    p = beamlineMarkupsNode->GetNthControlPointPosition(1);
    if (p)
    {
      beamlineMarkupsNode->SetNthControlPointPosition( 1, p1.GetX(), p1.GetY(), p1.GetZ());
    }
    else
    {
      beamlineMarkupsNode->AddControlPoint(p1);
    }

    // Update beamline markups transform node if it's changed    
    vtkMRMLTransformNode* markupsTransformNode = beamlineMarkupsNode->GetParentTransformNode();

    if (markupsTransformNode)
    {
      beamlineMarkupsNode->SetAndObserveTransformNodeID(transformNode->GetID());
    }
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
  if (scene->GetFirstNodeByName(BEAMLINE_MARKUPS_NODE_NAME))
  {
    vtkMRMLMarkupsLineNode* beamlineMarkupsNode = vtkMRMLMarkupsLineNode::SafeDownCast(
      scene->GetFirstNodeByName(BEAMLINE_MARKUPS_NODE_NAME));
    beamlineMarkupsNode->SetDisplayVisibility(int(toggled));
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

//---------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::RestoreOriginalGeometryTransformHierarchy()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("RestoreOriginalGeometryTransformHierarchy: Invalid MRML scene");
    return;
  }

  // Restore Ihep hierarchy
//  this->IhepLogic->RestoreIhepTransformHierarchy();
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerIhepStandGeometryLogic::UpdateBeamlineTransformFromBeam(vtkMRMLRTBeamNode* beamNode)
{
  if (!beamNode)
  {
    vtkErrorMacro("UpdateBeamlineTransformFromBeam: Invalid beam node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateBeamlineTransformFromBeam: Invalid MRML scene");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (!scene->GetFirstNodeByName(BEAMLINE_TRANSFORM_NODE_NAME))
  {
    transformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    transformNode->SetName(BEAMLINE_TRANSFORM_NODE_NAME);
    transformNode->SetHideFromEditors(1);
    transformNode->SetSingletonTag("IHEP_Transform");
    scene->AddNode(transformNode);
  }
  else
  {
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast(
      scene->GetFirstNodeByName(BEAMLINE_TRANSFORM_NODE_NAME));
  }

  // Update transforms in IHEP logic from beam node parameters
  this->IhepLogic->UpdateIHEPTransformsFromBeam(beamNode);

  // Dynamic transform from FixedReference to RAS
  // Transformation path:
  // FixedReference -> PatientSupport -> TableTopEccentricRotation -> TableTop -> Patient -> RAS
  vtkNew<vtkGeneralTransform> generalTransform;
  if (this->IhepLogic->GetTransformBetween( vtkSlicerIhepStandGeometryTransformLogic::FixedReference, 
    vtkSlicerIhepStandGeometryTransformLogic::RAS, generalTransform))
  {
    // Convert general transform to linear
    // This call also makes hard copy of the transform so that it doesn't change when other beam transforms change
    vtkNew<vtkTransform> linearTransform;
    if (!vtkMRMLTransformNode::IsGeneralTransformLinear(generalTransform, linearTransform))
    {
      vtkErrorMacro("UpdateImageTransformFromBeam: Unable to set transform with non-linear components to beam " << beamNode->GetName());
      return nullptr;
    }
    // Set transform to node
    transformNode->SetAndObserveTransformToParent(linearTransform);
  }
  return transformNode;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsLineNode* vtkSlicerIhepStandGeometryLogic::CreateBeamline(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkNew<vtkMRMLMarkupsLineNode> beamlineMarkupsNode;
  this->GetMRMLScene()->AddNode(beamlineMarkupsNode);
  beamlineMarkupsNode->SetName(BEAMLINE_MARKUPS_NODE_NAME);
  beamlineMarkupsNode->SetHideFromEditors(1);
//  std::string singletonTag = std::string("IHEP_") + BEAMLINE_MARKUPS_NODE_NAME;
//  beamlineMarkupsNode->SetSingletonTag(singletonTag.c_str());

  if (parameterNode)
  {
    // add points
    vtkVector3d p0( 0, 0, -5000.);
    vtkVector3d p1( 0, 0, 5000.);

    beamlineMarkupsNode->AddControlPoint(p0);
    beamlineMarkupsNode->AddControlPoint(p1);

    if (vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode())
    {
      vtkMRMLTransformNode* transformNode = this->UpdateBeamlineTransformFromBeam(beamNode);

      if (transformNode)
      {
        beamlineMarkupsNode->SetAndObserveTransformNodeID(transformNode->GetID());
      }
    }
  }
  return beamlineMarkupsNode;
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

  if ( !canyonModelNode || !canyonModelNode->GetPolyData()
    || !tableTopStandModelNode || !tableTopStandModelNode->GetPolyData()
    || !patientSupportModelNode || !patientSupportModelNode->GetPolyData()
    || !tableTopModelNode || !tableTopModelNode->GetPolyData() )
  {
    vtkErrorMacro("LoadTreatmentMachineModels: Failed to load every mandatory treatment machine component");
    return;
  }

  // Setup treatment machine model display and transforms
//  this->SetupTreatmentMachineModels(parameterNode);
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
  this->SetupTreatmentMachineModels(parameterNode);
/*
  vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode();
  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;

  // Update TableTop -> TableTopInferiorSuperiorMovement
  // Translation and rotation of the model
  vtkMRMLLinearTransformNode* tableTopToTableTopStandTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableTop, IHEP::TableTopStand);
  vtkTransform* tableTopToTableTopStandTransform = vtkTransform::SafeDownCast(
    tableTopToTableTopStandTransformNode->GetTransformToParent() );
  tableTopToTableTopStandTransform->Identity();
//  tableTopToTableTopStandTransform->Translate( 0., 490., -550.);
//  tableTopToTableTopStandTransform->RotateY(-1. * parameterNode->GetTableTopLongitudinalAngle());
//  tableTopToTableTopStandTransform->RotateX(-1. * parameterNode->GetTableTopLateralAngle());
  tableTopToTableTopStandTransform->Modified();

  // Update TableTopInferiorSuperiorMovement -> PatientSupport
  // Translation of the model
  vtkMRMLLinearTransformNode* tableTopStandToPatientSupportTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableTopStand, IHEP::PatientSupport);
  vtkTransform* tableTopStandToPatientSupportTransform = vtkTransform::SafeDownCast(
    tableTopStandToPatientSupportTransformNode->GetTransformToParent() );
  tableTopStandToPatientSupportTransform->Identity();
//  tableTopStandToPatientSupportTransform->Translate( 0., -490., 550.);
  tableTopStandToPatientSupportTransform->Modified();

  // Update PatientSupport -> FixedReference
  vtkMRMLLinearTransformNode* patientSupportRotationToFixedReferenceTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::PatientSupport, IHEP::FixedReference);
  vtkTransform* patientSupportRotationToFixedReferenceTransform = vtkTransform::SafeDownCast(
    patientSupportRotationToFixedReferenceTransformNode->GetTransformToParent() );
  patientSupportRotationToFixedReferenceTransform->Identity();
//  patientSupportRotationToFixedReferenceTransform->Translate( 0., 490., -550.);
  patientSupportRotationToFixedReferenceTransform->RotateZ(-1. * parameterNode->GetPatientSupportRotationAngle());
  patientSupportRotationToFixedReferenceTransform->Modified();

  // Update Collimator -> FixedFerence
  vtkMRMLLinearTransformNode* collimatorToFixedReferenceTransformNode =
   this->IhepLogic->GetTransformNodeBetween(IHEP::Collimator, IHEP::FixedReference);
  vtkTransform* collimatorToFixedReferenceTransform = vtkTransform::SafeDownCast(collimatorToFixedReferenceTransformNode->GetTransformToParent());
  collimatorToFixedReferenceTransform->Identity();
  collimatorToFixedReferenceTransform->RotateZ(90 - parameterNode->GetTableTopLongitudinalAngle());
  collimatorToFixedReferenceTransform->Modified();

  // Update Patient -> TableTop
  // Translation
  vtkMRMLLinearTransformNode* patientToTableTopTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::Patient, IHEP::TableTop);
  vtkTransform* patientToTableTopTransform = vtkTransform::SafeDownCast(
    patientToTableTopTransformNode->GetTransformToParent() );

//  double isocenter[3] = {};
//  vtkMRMLRTBeamNode* beam = parameterNode->GetBeamNode();
//  beam->GetPlanIsocenterPosition(isocenter);

  patientToTableTopTransform->Identity();
//  patientToTableTopTransform->Translate( isocenter[0], isocenter[1], isocenter[2]);
//  patientToTableTopTransform->Translate( isocenter[0], isocenter[1] - 490, isocenter[2] + 550);
  patientToTableTopTransform->Translate( 0., -490., 550.);
  patientToTableTopTransform->Modified();

//  this->UpdateTableTopToTableTopEccentricRotationTransform(parameterNode);

  // New Treatment machine position
  this->SetupTreatmentMachineModels(parameterNode);

  // Update table top vertical position according to isocenter position
//  this->UpdateTableTopToTableTopEccentricRotationTransform(parameterNode);
  // Update table top longitudinal position according to isocenter position
//  this->UpdateTableTopToTableTopEccentricRotationTransform(parameterNode);
  // Update patient support rotation angle
//  this->UpdatePatientSupportRotationToFixedReferenceTransform(parameterNode);

  // Set required transform to the models
//  double tmpIsocenter[3] = {};
//  this->MoveModelsToIsocenter( parameterNode, tmpIsocenter);
*/
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

  // Transform IHEP stand models to RAS
  vtkNew<vtkTransform> rotateYTransform;
  rotateYTransform->Identity();
  rotateYTransform->RotateX(-90.);
  rotateYTransform->RotateZ(180.);

  // Table top stand (Inferior<->Superior movement) model - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableTopStand
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
    rasToTableTopStandLinearTransform->Concatenate(rotateYTransform);

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
    rasToTableTopLinearTransform->Concatenate(rotateYTransform);

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
  // Transform path: RAS -> Patient -> TableTop -> TableTopStand -> PatientSupport
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
    rasToPatientSupportLinearTransform->Concatenate(rotateYTransform);

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

    rasToFixedReferenceLinearTransform->Concatenate(rotateYTransform);

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

/*
    // Find RasToPatientSupportTransform or create it
    vtkSmartPointer<vtkMRMLLinearTransformNode> rasToPatientSupportTransformNode;
    if (scene->GetFirstNodeByName("RasToPatientSupportTransform"))
    {
      rasToPatientSupportTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
        scene->GetFirstNodeByName("RasToPatientSupportTransform"));
    }

    // Create transform PatientSupportToFixedReferenceTransform
    vtkNew<vtkTransform> patientSupportToFixedReferenceTransform;
    patientSupportToFixedReferenceTransform->Identity();
    patientSupportToFixedReferenceTransform->RotateY(-1. * parameterNode->GetPatientSupportRotationAngle());
    patientSupportToFixedReferenceTransform->Modified();

    // Find PatientSupportToFixedReferenceTransform1 or create it
    vtkSmartPointer<vtkMRMLLinearTransformNode> patientSupportToFixedReferenceTransformNode1;
    if (scene->GetFirstNodeByName("PatientSupportToFixedReferenceTransform1"))
    {
      patientSupportToFixedReferenceTransformNode1 = vtkMRMLLinearTransformNode::SafeDownCast(
        scene->GetFirstNodeByName("PatientSupportToFixedReferenceTransform1"));
    }
    else
    {
      patientSupportToFixedReferenceTransformNode1 = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      patientSupportToFixedReferenceTransformNode1->SetName("PatientSupportToFixedReferenceTransform1");
//      patientSupportToFixedReferenceTransformNode1->SetHideFromEditors(1);
//      patientSupportToFixedReferenceTransformNode1->SetSingletonTag("IHEP_");
      scene->AddNode(patientSupportToFixedReferenceTransformNode1);

      patientSupportToFixedReferenceTransformNode1->SetAndObserveTransformToParent(
        patientSupportToFixedReferenceTransform);

      patientSupportToFixedReferenceTransformNode1->SetAndObserveTransformNodeID(
        rasToPatientSupportTransformNode->GetID());
    }

    if (patientSupportToFixedReferenceTransformNode1)
    {
      patientSupportToFixedReferenceTransformNode1->SetAndObserveTransformToParent(
        patientSupportToFixedReferenceTransform);
    }
*/
    vtkMRMLModelNode* canyonModel = vtkMRMLModelNode::SafeDownCast(
      this->GetMRMLScene()->GetFirstNodeByName(CANYON_MODEL_NAME) );
    if (!canyonModel)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to access table top model");
      return;
    }
    if (rasToFixedReferenceTransformNode)
//    if (patientSupportToFixedReferenceTransformNode1)
    {
      canyonModel->SetAndObserveTransformNodeID(rasToFixedReferenceTransformNode->GetID());
//      canyonModel->SetAndObserveTransformNodeID(patientSupportToFixedReferenceTransformNode1->GetID());
      canyonModel->CreateDefaultDisplayNodes();
      canyonModel->GetDisplayNode()->SetColor(0.7, 0.65, 0.65);
    }
  }
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::CalculateAngles(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode();
  if (beamNode)
  {
    vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();
    vtkTransform* beamTransform = nullptr;
    vtkNew<vtkMatrix4x4> mat;
    mat->Identity();

    if (beamTransformNode)
    {
      beamTransform = vtkTransform::SafeDownCast(beamTransformNode->GetTransformToParent());
      beamTransform->GetMatrix(mat);
    }
    else
    {
      vtkErrorMacro("CalculateAngles: Beam transform node is invalid");
      return;
    }
/*
    double tableLongitudinalAngle = 0.0;
    double couchRotationAngle = 0.0;

    if (beamNode->GetGantryAngle() >= 0. && beamNode->GetGantryAngle() <= 180. &&
      beamNode->GetCouchAngle() == 0.0)
    {
      tableLongitudinalAngle = beamNode->GetGantryAngle() - 90.;
      couchRotationAngle = 0.0;
    }
    else if (beamNode->GetGantryAngle() >= 270. && beamNode->GetGantryAngle() <= 360. &&
      beamNode->GetCouchAngle() == 0.0)
    {
      tableLongitudinalAngle = beamNode->GetGantryAngle() - 270.;
      couchRotationAngle = 180.0;
    }
*/
    this->IhepLogic->UpdateIHEPTransformsFromBeam(beamNode);
/*
    double viewUpVector[4] = { -1., 0., 0., 0. }; // beam negative X-axis
//    double viewUpVector[4] = { -1., 0., 0., 0. }; // beam negative X-axis
//    double viewUpVector[4] = { -1., 0., 0., 0. }; // beam negative X-axis
//    double viewUpVector[4] = { -1., 0., 0., 0. }; // beam negative X-axis
    double vup[4];
  
    mat->MultiplyPoint( viewUpVector, vup);

    // Translation to origin for in-place rotation

    vtkWarningMacro("CalculateAngles: Beam view-up Vector " << viewUpVector[0] << " " \
      << viewUpVector[1] << " " << viewUpVector[2]); 

    vtkWarningMacro("CalculateAngles: Beam view-up Vector in RAS " << vup[0] << " " \
      << vup[1] << " " << vup[2]); 
*/
  }
  // Transform IHEP stand models to RAS
  vtkNew<vtkTransform> rotateYTransform;
  rotateYTransform->Identity();
  rotateYTransform->RotateX(-90.);
  rotateYTransform->RotateZ(180.);

  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;
  // Fixed Reference (canyon) - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableTopStand -> PatientSupport -> FixedReference
  vtkNew<vtkGeneralTransform> rasToFixedReferenceGeneralTransform;
  vtkNew<vtkTransform> rasToFixedReferenceLinearTransform;
  if (this->IhepLogic->GetTransformBetween( IHEP::TableTop, IHEP::FixedReference, 
    rasToFixedReferenceGeneralTransform, false))
  {
    // Convert general transform to linear
    // This call also makes hard copy of the transform so that it doesn't change when other beam transforms change
    if (!vtkMRMLTransformNode::IsGeneralTransformLinear( rasToFixedReferenceGeneralTransform, 
      rasToFixedReferenceLinearTransform))
    {
      vtkErrorMacro("CalculateAngles: Unable to get transform hierarchy from RAS to FixedReference");
      return;
    }

    rasToFixedReferenceLinearTransform->Concatenate(rotateYTransform);
  }
  vtkWarningMacro("CalculateAngles: " << rasToFixedReferenceLinearTransform->GetMatrix()->GetElement( 0, 0) << " " \
    << rasToFixedReferenceLinearTransform->GetMatrix()->GetElement( 0, 1) << " " \
    << rasToFixedReferenceLinearTransform->GetMatrix()->GetElement( 0, 2) << " " \
    << rasToFixedReferenceLinearTransform->GetMatrix()->GetElement( 0, 3) << " " \
    << rasToFixedReferenceLinearTransform->GetMatrix()->GetElement( 1, 0) << " " \
    << rasToFixedReferenceLinearTransform->GetMatrix()->GetElement( 1, 1) << " " \
    << rasToFixedReferenceLinearTransform->GetMatrix()->GetElement( 1, 2) << " " \
    << rasToFixedReferenceLinearTransform->GetMatrix()->GetElement( 1, 3) << " " \
    << rasToFixedReferenceLinearTransform->GetMatrix()->GetElement( 2, 0) << " " \
    << rasToFixedReferenceLinearTransform->GetMatrix()->GetElement( 2, 1) << " " \
    << rasToFixedReferenceLinearTransform->GetMatrix()->GetElement( 2, 2) << " " \
    << rasToFixedReferenceLinearTransform->GetMatrix()->GetElement( 2, 3) << " " \
    << rasToFixedReferenceLinearTransform->GetMatrix()->GetElement( 3, 0) << " " \
    << rasToFixedReferenceLinearTransform->GetMatrix()->GetElement( 3, 1) << " " \
    << rasToFixedReferenceLinearTransform->GetMatrix()->GetElement( 3, 2) << " " \
    << rasToFixedReferenceLinearTransform->GetMatrix()->GetElement( 3, 3));
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::MoveModelsToIsocenter(vtkMRMLIhepStandGeometryNode* parameterNode, double isocenter[3])
{

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("MoveModelsToIsocenter: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("MoveModelsToIsocenter: Invalid parameter node");
    return;
  }
/*
  vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode();
  using IEC = vtkSlicerIECTransformLogic::CoordinateSystemIdentifier;

  // Update TableTop -> TableTopEccentricRotation
  vtkMRMLLinearTransformNode* tableTopToTableTopEccentricRotationTransformNode =
    this->IECLogic->GetTransformNodeBetween(IEC::TableTop, IEC::TableTopEccentricRotation);
  vtkTransform* tableTopToTableTopEccentricRotationTransform = vtkTransform::SafeDownCast(
    tableTopToTableTopEccentricRotationTransformNode->GetTransformToParent() );

//  tableTopToTableTopEccentricRotationTransform->Identity();
//  tableTopToTableTopEccentricRotationTransform->RotateY(beamNode->GetGantryAngle() - 90.);
//  tableTopToTableTopEccentricRotationTransform->Translate( 0., -1. * parameterNode->GetTableTopLongitudinalDisplacement(), -1. * parameterNode->GetTableTopVerticalDisplacement());
  tableTopToTableTopEccentricRotationTransform->Modified();

  // Update TableTopInferiorSuperiorMovement -> PatientSupportRotation
  vtkMRMLLinearTransformNode* tableTopInferiorSuperiorToPatientSupportRotationTransformNode =
    this->IECLogic->GetTransformNodeBetween(IEC::TableTopInferiorSuperiorMovement, IEC::PatientSupportRotation);
  vtkTransform* tableTopInferiorSuperiorToPatientSupportRotationTransform = vtkTransform::SafeDownCast(
    tableTopInferiorSuperiorToPatientSupportRotationTransformNode->GetTransformToParent() );

//  tableTopInferiorSuperiorToPatientSupportRotationTransform->Identity();
//  tableTopInferiorSuperiorToPatientSupportRotationTransform->Translate( 0., -1. * parameterNode->GetTableTopLongitudinalDisplacement(), 0.);
  tableTopInferiorSuperiorToPatientSupportRotationTransform->Modified();

  // Update PatientSupportRotation -> FixedReference
  vtkMRMLLinearTransformNode* patientSupportRotationToFixedReferenceTransformNode =
    this->IECLogic->GetTransformNodeBetween(IEC::PatientSupportRotation, IEC::FixedReference);
  vtkTransform* patientSupportRotationToFixedReferenceTransform = vtkTransform::SafeDownCast(
    patientSupportRotationToFixedReferenceTransformNode->GetTransformToParent() );
  patientSupportRotationToFixedReferenceTransform->Identity();
//  patientSupportRotationToFixedReferenceTransform->RotateZ(-1. * beamNode->GetCouchAngle());
  patientSupportRotationToFixedReferenceTransform->Modified();

  // Update Collimator -> Gantry
  vtkMRMLLinearTransformNode* collimatorToGantryTransformNode =
   this->IECLogic->GetTransformNodeBetween(IEC::Collimator, IEC::Gantry);
  vtkTransform* collimatorToGantryTransform = vtkTransform::SafeDownCast(collimatorToGantryTransformNode->GetTransformToParent());
  collimatorToGantryTransform->Identity();
  collimatorToGantryTransform->RotateZ(beamNode->GetCollimatorAngle());
  collimatorToGantryTransform->Modified();

  // Update Patient -> TableTop
  vtkMRMLLinearTransformNode* patientToTableTopTransformNode =
    this->IECLogic->GetTransformNodeBetween(IEC::Patient, IEC::TableTop);
  vtkTransform* patientToTableTopTransform = vtkTransform::SafeDownCast(
    patientToTableTopTransformNode->GetTransformToParent() );

  patientToTableTopTransform->Identity();
//  patientToTableTopTransform->Translate( isocenter[0], isocenter[1] + 490, isocenter[2] + 550);
  patientToTableTopTransform->Translate( 0., -490., 550.);
//  patientToTableTopTransform->Translate( 0, 0, 0);
  patientToTableTopTransform->Modified();

  // New Treatment machine position
  this->SetupTreatmentMachineModels(parameterNode);
*/
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

//  this->UpdatePatientToTableTopTransform(parameterNode);

  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* patientSupportToFixedReferenceTransformNode =
    this->IhepLogic->GetTransformNodeBetween( IHEP::PatientSupport, IHEP::FixedReference);

  if (patientSupportToFixedReferenceTransformNode)
  {
    double couchRotationAngle = parameterNode->GetPatientSupportRotationAngle();

    vtkNew<vtkTransform> patientSupportToFixedReferenceTransform;
    patientSupportToFixedReferenceTransform->Identity();
    patientSupportToFixedReferenceTransform->RotateZ(-1. * couchRotationAngle);
    patientSupportToFixedReferenceTransform->Modified();
    
    patientSupportToFixedReferenceTransformNode->SetAndObserveTransformToParent(
      patientSupportToFixedReferenceTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateTableTopToTableTopStandTransform(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableTopToTableTopStandTransform: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("UpdateTableTopToTableTopStandTransform: Invalid parameter node");
    return;
  }

  vtkMRMLModelNode* tableTopModel = vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->GetFirstNodeByName(TABLETOP_MODEL_NAME));
  if (!tableTopModel)
  {
    vtkDebugMacro("UpdateTableTopToTableTopStandTransform: Table top model not found");
    return;
  }
/*
  // Translation to origin for in-place rotation
  vtkPolyData* tableTopModelPolyData = tableTopModel->GetPolyData();

  double tableTopModelBounds[6] = { 0, 0, 0, 0, 0, 0 };
  tableTopModelPolyData->GetBounds(tableTopModelBounds);

  vtkWarningMacro("UpdateTableTopToTableTopStandTransform: Table top bounds " <<
    tableTopModelBounds[0] << " " << tableTopModelBounds[1] << " " <<
    tableTopModelBounds[2] << " " << tableTopModelBounds[3] << " " <<
    tableTopModelBounds[4] << " " << tableTopModelBounds[5]);
*/
  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* tableTopToTableTopMovementTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableTop, IHEP::TableTopStand);

//  this->UpdatePatientSupportRotationToFixedReferenceTransform(parameterNode);

  if (tableTopToTableTopMovementTransformNode)
  {
    double longitudinalRotationAngle = parameterNode->GetTableTopLongitudinalAngle();
    double lateralRotationAngle = parameterNode->GetTableTopLateralAngle();

    vtkNew<vtkTransform> tableTopToTableTopMovementTransform;
    tableTopToTableTopMovementTransform->Translate( 0., 0., -1. * parameterNode->GetTableTopVerticalPosition());
    tableTopToTableTopMovementTransform->RotateY(-1. * longitudinalRotationAngle);
    tableTopToTableTopMovementTransform->RotateX(-1. * lateralRotationAngle);
    tableTopToTableTopMovementTransformNode->SetAndObserveTransformToParent(tableTopToTableTopMovementTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateTableTopStandToPatientSupportTransform(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableTopStandToPatientSupportTransform: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("UpdateTableTopStandToPatientSupportTransform: Invalid parameter node");
    return;
  }

  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* tableTopStandToPatientSupportTransformNode =
    this->IhepLogic->GetTransformNodeBetween(IHEP::TableTopStand, IHEP::PatientSupport);

//  this->UpdatePatientSupportRotationToFixedReferenceTransform(parameterNode);

  if (tableTopStandToPatientSupportTransformNode)
  {
//    double longitudinalRotationAngle = parameterNode->GetTableTopLongitudinalAngle();
//    double lateralRotationAngle = parameterNode->GetTableTopLateralAngle();

    vtkNew<vtkTransform> tableTopStandToPatientSupportTransform;
    tableTopStandToPatientSupportTransform->Identity();
    tableTopStandToPatientSupportTransform->Translate( 0., -1. * parameterNode->GetTableTopLongitudinalPosition(), 0);
    tableTopStandToPatientSupportTransform->Modified();
    tableTopStandToPatientSupportTransformNode->SetAndObserveTransformToParent(tableTopStandToPatientSupportTransform);
  }
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

//  this->UpdatePatientSupportRotationToFixedReferenceTransform(parameterNode);

  if (patientToTableTopTransformNode)
  {
    double longitudinalRotationAngle = parameterNode->GetTableTopLongitudinalAngle();
    double lateralRotationAngle = parameterNode->GetTableTopLateralAngle();

    vtkNew<vtkTransform> patientToTableTopTransform;
    patientToTableTopTransform->Identity();
    patientToTableTopTransform->Translate( 0., 0., 0.);
    patientToTableTopTransform->Modified();
    patientToTableTopTransformNode->SetAndObserveTransformToParent(patientToTableTopTransform);
  }
}
