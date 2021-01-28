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

// SlicerRT IEC transformation logic from Beam module
#include <vtkSlicerIECTransformLogic.h>

// SlicerRT includes
#include <vtkMRMLRTPlanNode.h>
#include <vtkMRMLRTBeamNode.h>
#include <vtkMRMLRTIonBeamNode.h>

// MRML includes
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

const char* vtkSlicerIhepStandGeometryLogic::ORIENTATION_MARKER_MODEL_NODE_NAME = "IhepStandGeometryOrientationMarker";

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerIhepStandGeometryLogic);

//----------------------------------------------------------------------------
vtkSlicerIhepStandGeometryLogic::vtkSlicerIhepStandGeometryLogic()
  :
  IECLogic(vtkSlicerIECTransformLogic::New())
{
}

//----------------------------------------------------------------------------
vtkSlicerIhepStandGeometryLogic::~vtkSlicerIhepStandGeometryLogic()
{
  if (this->IECLogic)
  {
    this->IECLogic->Delete();
    this->IECLogic = nullptr;
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
  this->IECLogic->SetMRMLScene(newScene);
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

//---------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::BuildIhepStangGeometryTransformHierarchy()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("BuildIhepStangGeometryTransformHierarchy: Invalid MRML scene");
    return;
  }

  // Build IEC hierarchy
  this->IECLogic->BuildIECTransformHierarchy();
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
/*
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
*/
/*
  // Table top inferior-superior movement stand - mandatory
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
*/
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

/*
  if ( !canyonModelNode || !canyonModelNode->GetPolyData()
    || !tableTopStandModelNode || !tableTopStandModelNode->GetPolyData()
    || !patientSupportModelNode || !patientSupportModelNode->GetPolyData()
    || !tableTopModelNode || !tableTopModelNode->GetPolyData() )
  {
    vtkErrorMacro("LoadTreatmentMachineModels: Failed to load every mandatory treatment machine component");
    return;
  }
*/
  // Setup treatment machine model display and transforms
//  this->SetupTreatmentMachineModels();
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

  vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode();
  using IEC = vtkSlicerIECTransformLogic::CoordinateSystemIdentifier;

  // Update TableTop -> TableTopEccentricRotation
  vtkMRMLLinearTransformNode* tableTopToTableTopEccentricRotationTransformNode =
    this->IECLogic->GetTransformNodeBetween(IEC::TableTop, IEC::TableTopEccentricRotation);
  vtkTransform* tableTopToTableTopEccentricRotationTransform = vtkTransform::SafeDownCast(
    tableTopToTableTopEccentricRotationTransformNode->GetTransformToParent() );

  tableTopToTableTopEccentricRotationTransform->Modified();

  // Update TableTopInferiorSuperiorMovement -> PatientSupportRotation
  vtkMRMLLinearTransformNode* tableTopInferiorSuperiorToPatientSupportRotationTransformNode =
    this->IECLogic->GetTransformNodeBetween(IEC::TableTopInferiorSuperiorMovement, IEC::PatientSupportRotation);
  vtkTransform* tableTopInferiorSuperiorToPatientSupportRotationTransform = vtkTransform::SafeDownCast(
    tableTopInferiorSuperiorToPatientSupportRotationTransformNode->GetTransformToParent() );

  tableTopInferiorSuperiorToPatientSupportRotationTransform->Modified();

  // Update PatientSupportRotation -> FixedReference
  vtkMRMLLinearTransformNode* patientSupportRotationToFixedReferenceTransformNode =
    this->IECLogic->GetTransformNodeBetween(IEC::PatientSupportRotation, IEC::FixedReference);
  vtkTransform* patientSupportRotationToFixedReferenceTransform = vtkTransform::SafeDownCast(
    patientSupportRotationToFixedReferenceTransformNode->GetTransformToParent() );
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

  double isocenter[3] = {};
  vtkMRMLRTBeamNode* beam = parameterNode->GetBeamNode();
  beam->GetPlanIsocenterPosition(isocenter);

  patientToTableTopTransform->Identity();
//  patientToTableTopTransform->Translate( isocenter[0], isocenter[1] + 490, isocenter[2] + 550);
  patientToTableTopTransform->Translate( 0., -490., 550.);
  patientToTableTopTransform->Modified();

  this->UpdateTableTopToTableTopEccentricRotationTransform(parameterNode);

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
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdatePatientSupportRotationToFixedReferenceTransform(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdatePatientSupportRotationToFixedReferenceTransform: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("UpdatePatientSupportRotationToFixedReferenceTransform: Invalid parameter node");
    return;
  }

  using IEC = vtkSlicerIECTransformLogic::CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* patientSupportRotationToFixedReferenceTransformNode =
    this->IECLogic->GetTransformNodeBetween(IEC::PatientSupportRotation, IEC::FixedReference);

  if (patientSupportRotationToFixedReferenceTransformNode)
  {
    double rotationAngle = parameterNode->GetPatientSupportRotationAngle();
    vtkNew<vtkTransform> patientSupportToRotatedPatientSupportTransform;
    patientSupportToRotatedPatientSupportTransform->RotateZ(-1. * rotationAngle);
    patientSupportRotationToFixedReferenceTransformNode->SetAndObserveTransformToParent(patientSupportToRotatedPatientSupportTransform);
  }
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
  using IEC = vtkSlicerIECTransformLogic::CoordinateSystemIdentifier;

  // Transform IHEP stand models to RAS
  vtkNew<vtkTransform> rotateYTransform;
  rotateYTransform->Identity();
  rotateYTransform->RotateX(-90.);
  rotateYTransform->RotateZ(180.);
/*
  // Table top stand (Inferior<->Superior movement) model - mandatory
  // Transform path: RAS -> Patient -> TableTop -> Eccentric -> TableTopInferiorSuperior
  vtkNew<vtkGeneralTransform> rasToTableTopInferiorSuperiorGeneralTransform;
  vtkNew<vtkTransform> rasToTableTopInferiorSuperiorLinearTransform;
  if (this->IECLogic->GetTransformBetween( IEC::RAS, IEC::TableTopInferiorSuperiorMovement, 
    rasToTableTopInferiorSuperiorGeneralTransform, false))
  {
    // Convert general transform to linear
    // This call also makes hard copy of the transform so that it doesn't change when other beam transforms change
    if (!vtkMRMLTransformNode::IsGeneralTransformLinear( rasToTableTopInferiorSuperiorGeneralTransform, 
      rasToTableTopInferiorSuperiorLinearTransform))
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to get transform hierarchy from RAS to TableTopInferiorSuperior");
      return;
    }
    // Transform to RAS, set transform to node, transform the model
    rasToTableTopInferiorSuperiorLinearTransform->Concatenate(rotateYTransform);

    // Find RasToTableTopInferiorSuperiorTransform or create it
    vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableTopInferiorSuperiorTransformNode;
    if (scene->GetFirstNodeByName("RasToTableTopInferiorSuperiorTransform"))
    {
      rasToTableTopInferiorSuperiorTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
        scene->GetFirstNodeByName("RasToTableTopInferiorSuperiorTransform"));
    }
    else
    {
      rasToTableTopInferiorSuperiorTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      rasToTableTopInferiorSuperiorTransformNode->SetName("RasToTableTopInferiorSuperiorTransform");
//      rasToTableTopInferiorSuperiorTransformNode->SetHideFromEditors(1);
      scene->AddNode(rasToTableTopInferiorSuperiorTransformNode);
    }

    vtkMRMLModelNode* tableTopInferiorSuperiorModel = vtkMRMLModelNode::SafeDownCast(
      scene->GetFirstNodeByName(TABLETOPSTAND_MODEL_NAME) );
    if (!tableTopInferiorSuperiorModel)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to access table top stand model");
      return;
    }
    if (rasToTableTopInferiorSuperiorTransformNode)
    {
      rasToTableTopInferiorSuperiorTransformNode->SetAndObserveTransformToParent(rasToTableTopInferiorSuperiorLinearTransform);
      tableTopInferiorSuperiorModel->SetAndObserveTransformNodeID(rasToTableTopInferiorSuperiorTransformNode->GetID());
      tableTopInferiorSuperiorModel->CreateDefaultDisplayNodes();
      tableTopInferiorSuperiorModel->GetDisplayNode()->SetColor(0.95, 0.95, 0.95);
    }
  }
*/

  // Table top - mandatory
  // Transform path: RAS -> Patient -> TableTop
  vtkNew<vtkGeneralTransform> rasToTableTopGeneralTransform;
  vtkNew<vtkTransform> rasToTableTopLinearTransform;
  if (this->IECLogic->GetTransformBetween( IEC::RAS, IEC::TableTop, 
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

  // Fixed Reference (canyon) - mandatory
  // Transform path: RAS -> Patient -> TableTop -> Eccentric -> TableTopInferiorSuperiorMovement -> PatientSupportRotation -> FixedReference
  vtkNew<vtkGeneralTransform> rasToFixedReferenceGeneralTransform;
  vtkNew<vtkTransform> rasToFixedReferenceLinearTransform;
  if (this->IECLogic->GetTransformBetween( IEC::RAS, IEC::FixedReference, 
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

    // observe ras to fixed frame node for patient support rotation transform
//    this->IECLogic->GetTransformNodeBetween(IEC::PatientSupportRotation, IEC::FixedReference)->SetAndObserveTransformNodeID(
//      rasToFixedReferenceTransformNode->GetID() );
  }
/*
  // Patient support - mandatory
  // Transform path: RAS -> Patient -> TableTop -> Eccentric -> TableTopInferiorSuperiorMovement -> PatientSupportRotation -> FixedReference
  vtkNew<vtkGeneralTransform> rasToPatientSupportRotationGeneralTransform;
  vtkNew<vtkTransform> rasToPatientSupportRotationLinearTransform;
  if (this->IECLogic->GetTransformBetween( IEC::RAS, IEC::PatientSupportRotation, 
    rasToPatientSupportRotationGeneralTransform, false))
  {
    // Convert general transform to linear
    // This call also makes hard copy of the transform so that it doesn't change when other beam transforms change
    if (!vtkMRMLTransformNode::IsGeneralTransformLinear( rasToPatientSupportRotationGeneralTransform, 
      rasToPatientSupportRotationLinearTransform))
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to get transform hierarchy from RAS to PatientSupportRotation");
      return;
    }

    vtkMRMLLinearTransformNode* patientSupportRotationToFixedReferenceTransformNode =
      this->IECLogic->GetTransformNodeBetween(IEC::PatientSupportRotation, IEC::FixedReference);
    vtkTransform* patientSupportToFixedReferenceTransform = vtkTransform::SafeDownCast(patientSupportRotationToFixedReferenceTransformNode->GetTransformToParent());
  
    // Transform to RAS, set transform to node, transform the model
    rasToPatientSupportRotationLinearTransform->Identity();
    rasToPatientSupportRotationLinearTransform->Concatenate(rasToFixedReferenceLinearTransform);
    rasToPatientSupportRotationLinearTransform->Concatenate(patientSupportToFixedReferenceTransform);
    // Transform to RAS, set transform to node, transform the model
    rasToPatientSupportRotationLinearTransform->Concatenate(rotateYTransform);
    rasToPatientSupportRotationLinearTransform->Modified();

    // Find RasToTableTopInferiorSuperiorTransform or create it
    vtkSmartPointer<vtkMRMLLinearTransformNode> rasToPatientSupportRotationTransformNode;
    if (scene->GetFirstNodeByName("RasToPatientSupportRotationTransform"))
    {
      rasToPatientSupportRotationTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
        scene->GetFirstNodeByName("RasToPatientSupportRotationTransform"));
    }
    else
    {
      rasToPatientSupportRotationTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      rasToPatientSupportRotationTransformNode->SetName("RasToPatientSupportRotationTransform");
//      rasToTableTopInferiorSuperiorTransformNode->SetHideFromEditors(1);
      scene->AddNode(rasToPatientSupportRotationTransformNode);
    }

    vtkMRMLModelNode* patientSupportRotationModel = vtkMRMLModelNode::SafeDownCast(
      scene->GetFirstNodeByName(PATIENTSUPPORT_MODEL_NAME) );
    if (!patientSupportRotationModel)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to access table top stand model");
      return;
    }
    if (rasToPatientSupportRotationTransformNode)
    {
      rasToPatientSupportRotationTransformNode->SetAndObserveTransformToParent(rasToPatientSupportRotationLinearTransform);
      patientSupportRotationModel->SetAndObserveTransformNodeID(rasToPatientSupportRotationTransformNode->GetID());
      patientSupportRotationModel->CreateDefaultDisplayNodes();
      patientSupportRotationModel->GetDisplayNode()->SetColor(0.85, 0.85, 0.85);
    }
  }
*/
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
//  patientSupportRotationToFixedReferenceTransform->Identity();
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
}

//-----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateTableTopToTableTopEccentricRotationTransform(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableTopToTableTopEccentricRotationTransform: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("UpdateTableTopToTableTopEccentricRotationTransform: Invalid parameter node");
    return;
  }

  vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode();

  using IEC = vtkSlicerIECTransformLogic::CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* tableTopToTableTopEccentricRotationTransformNode =
    this->IECLogic->GetTransformNodeBetween(IEC::TableTop, IEC::TableTopEccentricRotation);
  vtkTransform* tableTopToTableTopEccentricRotationTransform = vtkTransform::SafeDownCast(
    tableTopToTableTopEccentricRotationTransformNode->GetTransformToParent() );

  tableTopToTableTopEccentricRotationTransform->Identity();
  tableTopToTableTopEccentricRotationTransform->RotateY(beamNode->GetGantryAngle() - 90.);
//  tableTopToTableTopEccentricRotationTransform->Translate( 0., -1. * parameterNode->GetTableTopLongitudinalPosition(), -1. * parameterNode->GetTableTopVerticalPosition());
  tableTopToTableTopEccentricRotationTransform->Translate( 0., 0., -1. * parameterNode->GetTableTopVerticalPosition());
  tableTopToTableTopEccentricRotationTransform->Modified();
}

//-----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateTableTopInferiorSuperiorToPatientSupportRotationTransform(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateTableTopInferiorSuperiorToPatientSupportRotationTransform: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("UpdateTableTopInferiorSuperiorToPatientSupportRotationTransform: Invalid parameter node");
    return;
  }

  using IEC = vtkSlicerIECTransformLogic::CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* tableTopInferiorSuperiorToPatientSupportRotationTransformNode =
    this->IECLogic->GetTransformNodeBetween(IEC::TableTopInferiorSuperiorMovement, IEC::PatientSupportRotation);
  vtkTransform* tableTopInferiorSuperiorToPatientSupportRotationTransform = vtkTransform::SafeDownCast(
    tableTopInferiorSuperiorToPatientSupportRotationTransformNode->GetTransformToParent() );

  tableTopInferiorSuperiorToPatientSupportRotationTransform->Identity();
  tableTopInferiorSuperiorToPatientSupportRotationTransform->Translate( 0., -1. * parameterNode->GetTableTopLongitudinalPosition(), 0.);
//  tableTopInferiorSuperiorToPatientSupportRotationTransform->Translate( 0., -1. * parameterNode->GetTableTopLongitudinalPosition(), -1. * parameterNode->GetTableTopVerticalPosition());
  tableTopInferiorSuperiorToPatientSupportRotationTransform->Modified();

//  double translationArray[3] =
//    { 0., -1. * parameterNode->GetTableTopLongitudinalDisplacement(), 0. };

//  vtkNew<vtkMatrix4x4> tableTopInferiorSuperiorToPatientSupportRotationMatrix;
//  tableTopInferiorSuperiorToPatientSupportRotationMatrix->SetElement(0,3, translationArray[0]);
//  tableTopInferiorSuperiorToPatientSupportRotationMatrix->SetElement(1,3, translationArray[1]);
//  tableTopInferiorSuperiorToPatientSupportRotationMatrix->SetElement(2,3, translationArray[2]);
//  tableTopInferiorSuperiorToPatientSupportRotationTransform->SetMatrix(tableTopInferiorSuperiorToPatientSupportRotationMatrix);
//  tableTopInferiorSuperiorToPatientSupportRotationTransform->Modified();
}
